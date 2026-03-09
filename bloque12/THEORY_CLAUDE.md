# THEORY_CLAUDE.md — Bloque 12: Proyecto Final Integrador — MiniCloud

> Este es el bloque donde todo converge. Los 11 bloques anteriores te dieron las piezas: procesos, memoria, señales, hilos, sockets, seguridad, filesystems, namespaces, cgroups. Ahora las ensamblas en una **plataforma de microservicios en C** — un MiniCloud con service discovery, gateway, monitoreo, secretos y ejecución aislada de workloads.

---

## Mapa del Bloque

```
Tema 1: Visión y arquitectura   →  5 servicios, data plane vs control plane
Tema 2: Registry                →  Service discovery con heartbeats y expiración
Tema 3: Gateway                 →  Routing, health-aware forwarding, request-id
Tema 4: Monitor                 →  Métricas, percentiles, error rate
Tema 5: Vault                   →  Gestión de secretos con TTL y auditoría
Tema 6: Runner                  →  Ejecución aislada con namespaces/cgroups/seccomp
Tema 7: Readiness global        →  Liveness vs readiness vs platform readiness
Tema 8: Operación               →  Config atómica, degradación, testing
```

---

## Tema 1 — Arquitectura de MiniCloud

### Los 5 servicios

```
                              ┌─────────────┐
                              │   MONITOR   │
                              │ observa     │
                              │ métricas    │
                              └──────┬──────┘
                                     │ (scrape)
┌──────────┐    ┌──────────┐    ┌────┴─────┐    ┌──────────┐
│ REGISTRY │◀──▶│ GATEWAY  │───▶│  RUNNER  │◀───│  VAULT   │
│ discovery│    │ routing  │    │ workloads│    │ secrets  │
│ heartbeat│    │ balancing│    │ isolation│    │ TTL/audit│
└──────────┘    └──────────┘    └──────────┘    └──────────┘
     ▲               │
     │               │  Data plane (requests de usuario)
     │               ▼
     │          ┌──────────┐
     └──────────│ CLIENTS  │
   Control plane└──────────┘
```

### Data plane vs Control plane

| Plano | Qué transporta | Prioridad | Ejemplos |
|-------|----------------|-----------|----------|
| **Data plane** | Tráfico de requests de usuario | Latencia, throughput | Request HTTP → gateway → runner |
| **Control plane** | Config, discovery, health, políticas | Consistencia, trazabilidad | Heartbeat → registry, métricas → monitor |

> [!CAUTION]
> **No mezcles data plane y control plane en el mismo flujo.** Si el tráfico de datos satura el control plane (ej: healthchecks compiten con requests), tus componentes pierden coordinación justo cuando más la necesitan.

---

## Tema 2 — Registry: service discovery

### Qué almacena

```c
typedef struct {
    char     service[64];       // "gateway", "runner-1", etc.
    char     endpoint[128];     // "192.168.1.10:8080"
    char     status[16];        // "UP", "DOWN", "STARTING"
    time_t   last_heartbeat;    // timestamp del último heartbeat
    uint32_t version;           // versión de la entrada
} registry_entry_t;
```

### Heartbeats y expiración

```c
#define HEARTBEAT_TTL_SEC  30   // si no hay heartbeat en 30s, marcar DOWN

void expire_stale_entries(registry_entry_t *entries, int n) {
    time_t now = time(NULL);
    for (int i = 0; i < n; i++) {
        if (strcmp(entries[i].status, "UP") == 0 &&
            difftime(now, entries[i].last_heartbeat) > HEARTBEAT_TTL_SEC) {
            snprintf(entries[i].status, sizeof(entries[i].status), "DOWN");
            entries[i].version++;
            printf("[WARN] %s en %s: heartbeat expirado, marcado DOWN\n",
                   entries[i].service, entries[i].endpoint);
        }
    }
}
```

### Snapshot atómico para consumidores

El gateway necesita leer el estado de todos los servicios sin ver entradas a medio actualizar. Patrón: **snapshot inmutable por versión**:

```c
typedef struct {
    registry_entry_t *entries;  // array copiado (snapshot)
    int               count;
    uint64_t          snapshot_version;
} registry_snapshot_t;

// El registry genera snapshots atómicos:
registry_snapshot_t take_snapshot(const registry_t *reg) {
    pthread_mutex_lock(&reg->lock);
    registry_snapshot_t snap = {
        .entries = malloc(reg->count * sizeof(registry_entry_t)),
        .count = reg->count,
        .snapshot_version = reg->version
    };
    memcpy(snap.entries, reg->entries, reg->count * sizeof(registry_entry_t));
    pthread_mutex_unlock(&reg->lock);
    return snap;
}
```

