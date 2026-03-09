# THEORY_CLAUDE.md — Bloque 10: Servicios de Red en Linux/C

> En producción, tu máquina Linux no existe sola. Interactúa con DNS, sirve HTTPS, envía correo por SMTP, comparte archivos por NFS/Samba, y rutea tráfico a través de proxies. Este bloque te enseña a construir herramientas en C que **parsean, validan, auditan y diagnostican** estos servicios — la base del trabajo real de un SysAdmin/SRE.

---

## Mapa del Bloque

```
Tema 1: DNS            →  Zonas, tipos de registro, validación semántica
Tema 2: HTTPS/TLS      →  Modelo de capas, política TLS mínima segura
Tema 3: URLs            →  Parser seguro, prevención de SSRF
Tema 4: SMTP            →  Máquina de estados, respuestas multilínea
Tema 5: NFS             →  /etc/exports, root_squash, auditoría
Tema 6: Samba           →  smb.conf, shares, detección de riesgos
Tema 7: Reverse proxy   →  Longest-prefix match, backends, health checks
Tema 8: Operación       →  Timeouts, límites, logging, config validation
```

---

## Tema 1 — DNS: parsear y validar zonas

### Tipos de registro esenciales

| Tipo | Ejemplo | Significado |
|------|---------|-------------|
| `SOA` | `@ SOA ns1.example.com. admin.example.com. 2024010101 3600 900 604800 86400` | Autoridad de la zona (serial, refresh, retry, expire, negative TTL) |
| `NS` | `@ NS ns1.example.com.` | Servidor autoritativo |
| `A` | `www A 93.184.216.34` | Nombre → IPv4 |
| `AAAA` | `www AAAA 2606:2800:220:1::248` | Nombre → IPv6 |
| `CNAME` | `blog CNAME www.example.com.` | Alias (apunta a otro nombre) |
| `MX` | `@ MX 10 mail.example.com.` | Servidor de correo (prioridad + host) |
| `TXT` | `@ TXT "v=spf1 include:_spf.google.com ~all"` | Metadatos (SPF, DKIM, verificaciones) |

### Reglas semánticas que un validador debe verificar

```c
// Regla 1: CNAME no puede coexistir con otros tipos para el mismo owner
// ❌ Inválido:
//   www  CNAME  cdn.example.com.
//   www  A      1.2.3.4            ← conflicto con CNAME

// Regla 2: Target de MX debe tener A o AAAA (no CNAME)
// ❌ Riesgo:
//   @    MX 10  mail-alias.example.com.
//   mail-alias  CNAME  real-mail.example.com.  ← MX apunta a CNAME

// Regla 3: Serial de SOA debe ser monótonamente creciente
//   Si decrementas el serial, los slaves no actualizan

// Regla 4: TTL tiene impacto operativo
//   TTL=60    → alta carga DNS, pero cambios rápidos
//   TTL=86400 → baja carga, pero cambios tardan 24h en propagar
```

### Parser de zona en C

```c
typedef struct {
    char owner[256];
    uint32_t ttl;
    char class_str[8];   // IN
    char type[16];       // A, AAAA, MX, CNAME, etc.
    char rdata[512];     // datos del registro
    int  line_number;    // para reportar errores
} dns_record_t;

int parse_zone_line(const char *line, int lineno, dns_record_t *rec) {
    // Ignorar comentarios (;) y líneas vacías
    const char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == ';' || *p == '\n' || *p == '\0') return 0;

    // Tokenizar: owner [ttl] [class] type rdata
    // El orden y presencia de TTL/class es variable
    memset(rec, 0, sizeof(*rec));
    rec->line_number = lineno;

    // Parseo simplificado (un parser real necesita manejar más casos):
    int n = sscanf(line, "%255s %u %7s %15s %511[^\n]",
                   rec->owner, &rec->ttl, rec->class_str, rec->type, rec->rdata);

    if (n < 4) {
        // Intentar sin TTL
        rec->ttl = 0;
        n = sscanf(line, "%255s %7s %15s %511[^\n]",
                   rec->owner, rec->class_str, rec->type, rec->rdata);
        if (n < 3) return -1;
    }

    return 1;
}
```

