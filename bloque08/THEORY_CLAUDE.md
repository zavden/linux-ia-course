# THEORY_CLAUDE.md — Bloque 08: Almacenamiento Avanzado en Linux/C

> Este bloque conecta tu C con infraestructura real de almacenamiento. No se trata de hacer `df` a mano — se trata de construir herramientas que **auditen, monitoreen y alerten** sobre capacidad, montajes, permisos extendidos y eventos del filesystem, todo programáticamente y con la robustez que exige producción.

---

## Mapa del Bloque

```
Tema 1: Capacidad (statvfs)     →  Métricas de espacio, bfree vs bavail
Tema 2: Montajes                →  /proc/mounts vs /etc/fstab, detección de drift
Tema 3: Extended attributes     →  xattr: metadatos key-value sobre archivos
Tema 4: ACLs                    →  Permisos granulares más allá de rwx
Tema 5: inotify                 →  Monitoreo de cambios en el filesystem en tiempo real
Tema 6: LVM                     →  PV/VG/LV, parseo de reportes
Tema 7: Quotas                  →  Límites por usuario/proyecto
Tema 8: Auditor de storage      →  Arquitectura collector → rules → reporter
```

---

## Tema 1 — Capacidad del filesystem: `statvfs`

### La syscall que reemplaza a `df`

```c
#include <sys/statvfs.h>

struct statvfs st;
if (statvfs("/home", &st) == -1) {
    perror("statvfs");
    return -1;
}

unsigned long total  = st.f_blocks * st.f_frsize;   // bytes totales
unsigned long avail  = st.f_bavail * st.f_frsize;   // bytes disponibles para usuarios
unsigned long used   = total - (st.f_bfree * st.f_frsize);
double pct_used      = 100.0 * (1.0 - (double)st.f_bavail / st.f_blocks);

printf("Total: %lu MB, Usado: %lu MB (%.1f%%)\n",
       total / (1024*1024), used / (1024*1024), pct_used);
```

### Campos clave de `struct statvfs`

| Campo | Tipo | Significado |
|-------|------|-------------|
| `f_blocks` | `fsblkcnt_t` | Bloques totales del filesystem |
| `f_bfree` | `fsblkcnt_t` | Bloques libres (**incluyendo los reservados para root**) |
| `f_bavail` | `fsblkcnt_t` | Bloques libres **para usuarios no-root** |
| `f_frsize` | `unsigned long` | Tamaño real del bloque en bytes |
| `f_files` | `fsfilcnt_t` | Inodos totales |
| `f_ffree` | `fsfilcnt_t` | Inodos libres |
| `f_namemax` | `unsigned long` | Longitud máxima de nombre de archivo |

> [!IMPORTANT]
> **Usa `f_bavail`, no `f_bfree`, para alertas de capacidad.** Los filesystems ext4 reservan ~5% de espacio para root (para que el sistema no se quede completamente sin espacio). `f_bfree` incluye esos bloques reservados; `f_bavail` refleja lo que un usuario normal realmente puede usar.

### Mini-df sobre múltiples rutas

```c
void check_paths(const char **paths, int n) {
    for (int i = 0; i < n; i++) {
        struct statvfs st;
        if (statvfs(paths[i], &st) == -1) {
            fprintf(stderr, "statvfs %s: %s\n", paths[i], strerror(errno));
            continue;   // reportar y seguir, no abortar
        }

        double pct = 100.0 * (1.0 - (double)st.f_bavail / st.f_blocks);
        const char *status = pct > 90 ? "CRIT" : pct > 80 ? "WARN" : "OK";
        printf("[%s] %s: %.1f%% usado\n", status, paths[i], pct);
    }
}
```

> [!CAUTION]
> **Rutas distintas pueden estar en el mismo filesystem.** `/home/user/docs` y `/home/user/photos` probablemente están en el mismo FS y retornarán métricas idénticas. Para un auditor real, agrupa por dispositivo (`st.f_fsid`).

---

## Tema 2 — Inventario de montajes

### `/proc/mounts` vs `/etc/fstab`

