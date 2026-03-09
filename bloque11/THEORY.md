# THEORY.md - Bloque 11: Kernel y Bajo Nivel en Linux/C

Este bloque explica la capa donde Linux implementa aislamiento, límites de recursos
y control fino de ejecución. El objetivo no es escribir un kernel, sino **entender y usar
interfaces del kernel con criterio de producción** desde C.

---

## 1. Modelo mental: userspace vs kernelspace

En Linux, tu programa C vive en userspace y pide servicios al kernel vía syscalls.
Lo importante en este bloque:

1. El kernel es quien aplica aislamiento real.
2. Los mecanismos (namespaces, cgroups, seccomp) son complementarios.
3. La robustez depende de combinar bien esas capas, no de usar una sola.

---

## 2. Namespaces: aislamiento de visión

Un namespace no "borra" recursos: cambia qué subconjunto ve el proceso.
Tipos clave:
- `mnt`: vista de montajes
- `pid`: jerarquía de procesos
- `net`: interfaces/rutas/sockets
- `uts`: hostname/domainname
- `ipc`: semáforos/colas compartidas
- `user`: mapeo UID/GID y privilegios
- `cgroup`: vista de jerarquía de control
- `time`: offsets de clocks (kernels modernos)

### 2.1 Flags `CLONE_NEW*`
Se solicitan al crear procesos (`clone`/`unshare`/`setns`).
Decodificar estas máscaras es básico para auditar aislamiento pedido vs aplicado.

### 2.2 Riesgos típicos
1. crear namespace sin preparar rootfs/mounts
2. creer que `pid` aislado implica `net` aislado
3. usar `user` namespace sin entender mapeos UID/GID

---

## 3. cgroups v2: control de recursos

Mientras namespaces aíslan vista, cgroups limitan consumo.

### 3.1 Archivos clave
- `cpu.max`: cuota/periodo de CPU
- `cpu.weight`: peso relativo en contención
- `memory.max`: límite duro de memoria
- `memory.current`: uso actual
- `memory.swap.max`: límite de swap

### 3.2 Interpretación operacional
- `cpu.max = 50000 100000` -> ~50% de un core teórico
- `memory.max = max` -> sin límite explícito
- `memory.current / memory.max` -> presión de memoria

### 3.3 Errores comunes
1. asumir que `memory.current` es porcentaje sin normalizar
2. olvidar que algunos campos aceptan literal `max`
3. configurar límites imposibles y luego culpar al scheduler

---

## 4. seccomp: reducir superficie de syscalls

seccomp limita qué syscalls puede ejecutar un proceso.
No reemplaza namespaces/cgroups; agrega otra capa.

### 4.1 Política base razonable
- `defaultAction` restrictiva (denegar/errar por defecto)
- allowlist mínima (`read`, `write`, `exit`, `rt_sigreturn`, etc.)
- bloqueo explícito de syscalls peligrosas no necesarias

### 4.2 Riesgos de perfiles débiles
1. `defaultAction=ALLOW` deja bypass trivial
2. allowlist inflada aumenta superficie de ataque
3. falta de trazabilidad dificulta depuración de fallos legítimos

---

## 5. Parámetros del kernel (`sysctl`)

`sysctl` permite leer/ajustar parámetros runtime del kernel.
Desde ingeniería de plataformas, esto se usa para:
- hardening
- tuning de red
- tuning de memoria

### 5.1 Ejemplos de hardening
- `kernel.kptr_restrict=1`
- `kernel.dmesg_restrict=1`
- `kernel.unprivileged_bpf_disabled=1`
- `fs.protected_symlinks=1`

### 5.2 Ejemplos de tuning
- `vm.swappiness`
- `net.ipv4.conf.all.rp_filter`

### 5.3 Buenas prácticas
1. validar sintaxis y rango antes de aplicar
2. separar baseline de hardening y tuning específico por workload
3. versionar cambios y mantener evidencia de auditoría

---

## 6. Módulos de kernel: observación y diagnóstico