### Selección de MX por prioridad

El registro MX con **menor** valor de prioridad es el preferido:

```c
typedef struct {
    int  priority;
    char host[256];
} mx_record_t;

// Comparador para qsort
int cmp_mx(const void *a, const void *b) {
    return ((const mx_record_t *)a)->priority - ((const mx_record_t *)b)->priority;
}

// Después de recoger todos los MX:
qsort(mx_records, nmx, sizeof(mx_record_t), cmp_mx);
printf("MX preferido: %s (prioridad %d)\n", mx_records[0].host, mx_records[0].priority);
```

---

## Tema 2 — HTTPS/TLS: política y validación

### Las dos capas de HTTPS

```
Tu aplicación
    │
    ├── HTTP: método, headers, body, status codes
    │         (semántica de la comunicación)
    │
    └── TLS: handshake, cifrado, certificados
              (seguridad del canal)
```

### Política TLS mínima segura (2024+)

| Parámetro | Valor seguro | Inseguro |
|-----------|-------------|----------|
| Versión mínima | TLSv1.2 | SSLv3, TLSv1.0, TLSv1.1 |
| Suites de cifrado | ECDHE + AES-GCM, ChaCha20-Poly1305 | RC4, 3DES, NULL, MD5, export |
| Tamaño de clave RSA | ≥ 2048 bits | < 2048 |
| Curvas ECDHE | P-256, P-384, X25519 | P-192 |

### Linter de política TLS en C

```c
typedef struct {
    char version_min[16];
    char version_max[16];
    char ciphers[512];
    int  key_bits;
} tls_policy_t;

typedef struct {
    const char *rule;
    const char *status;    // OK, WARN, CRIT
    const char *detail;
} tls_finding_t;

void audit_tls_policy(const tls_policy_t *pol, tls_finding_t *findings, int *n) {
    *n = 0;

    // Verificar versión mínima
    if (strcmp(pol->version_min, "TLSv1.2") < 0) {
        findings[(*n)++] = (tls_finding_t){
            .rule = "tls_version_min",
            .status = "CRIT",
            .detail = "TLS version mínima menor a 1.2"
        };
    }

    // Buscar suites inseguras
    const char *weak[] = {"RC4", "3DES", "NULL", "MD5", "EXPORT", NULL};
    for (int i = 0; weak[i]; i++) {
        if (strstr(pol->ciphers, weak[i])) {
            findings[(*n)++] = (tls_finding_t){
                .rule = "weak_cipher",
                .status = "CRIT",
                .detail = weak[i]
            };
        }
    }

    // Verificar tamaño de clave
    if (pol->key_bits < 2048) {
        findings[(*n)++] = (tls_finding_t){
            .rule = "key_size",
            .status = "WARN",
            .detail = "RSA key < 2048 bits"
        };
    }
}
```

---

## Tema 3 — Parseo seguro de URLs

### Anatomía de una URL

```
https://user:pass@api.example.com:8443/v1/users?id=42#section
└─┬──┘ └───┬───┘ └──────┬───────┘└┬─┘└───┬───┘└──┬──┘└──┬──┘
scheme  userinfo       host     port   path    query  fragment
```

### Parser con validación de seguridad