| Archivo | Qué contiene | Cuándo leerlo |
|---------|-------------|---------------|
| `/proc/mounts` | Montajes **activos** (estado real del kernel) | Auditoría del estado actual |
| `/etc/fstab` | Montajes **declarados** (configuración deseada) | Verificar qué debería estar montado |

Detectar **drift** (diferencia entre lo declarado y lo real) es una tarea de auditoría clásica en SRE.

### Formato de `/proc/mounts`

```
/dev/sda1 / ext4 rw,relatime 0 0
tmpfs /tmp tmpfs rw,nosuid,nodev 0 0
/dev/sdb1 /data xfs rw,relatime 0 0
```

Campos: `dispositivo punto_montaje tipo_fs opciones dump pass`

### Parser robusto

```c
typedef struct {
    char device[256];
    char mountpoint[PATH_MAX];
    char fstype[64];
    char options[512];
} mount_entry_t;

int parse_mounts(const char *path, mount_entry_t *entries, int max_entries) {
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); return -1; }

    char line[1024];
    int count = 0;
    while (fgets(line, sizeof(line), f) && count < max_entries) {
        // Ignorar comentarios y líneas vacías
        if (line[0] == '#' || line[0] == '\n') continue;

        int parsed = sscanf(line, "%255s %s %63s %511s",
                            entries[count].device,
                            entries[count].mountpoint,
                            entries[count].fstype,
                            entries[count].options);
        if (parsed >= 3) count++;  // mínimo 3 campos
    }
    fclose(f);
    return count;
}
```

### Detectar drift: fstab vs mounts

```c
void audit_mounts(void) {
    mount_entry_t fstab[128], mounts[128];
    int nfstab = parse_mounts("/etc/fstab", fstab, 128);
    int nmounts = parse_mounts("/proc/mounts", mounts, 128);

    for (int i = 0; i < nfstab; i++) {
        int found = 0;
        for (int j = 0; j < nmounts; j++) {
            if (strcmp(fstab[i].mountpoint, mounts[j].mountpoint) == 0) {
                found = 1;
                if (strcmp(fstab[i].fstype, mounts[j].fstype) != 0) {
                    printf("[WARN] %s: fstype mismatch (fstab=%s, actual=%s)\n",
                           fstab[i].mountpoint, fstab[i].fstype, mounts[j].fstype);
                }
                break;
            }
        }
        if (!found) {
            printf("[CRIT] %s: declarado en fstab pero NO montado\n",
                   fstab[i].mountpoint);
        }
    }
}
```

---

## Tema 3 — Extended Attributes (xattr)

### Qué son

Los xattrs son pares **clave-valor** adjuntos a inodos. Piensa en ellos como metadatos arbitrarios del archivo que no caben en los campos estándar de `stat`:

```
archivo.txt
├── stat: size=1024, mode=0644, uid=1000, mtime=...  (metadatos estándar)
└── xattr:
    ├── user.backup_date = "2025-03-01"
    ├── user.classification = "confidential"
    └── security.selinux = "system_u:object_r:user_home_t:s0"
```

### API de xattr en Linux

```c
#include <sys/xattr.h>

// Escribir un atributo
const char *value = "2025-03-01";
if (setxattr(path, "user.backup_date", value, strlen(value), 0) == -1) {
    perror("setxattr");
}

// Leer un atributo
char buf[256];
ssize_t len = getxattr(path, "user.backup_date", buf, sizeof(buf));
if (len == -1) {
    if (errno == ENODATA) printf("Atributo no existe\n");
    else perror("getxattr");
} else {
    buf[len] = '\0';  // getxattr NO null-termina
    printf("backup_date = %s\n", buf);
}

// Listar todos los atributos
char list[1024];
ssize_t list_len = listxattr(path, list, sizeof(list));
// list contiene nombres separados por '\0': "user.a\0user.b\0"
for (char *p = list; p < list + list_len; p += strlen(p) + 1) {
    printf("  attr: %s\n", p);
}

// Eliminar
removexattr(path, "user.backup_date");
```

> [!WARNING]
> **No todos los filesystems soportan xattrs.** tmpfs por defecto no. Algunos NFS tampoco. Tu código debe manejar `ENOTSUP` (operación no soportada) sin crashear — es una limitación del entorno, no un bug.