`/proc/modules` y `lsmod` muestran módulos cargados y uso.
No necesitas escribir módulos para auditar estado:
- qué módulos están activos
- cuáles están siendo usados
- dependencias cargadas

Esto sirve para troubleshooting y cumplimiento (qué capacidades kernel están presentes).

---

## 7. Diseño defensivo de runtime tipo "minicontainer"

Un runtime simplificado requiere pipeline claro:
1. **validate**: config, rutas, límites, políticas
2. **prepare**: rootfs, argumentos, contexto
3. **isolate**: namespaces + cgroups + seccomp + caps
4. **run**: ejecutar proceso objetivo
5. **cleanup**: liberar recursos incluso en fallo parcial

Si una etapa falla, debes abortar con mensaje accionable y limpieza consistente.

---

## 8. Configuración declarativa y validación temprana

Nunca apliques aislamiento con config no validada.
Checklist mínimo de validación:
- campos obligatorios presentes
- paths absolutos
- límites numéricos en rango seguro
- namespaces requeridos presentes
- perfil de seguridad referenciado correctamente

La validación temprana reduce fallos en runtime privilegiado.

---

## 9. Observabilidad y diagnóstico en bajo nivel

Herramientas de bajo nivel fallan por muchas razones (permiso, kernel version, mount state).
Por eso la salida debe incluir:
- etapa donde falló
- operación exacta
- recurso afectado
- código/errno asociado

Sin esta disciplina, debug de contención/aislamiento se vuelve lento y frágil.

---

## 10. Errores recurrentes en este bloque

1. confundir aislamiento con seguridad total
2. asumir que config textual ya es válida semánticamente
3. no contemplar valores especiales (`max`) en cgroups
4. aplicar seccomp sin estrategia de fallback/diagnóstico
5. no limpiar recursos en fallos intermedios

---

## 11. Estrategia de testing para kernel/lower-level

Para ejercicios del curso, conviene separar:
- tests de parseo/lógica con fixtures (portables y reproducibles)
- tests de integración real (requieren privilegios/entorno Linux específico)

En este bloque práctico, los resueltos se enfocan en la primera capa,
que es la base para luego integrar con el kernel real.

---

## 12. Checklist antes de dar por bueno un runtime bajo nivel

1. ¿La configuración se valida completa antes de aplicar cambios?
2. ¿Hay límites de CPU/memoria interpretados correctamente?
3. ¿La política seccomp es deny-by-default?
4. ¿Se registran fallos con contexto útil?
5. ¿Hay limpieza garantizada en cada salida de error?

---

## 13. Mapa de práctica del bloque

### Resueltos
- `e01_proc_modules_parser`: métricas básicas desde `/proc/modules`.
- `e02_lsmod_like_summary`: resumen operativo de tabla `lsmod`.
- `e03_namespace_flags_decoder`: interpretación de `CLONE_NEW*`.
- `e04_cgroupv2_cpu_parser`: límites CPU (`cpu.max`/`cpu.weight`).
- `e05_cgroupv2_memory_parser`: límites y uso de memoria en cgroup v2.
- `e06_seccomp_profile_linter`: validación base de perfil seccomp.
- `e07_sysctl_kv_parser`: parseo de `sysctl.conf` y señales de riesgo.
- `e08_proc_pid_ns_inspector`: inventario de namespaces por PID (mock).
- `e09_kernel_tuning_auditor`: auditoría de baseline kernel.
- `e10_minicontainer_config_validator`: validación de config declarativa.

### Complejos
- `c01_namespace_sandbox_launcher`: lanzador aislado por namespaces.
- `c02_cgroup_controller_manager`: gestor declarativo de límites cgroup.
- `c03_minicontainer_runtime_skeleton`: runtime integral con pipeline seguro.

---

## 14. Relación con operación real

Este bloque prepara tareas reales de plataforma:
- inspección rápida de estado kernel/namespace/cgroup
- validación de hardening antes de despliegue
- diagnóstico de límites de recursos en producción
- construcción de utilidades internas de aislamiento en C

Dominar esta capa te permite entender "cómo funciona Docker por dentro" sin magia,
y diseñar herramientas de runtime más confiables.
