# THEORY_CLAUDE.md — Bloque 11: Kernel y Bajo Nivel en Linux/C

> Docker no es magia — es C hablando con el kernel. Cada contenedor que ejecutas usa namespaces para aislamiento, cgroups para límites de recursos, y seccomp para restringir syscalls. Este bloque te enseña a usar esas mismas interfaces del kernel directamente desde C, para construir tus propias herramientas de aislamiento.

---

## Mapa del Bloque

```
Tema 1: Userspace vs Kernel    →  Syscalls, la frontera, modelo de privilegios
Tema 2: Namespaces             →  Aislamiento de visión (pid, net, mnt, uts, user)
Tema 3: cgroups v2             →  Límites de CPU, memoria, swap
Tema 4: seccomp                →  Filtro de syscalls (deny-by-default)
Tema 5: sysctl                 →  Parámetros de kernel: hardening y tuning
Tema 6: Módulos de kernel      →  Observación y diagnóstico desde /proc/modules
Tema 7: minicontainer          →  Pipeline: validate → prepare → isolate → run → cleanup
```

---

## Tema 1 — Userspace vs Kernelspace

### El modelo de dos mundos

```
┌─────────────────────────────────────────────────────────┐
│                    USERSPACE                             │
│  Tu programa C, librerías, todo lo que escribes         │
│  • No puede acceder al hardware directamente            │
│  • No puede modificar page tables, IRQs, etc.           │
│  • Pide servicios al kernel vía SYSCALLS                │
├─────────────────────── syscall barrier ─────────────────┤
│                    KERNELSPACE                           │
│  El kernel Linux                                        │
│  • Controla hardware, memoria, scheduling               │
│  • Aplica aislamiento (namespaces)                       │
│  • Aplica límites (cgroups)                              │
│  • Filtra syscalls (seccomp)                             │
│  • Decide qué puede hacer cada proceso                   │
└─────────────────────────────────────────────────────────┘
```

Cada `open`, `read`, `write`, `fork`, `mmap` cruza esta barrera. El kernel **decide** si lo permite o no.

### Las tres capas de control del kernel

| Mecanismo | Qué controla | Pregunta que responde |
|-----------|-------------|----------------------|
| **Namespaces** | Qué **ve** el proceso | "¿Puedo ver el PID 1 real? ¿la interfaz eth0?" |
| **cgroups** | Cuánto **consume** el proceso | "¿Puedo usar más de 256 MB de RAM?" |
| **seccomp** | Qué **puede hacer** el proceso | "¿Puedo llamar a `mount`? ¿a `ptrace`?" |

Estas capas son **complementarias**. Un namespace PID no limita memoria. Un cgroup no restringe syscalls. Necesitas las tres para aislamiento real.

---

## Tema 2 — Namespaces: cambiar lo que ve el proceso

### Los 8 tipos de namespace

| Namespace | Flag `CLONE_NEW*` | Qué aísla | Ejemplo |
|-----------|-------------------|-----------|---------|
| **Mount** | `CLONE_NEWNS` | Puntos de montaje | El contenedor tiene su propio `/proc`, `/tmp` |
| **PID** | `CLONE_NEWPID` | Jerarquía de PIDs | El primer proceso del contenedor es PID 1 |
| **Network** | `CLONE_NEWNET` | Interfaces, rutas, IPs, puertos | El contenedor tiene su propia `eth0` virtual |
| **UTS** | `CLONE_NEWUTS` | Hostname y domainname | Tu contenedor se llama `webserver-prod-1` |
| **IPC** | `CLONE_NEWIPC` | Semáforos, colas de mensajes, shared memory | Aislamiento de IPC entre contenedores |
| **User** | `CLONE_NEWUSER` | Mapeo UID/GID y capabilities | Root dentro del contenedor = nobody fuera |
| **Cgroup** | `CLONE_NEWCGROUP` | Vista de la jerarquía de cgroups | El proceso solo ve su propio cgroup |
| **Time** | `CLONE_NEWTIME` | Clock offsets (kernel 5.6+) | El contenedor puede tener su propia hora |

### Decodificar flags de namespace