```c
typedef struct {
    char scheme[16];
    char host[256];
    uint16_t port;
    char path[1024];
} parsed_url_t;

int parse_url(const char *url, parsed_url_t *out) {
    memset(out, 0, sizeof(*out));

    // 1. Esquema
    const char *sep = strstr(url, "://");
    if (!sep) return -1;
    size_t scheme_len = sep - url;
    if (scheme_len >= sizeof(out->scheme)) return -1;
    memcpy(out->scheme, url, scheme_len);

    // 2. Validar esquema esperado
    if (strcmp(out->scheme, "https") != 0 && strcmp(out->scheme, "http") != 0)
        return -1;

    // 3. Host
    const char *host_start = sep + 3;
    const char *port_sep = strchr(host_start, ':');
    const char *path_sep = strchr(host_start, '/');

    const char *host_end = port_sep ? port_sep :
                           path_sep ? path_sep :
                           host_start + strlen(host_start);

    size_t host_len = host_end - host_start;
    if (host_len == 0 || host_len >= sizeof(out->host)) return -1;
    memcpy(out->host, host_start, host_len);

    // 4. Puerto
    if (port_sep && port_sep < path_sep) {
        char port_str[8];
        const char *ps = port_sep + 1;
        const char *pe = path_sep ? path_sep : ps + strlen(ps);
        size_t plen = pe - ps;
        if (plen == 0 || plen >= sizeof(port_str)) return -1;
        memcpy(port_str, ps, plen);
        port_str[plen] = '\0';

        char *endptr;
        long p = strtol(port_str, &endptr, 10);
        if (*endptr != '\0' || p < 1 || p > 65535) return -1;
        out->port = (uint16_t)p;
    } else {
        out->port = strcmp(out->scheme, "https") == 0 ? 443 : 80;
    }

    // 5. Path
    if (path_sep) {
        snprintf(out->path, sizeof(out->path), "%s", path_sep);
    } else {
        strcpy(out->path, "/");
    }

    return 0;
}
```

> [!CAUTION]
> **SSRF (Server-Side Request Forgery):** Si tu servidor hace requests a URLs proporcionadas por el usuario, un atacante puede enviarte `http://169.254.169.254/metadata` (la API de metadatos del cloud) o `http://localhost:6379/` (Redis local). **Siempre valida** el host contra una whitelist y bloquea rangos de IP privados.

---

## Tema 4 — SMTP: máquina de estados

### El protocolo como flujo de estados

```
     ┌─────────┐
     │ CONNECT │
     └────┬────┘
          │ (recibir 220)
          ▼
     ┌─────────┐
     │  EHLO   │ ──────── (enviar EHLO, recibir 250)
     └────┬────┘
          ▼
     ┌───────────┐
     │ MAIL FROM │ ──────── (enviar MAIL FROM:<...>, recibir 250)
     └────┬──────┘
          ▼
     ┌──────────┐
     │ RCPT TO  │ ──────── (enviar RCPT TO:<...>, recibir 250)
     └────┬─────┘          (puede repetirse para múltiples destinatarios)
          ▼
     ┌──────┐
     │ DATA │ ──────────── (enviar DATA, recibir 354)
     └──┬───┘
        │  (enviar cuerpo del mensaje)
        │  (enviar línea "." para terminar)
        ▼
     ┌──────┐
     │ QUIT │ ──────────── (enviar QUIT, recibir 221)
     └──────┘
```

### Validador de flujo SMTP

```c
typedef enum {
    SMTP_INIT,        // esperando 220
    SMTP_EHLO,        // enviando EHLO
    SMTP_MAIL_FROM,   // enviando MAIL FROM
    SMTP_RCPT_TO,     // enviando RCPT TO (puede repetir)
    SMTP_DATA,        // enviando DATA
    SMTP_BODY,        // enviando cuerpo
    SMTP_QUIT,        // enviando QUIT
    SMTP_DONE,        // sesión terminada
    SMTP_ERROR        // error de protocolo
} smtp_state_t;

const char *valid_transitions[] = {
    [SMTP_INIT]      = "EHLO",
    [SMTP_EHLO]      = "MAIL FROM",
    [SMTP_MAIL_FROM] = "RCPT TO",
    [SMTP_RCPT_TO]   = "RCPT TO|DATA",   // puede repetir RCPT o ir a DATA
    [SMTP_DATA]      = NULL,               // transición por respuesta 354
    [SMTP_BODY]      = NULL,               // transición por "."
    [SMTP_QUIT]      = NULL,
};
```

### Respuestas multilínea SMTP

```
250-smtp.example.com Hello
250-SIZE 14680064
250-PIPELINING
250 OK
```

Las líneas con `-` después del código indican continuación. Solo la línea con espacio es la final:

