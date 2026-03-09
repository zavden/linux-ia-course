# 🚀 Proyecto 0 — "build-lab": Sistema de Build Multi-Distro

## 🎯 Objetivo

Integrar los conocimientos del Bloque 00 para crear una herramienta real y útil. Construirás **build-lab**, un script que automatiza la compilación y ejecución de tests en múltiples distribuciones usando Docker, y genera un reporte comparativo. Usaremos esta herramienta en el resto del curso.

## 📋 Requisitos Funcionales

Debes escribir un script llamado `build_lab.sh` que reciba como argumento la ruta a un directorio de ejercicio (ej. `../ej01/`) y haga lo siguiente:

1. **Validación:**
   Verificar que el directorio exista y contenga `Dockerfile.fedora`, `Dockerfile.debian` y un directorio de `tests/`.

2. **Ejecución Automatizada:**
   Para cada distribución (Fedora y Debian):
   - Construir la imagen Docker: `docker build -f Dockerfile.<distro> -t test-<distro> .`
   - Ejecutar el contenedor (que por defecto corre `make run`).
   - Sobrescribir el Entrypoint para correr los tests: `docker run --rm --entrypoint make test-<distro> test`
   - Capturar el *Exit Code* (0 = Éxito, otro = Fallo) y el output.

3. **Reporte Comparativo:**
   Al finalizar, el script debe imprimir un resumen como este:
   ```
   [build-lab] Reporte de tests para: ../ej01/
   - Fedora: ✅ PASSED
   - Debian: ❌ FAILED
   ```

## 📐 Arquitectura del Proyecto

Tú escribirás el shell script maestro:

```
src/
└── build_lab.sh
```

El script operará invocando a los ejecutables de Docker directamente.

## 🛠️ Implementación Recomendada

### Fase 1 — Argumentos y Validación
Asegúrate de manejar el paso de argumentos (`$1`). Utiliza `cd "$1"` para moverte al directorio objetivo. Si falla, aborta.

### Fase 2 — Loop por distro
Crea un array de distribuciones `DISTROS=("fedora" "debian")`. Itera sobre él, construye las imágenes silenciando la salida (`> /dev/null`) para mantener el reporte limpio.

### Fase 3 — Test y Captura de Códigos
Ejecuta el container y usa `$?` para capturar el código de salida. Imprime el resumen.

## ✅ Criterios de Éxito

- [ ] `build_lab.sh` tiene permisos de ejecución (`chmod +x`).
- [ ] Script toma el path como `$1` y valida su existencia.
- [ ] Logra ejecutar las pruebas automatizadas tanto en la imagen de Fedora como en la de Debian.
- [ ] Muestra un resumen claro y conciso del resultado.

## 🐳 Docker

Para este proyecto en sí, no probarás "tu código C" compilándolo con Docker, sino que probarás tu script "corriendo en tu host" (o en un entorno capaz de lanzar Docker) *contra* otros ejercicios.

```bash
# Pruébalo contra el ejercicio 01
./src/build_lab.sh ../ej01/
```

## 📖 Referencias

- `man bash` — variables, statements if, loops
- `docker build --help`
- `docker run --help` — presta atención a `--entrypoint`