### Namespaces de xattr

| Namespace | Quién puede usarlo | Ejemplo |
|-----------|--------------------|---------| 
| `user.*` | Cualquier usuario con permisos al archivo | `user.tags`, `user.backup_date` |
| `security.*` | Root / módulos de seguridad | `security.selinux` |
| `system.*` | Kernel (ej: ACL almacenadas) | `system.posix_acl_access` |
| `trusted.*` | Solo root | `trusted.internal_flag` |

---

## Tema 4 — ACLs: permisos más allá de owner/group/other

### El problema que resuelven

Con permisos Unix clásicos (`rwx` para user/group/other), no puedes decir "el usuario `alice` tiene lectura pero `bob` tiene lectura+escritura" sin restructurar grupos. Las **ACLs** permiten reglas por usuario y grupo específico.

### Formato de `getfacl`

```
# file: proyecto/
# owner: carlos
# group: devteam
user::rwx                     ← permisos del propietario
user:alice:r-x                ← alice: solo lectura+ejecución
group::r-x                    ← grupo devteam
group:ops:rwx                 ← grupo ops: acceso total
mask::rwx                     ← máscara (techo de permisos nombrados)
other::---                    ← nadie más
default:user::rwx             ← herencia para archivos nuevos
default:group::r-x
default:other::---
```

### Parsear output de `getfacl` en C

```c
typedef struct {
    char type[16];      // "user", "group", "mask", "other", "default:user", etc.
    char qualifier[64]; // nombre del usuario/grupo (vacío para entries sin nombre)
    char perms[4];      // "rwx", "r-x", etc.
} acl_entry_t;

int parse_acl_line(const char *line, acl_entry_t *entry) {
    // Ignorar comentarios y líneas vacías
    if (line[0] == '#' || line[0] == '\n') return 0;

    // Formato: "type:qualifier:perms"
    char buf[256];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    // Eliminar newline
    char *nl = strchr(buf, '\n');
    if (nl) *nl = '\0';

    // Separar por ':'
    char *first = buf;
    char *second = strchr(first, ':');
    if (!second) return 0;
    *second++ = '\0';
    char *third = strchr(second, ':');
    if (!third) return 0;
    *third++ = '\0';

    snprintf(entry->type, sizeof(entry->type), "%s", first);
    snprintf(entry->qualifier, sizeof(entry->qualifier), "%s", second);
    snprintf(entry->perms, sizeof(entry->perms), "%s", third);
    return 1;
}
```

---

## Tema 5 — `inotify`: monitoreo de cambios en tiempo real

### Cómo funciona

`inotify` es una API del kernel Linux que te permite **suscribirte a eventos** en archivos y directorios sin hacer polling:

```
Tu programa ──▶ inotify_add_watch("/var/log", IN_CREATE | IN_MODIFY)
                    │
                    ▼
               [kernel observa /var/log]
                    │
                    ▼ (archivo creado o modificado)
               Tu programa recibe evento via read()
```

### Flujo completo

```c
#include <sys/inotify.h>
#include <limits.h>

int monitor_directory(const char *path) {
    // 1. Crear instancia de inotify
    int ifd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (ifd == -1) { perror("inotify_init1"); return -1; }

    // 2. Añadir watch
    int wd = inotify_add_watch(ifd, path,
                                IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
    if (wd == -1) { perror("inotify_add_watch"); close(ifd); return -1; }

    // 3. Leer eventos (integrable con poll/epoll)
    char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));

    printf("Monitoreando %s...\n", path);
    while (1) {
        ssize_t len = read(ifd, buf, sizeof(buf));
        if (len == -1) {
            if (errno == EAGAIN) {
                // No hay eventos — en un reactor real, vuelves a poll/epoll
                usleep(100000);
                continue;
            }
            perror("read"); break;
        }

        // 4. Procesar eventos
        for (char *ptr = buf; ptr < buf + len; ) {
            struct inotify_event *ev = (struct inotify_event *)ptr;

            printf("  Evento: ");
            if (ev->mask & IN_CREATE)  printf("CREATED ");
            if (ev->mask & IN_DELETE)  printf("DELETED ");
            if (ev->mask & IN_MODIFY)  printf("MODIFIED ");
            if (ev->mask & IN_MOVED_FROM) printf("MOVED_FROM ");
            if (ev->mask & IN_MOVED_TO)   printf("MOVED_TO ");
            if (ev->len > 0) printf("- %s", ev->name);
            printf("\n");

            ptr += sizeof(struct inotify_event) + ev->len;
        }
    }

    inotify_rm_watch(ifd, wd);
    close(ifd);
    return 0;
}
```

