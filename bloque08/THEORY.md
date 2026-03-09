# THEORY.md — Bloque 08: Almacenamiento Avanzado en Linux/C

Este bloque conecta programación en C con operación real de storage.
La meta es construir herramientas que **observen, auditen y gestionen capacidad** sin depender solo de comandos manuales.

---

## 1. Capacidad y uso de filesystem

### 1.1 `statvfs`
`statvfs(path, &st)` entrega métricas del filesystem asociado a una ruta.
Campos clave:
- `f_blocks`: bloques totales
- `f_bfree`: bloques libres totales
- `f_bavail`: bloques disponibles para usuario no privilegiado
- `f_frsize`: tamaño real de bloque para cálculos

Cálculo típico:
- `total_bytes = f_blocks * f_frsize`
- `avail_bytes = f_bavail * f_frsize`
- `used = total - avail`

### 1.2 Diferencia entre `bfree` y `bavail`
`bfree` incluye bloques reservados para root.
`bavail` refleja lo realmente utilizable por procesos normales.
Para alertas operativas suele ser mejor basarse en `bavail`.

---

## 2. Inventario de montajes

Fuentes frecuentes:
- `/proc/mounts` (estado actual)
- `/etc/fstab` (configuración declarada)

Buenas prácticas de parser:
1. ignorar comentarios y líneas vacías
2. validar mínimo de columnas
3. no asumir orden/espacios rígidos
4. manejar rutas con escapes cuando aplique

Objetivo operativo: detectar discrepancias entre lo configurado y lo realmente montado.

---

## 3. Extended Attributes (xattr)

xattrs son pares clave/valor asociados a archivos/directorios.
Casos de uso:
- metadatos de clasificación
- marcas de backup
- integración con políticas de seguridad

Operaciones base:
- `setxattr`
- `getxattr`
- `listxattr`
- `removexattr`

Notas:
- no todos los filesystems soportan xattr
- namespaces dependen de plataforma (`user.*`, `security.*`, etc.)
- la API difiere levemente entre Linux y macOS

---

## 4. ACLs más allá de permisos rwx

Permisos tradicionales (`ugo/rwx`) son limitados.
ACL permite reglas por usuario/grupo específico, incluyendo defaults en directorios.

Conceptos útiles al parsear salidas tipo `getfacl`:
- `user::`, `group::`, `other::`
- entradas nominales: `user:alice:rw-`
- `mask::`
- `default:*` para herencia en directorios

Aunque se gestione vía herramientas externas, saber parsearlo en C habilita auditorías automáticas.

---

## 5. Monitoreo de cambios en filesystem

### 5.1 `inotify` (Linux)
Permite suscribirse a eventos como creación, borrado o modificación.
Flujo base:
1. `inotify_init1`
2. `inotify_add_watch`
3. `read` de eventos
4. `inotify_rm_watch`

### 5.2 Fallback portable
Fuera de Linux puedes usar polling (`stat/access`) con intervalos cortos.
No es tan eficiente, pero mantiene portabilidad para tests y tooling básico.

---

## 6. LVM: qué mirar desde software

LVM organiza almacenamiento en capas:
- PV (Physical Volume)
- VG (Volume Group)
- LV (Logical Volume)

En herramientas C de auditoría, muchas veces basta parsear salidas estructuradas de `pvs/vgs/lvs`.
Métricas típicas:
- cantidad de LVs
- tamaño total asignado
- presencia de thin pools/snapshots

Para operaciones destructivas, siempre incluir:
- modo dry-run
- confirmación explícita
- log de auditoría

---

## 7. Quotas: límites por usuario/proyecto

Una cuota combina:
- uso actual
- soft limit (umbral)
- hard limit (límite estricto)

Reglas operativas comunes:
- `usage > soft` => warning
- `usage > hard` => critical

Desde C, si no dispones de APIs/kernel features en entorno de pruebas, parsear reportes también es válido para lógica de negocio y alertas.

---

## 8. Diseño de auditor de storage

Un auditor robusto suele tener 3 capas:
1. **Collectors**: capacidad, mounts, LVM, cuotas, eventos
2. **Rules**: umbrales y políticas
3. **Reporter**: salida legible/JSON + estado final

Estados sugeridos:
- `OK`
- `WARN`
- `CRIT`

Principios:
- idempotencia
- tolerancia a fuentes ausentes
- salida estable para automatización

---

## 9. Riesgos y errores comunes

1. asumir que todos los paths pertenecen al mismo filesystem
2. usar `f_bfree` en vez de `f_bavail` para alertas de usuario
3. tratar ausencia de xattr como bug lógico (a veces es limitación del FS)
4. parsear salidas con formatos frágiles sin validación
5. ejecutar acciones de LVM sin modo simulación
6. no registrar contexto al reportar errores de I/O

---

## 10. Checklist de implementación

Antes de cerrar una herramienta de storage:
1. ¿Maneja errores de lectura/permiso con contexto claro?
2. ¿Tolera plataformas/FS sin features avanzadas?
3. ¿Tiene tests con fixtures (no solo host real)?
4. ¿Separa recolección de reglas de evaluación?
5. ¿Incluye modo seguro/no destructivo para operaciones sensibles?

---

## 11. Mapa de práctica del bloque

### Resueltos
- `e01_statvfs_basico`: capacidad y porcentaje usado.
- `e02_parser_mounts_fstab`: inventario de montajes configurados/activos.
- `e03_xattr_basico`: ciclo completo de atributos extendidos.
- `e04_acl_text_parser`: parseo de ACL textual.
- `e05_df_simplificado`: vista de uso por rutas.
- `e06_watcher_inotify_fallback`: eventos de filesystem con fallback.
- `e07_lvm_report_parser`: métricas desde reportes LVM.
- `e08_quota_report_parser`: detección de usuarios sobre cuota.
- `e09_storage_inventory_builder`: agregador de inventario.
- `e10_mini_storage_auditor`: evaluación `OK/WARN/CRIT`.

### Complejos
- `c01_minilvm_tui_skeleton`: interfaz de gestión de storage.
- `c02_storage_health_daemon`: monitoreo continuo + alertas.
- `c03_snapshot_quota_manager`: políticas de snapshot y cuotas.

---

## 12. Relación con operación real

Este bloque prepara exactamente lo necesario para:
- troubleshooting de espacio en producción
- auditorías de configuración (`fstab` vs mounts)
- alertas preventivas por cuotas/capacidad
- herramientas internas de SRE/infra escritas en C

La combinación de parsers robustos + métricas confiables + reglas claras es el núcleo de cualquier plataforma de almacenamiento bien operada.