---

## Tema 3 — Gateway: routing y forwarding

### Flujo de un request

```
Cliente
  │
  │ HTTP request: GET /api/v1/jobs/42
  ▼
┌──────────────────────────────┐
│          GATEWAY             │
│                              │
│ 1. Generar request-id        │
│ 2. Longest-prefix match      │  /api/v1/jobs → runner
│ 3. Consultar registry        │  runner-1: UP, runner-2: DOWN
│ 4. Seleccionar backend sano  │  runner-1 ← seleccionado
│ 5. Forward request           │
│ 6. Medir latencia            │
│ 7. Retornar respuesta        │
└──────────────────────────────┘
  │
  ▼
runner-1:8080
```

### Request-ID para trazabilidad

```c
#include <stdint.h>
#include <time.h>

void generate_request_id(char *buf, size_t bufsize) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t ns = (uint64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
    snprintf(buf, bufsize, "req-%016lx", (unsigned long)ns);
}

// Uso en logging:
// [req-0001a3f4b5c6d7e8] GET /api/v1/jobs → runner-1 → 200 (12ms)
```

### Retry con budget

```c
#define MAX_RETRIES    2
#define RETRY_DELAY_MS 100

int forward_with_retry(const char *path, const backend_t *backends, int n) {
    char req_id[32];
    generate_request_id(req_id, sizeof(req_id));

    for (int attempt = 0; attempt <= MAX_RETRIES; attempt++) {
        // Seleccionar backend sano
        const backend_t *b = select_healthy_backend(backends, n);
        if (!b) {
            LOG_ERROR("[%s] No hay backends sanos", req_id);
            return 503;
        }

        int status = forward_request(b, path, req_id);
        if (status >= 200 && status < 500) {
            return status;  // respuesta válida (incluyendo 4xx)
        }

        // Solo reintentar en errores 5xx o timeout
        LOG_WARN("[%s] Backend %s retornó %d, reintento %d/%d",
                 req_id, b->name, status, attempt + 1, MAX_RETRIES);

        if (attempt < MAX_RETRIES) {
            usleep(RETRY_DELAY_MS * 1000);
        }
    }
    return 502;  // agotados los reintentos
}
```

> [!WARNING]
> **Errores 4xx (Bad Request, Not Found, etc.) no se reintentan.** Son errores del cliente, no del backend. Reintentar un 401 o 404 solo añade carga sin resolver nada. Solo reintenta en 5xx y timeouts.

---

## Tema 4 — Monitor: métricas y observabilidad

### Las 4 señales doradas (Google SRE)

| Señal | Qué mide | Cómo se calcula |
|-------|----------|-----------------|
| **Latencia** | Tiempo de respuesta | p50, p95, p99 |
| **Error rate** | Proporción de errores | errors / total |
| **Saturación** | Cuánto recurso se está usando | CPU%, memoria%, cola length |
| **Tráfico** | Volumen de requests | requests/segundo |

### Cálculo de percentiles sin sorting

Para calcular p50/p95/p99 sin acumular todas las mediciones:

```c
// Histograma con buckets logarítmicos:
typedef struct {
    uint64_t buckets[20];    // 0-1ms, 1-2ms, 2-4ms, 4-8ms, ..., 512ms+
    uint64_t total_count;
    double   total_sum_ms;
} latency_histogram_t;

void record_latency(latency_histogram_t *h, double ms) {
    h->total_count++;
    h->total_sum_ms += ms;

    // Encontrar bucket
    int bucket = 0;
    double threshold = 1.0;
    while (bucket < 19 && ms >= threshold) {
        bucket++;
        threshold *= 2;
    }
    h->buckets[bucket]++;
}

double percentile(const latency_histogram_t *h, double pct) {
    uint64_t target = (uint64_t)(h->total_count * pct / 100.0);
    uint64_t cumulative = 0;
    double lower = 0, upper = 1.0;

    for (int i = 0; i < 20; i++) {
        cumulative += h->buckets[i];
        if (cumulative >= target) {
            // Interpolación lineal dentro del bucket
            uint64_t prev = cumulative - h->buckets[i];
            double frac = (target - prev) / (double)h->buckets[i];
            return lower + frac * (upper - lower);
        }
        lower = upper;
        upper *= 2;
    }
    return upper;
}
```

### Health status reducido

