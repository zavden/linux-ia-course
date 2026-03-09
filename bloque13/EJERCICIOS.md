# EJERCICIOS.md - Bloque 13 (Indice)

Este bloque se enfoca en un proyecto distribuido real por sockets.
No hay lista de ejercicios sueltos: la practica esta integrada en `proyecto13`.

Ruta principal: `bloque13/proyecto13/`.

## Componentes

- `minicloud-registry`: discovery y resolucion de rutas.
- `minicloud-gateway`: orquestacion de request entre servicios.
- `minicloud-monitor`: eventos y reporte de salud.
- `minicloud-vault`: lookup de secretos por nombre.
- `minicloud-runner`: admision de jobs por capacidad.

## Validacion

- Tests por servicio en `minicloud-*/tests/`.
- Integracion en `proyecto13/tests/test_stack.sh`.
