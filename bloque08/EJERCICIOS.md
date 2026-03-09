# EJERCICIOS.md — Bloque 08 (Índice)

Este archivo es solo mapa de ejercicios.
No incluye enunciados largos ni solución embebida.

Práctica completa en `bloque08/practica/`.

---

## Resueltos (10)

## E01 — statvfs básico
- Objetivo: obtener métricas de capacidad y uso de un filesystem.
- Qué hace: calcula total/disponible/porcentaje usado para una ruta.
- Ruta: `bloque08/practica/resueltos/e01_statvfs_basico`

## E02 — parser de mounts y fstab
- Objetivo: parsear inventario de montajes activos y declarados.
- Qué hace: cuenta entradas válidas y extrae mountpoints iniciales.
- Ruta: `bloque08/practica/resueltos/e02_parser_mounts_fstab`

## E03 — xattr básico
- Objetivo: practicar set/get/list/remove de atributos extendidos.
- Qué hace: opera xattr en archivo temporal y usa fallback sidecar si aplica.
- Ruta: `bloque08/practica/resueltos/e03_xattr_basico`

## E04 — parser ACL textual
- Objetivo: entender estructura de ACLs en formato textual.
- Qué hace: cuenta entradas `user`, `group` y `default` desde muestra `getfacl`.
- Ruta: `bloque08/practica/resueltos/e04_acl_text_parser`

## E05 — df simplificado por rutas
- Objetivo: construir reporte de uso estilo `df` para múltiples paths.
- Qué hace: recorre rutas y muestra total/avail/used% por cada una.
- Ruta: `bloque08/practica/resueltos/e05_df_simplificado`

## E06 — watcher de filesystem (inotify + fallback)
- Objetivo: detectar creación de archivos por eventos o sondeo.
- Qué hace: usa inotify en Linux y polling portable como respaldo.
- Ruta: `bloque08/practica/resueltos/e06_watcher_inotify_fallback`

## E07 — parser de reporte LVM
- Objetivo: extraer métricas de volúmenes desde salida estructurada.
- Qué hace: cuenta LVs, detecta thin volumes y suma tamaño total.
- Ruta: `bloque08/practica/resueltos/e07_lvm_report_parser`

## E08 — parser de reporte de quotas
- Objetivo: identificar usuarios sobre límites soft/hard.
- Qué hace: procesa tabla de cuotas y genera conteos de riesgo.
- Ruta: `bloque08/practica/resueltos/e08_quota_report_parser`

## E09 — constructor de inventario de storage
- Objetivo: integrar fuentes heterogéneas en un resumen único.
- Qué hace: combina mounts + LVM + quotas en salida JSON compacta.
- Ruta: `bloque08/practica/resueltos/e09_storage_inventory_builder`

## E10 — mini auditor de almacenamiento
- Objetivo: evaluar salud global con reglas y umbrales.
- Qué hace: mezcla uso de FS y cuotas para emitir `OK/WARN/CRIT`.
- Ruta: `bloque08/practica/resueltos/e10_mini_storage_auditor`

---

## Complejos (3)

## C01 — minilvm TUI (skeleton)
- Objetivo: interfaz interactiva para inspección/gestión de LVM y mounts.
- Qué debe hacer: paneles, navegación, acciones seguras y auditoría.
- Ruta: `bloque08/practica/complejos/c01_minilvm_tui_skeleton`

## C02 — storage health daemon
- Objetivo: monitoreo continuo de capacidad, cuotas y eventos de filesystem.
- Qué debe hacer: collectors + reglas + logging estructurado.
- Ruta: `bloque08/practica/complejos/c02_storage_health_daemon`

## C03 — snapshot + quota manager
- Objetivo: aplicar políticas de retención y límites de almacenamiento.
- Qué debe hacer: planificar snapshots, ajustar cuotas y reportar cumplimiento.
- Ruta: `bloque08/practica/complejos/c03_snapshot_quota_manager`

---

## Notas

1. Cada ejercicio incluye `README.md`, `src/`, `tests/`, `Makefile`.
2. Los resueltos incluyen comentarios explicativos en el código.
3. Los complejos son plantillas guiadas para implementación propia.
