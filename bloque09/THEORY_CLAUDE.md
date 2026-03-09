# THEORY_CLAUDE.md — Bloque 09: Seguridad en Linux/C — Hardening, Integridad y Defensa

> En C, cualquier error de memoria puede ser una vulnerabilidad. Un buffer overflow no es un crash — es un vector de ejecución remota de código. Este bloque enseña a pensar como un atacante para programar como un defensor: validación estricta, mínimo privilegio, integridad criptográfica, y herramientas que auditan la postura de seguridad de un sistema.

---

## Mapa del Bloque

```
Tema 1: Modelo de seguridad    →  Capas de defensa, superficie de ataque
Tema 2: Capabilities           →  Privilegio mínimo sin root completo
Tema 3: SELinux/AppArmor       →  Mandatory Access Control (MAC)
Tema 4: Integridad cripto      →  SHA-256, HMAC-SHA256, comparación constante
Tema 5: Validación de entrada  →  Política explícita, integer overflow
Tema 6: Path traversal         →  Composición segura de rutas
Tema 7: PAM                    →  Autenticación delegada al sistema
Tema 8: Hardening de build     →  Flags de compilación defensivos
Tema 9: Secretos en memoria    →  Limpieza, tiempo de vida, logging seguro
```

---

## Tema 1 — Modelo de seguridad: pensar en capas

### La superficie de ataque de un programa C

```
Entradas externas (cada una es un vector de ataque potencial):
┌─────────────────────────────────────────────────┐
│ argv/argc        ← argumentos CLI               │
│ stdin            ← input del usuario             │
│ archivos         ← configuración, datos          │
│ sockets          ← datos de red (potencialmente  │
│                    controlados por atacante)      │
│ environ          ← variables de entorno           │
│ /proc, /sys      ← estado del sistema             │
│ señales          ← modifican flujo de control     │
└─────────────────────────────────────────────────┘
```

### Capas de defensa (defense in depth)

| Capa | Tipo | Ejemplo |
|------|------|---------|
| **1. Preventiva** | Evitar que el bug sea posible | Validación estricta de input |
| **2. Limitante** | Reducir el impacto si ocurre | Mínimo privilegio (capabilities) |
| **3. Detectiva** | Descubrir que algo va mal | Logs, auditoría, checks de integridad |
| **4. Correctiva** | Recuperar el estado seguro | Fail-safe, rollback, kill del proceso |

> [!IMPORTANT]
> **Principio central:** todo dato externo es hostil hasta demostrar lo contrario. No importa si viene de un archivo local, un argumento CLI, o una variable de entorno — valídalo como si un atacante lo controlara, porque en muchos escenarios, lo controla.

---

## Tema 2 — Capabilities: mínimo privilegio sin root

### El problema de root

Tradicionalmente, un proceso es "root" (UID 0) o no. Root puede hacer **todo**: abrir cualquier archivo, bindear puertos privilegiados, matar cualquier proceso, cargar módulos del kernel. Si un servicio con root es comprometido, el atacante tiene acceso total.

### Capabilities: fragmentar root en bits

Las **capabilities** dividen los privilegios de root en ~40 permisos individuales:

| Capability | Qué permite | Ejemplo |
|------------|-------------|---------|
| `CAP_NET_BIND_SERVICE` | Bindear puertos < 1024 | Servidor web en puerto 80 |
| `CAP_NET_RAW` | Sockets raw (ping, tcpdump) | Herramienta de diagnóstico de red |
| `CAP_DAC_OVERRIDE` | Ignorar permisos de archivos | Casi nunca necesario |
| `CAP_SYS_ADMIN` | "Comodín" — demasiados privilegios | Evitar si es posible |
| `CAP_CHOWN` | Cambiar owner de archivos | Instaladores de paquetes |

