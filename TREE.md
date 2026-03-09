# 🌳 TREE.md — Estructura del Curso Linux-C

## Visión General

```
Linux/
├── MAIN.md                    # Programa completo del curso
├── TREE.md                    # ← Este archivo: guía de estructura
├── _templates/                # Templates reutilizables (NO modificar)
│
├── bloque00/                  # Entorno y Docker
├── bloque01/                  # Fundamentos de C en Linux
├── bloque02/                  # Archivos y Sistema de Archivos
├── bloque03/                  # Procesos y Señales
├── bloque04/                  # Memoria
├── bloque05/                  # Hilos y Concurrencia
├── bloque06/                  # Redes y Sockets
├── bloque07/                  # Administración de Sistemas
├── bloque08/                  # Almacenamiento Avanzado
├── bloque09/                  # Seguridad
├── bloque10/                  # Servicios de Red
├── bloque11/                  # Kernel y Bajo Nivel
└── bloque12/                  # Proyecto Final Integrador
```

---

## 📁 Anatomía de un Ejercicio

Cada ejercicio sigue esta estructura idéntica:

```
bloqueXX/ejNN/
├── README.md              # Enunciado, teoría, instrucciones, criterios
├── Dockerfile.fedora      # Imagen Fedora (pre-configurada)
├── Dockerfile.debian      # Imagen Debian (pre-configurada)
├── docker-compose.yml     # Orquestación dual-distro
├── Makefile               # Build system (all, clean, run, test, debug)
├── src/                   # TU CÓDIGO VA AQUÍ
│   └── .gitkeep
├── tests/                 # Scripts de test (bash)
│   └── .gitkeep
└── solucion/              # Solución de referencia
    └── src/
        └── .gitkeep
```

## 📁 Anatomía de un Proyecto

Los proyectos son más grandes pero siguen la misma base:

```
bloqueXX/proyectoXX/
├── README.md
├── Dockerfile.fedora
├── Dockerfile.debian
├── docker-compose.yml
├── Makefile
├── src/                   # Código fuente del proyecto
│   └── .gitkeep
└── tests/
    └── .gitkeep
```

## 📁 Proyecto Final (Bloque 12)

El proyecto final tiene múltiples componentes:

```
bloque12/proyecto-final/
├── docker-compose.yml         # Orquesta todos los servicios
├── minicloud-registry/
│   ├── Dockerfile.fedora
│   ├── Dockerfile.debian
│   ├── Makefile
│   └── src/
├── minicloud-gateway/
│   ├── Dockerfile.fedora
│   ├── Dockerfile.debian
│   ├── Makefile
│   └── src/
├── minicloud-monitor/
│   ├── ...
├── minicloud-vault/
│   ├── ...
├── minicloud-runner/
│   ├── ...
├── tests/                     # Tests de integración
└── docs/                      # Documentación, man pages
```

---

## 🐳 Templates (_templates/)

> [!CAUTION]
> **NO modifiques los archivos en `_templates/`.** Son la fuente maestra. Si necesitas cambios, edita la copia dentro de cada ejercicio.

Existen **6 variantes** de Dockerfiles, asignadas automáticamente según el bloque:

| Template | Paquetes clave | Usado en |
|---|---|---|
| **Base** | gcc, make, gdb, valgrind | B0, B1, B2, B3, B4, B5, B11 |
| **Net** | + openssl-devel, iproute, nmap-ncat, curl | B6, B10, B12 |
| **Systemd** | + systemd-devel, dbus-devel, pam-devel | B7 |
| **Storage** | + lvm2-devel, quota, libacl-devel, ncurses | B8 |
| **Security** | + openssl-devel, libcap-devel, pam-devel, seccomp | B9 |
| **Systemd** | + SELinux (Fedora) / AppArmor (Debian) | B7 |

Otros templates disponibles:

| Archivo | Descripción |
|---|---|
| `Makefile` | Build con C17, flags de seguridad, debug con ASan |
| `docker-compose.yml` | Orquesta Fedora + Debian en paralelo |
| `README.ejercicio.md` | Plantilla para enunciados de ejercicios |
| `README.proyecto.md` | Plantilla para enunciados de proyectos |

---

## 🚦 Cómo Seguir el Curso

### Regla #1: Orden Secuencial

Los bloques tienen dependencias. **No saltar:**

```
B0 → B1 → B2 → B3 → B4 → B5 → B6
                                  ↓
                    B7 → B8 → B9 → B10 → B11 → B12
```

### Regla #2: Flujo por Ejercicio

Para cada ejercicio, seguir este ciclo:

```
1. Leer README.md           → Entender el problema
2. Escribir código en src/  → Implementar
3. make all                 → Compilar (0 warnings)
4. make run                 → Ejecutar
5. make test                → Pasar tests
6. docker compose up        → Verificar en AMBAS distros
7. git add && git commit    → Versionar
```

### Regla #3: Dual-Distro Siempre

**Todo ejercicio debe compilar y pasar tests en Fedora Y Debian.** El `docker-compose.yml` ya está preparado:

```bash
# Desde cualquier ejercicio:
docker compose up --build

# O manualmente:
docker build -f Dockerfile.fedora -t test-fedora . && docker run --rm test-fedora
docker build -f Dockerfile.debian -t test-debian . && docker run --rm test-debian
```

### Regla #4: Proyectos son Obligatorios

Los 5 ejercicios son preparación. El proyecto es la evaluación real.

```
ej01 ─┐
ej02 ─┤
ej03 ─┼→ Proyecto (combina todo)
ej04 ─┤
ej05 ─┘
```

> [!IMPORTANT]
> No avanzar al siguiente bloque sin completar el proyecto del bloque actual.

### Regla #5: Git Disciplinado

```bash
# Inicializar (una vez)
cd ~/Learning/Linux
git init
echo "build/" >> .gitignore
echo "*.o" >> .gitignore

# Un commit por ejercicio completado
git add bloque00/ej01/
git commit -m "B00-E01: Hello Docker - completado"

# Un commit por proyecto completado
git add bloque00/proyecto00/
git commit -m "B00-P00: build-lab - completado"
```

---

## 🔧 Comandos Rápidos de Referencia

### Makefile

```bash
make all        # Compilar (producción, -O2)
make debug      # Compilar con debug (ASan, -g, -O0)
make run        # Compilar y ejecutar
make test       # Ejecutar tests
make clean      # Limpiar build/
```

### Docker

```bash
# Build + run rápido
docker build -f Dockerfile.fedora -t ejXX-fedora . && docker run --rm ejXX-fedora

# Modo interactivo (debug)
docker build -f Dockerfile.fedora -t ejXX-fedora .
docker run --rm -it ejXX-fedora /bin/bash

# Con privilegios (bloques 8, 11)
docker run --rm -it --privileged ejXX-fedora /bin/bash

# Con ptrace (GDB/strace)
docker run --rm -it --cap-add=SYS_PTRACE ejXX-fedora /bin/bash

# Dual-distro
docker compose up --build

# Limpieza
docker system prune -f
```

### Flags del Compilador

```bash
# Producción
gcc -Wall -Wextra -Werror -pedantic -std=c17 -O2

# Debug
gcc -Wall -Wextra -Werror -pedantic -std=c17 -g -O0 -fsanitize=address,undefined

# Valgrind (no mezclar con ASan)
gcc -Wall -Wextra -Werror -pedantic -std=c17 -g -O0
valgrind --leak-check=full --show-leak-kinds=all ./programa
```

---

## 📊 Progreso del Curso

Usa esta tabla para trackear tu avance. Cópiala a un archivo `PROGRESO.md`:

```markdown
| Bloque | Ej1 | Ej2 | Ej3 | Ej4 | Ej5 | Proyecto | Estado |
|--------|-----|-----|-----|-----|-----|----------|--------|
| B00    | [x] | [x] | [x] | [x] | [x] | [x]      | ✅     |
| B01    | [x] | [x] | [x] | [x] | [x] | [x]      | ✅     |
| B02    | [x] | [x] | [x] | [x] | [x] | [x]      | ✅     |
| B03    | [x] | [x] | [x] | [x] | [x] | [x]      | ✅     |
| B04    | [x] | [x] | [x] | [x] | [x] | [x]      | ✅     |
| B05    | [x] | [x] | [x] | [x] | [x] | [x]      | ✅     |
| B06    | [x] | [x] | [x] | [x] | [x] | [x]      | ✅     |
| B07    | [x] | [x] | [ ] | [ ] | [ ] | [ ]      | ⬜     |
| B08    | [ ] | [ ] | [ ] | [ ] | [ ] | [ ]      | ⬜     |
| B09    | [ ] | [ ] | [ ] | [ ] | [ ] | [ ]      | ⬜     |
| B10    | [ ] | [ ] | [ ] | [ ] | [ ] | [ ]      | ⬜     |
| B11    | [ ] | [ ] | [ ] | [ ] | [ ] | [ ]      | ⬜     |
| B12    | —   | —   | —   | —   | —   | [ ]      | ⬜     |

Leyenda: [ ] pendiente | [x] completado | ⬜ no iniciado | 🟡 en progreso | ✅ completo
```

---

## ⚠️ Diferencias Fedora vs Debian

Algunos paquetes tienen nombres distintos. Los Dockerfiles ya los manejan, pero para referencia:

| Propósito | Fedora (dnf) | Debian (apt) |
|---|---|---|
| Compilador C | `gcc` | `gcc` |
| OpenSSL dev | `openssl-devel` | `libssl-dev` |
| Systemd dev | `systemd-devel` | `libsystemd-dev` |
| D-Bus dev | `dbus-devel` | `libdbus-1-dev` |
| SELinux | `libselinux-devel` | N/A (usar `libapparmor-dev`) |
| PAM | `pam-devel` | `libpam0g-dev` |
| ncurses | `ncurses-devel` | `libncurses-dev` |
| ACL | `libacl-devel` | `libacl1-dev` |
| LVM | `lvm2-devel` | `libdevmapper-dev` |
| Capabilities | `libcap-devel` | `libcap-dev` |
| seccomp | `libseccomp-devel` | `libseccomp-dev` |
| man pages | `man-db man-pages` | `man-db manpages-dev` |