### Eventos útiles

| Evento | Cuándo se genera |
|--------|------------------|
| `IN_CREATE` | Archivo/directorio creado |
| `IN_DELETE` | Archivo/directorio eliminado |
| `IN_MODIFY` | Contenido del archivo modificado |
| `IN_MOVED_FROM` | Archivo movido FUERA del directorio vigilado |
| `IN_MOVED_TO` | Archivo movido AL directorio vigilado |
| `IN_CLOSE_WRITE` | Archivo cerrado tras escritura |
| `IN_ATTRIB` | Metadatos cambiados (permisos, timestamps) |

> [!NOTE]
> **`inotify` no es recursivo.** Si vigilas `/var/log`, no recibirás eventos para `/var/log/subdir/file.log`. Para monitorear subdirectorios, necesitas añadir watches a cada uno (y detectar nuevos subdirectorios vía `IN_CREATE`).

### Fallback portable: polling con `stat`

Para macOS o sistemas sin `inotify`:

```c
void poll_for_changes(const char *path, int interval_sec) {
    struct stat prev_st;
    if (stat(path, &prev_st) == -1) return;

    while (1) {
        sleep(interval_sec);
        struct stat cur_st;
        if (stat(path, &cur_st) == -1) continue;

        if (cur_st.st_mtime != prev_st.st_mtime) {
            printf("Cambio detectado en %s\n", path);
            prev_st = cur_st;
        }
    }
}
```

---

## Tema 6 — LVM: conceptos para herramientas de auditoría

### La pila de abstracción

```
Disco físico (/dev/sda, /dev/sdb)
    │
    ▼
Physical Volumes (PV)     ← pvcreate /dev/sda1
    │
    ▼
Volume Groups (VG)        ← vgcreate data_vg /dev/sda1 /dev/sdb1
    │
    ▼
Logical Volumes (LV)      ← lvcreate -L 50G -n app_lv data_vg
    │
    ▼
Filesystem                ← mkfs.ext4 /dev/data_vg/app_lv
```

### Parsear reportes de LVM

En herramientas C de auditoría, la forma más pragmática es parsear la salida JSON o CSV de `pvs`/`vgs`/`lvs`:

```bash
# Salida parseable:
lvs --reportformat json -o lv_name,vg_name,lv_size,lv_attr
```

```c
// O con separador:
// lvs --separator='|' --noheadings -o lv_name,vg_name,lv_size
//   app_lv|data_vg|50.00g

void parse_lvs_line(const char *line) {
    char lv_name[64], vg_name[64], lv_size[32];
    if (sscanf(line, " %63[^|]|%63[^|]|%31s", lv_name, vg_name, lv_size) == 3) {
        printf("LV: %s, VG: %s, Size: %s\n", lv_name, vg_name, lv_size);
    }
}
```

> [!CAUTION]
> **Nunca ejecutes operaciones destructivas de LVM sin confirmación explícita.** Siempre implementa modo dry-run (`--test` en LVM) y requiere flag de confirmación (`--yes-i-am-sure`). Un `lvremove` accidental destruye datos irrecuperablemente.

---

## Tema 7 — Quotas: límites de almacenamiento

### Modelo de quotas

```
                      soft limit      hard limit
                         │                │
  ■■■■■■■■■■░░░░░░░░░░░│░░░░░░░░░░░░░░░│
  └── uso actual ──────┘                │
       OK                WARN (grace)   CRIT (bloqueado)
```