```c
int parse_smtp_reply(const char *response, int *code, int *is_last) {
    if (strlen(response) < 4) return -1;

    char separator = response[3];  // '-' o ' '
    *is_last = (separator == ' ');

    char code_str[4] = { response[0], response[1], response[2], '\0' };
    char *endptr;
    *code = (int)strtol(code_str, &endptr, 10);
    if (*endptr != '\0') return -1;

    return 0;
}
```

---

## Tema 5 — NFS: auditoría de exports

### Formato de `/etc/exports`

```
/shared     192.168.1.0/24(rw,sync,no_subtree_check)
/backups    *(ro,root_squash)
/data       10.0.0.0/8(rw,no_root_squash,async)    # ← peligro
```

### Opciones que importan para seguridad

| Opción | Seguro | Riesgo |
|--------|--------|--------|
| `root_squash` (default) | ✅ Root remoto → nobody | — |
| `no_root_squash` | ❌ Root remoto = root local | Acceso total al filesystem |
| `ro` (read-only) | ✅ Solo lectura | — |
| `rw` | Depende del contexto | Escritura remota al FS |
| `sync` | ✅ Escrituras confirmadas | — |
| `async` | ❌ Escrituras pueden perderse en crash | Pérdida de datos |
| `*` (sin restricción de red) | ❌ Cualquier host puede montar | Expuesto a toda la red |

### Auditor de NFS exports

```c
void audit_export(const char *path, const char *clients, const char *options) {
    if (strcmp(clients, "*") == 0) {
        printf("[CRIT] %s: export abierto a todo el mundo\n", path);
    }
    if (strstr(options, "no_root_squash")) {
        printf("[CRIT] %s: no_root_squash permite root remoto como root local\n", path);
    }
    if (strstr(options, "rw") && !strstr(options, "sync")) {
        printf("[WARN] %s: rw sin sync explícito (default puede ser async)\n", path);
    }
}
```

---

## Tema 6 — Samba: auditoría de shares

### Formato de `smb.conf`

```ini
[global]
    workgroup = MYGROUP
    security = user

[documents]
    path = /srv/docs
    read only = yes
    browseable = yes

[public]
    path = /srv/public
    writable = yes
    guest ok = yes        # ← riesgo: acceso sin autenticación
    browseable = yes
```

### Detección de riesgos

```c
typedef struct {
    char name[128];
    char path[PATH_MAX];
    int  writable;
    int  guest_ok;
    int  browseable;
} samba_share_t;

void audit_share(const samba_share_t *share) {
    if (share->writable && share->guest_ok) {
        printf("[CRIT] [%s]: writable + guest ok = escritura anónima\n", share->name);
    }
    if (share->guest_ok) {
        printf("[WARN] [%s]: acceso sin autenticación habilitado\n", share->name);
    }
}
```

---

## Tema 7 — Reverse proxy: routing y health checks

### Longest-prefix match

```
Reglas de ruteo:
  /api/v1/users  →  backend_users
  /api/v1        →  backend_api
  /api           →  backend_api_legacy
  /              →  backend_default

Request: /api/v1/users/42
Match: /api/v1/users (14 chars) > /api/v1 (7) > /api (4) > / (1)
Backend seleccionado: backend_users
```

```c
typedef struct {
    char prefix[256];
    char backend[128];
    size_t prefix_len;
} route_t;

const char *match_route(const route_t *routes, int n, const char *path) {
    const route_t *best = NULL;
    for (int i = 0; i < n; i++) {
        if (strncmp(path, routes[i].prefix, routes[i].prefix_len) == 0) {
            if (!best || routes[i].prefix_len > best->prefix_len) {
                best = &routes[i];
            }
        }
    }
    return best ? best->backend : NULL;
}
```

### Health checks activos

```c
typedef struct {
    char     name[64];
    char     host[256];
    uint16_t port;
    int      consecutive_failures;
    int      consecutive_successes;
    int      is_healthy;
} backend_t;

#define FAIL_THRESHOLD  3    // marcar DOWN tras 3 fallos consecutivos
#define OK_THRESHOLD    2    // marcar UP tras 2 éxitos consecutivos

void update_health(backend_t *b, int check_ok) {
    if (check_ok) {
        b->consecutive_failures = 0;
        b->consecutive_successes++;
        if (!b->is_healthy && b->consecutive_successes >= OK_THRESHOLD) {
            b->is_healthy = 1;
            printf("[INFO] %s: marcado como HEALTHY\n", b->name);
        }
    } else {
        b->consecutive_successes = 0;
        b->consecutive_failures++;
        if (b->is_healthy && b->consecutive_failures >= FAIL_THRESHOLD) {
            b->is_healthy = 0;
            printf("[WARN] %s: marcado como UNHEALTHY\n", b->name);
        }
    }
}
```

