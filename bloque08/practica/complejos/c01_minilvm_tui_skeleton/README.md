# C01 — minilvm TUI (skeleton)

## Objetivo
Construir una interfaz de terminal para inspección/gestión de storage tipo LVM.

## Qué debe hacer
- Mostrar inventario: dispositivos, VGs, LVs, mounts y uso.
- Navegación por teclado con paneles (ncurses o ANSI).
- Acciones guiadas: create/extend/remove LV (modo dry-run primero).
- Registro de operaciones con confirmación de seguridad.

## Pistas
- Empieza con modo solo lectura y parser de reportes `pvs/vgs/lvs`.
- Implementa capa de comandos separada de la UI.
- Añade modo “simulación” para probar sin privilegios.