```c
typedef struct {
    char component[64];
    char status[8];       // OK, WARN, CRIT
    char detail[256];
} health_check_t;

const char *reduce_health(const health_check_t *checks, int n) {
    int has_warn = 0;
    for (int i = 0; i < n; i++) {
        if (strcmp(checks[i].status, "CRIT") == 0) return "CRIT";
        if (strcmp(checks[i].status, "WARN") == 0) has_warn = 1;
    }
    return has_warn ? "WARN" : "OK";
}
```

---

## Tema 5 — Vault: secretos con TTL

### Modelo de datos

```c
typedef struct {
    char     name[128];           // "runner/db_password"
    char     value[512];          // el secreto (en producción: cifrado)
    time_t   created_at;
    time_t   expires_at;          // 0 = no expira (mala práctica)
    char     owner[64];           // "runner-service"
    int      access_count;
} vault_secret_t;
```

### Validación de manifests de secretos

```c
int validate_secret(const vault_secret_t *s) {
    int errors = 0;

    // TTL obligatorio
    if (s->expires_at == 0) {
        printf("[WARN] Secreto '%s': sin expiración definida\n", s->name);
        errors++;
    } else if (s->expires_at <= time(NULL)) {
        printf("[CRIT] Secreto '%s': ya expiró\n", s->name);
        errors++;
    }

    // Owner obligatorio
    if (s->owner[0] == '\0') {
        printf("[WARN] Secreto '%s': sin owner asignado\n", s->name);
        errors++;
    }

    // Nombre con namespace
    if (!strchr(s->name, '/')) {
        printf("[WARN] Secreto '%s': sin namespace (usar servicio/nombre)\n", s->name);
    }

    return errors;
}
```

### Auditoría de acceso (sin loguear el valor)

```c
void audit_access(const char *secret_name, const char *requester, int granted) {
    time_t now = time(NULL);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));

    // ✅ Loguea quién, qué, cuándo, resultado — NUNCA el valor
    fprintf(stderr, "%s vault_access secret=%s requester=%s granted=%s\n",
            ts, secret_name, requester, granted ? "yes" : "no");
}
```

---

## Tema 6 — Runner: ejecución aislada

### Job spec y validación

```c
typedef struct {
    char     command[PATH_MAX];
    char     args[16][256];
    int      argc;
    long     cpu_quota;           // microsegundos por periodo
    long     memory_max;          // bytes
    int      timeout_sec;         // máximo tiempo de ejecución
    char     secrets_allowed[8][128]; // qué secretos puede acceder
} job_spec_t;

int validate_job(const job_spec_t *job) {
    if (job->command[0] != '/') {
        LOG_ERROR("command debe ser path absoluto");
        return -1;
    }
    if (job->timeout_sec <= 0 || job->timeout_sec > 3600) {
        LOG_ERROR("timeout fuera de rango (1-3600s)");
        return -1;
    }
    if (job->memory_max > 0 && job->memory_max < 4 * 1024 * 1024) {
        LOG_ERROR("memory_max mínimo 4 MB");
        return -1;
    }
    return 0;
}
```

### Pipeline de ejecución (integra Bloque 3, 4, 5, 11)

```c
int run_job(const job_spec_t *job, const vault_t *vault) {
    // 1. VALIDATE
    if (validate_job(job) != 0) return -1;

    // 2. PREPARE — obtener secretos autorizados
    for (int i = 0; i < 8 && job->secrets_allowed[i][0]; i++) {
        vault_secret_t *s = vault_get(vault, job->secrets_allowed[i]);
        if (!s || s->expires_at <= time(NULL)) {
            LOG_ERROR("Secreto '%s' no disponible o expirado", job->secrets_allowed[i]);
            return -1;
        }
    }

    // 3. ISOLATE (Bloque 11)
    // - Crear cgroup con cpu_quota y memory_max
    // - unshare(CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET)
    // - Aplicar perfil seccomp
    // - Drop capabilities

    // 4. RUN
    pid_t pid = fork();
    if (pid == 0) {
        // Hijo: exec el job con timeout
        alarm(job->timeout_sec);  // SIGALRM si excede
        execv(job->command, ...);
        _exit(127);
    }

    // 5. WAIT + CLEANUP
    int status;
    waitpid(pid, &status, 0);
    // Eliminar cgroup, limpiar secretos de memoria

    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}
```

---

## Tema 7 — Readiness: ¿la plataforma está lista?

### Tres niveles de health