```c
#include <sched.h>

typedef struct {
    int         flag;
    const char *name;
} ns_flag_t;

static const ns_flag_t NS_FLAGS[] = {
    { CLONE_NEWNS,     "mnt"    },
    { CLONE_NEWPID,    "pid"    },
    { CLONE_NEWNET,    "net"    },
    { CLONE_NEWUTS,    "uts"    },
    { CLONE_NEWIPC,    "ipc"    },
    { CLONE_NEWUSER,   "user"   },
    { CLONE_NEWCGROUP, "cgroup" },
    { 0, NULL }
};

void decode_ns_flags(int flags) {
    printf("Namespaces solicitados: ");
    for (int i = 0; NS_FLAGS[i].name; i++) {
        if (flags & NS_FLAGS[i].flag) {
            printf("[%s] ", NS_FLAGS[i].name);
        }
    }
    printf("\n");
}

// Ejemplo:
decode_ns_flags(CLONE_NEWPID | CLONE_NEWNET | CLONE_NEWNS);
// Namespaces solicitados: [mnt] [pid] [net]
```

### Ver namespaces de un proceso

```bash
ls -la /proc/$$/ns/
# lrwxrwxrwx  ipc -> ipc:[4026531839]
# lrwxrwxrwx  mnt -> mnt:[4026531840]
# lrwxrwxrwx  net -> net:[4026531969]
# lrwxrwxrwx  pid -> pid:[4026531836]
# ...
# Los números entre [] son los inode IDs del namespace
```

```c
void inspect_namespaces(pid_t pid) {
    const char *ns_names[] = {"ipc", "mnt", "net", "pid", "user", "uts", "cgroup", NULL};
    for (int i = 0; ns_names[i]; i++) {
        char path[64], link[256];
        snprintf(path, sizeof(path), "/proc/%d/ns/%s", pid, ns_names[i]);
        ssize_t len = readlink(path, link, sizeof(link) - 1);
        if (len == -1) {
            printf("  %6s: no disponible\n", ns_names[i]);
        } else {
            link[len] = '\0';
            printf("  %6s: %s\n", ns_names[i], link);
        }
    }
}
```

> [!WARNING]
> **PID namespace no implica network namespace.** Un proceso con `CLONE_NEWPID` tiene su propio árbol de PIDs, pero sigue viendo las interfaces de red del host. Para aislamiento de red, necesitas `CLONE_NEWNET` + configuración de veth/bridge. Cada namespace aísla **una sola dimensión**.

---

## Tema 3 — cgroups v2: limitar recursos

### La jerarquía del filesystem

En cgroups v2, los límites se configuran escribiendo en archivos dentro de `/sys/fs/cgroup/`:

```
/sys/fs/cgroup/
├── cpu.max                  ← límites del cgroup raíz
├── memory.max
├── my_container/            ← cgroup para tu contenedor
│   ├── cpu.max              ← "50000 100000" = 50% de un core
│   ├── cpu.weight           ← peso relativo (1-10000, default 100)
│   ├── memory.max           ← "268435456" (256 MB) o "max" (sin límite)
│   ├── memory.current       ← uso actual en bytes
│   ├── memory.swap.max      ← límite de swap
│   └── cgroup.procs         ← PIDs de los procesos en este cgroup
```

### Parsear `cpu.max`

```c
typedef struct {
    long quota;      // microsegundos permitidos por periodo (-1 = ilimitado)
    long period;     // duración del periodo en microsegundos
    double pct;      // porcentaje de un core
} cpu_limit_t;

int parse_cpu_max(const char *path, cpu_limit_t *limit) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char buf[64];
    if (!fgets(buf, sizeof(buf), f)) { fclose(f); return -1; }
    fclose(f);

    // Formato: "quota period" o "max period"
    if (strncmp(buf, "max", 3) == 0) {
        limit->quota = -1;
        sscanf(buf + 4, "%ld", &limit->period);
        limit->pct = -1;  // sin límite
    } else {
        sscanf(buf, "%ld %ld", &limit->quota, &limit->period);
        limit->pct = 100.0 * (double)limit->quota / limit->period;
    }

    return 0;
}

// "50000 100000" → quota=50000, period=100000, pct=50.0%
// "max 100000"   → sin límite de CPU
```

### Parsear `memory.max` y `memory.current`

