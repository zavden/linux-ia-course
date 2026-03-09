# THEORY.md — Bloque 07: Debugging, Build Systems y Metaprogramación en C

Este bloque no trata de “escribir más código”, sino de **hacerlo mantenible, depurable y verificable**.
La diferencia entre un programa hobby y uno profesional suele estar aquí.

---

## 1. Modos de compilación: debug vs release

### 1.1 Objetivos distintos
- **Debug**: máxima observabilidad.
- **Release**: máximo rendimiento/tamaño razonable.

### 1.2 Flags típicas
Debug:
- `-g`
- `-O0` (o `-Og`)
- warnings estrictos (`-Wall -Wextra -Werror`)

Release:
- `-O2` o `-O3`
- `-DNDEBUG` (opcional) para desactivar `assert`

### 1.3 `assert`
`assert` es útil para documentar invariantes internas.
- No reemplaza validación de entradas de usuario.
- Puede desactivarse en release (`NDEBUG`), así que no pongas lógica crítica solo ahí.

---

## 2. Errores de memoria: categorías y síntomas

Tipos comunes:
1. leak
2. use-after-free
3. out-of-bounds read/write
4. double-free
5. punteros no inicializados

Síntomas reales:
- segfault tardío en función “inocente”
- corrupción silenciosa de datos
- comportamiento no determinista entre ejecuciones

Regla práctica: si un bug “aparece y desaparece”, sospecha memoria o concurrencia.

---

## 3. GDB: depuración interactiva

Comandos mínimos útiles:
- `run`
- `break <func|file:line>`
- `next` / `step`
- `print <expr>`
- `backtrace`
- `frame <n>`
- `info locals`

Flujo recomendado:
1. reproducir bug con entrada pequeña
2. breakpoint cerca del fallo
3. inspeccionar estado justo antes del crash
4. verificar hipótesis y repetir

---

## 4. Core dumps y análisis post-mortem

Cuando un proceso muere por señal grave (`SIGSEGV`, `SIGABRT`, etc.), el core dump permite inspección posterior.

Checklist básico:
1. habilitar core dumps en entorno (`ulimit`, políticas del SO)
2. compilar binario con símbolos (`-g`)
3. abrir con debugger (`gdb <binario> <core>`)
4. inspeccionar stack, variables y memoria

Ventaja: no necesitas reproducir bug en vivo para investigarlo.

---

## 5. Sanitizers (ASan/UBSan)

### 5.1 AddressSanitizer
Detecta:
- out-of-bounds
- use-after-free
- stack/heap overflows

### 5.2 UndefinedBehaviorSanitizer
Detecta UB como:
- signed overflow
- shifts inválidos
- conversiones peligrosas

Uso típico:
```bash
gcc -fsanitize=address,undefined -g -O0 ...
```

En CI/desarrollo, sanitizers suelen dar feedback más rápido que valgrind.

---

## 6. Valgrind/Memcheck (cuando esté disponible)

Muy útil para auditoría de memoria:
- leaks exactos
- accesos inválidos
- rastreo detallado de origen

Tradeoff:
- ejecución mucho más lenta
- no siempre disponible en todos los entornos

Recomendación: úsalo en casos reproducibles pequeños y bien acotados.

---

## 7. `errno`, contexto y reporting de errores

Error handling profesional en C:
- validar valor de retorno de syscalls/libc
- capturar `errno` inmediatamente tras fallo
- reportar **qué operación** y **sobre qué recurso** falló

Malo:
```c
printf("error\n");
```

Mejor:
```c
fprintf(stderr, "open %s failed: %s\n", path, strerror(errno));
```

La depuración real depende de ese contexto.

---

## 8. Makefiles y construcción incremental

### 8.1 Concepto clave
`make` recompila solo lo necesario según dependencias y timestamps.

### 8.2 Buenas prácticas
- variables (`CC`, `CFLAGS`, `LDFLAGS`, `LDLIBS`)
- targets estándar (`all`, `clean`, `run`, `test`, `debug`)
- separar compilación (`.o`) y link final
- evitar comandos duplicados

### 8.3 Escalado modular
En proyectos medianos/grandes:
- varios `.c/.h`
- bibliotecas internas
- profiles de build por entorno

---

## 9. Preprocesador y feature flags

Herramientas clave:
- `#ifdef`, `#if`, `#ifndef`
- macros parametrizadas
- constantes de compilación (`-DFAST_MODE=1`)

Usos correctos:
- activar instrumentación
- seleccionar implementación por plataforma
- aislar código experimental

Riesgos:
- macros opacas con efectos laterales
- caminos de compilación no testeados
- exceso de condicionales difíciles de mantener

---

## 10. Librerías estáticas y compartidas

### 10.1 Estática (`.a`)
- el código se integra al binario final
- despliegue simple
- binario más grande

### 10.2 Compartida (`.so`/`.dylib`)
- se carga en runtime
- binarios más pequeños
- gestión de paths/versionado más delicada

Decisión depende de distribución, compatibilidad y operación.

---

## 11. Instrumentación ligera de rendimiento

Antes de optimizar:
1. medir
2. identificar hotspots
3. optimizar lo crítico
4. volver a medir

En este nivel basta con:
- `clock_gettime(CLOCK_MONOTONIC, ...)`
- medición por scope
- comparación antes/después de cambios

Sin medición, “optimizar” suele ser intuición incorrecta.

---

## 12. Mapa del bloque (práctica)

### Resueltos
- `e01_assert_failfast`: invariantes y validación defensiva.
- `e02_trace_macros`: logging por niveles con macros.
- `e03_errno_contexto`: errores con contexto y `errno`.
- `e04_multifile_make_basico`: proyecto modular `.c/.h`.
- `e05_mini_unittest_harness`: harness simple de tests en C.
- `e06_preprocessor_feature_flags`: rutas por `#ifdef`.
- `e07_static_library_local`: build + uso de librería estática.
- `e08_dispatch_table_comandos`: tabla de comandos y punteros a función.
- `e09_clock_benchmark_basico`: medición temporal básica.
- `e10_mini_profiler_scopes`: instrumentación por etapas.

### Complejos
- `c01_debuggable_cli_calculadora`: CLI modular depurable con gdb.
- `c02_build_system_static_shared`: pipeline de build estática/compartida.
- `c03_memory_bug_lab_guiado`: laboratorio reproducible de bugs de memoria.

---

## 13. Checklist de calidad antes de entregar

1. ¿Compila en `debug` y `release`?
2. ¿Warnings en cero con flags estrictas?
3. ¿Errores reportan contexto útil?
4. ¿Tests cubren rutas de error, no solo “happy path”?
5. ¿Hay instrumentación mínima para tiempos críticos?

Si cumples esto, tu código en C será mucho más fácil de evolucionar y operar.
