# EJERCICIOS.md - Bloque 11 (Índice)

Este archivo es solo mapa de ejercicios.
No incluye enunciados largos ni solución embebida.

Práctica completa en `bloque11/practica/`.

---

## Resueltos (10)

## E01 - parser de /proc/modules
- Objetivo: extraer inventario de módulos cargados y uso.
- Qué hace: cuenta módulos, dependencias y estado `Live`.
- Ruta: `bloque11/practica/resueltos/e01_proc_modules_parser`

## E02 - resumen tipo lsmod
- Objetivo: obtener métricas operativas de tabla de módulos.
- Qué hace: cuenta módulos activos y detecta el de mayor tamaño.
- Ruta: `bloque11/practica/resueltos/e02_lsmod_like_summary`

## E03 - decoder de flags de namespaces
- Objetivo: interpretar máscara `CLONE_NEW*`.
- Qué hace: traduce bits a namespaces y total de aislamientos solicitados.
- Ruta: `bloque11/practica/resueltos/e03_namespace_flags_decoder`

## E04 - parser CPU en cgroup v2
- Objetivo: leer límites de CPU de forma correcta.
- Qué hace: parsea `cpu.max`/`cpu.weight` y calcula ratio de cuota.
- Ruta: `bloque11/practica/resueltos/e04_cgroupv2_cpu_parser`

## E05 - parser memoria en cgroup v2
- Objetivo: evaluar presión de memoria contra límites.
- Qué hace: interpreta `memory.max/current/swap.max` y porcentaje usado.
- Ruta: `bloque11/practica/resueltos/e05_cgroupv2_memory_parser`

## E06 - linter de perfil seccomp
- Objetivo: validar baseline de reducción de syscalls.
- Qué hace: verifica acción por defecto, allowlist mínima y syscalls peligrosas.
- Ruta: `bloque11/practica/resueltos/e06_seccomp_profile_linter`

## E07 - parser de sysctl.conf
- Objetivo: analizar configuración kernel `key=value`.
- Qué hace: cuenta dominios de parámetros y detecta settings inseguros.
- Ruta: `bloque11/practica/resueltos/e07_sysctl_kv_parser`

## E08 - inspector de namespaces por PID (mock)
- Objetivo: construir inventario de namespaces por proceso.
- Qué hace: parsea `name:[inode]`, cuenta tipos e IDs únicos.
- Ruta: `bloque11/practica/resueltos/e08_proc_pid_ns_inspector`

## E09 - auditor de tuning de kernel
- Objetivo: evaluar parámetros contra una política de hardening.
- Qué hace: aplica checks y emite estado `OK/WARN/CRIT`.
- Ruta: `bloque11/practica/resueltos/e09_kernel_tuning_auditor`

## E10 - validador de config de minicontenedor
- Objetivo: evitar ejecución con configuración inválida.
- Qué hace: valida campos obligatorios, rangos y formato de política.
- Ruta: `bloque11/practica/resueltos/e10_minicontainer_config_validator`

---

## Complejos (3)

## C01 - namespace sandbox launcher
- Objetivo: lanzar procesos aislados con namespaces seleccionables.
- Qué debe hacer: aislamiento, hardening de privilegios y diagnóstico por etapas.
- Ruta: `bloque11/practica/complejos/c01_namespace_sandbox_launcher`

## C02 - cgroup controller manager
- Objetivo: gestionar límites de CPU/memoria en cgroup v2.
- Qué debe hacer: reconciliar estado deseado vs estado real con validación.
- Ruta: `bloque11/practica/complejos/c02_cgroup_controller_manager`

## C03 - minicontainer runtime skeleton
- Objetivo: integrar namespaces + cgroups + políticas en un runtime mínimo.
- Qué debe hacer: pipeline completo de validate/prepare/isolate/run/cleanup.
- Ruta: `bloque11/practica/complejos/c03_minicontainer_runtime_skeleton`

---

## Notas

1. Cada ejercicio incluye `README.md`, `src/`, `tests/`, `Makefile`.
2. Los resueltos incluyen comentarios explicativos en el código.
3. Los complejos son plantillas guiadas para implementación propia.