```c
typedef struct {
    long max_bytes;     // -1 si "max"
    long current_bytes;
    double pct_used;
} mem_limit_t;

int parse_memory(const char *cgroup_path, mem_limit_t *mem) {
    char path[PATH_MAX];

    // memory.max
    snprintf(path, sizeof(path), "%s/memory.max", cgroup_path);
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    char buf[64];
    fgets(buf, sizeof(buf), f);
    fclose(f);
    mem->max_bytes = (strncmp(buf, "max", 3) == 0) ? -1 : atol(buf);

    // memory.current
    snprintf(path, sizeof(path), "%s/memory.current", cgroup_path);
    f = fopen(path, "r");
    if (!f) return -1;
    fgets(buf, sizeof(buf), f);
    fclose(f);
    mem->current_bytes = atol(buf);

    // Porcentaje
    if (mem->max_bytes > 0) {
        mem->pct_used = 100.0 * (double)mem->current_bytes / mem->max_bytes;
    } else {
        mem->pct_used = -1;  // sin límite
    }

    return 0;
}
```

> [!CAUTION]
> **El literal `max` no es un número.** Muchos campos de cgroups v2 aceptan `max` como valor, significando "sin límite". Parsearlo con `atol` sin verificar primero retorna 0 — muy diferente de "sin límite". Siempre verifica `strncmp(buf, "max", 3)` antes de convertir.

---

## Tema 4 — seccomp: filtrar syscalls

### Qué es seccomp

seccomp (Secure Computing Mode) permite que un proceso **restrinja las syscalls** que puede ejecutar. Es la diferencia entre "este proceso puede hacer cualquier cosa que el kernel permite" y "este proceso solo puede hacer read, write y exit".

```
Sin seccomp:
  Proceso → mount()      ✅ (si tiene permisos)
  Proceso → ptrace()     ✅
  Proceso → reboot()     ✅

Con seccomp (allowlist):
  Proceso → read()       ✅ (permitida)
  Proceso → write()      ✅ (permitida)
  Proceso → exit_group() ✅ (permitida)
  Proceso → mount()      ❌ → SIGKILL o EPERM
  Proceso → ptrace()     ❌ → SIGKILL o EPERM
```

### Perfil JSON de seccomp (formato Docker/OCI)

```json
{
  "defaultAction": "SCMP_ACT_ERRNO",
  "architectures": ["SCMP_ARCH_X86_64"],
  "syscalls": [
    {
      "names": ["read", "write", "close", "fstat", "mmap",
                "mprotect", "munmap", "brk", "rt_sigaction",
                "rt_sigprocmask", "rt_sigreturn", "exit_group",
                "arch_prctl", "set_tid_address"],
      "action": "SCMP_ACT_ALLOW"
    }
  ]
}
```

### Linter de perfil seccomp

```c
typedef struct {
    char default_action[32];
    int  n_allowed;
    char allowed[128][32];    // syscalls permitidas
} seccomp_profile_t;

void lint_seccomp(const seccomp_profile_t *prof) {
    // Regla 1: default DEBE ser restrictivo
    if (strcmp(prof->default_action, "SCMP_ACT_ALLOW") == 0) {
        printf("[CRIT] defaultAction es ALLOW — perfil no protege nada\n");
    }

    // Regla 2: buscar syscalls peligrosas en la allowlist
    const char *dangerous[] = {
        "mount", "umount2", "ptrace", "reboot", "kexec_load",
        "init_module", "finit_module", "delete_module",
        "pivot_root", "sethostname", NULL
    };

    for (int i = 0; i < prof->n_allowed; i++) {
        for (int d = 0; dangerous[d]; d++) {
            if (strcmp(prof->allowed[i], dangerous[d]) == 0) {
                printf("[WARN] Syscall peligrosa permitida: %s\n", prof->allowed[i]);
            }
        }
    }

    // Regla 3: verificar syscalls esenciales
    const char *essential[] = {"read", "write", "exit_group", "rt_sigreturn", NULL};
    for (int e = 0; essential[e]; e++) {
        int found = 0;
        for (int i = 0; i < prof->n_allowed; i++) {
            if (strcmp(prof->allowed[i], essential[e]) == 0) { found = 1; break; }
        }
        if (!found) {
            printf("[WARN] Syscall esencial ausente: %s (el proceso puede no funcionar)\n",
                   essential[e]);
        }
    }
}
```

---

## Tema 5 — sysctl: parámetros de kernel

### Hardening via sysctl