> [!IMPORTANT]
> **Los umbrales de transición evitan flapping.** Sin ellos, un backend con conectividad intermitente alterna rápidamente entre UP y DOWN, generando cascadas de re-routing. Los contadores consecutivos requieren estabilidad antes de cambiar estado.

---

## Tema 8 — Operación: timeouts, límites y logging

### Timeouts defensivos para servidores

| Timeout | Protege contra | Valor típico |
|---------|---------------|--------------|
| Handshake TLS | Slowloris (conexión lenta) | 5-10s |
| Lectura de headers | Clientes que envían headers gota a gota | 10-30s |
| Lectura de body | Upload enorme o lento intencional | 60s |
| Escritura de respuesta | Cliente que no lee la respuesta | 30s |
| Keep-alive idle | Conexiones abandonadas | 60-120s |

### Logging estructurado por request

```c
void log_request(const char *client_ip, const char *method,
                 const char *path, int status, double latency_ms) {
    time_t now = time(NULL);
    struct tm *tm = gmtime(&now);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", tm);

    // Formato parseable por herramientas de análisis:
    fprintf(stderr, "%s client=%s method=%s path=%s status=%d latency_ms=%.1f\n",
            ts, client_ip, method, path, status, latency_ms);
}
// 2025-03-09T15:04:05Z client=192.168.1.42 method=GET path=/api/v1/users status=200 latency_ms=12.3
```

### Validación atómica de configuración

```c
// Patrón: parsear → validar → swap atómico
config_t *reload_config(const char *path, const config_t *current) {
    // 1. Parsear en nueva estructura
    config_t *candidate = parse_config(path);
    if (!candidate) {
        LOG_ERROR("Config parse failed, keeping current");
        return NULL;
    }

    // 2. Validar semántica
    if (validate_config(candidate) != 0) {
        LOG_ERROR("Config validation failed, keeping current");
        free_config(candidate);
        return NULL;
    }

    // 3. Swap: apuntar a la nueva, liberar la vieja
    // (en un servidor real con threads, esto requiere sincronización)
    return candidate;
}
```

---

## Checklist de salida del Bloque 10

- [ ] Parsear archivo de zona DNS y contar registros por tipo
- [ ] Validar reglas semánticas de DNS (CNAME exclusivo, MX con A/AAAA)
- [ ] Seleccionar MX preferido por menor prioridad
- [ ] Auditar política TLS (detectar versiones y suites inseguras)
- [ ] Parsear URL con validación de esquema, host, puerto y path
- [ ] Implementar máquina de estados para sesión SMTP
- [ ] Parsear respuestas multilínea SMTP (distinguir continuación y final)
- [ ] Auditar `/etc/exports` NFS detectando `no_root_squash` y exports abiertos
- [ ] Auditar `smb.conf` detectando shares con `guest ok` + `writable`
- [ ] Implementar longest-prefix match para routing de reverse proxy
- [ ] Health checks con umbrales de transición (evitar flapping)

---

## Referencias

| Recurso | Comando/Link |
|---------|-------------|
| DNS zone format | `man 5 named.conf`, [RFC 1035](https://datatracker.ietf.org/doc/html/rfc1035) |
| TLS | [Mozilla SSL Configuration Generator](https://ssl-config.mozilla.org/) |
| SMTP | [RFC 5321](https://datatracker.ietf.org/doc/html/rfc5321) |
| NFS exports | `man 5 exports` |
| Samba config | `man 5 smb.conf`, `testparm` |
| URL parsing | [RFC 3986](https://datatracker.ietf.org/doc/html/rfc3986) |