```
Liveness:          ¿El proceso está vivo?       (heartbeat / TCP check)
Readiness:         ¿Puede atender correctamente? (dependencias OK)
Platform readiness: ¿TODOS los servicios listos?  (el sistema funciona)
```

```c
typedef struct {
    char name[64];
    int  is_live;       // proceso responde
    int  is_ready;      // dependencias OK
    char deps_status[256];
} service_health_t;

int check_platform_readiness(const service_health_t *services, int n) {
    for (int i = 0; i < n; i++) {
        if (!services[i].is_live) {
            printf("[CRIT] %s: no está vivo\n", services[i].name);
            return 0;
        }
        if (!services[i].is_ready) {
            printf("[WARN] %s: vivo pero no ready (%s)\n",
                   services[i].name, services[i].deps_status);
            return 0;
        }
    }
    printf("[OK] Plataforma ready: %d/%d servicios\n", n, n);
    return 1;
}
```

> [!IMPORTANT]
> **Un servicio vivo no es un servicio listo.** El gateway puede estar "up" (respondiendo a pings) pero sin conexión al registry. En ese estado, no puede rutear. `liveness != readiness`.

---

## Tema 8 — Operación: config, degradación y testing

### Config atómica con rollback

```
1. parse(new_config.json)     → ¿Sintaxis OK?
2. validate(parsed)            → ¿Semántica OK? ¿Referencias cruzadas?
3. snapshot = build(parsed)    → Crear nueva estructura en memoria
4. swap(current, snapshot)     → Intercambio atómico
5. if (health_check() fails)   → rollback(previous)
```

### Degradación controlada

| Escenario | Comportamiento degradado | NO hacer |
|-----------|--------------------------|----------|
| Registry caído | Gateway usa último snapshot conocido | Parar de rutear |
| 1 de 3 runners DOWN | Enviar tráfico a los 2 sanos | Enviar al DOWN |
| Vault inalcanzable | Jobs nuevos fallan, jobs activos continúan | Exponer secretos por otro canal |
| Monitor sin métricas | Alertar que observabilidad está degradada | Silenciar sin aviso |

### Testing del integrador

| Capa | Qué testea | Ejemplo |
|------|-----------|---------|
| **Unitaria** | Parsers, validadores, reglas | `parse_zone_line` con fixture |
| **Integración por componente** | Un servicio con dependencias mock | Gateway con registry fake |
| **End-to-end** | Múltiples servicios coordinados | Request → gateway → runner → respuesta |
| **Degradación** | Comportamiento con fallos inyectados | Kill registry, verificar que gateway usa cache |

---

## Checklist de salida del Bloque 12

- [ ] Cada servicio tiene contrato explícito (input/output/status codes/timeouts)
- [ ] Registry mantiene heartbeats con expiración y snapshot atómico
- [ ] Gateway implementa longest-prefix match con selección health-aware de backends
- [ ] Gateway genera request-id para trazabilidad end-to-end
- [ ] Monitor calcula latencia p50/p95/p99 y error rate
- [ ] Vault gestiona secretos con TTL, owner, y auditoría sin filtrar valores
- [ ] Runner valida job specs y ejecuta con aislamiento (namespaces/cgroups/seccomp)
- [ ] Readiness global evalúa dependencias entre servicios
- [ ] Config se aplica atómicamente con posibilidad de rollback
- [ ] Tests cubren happy path, errores, y escenarios de degradación

---

## Bloques previos que usa cada componente

| Componente | Bloques |
|-----------|---------|
| **Registry** | B06 (sockets/reactor), B05 (mutex/condvar para snapshot) |
| **Gateway** | B06 (networking), B10 (routing/proxy), B03 (fork/exec) |
| **Monitor** | B04 (/proc), B02 (file I/O), B08 (statvfs) |
| **Vault** | B09 (SHA-256, HMAC, constant-time compare, hardening) |
| **Runner** | B03 (fork/exec), B04 (mmap, cgroups), B11 (namespaces, seccomp) |
| **Integración** | B07 (Makefile, GDB, sanitizers, logging) |

---

## Referencias

| Recurso | Descripción |
|---------|-------------|
| [Google SRE Book, Ch. 6](https://sre.google/sre-book/monitoring-distributed-systems/) | Las 4 señales doradas |
| [12 Factor App](https://12factor.net/) | Principios de diseño de servicios |
| `man 7 namespaces` | Aislamiento de kernel |
| `man 7 cgroups` | Límites de recursos |
| Bloques 01-11 del curso | Todas las APIs y patrones usados en MiniCloud |
