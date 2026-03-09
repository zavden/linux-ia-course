# EJERCICIOS.md - Bloque 14 (Indice)

Este bloque se trabaja como proyecto distribuido HTTP integrado.

Ruta principal: `bloque14/proyecto14/`.

## Componentes

- `minicloud-registry`: discovery por endpoint HTTP.
- `minicloud-vault`: secretos por endpoint HTTP.
- `minicloud-runner`: admision de jobs por endpoint HTTP.
- `minicloud-monitor`: eventos y reporte HTTP.
- `minicloud-gateway`: orquestacion API entre servicios.

## Validacion

- tests por servicio en `minicloud-*/tests/`
- integracion completa en `proyecto14/tests/test_stack.sh`