| Estado | Condición | Efecto |
|--------|-----------|--------|
| **OK** | `usage < soft` | Normal |
| **WARN** | `soft ≤ usage < hard` | Grace period: el usuario debe liberar espacio |
| **CRIT** | `usage ≥ hard` | Escrituras bloqueadas |

### Verificación de umbrales

```c
typedef struct {
    char     username[64];
    uint64_t usage_kb;
    uint64_t soft_kb;
    uint64_t hard_kb;
} quota_entry_t;

const char *evaluate_quota(const quota_entry_t *q) {
    if (q->hard_kb > 0 && q->usage_kb >= q->hard_kb) return "CRIT";
    if (q->soft_kb > 0 && q->usage_kb >= q->soft_kb) return "WARN";
    return "OK";
}
```

---

## Tema 8 — Arquitectura del auditor de storage

### Las 3 capas

```
┌─────────────────────────────────────┐
│          REPORTER                   │
│  Genera salida: texto / JSON        │
│  Estado global: OK / WARN / CRIT    │
├─────────────────────────────────────┤
│          RULES (evaluación)         │
│  Umbrales: disco > 90% → CRIT      │
│  Drift: fstab ≠ /proc/mounts → WARN│
│  Quotas: usage > soft → WARN       │
├─────────────────────────────────────┤
│          COLLECTORS (recolección)   │
│  statvfs → métricas de capacidad    │
│  /proc/mounts → montajes activos   │
│  xattrs → metadatos de archivos    │
│  LVM reports → estado de volúmenes  │
└─────────────────────────────────────┘
```

### Principios de diseño

| Principio | Implementación |
|-----------|---------------|
| **Idempotencia** | Ejecutar dos veces produce el mismo resultado |
| **Tolerancia** | Si `/proc/mounts` no existe (ej: macOS), reportar y continuar |
| **Salida estable** | JSON con campos fijos, parseable por scripts |
| **Estado global** | `max(todos los checks)` — si cualquier check es CRIT, el reporte es CRIT |

### Reporte JSON

```c
void report_json(check_result_t *checks, int n) {
    const char *global = "OK";
    for (int i = 0; i < n; i++) {
        if (strcmp(checks[i].status, "CRIT") == 0) global = "CRIT";
        else if (strcmp(checks[i].status, "WARN") == 0 && strcmp(global, "CRIT") != 0)
            global = "WARN";
    }

    printf("{\"status\": \"%s\", \"checks\": [\n", global);
    for (int i = 0; i < n; i++) {
        printf("  {\"name\": \"%s\", \"status\": \"%s\", \"detail\": \"%s\"}%s\n",
               checks[i].name, checks[i].status, checks[i].detail,
               i < n-1 ? "," : "");
    }
    printf("]}\n");
}
```

---

## Checklist de salida del Bloque 08

- [ ] Obtener capacidad de un filesystem con `statvfs` usando `f_bavail` (no `f_bfree`)
- [ ] Parsear `/proc/mounts` y `/etc/fstab` y detectar montajes faltantes
- [ ] Leer, escribir y listar xattrs con manejo de `ENOTSUP`
- [ ] Parsear salida de `getfacl` e identificar entradas con más permisos que la máscara
- [ ] Monitorear un directorio con `inotify` y reportar archivos creados/eliminados
- [ ] Parsear reportes de `lvs` y calcular espacio total asignado
- [ ] Evaluar quotas con umbrales soft/hard y reportar estado OK/WARN/CRIT
- [ ] Construir un auditor con arquitectura collector → rules → reporter

---

## Referencias

| Recurso | Comando |
|---------|---------|
| Capacidad | `man 2 statvfs` |
| Montajes | `man 5 proc` (buscar `/proc/mounts`), `man 5 fstab` |
| Extended attrs | `man 2 setxattr`, `man 2 getxattr`, `man 2 listxattr` |
| ACLs | `man 5 acl`, `man 1 getfacl`, `man 1 setfacl` |
| inotify | `man 7 inotify`, `man 2 inotify_init1`, `man 2 inotify_add_watch` |
| LVM | `man 8 lvs`, `man 8 vgs`, `man 8 pvs` |
| Quotas | `man 1 quota`, `man 2 quotactl` |