### Leer capabilities del proceso actual

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void read_caps(void) {
    FILE *f = fopen("/proc/self/status", "r");
    if (!f) { perror("fopen"); return; }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "Cap", 3) == 0) {
            printf("%s", line);
        }
    }
    fclose(f);
}
// Salida:
// CapInh: 0000000000000000
// CapPrm: 0000000000000000
// CapEff: 0000000000000000    ← effective = lo que puedes hacer AHORA
// CapBnd: 000001ffffffffff
// CapAmb: 0000000000000000
```

### Decodificar la máscara

```c
// Cada bit de CapEff corresponde a una capability:
// Bit 0  = CAP_CHOWN
// Bit 10 = CAP_NET_BIND_SERVICE
// Bit 13 = CAP_NET_RAW
// ...

uint64_t cap_eff = 0x00000000a80425fb;  // ejemplo
if (cap_eff & (1ULL << 10)) printf("Tiene CAP_NET_BIND_SERVICE\n");
if (cap_eff & (1ULL << 13)) printf("Tiene CAP_NET_RAW\n");
```

### Patrón: drop de privilegios

```c
// Con libcap (recomendado en producción):
#include <sys/capability.h>

void drop_all_caps(void) {
    cap_t caps = cap_get_proc();
    cap_clear(caps);                   // eliminar todas
    cap_set_proc(caps);                // aplicar
    cap_free(caps);
}