```
/proc/sys/
├── kernel/
│   ├── kptr_restrict       ← ocultar punteros de kernel (1 o 2)
│   ├── dmesg_restrict      ← restringir acceso a dmesg (1)
│   ├── unprivileged_bpf_disabled ← deshabilitar BPF sin privilegios (1)
│   └── randomize_va_space  ← ASLR (2 = full)
├── fs/
│   ├── protected_symlinks  ← prevenir symlink attacks (1)
│   └── protected_hardlinks ← prevenir hardlink attacks (1)
└── net/
    └── ipv4/
        ├── ip_forward              ← routing entre interfaces (1 o 0)
        ├── conf/all/rp_filter      ← reverse path filtering (1)
        └── tcp_syncookies          ← protección contra SYN flood (1)
```

### Auditor de baseline de kernel

```c
typedef struct {
    const char *key;
    const char *expected;
    const char *severity;   // WARN o CRIT
    const char *rationale;
} sysctl_check_t;

static const sysctl_check_t HARDENING_BASELINE[] = {
    {"kernel.kptr_restrict",           "1", "WARN", "ocultar punteros de kernel"},
    {"kernel.dmesg_restrict",          "1", "WARN", "restringir dmesg a root"},
    {"kernel.randomize_va_space",      "2", "CRIT", "ASLR completo"},
    {"fs.protected_symlinks",          "1", "WARN", "prevenir symlink attacks"},
    {"fs.protected_hardlinks",         "1", "WARN", "prevenir hardlink attacks"},
    {"net.ipv4.conf.all.rp_filter",    "1", "WARN", "anti-spoofing"},
    {"net.ipv4.tcp_syncookies",        "1", "WARN", "protección SYN flood"},
    {NULL, NULL, NULL, NULL}
};

void audit_sysctl(void) {
    for (int i = 0; HARDENING_BASELINE[i].key; i++) {
        char path[256];
        // Convertir key: kernel.kptr_restrict → /proc/sys/kernel/kptr_restrict
        snprintf(path, sizeof(path), "/proc/sys/%s", HARDENING_BASELINE[i].key);
        // Reemplazar '.' por '/' en el path
        for (char *p = path + strlen("/proc/sys/"); *p; p++) {
            if (*p == '.') *p = '/';
        }

        FILE *f = fopen(path, "r");
        if (!f) {
            printf("[INFO] %s: no disponible\n", HARDENING_BASELINE[i].key);
            continue;
        }

        char value[64];
        if (fgets(value, sizeof(value), f)) {
            // Trim newline
            value[strcspn(value, "\n")] = '\0';
            const char *status =
                strcmp(value, HARDENING_BASELINE[i].expected) == 0 ? "OK" :
                HARDENING_BASELINE[i].severity;
            printf("[%s] %s = %s (esperado: %s) — %s\n",
                   status, HARDENING_BASELINE[i].key, value,
                   HARDENING_BASELINE[i].expected,
                   HARDENING_BASELINE[i].rationale);
        }
        fclose(f);
    }
}
```

---

## Tema 6 — Módulos de kernel

### Parsear `/proc/modules`

```
ext4 827392 2 - Live 0xffffffffc0200000
xfs 1576960 0 - Live 0xffffffffc0100000
nf_tables 262144 10 nft_chain_nat, Live 0xffffffffc0000000
```

Campos: `nombre tamaño instancias_en_uso dependencias estado dirección`

```c
typedef struct {
    char   name[64];
    long   size_bytes;
    int    instances;
    char   deps[256];
    char   state[16];
} kernel_module_t;

int parse_module_line(const char *line, kernel_module_t *mod) {
    int n = sscanf(line, "%63s %ld %d %255s %15s",
                   mod->name, &mod->size_bytes, &mod->instances,
                   mod->deps, mod->state);
    return (n >= 3) ? 0 : -1;
}

// Resumen operativo:
void module_summary(kernel_module_t *mods, int n) {
    int total = n, live = 0;
    long total_size = 0;
    for (int i = 0; i < n; i++) {
        total_size += mods[i].size_bytes;
        if (strcmp(mods[i].state, "Live") == 0) live++;
    }
    printf("Módulos: %d total, %d live, %.1f MB total\n",
           total, live, total_size / (1024.0 * 1024));
}
```

---

## Tema 7 — Pipeline de minicontainer

### La arquitectura

```
┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────┐    ┌──────────┐
│ VALIDATE │───▶│ PREPARE  │───▶│ ISOLATE  │───▶│ RUN  │───▶│ CLEANUP  │
└──────────┘    └──────────┘    └──────────┘    └──────┘    └──────────┘
     │               │               │              │            │
     ▼               ▼               ▼              ▼            ▼
 Config OK?     rootfs,          unshare(ns)    exec(cmd)   liberar cgroup,
 paths válidos? argumentos,      cgroup limits              cerrar FDs,
 límites sanos? env vars         seccomp filter             reportar resultado
```

### Configuración declarativa

```c
typedef struct {
    // Qué ejecutar
    char     command[PATH_MAX];
    char     argv[16][256];
    int      argc;

    // Aislamiento
    int      ns_flags;              // CLONE_NEWPID | CLONE_NEWNET | ...

    // Límites
    long     cpu_quota;             // microsegundos por periodo (-1 = sin límite)
    long     cpu_period;            // microsegundos (default 100000)
    long     memory_max_bytes;      // -1 = sin límite

    // Seguridad
    char     seccomp_profile[PATH_MAX];
    int      drop_all_caps;         // 1 = drop capabilities
} container_config_t;
```

### Validación temprana

```c
int validate_config(const container_config_t *cfg) {
    int errors = 0;

    // Comando obligatorio y absoluto
    if (cfg->command[0] != '/') {
        fprintf(stderr, "ERROR: command debe ser path absoluto: '%s'\n", cfg->command);
        errors++;
    }

    // Límites de CPU coherentes
    if (cfg->cpu_quota > 0 && cfg->cpu_period <= 0) {
        fprintf(stderr, "ERROR: cpu_quota requiere cpu_period positivo\n");
        errors++;
    }
    if (cfg->cpu_quota > cfg->cpu_period && cfg->cpu_period > 0) {
        fprintf(stderr, "WARN: cpu_quota > cpu_period (más de un core)\n");
    }

    // Memoria razonable (mínimo 4 MB)
    if (cfg->memory_max_bytes > 0 && cfg->memory_max_bytes < 4 * 1024 * 1024) {
        fprintf(stderr, "ERROR: memory_max demasiado bajo (%ld bytes)\n",
                cfg->memory_max_bytes);
        errors++;
    }

    // Perfil seccomp existe
    if (cfg->seccomp_profile[0] && access(cfg->seccomp_profile, R_OK) == -1) {
        fprintf(stderr, "ERROR: seccomp profile no accesible: %s\n",
                cfg->seccomp_profile);
        errors++;
    }

    return errors == 0 ? 0 : -1;
}
```

> [!IMPORTANT]
> **Valida toda la config ANTES de crear namespaces o cgroups.** Si falla a mitad del pipeline (después de crear un cgroup y antes de lanzar el proceso), necesitas cleanup perfecto. Validar primero elimina la mayoría de esos casos.

---

## Checklist de salida del Bloque 11

- [ ] Decodificar flags `CLONE_NEW*` e identificar qué namespaces solicita un proceso
- [ ] Inspeccionar namespaces de un PID desde `/proc/<pid>/ns/`
- [ ] Parsear `cpu.max` y `cpu.weight` de cgroups v2, manejando el literal `max`
- [ ] Parsear `memory.max` y `memory.current`, calcular porcentaje de uso
- [ ] Validar perfil seccomp: default deny, syscalls peligrosas, syscalls esenciales
- [ ] Auditar parámetros de kernel contra baseline de hardening via `/proc/sys/`
- [ ] Parsear `/proc/modules` y generar resumen operativo
- [ ] Diseñar pipeline de minicontainer con validación temprana y cleanup garantizado
- [ ] Explicar por qué namespaces + cgroups + seccomp son complementarios

---

## Referencias

| Recurso | Comando/Link |
|---------|-------------|
| Namespaces | `man 7 namespaces`, `man 2 unshare`, `man 2 clone` |
| cgroups v2 | `man 7 cgroups`, [kernel docs](https://docs.kernel.org/admin-guide/cgroup-v2.html) |
| seccomp | `man 2 seccomp`, `man 3 seccomp_rule_add` (libseccomp) |
| sysctl | `man 8 sysctl`, `man 5 sysctl.conf` |
| Modules | `man 8 lsmod`, `man 5 modules` |
| Capabilities | `man 7 capabilities` |
| Container internals | [Linux containers in 500 lines of code](https://blog.lizzie.io/linux-containers-in-500-loc.html) |