// O mantener solo lo necesario:
void keep_only_net_bind(void) {
    cap_t caps = cap_get_proc();
    cap_clear(caps);
    cap_value_t keep[] = { CAP_NET_BIND_SERVICE };
    cap_set_flag(caps, CAP_EFFECTIVE, 1, keep, CAP_SET);
    cap_set_flag(caps, CAP_PERMITTED, 1, keep, CAP_SET);
    cap_set_proc(caps);
    cap_free(caps);
}
```

> [!TIP]
> **Desde línea de comandos,** puedes asignar capabilities a un binario sin hacerlo SUID root:
> ```bash
> sudo setcap cap_net_bind_service=+ep ./webserver
> # Ahora ./webserver puede bindear puerto 80 sin ser root
> ```

---

## Tema 3 — SELinux y AppArmor: confinamiento obligatorio

### Mandatory Access Control (MAC)

Capabilities controlan lo que un **proceso** puede hacer. MAC controla lo que un **proceso puede acceder**, incluso si tiene los permisos Unix correctos:

```
Sin MAC:  proceso con UID=www-data puede leer cualquier archivo de www-data
Con MAC:  proceso nginx solo puede leer /var/www/* y /etc/nginx/* 
          (aunque tenga permisos Unix para leer /home/*)
```

### SELinux (Red Hat, Fedora, CentOS)

```bash
# Estado actual:
getenforce       # Enforcing, Permissive, o Disabled

# Contexto de un proceso:
ps -eZ | grep nginx
# system_u:system_r:httpd_t:s0   ← tipo = httpd_t

# Contexto de un archivo:
ls -Z /var/www/index.html
# system_u:object_r:httpd_sys_content_t:s0
```

Verificar en C:

```c
void check_selinux(void) {
    // Leer modo actual
    FILE *f = fopen("/sys/fs/selinux/enforce", "r");
    if (!f) {
        printf("[INFO] SELinux no disponible\n");
        return;
    }
    int enforce;
    fscanf(f, "%d", &enforce);
    fclose(f);
    printf("[%s] SELinux: %s\n",
           enforce ? "OK" : "WARN",
           enforce ? "enforcing" : "permissive");
}
```

### AppArmor (Ubuntu, Debian, SUSE)

```bash
# Estado actual:
sudo aa-status     # perfiles en enforce/complain

# Modo de un perfil:
cat /sys/kernel/security/apparmor/profiles
# /usr/sbin/nginx (enforce)
```

---

## Tema 4 — Integridad criptográfica

### SHA-256: huella de contenido

SHA-256 produce un hash de 32 bytes (256 bits) que detecta cualquier cambio en la entrada:

```c
#include <stdint.h>
#include <string.h>

// Implementación simplificada del API (con OpenSSL o implementación propia):
// SHA256(data, len, digest)  → guarda 32 bytes en digest

void verify_file_integrity(const char *path, const uint8_t *expected_hash) {
    // 1. Leer archivo
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return; }

    // 2. Calcular hash incremental
    SHA256_CTX ctx;
    sha256_init(&ctx);
    uint8_t buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        sha256_update(&ctx, buf, n);
    }
    fclose(f);

    uint8_t digest[32];
    sha256_final(&ctx, digest);

    // 3. Comparar con hash esperado (¡en tiempo constante!)
    if (constant_time_compare(digest, expected_hash, 32)) {
        printf("[OK] %s: integridad verificada\n", path);
    } else {
        printf("[CRIT] %s: HASH NO COINCIDE\n", path);
    }
}
```

### HMAC-SHA256: integridad + autenticación

SHA-256 detecta cambios, pero un atacante puede recalcular el hash tras modificar el archivo. **HMAC** requiere una clave secreta, así que solo quien la tiene puede generar un MAC válido:

```
SHA-256:  hash = SHA256(mensaje)              ← cualquiera puede recalcular
HMAC:     mac  = HMAC(clave, mensaje)         ← solo quien tiene la clave
```

```c
// HMAC-SHA256(key, key_len, data, data_len, mac_out)
// mac_out = 32 bytes

void sign_and_store(const char *path, const uint8_t *key, size_t key_len) {
    // Leer datos
    uint8_t *data = read_file(path, &data_len);

    // Calcular HMAC
    uint8_t mac[32];
    hmac_sha256(key, key_len, data, data_len, mac);

    // Guardar MAC junto a los datos (ej: en .mac file)
    char mac_path[PATH_MAX];
    snprintf(mac_path, sizeof(mac_path), "%s.mac", path);
    FILE *f = fopen(mac_path, "wb");
    fwrite(mac, 1, 32, f);
    fclose(f);
    free(data);
}
```

### Comparación en tiempo constante

```c
// ❌ VULNERABLE a timing attack:
if (memcmp(received_mac, expected_mac, 32) != 0) {
    return AUTH_FAIL;
}
// memcmp corta al primer byte distinto → el atacante puede
// medir el tiempo y adivinar byte por byte

// ✅ SEGURO:
int constant_time_compare(const uint8_t *a, const uint8_t *b, size_t len) {
    volatile uint8_t result = 0;   // volatile evita optimización del compilador
    for (size_t i = 0; i < len; i++) {
        result |= a[i] ^ b[i];    // acumula diferencias, no corta nunca
    }
    return result == 0;  // 1 si iguales, 0 si distintos
}
```

> [!CAUTION]
> **Nunca uses `strcmp` o `memcmp` para comparar MACs, tokens, o passwords.** La diferencia de tiempo entre "primer byte correcto" y "primer byte incorrecto" puede ser de nanosegundos, pero con miles de intentos, es medible.

---

## Tema 5 — Validación estricta de entrada

### Política explícita, no "que no crashee"

```c
// ❌ INSUFICIENTE — no crashea pero acepta basura:
int port = atoi(argv[1]);

// ✅ VALIDACIÓN COMPLETA:
int parse_port(const char *str, uint16_t *port) {
    if (!str || !*str) return -1;  // vacío/null

    char *endptr;
    errno = 0;
    long val = strtol(str, &endptr, 10);

    // 1. ¿Se consumió toda la string?
    if (*endptr != '\0') return -1;          // "80abc" → rechazar

    // 2. ¿Hubo overflow?
    if (errno == ERANGE) return -1;          // "99999999999" → rechazar

    // 3. ¿Está en rango válido?
    if (val < 1 || val > 65535) return -1;   // 0 o negativo → rechazar

    *port = (uint16_t)val;
    return 0;
}
```

### Validación de strings

```c
// Validar que un nombre de usuario solo contiene caracteres permitidos:
int validate_username(const char *name) {
    if (!name || !*name) return -1;
    size_t len = strlen(name);
    if (len < 3 || len > 32) return -1;   // longitud

    for (size_t i = 0; i < len; i++) {
        char c = name[i];
        // Solo alfanuméricos + underscore + guión
        if (!((c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') ||
              c == '_' || c == '-')) {
            return -1;
        }
    }
    return 0;
}
```

### Integer overflow en cálculos de tamaño

```c
// ❌ VULNERABLE:
size_t total = num_records * sizeof(record_t);  // puede desbordar
record_t *records = malloc(total);               // asigna muy poco
// Luego escribes num_records records → buffer overflow

// ✅ SEGURO:
if (num_records > SIZE_MAX / sizeof(record_t)) {
    fprintf(stderr, "overflow en cálculo de tamaño\n");
    return NULL;
}
size_t total = num_records * sizeof(record_t);
record_t *records = malloc(total);
```

---

## Tema 6 — Path traversal: el enemigo de las rutas

### El ataque

```
Tu servidor: abre root_dir + "/" + user_input
Atacante envía: "../../../etc/shadow"
Resultado: tu servidor abre /etc/shadow
```

### Defensa en capas

```c
#include <string.h>
#include <stdlib.h>

// Capa 1: Rechazar componentes peligrosos
int has_traversal(const char *path) {
    // Rechazar rutas absolutas
    if (path[0] == '/') return 1;

    // Rechazar ".." como componente
    const char *p = path;
    while (*p) {
        if (p[0] == '.' && p[1] == '.' && (p[2] == '/' || p[2] == '\0'))
            return 1;
        p = strchr(p, '/');
        if (!p) break;
        p++;
    }
    return 0;
}

// Capa 2: Verificar que la ruta resuelta está dentro del directorio base
int safe_path(const char *base, const char *user_input, char *result, size_t rsize) {
    if (has_traversal(user_input)) return -1;

    // Construir ruta candidata
    char candidate[PATH_MAX];
    snprintf(candidate, sizeof(candidate), "%s/%s", base, user_input);

    // Resolver a ruta absoluta canónica
    if (!realpath(candidate, result)) return -1;

    // Verificar que empieza con el directorio base
    char resolved_base[PATH_MAX];
    if (!realpath(base, resolved_base)) return -1;

    size_t base_len = strlen(resolved_base);
    if (strncmp(result, resolved_base, base_len) != 0) return -1;
    if (result[base_len] != '/' && result[base_len] != '\0') return -1;

    return 0;
}
```

> [!WARNING]
> **`realpath` tiene una vulnerabilidad TOCTOU.** Entre la llamada a `realpath` y el `open`, un atacante puede crear un symlink. Para defensa máxima, usa `openat` con el FD del directorio base y `O_NOFOLLOW`:
> ```c
> int base_fd = open(base_dir, O_RDONLY | O_DIRECTORY);
> int fd = openat(base_fd, user_input, O_RDONLY | O_NOFOLLOW);
> ```

---

## Tema 7 — PAM: autenticación delegada

### Qué es PAM

PAM (Pluggable Authentication Modules) separa la **política de autenticación** del código de tu aplicación. En vez de verificar passwords tú mismo, delegas al stack del sistema:

```
Tu aplicación
    │
    ▼
libpam ──▶ /etc/pam.d/tu-app
    │         │
    │         ├── pam_unix.so      ← verifica /etc/shadow
    │         ├── pam_faillock.so  ← lockout tras N intentos
    │         └── pam_audit.so     ← logea intentos
    │
    ▼
Resultado: PAM_SUCCESS o PAM_AUTH_ERR
```

### Buenas prácticas con PAM

| Práctica | Por qué |
|----------|---------|
| Encapsular PAM detrás de tu propia interfaz | Testeable sin PAM real |
| No revelar si el usuario existe | Evitar enumeración (siempre "credenciales inválidas") |
| Rate limit / lockout | Proteger contra fuerza bruta |
| Logear intentos CON contexto pero SIN secretos | Auditoría útil sin filtrar datos |

---

## Tema 8 — Hardening de compilación

### Flags defensivas completas

```bash
gcc -Wall -Wextra -Werror -pedantic -std=c17 \
    -fstack-protector-strong \          # canary en el stack
    -D_FORTIFY_SOURCE=2 \              # checks de buffer en libc
    -fPIE -pie \                        # ASLR para el ejecutable
    -Wl,-z,relro,-z,now \              # proteger GOT/PLT
    -fsanitize=address,undefined \      # solo en desarrollo
    -o programa src/main.c
```

### Qué hace cada flag

| Flag | Protección | Contra qué |
|------|-----------|-------------|
| `-fstack-protector-strong` | Canary en el stack | Stack buffer overflow |
| `-D_FORTIFY_SOURCE=2` | Checks de longitud en `memcpy`, `strcpy`, etc. | Buffer overflows en funciones de libc |
| `-fPIE -pie` | Executable con direcciones aleatorias (ASLR) | Ataques que dependen de direcciones fijas |
| `-Wl,-z,relro,-z,now` | GOT de solo lectura | Overwrite de punteros en GOT |
| `-Wformat -Wformat-security` | Detectar format strings inseguros | `printf(user_input)` → format string attack |

> [!CAUTION]
> **`-D_FORTIFY_SOURCE=2` requiere al menos `-O1`.** Con `-O0`, el compilador no puede aplicar las comprobaciones. Úsalo en release, no en debug con ASan.

### Verificar protecciones de un binario

```bash
# Usando checksec (de pwntools o paquete checksec):
checksec --file=./programa
# RELRO:    Full RELRO
# Stack:    Canary found
# NX:       NX enabled
# PIE:      PIE enabled
```

---

## Tema 9 — Secretos en memoria

### Reglas para manejo de secretos

| Regla | Implementación |
|-------|---------------|
| Minimizar tiempo de vida | Sobrescribir con ceros inmediatamente después de usar |
| No loguear secretos | `LOG("Auth attempt for user=%s", user)` — nunca el password |
| Evitar copias innecesarias | No duplicar en variables temporales |
| No mezclar con datos normales | Struct separado, limpieza explícita |

### Limpieza segura de buffers

```c
// ❌ El compilador puede eliminar este memset como "escritura muerta":
char password[64];
// ... usar password ...
memset(password, 0, sizeof(password));  // optimizado away!

// ✅ Función que el compilador no puede eliminar:
#include <string.h>

// POSIX define explicit_bzero (glibc 2.25+):
explicit_bzero(password, sizeof(password));

// O manualmente con volatile:
static void secure_zero(void *ptr, size_t len) {
    volatile unsigned char *p = ptr;
    while (len--) *p++ = 0;
}
```

---

## Checklist de salida del Bloque 09

- [ ] Leer y decodificar `CapEff` del proceso actual, identificando capabilities activas
- [ ] Implementar plan de drop de capabilities (conservar solo las necesarias)
- [ ] Verificar estado de SELinux/AppArmor y reportar si no está en enforcing
- [ ] Calcular SHA-256 de un archivo y verificar contra hash esperado
- [ ] Implementar HMAC-SHA256 para integridad autenticada
- [ ] Comparar MACs en tiempo constante sin early-return
- [ ] Validar input numérico con `strtol` + verificación de rango + endptr
- [ ] Detectar y bloquear path traversal (`../`, rutas absolutas, symlinks)
- [ ] Compilar con todas las flags de hardening y verificar con `checksec`
- [ ] Limpiar buffers de secretos con `explicit_bzero`

---

## Referencias

| Recurso | Comando/Link |
|---------|-------------|
| Capabilities | `man 7 capabilities`, `man 8 setcap`, `man 3 cap_set_proc` |
| SELinux | `man 8 getenforce`, `man 8 setenforce` |
| AppArmor | `man 8 aa-status`, `man 5 apparmor.d` |
| Cryptography | `man 3 SHA256`, `man 3 HMAC` (OpenSSL) |
| Fortify | `man 7 feature_test_macros` (buscar `_FORTIFY_SOURCE`) |
| PAM | `man 3 pam_authenticate`, `man 5 pam.conf` |
| Secure coding | [CERT C Coding Standard](https://wiki.sei.cmu.edu/confluence/display/c) |
