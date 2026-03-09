# Proyecto Final - MiniCloud (MVP Integrado)

Este proyecto final conecta cinco componentes:
- `minicloud-registry`
- `minicloud-gateway`
- `minicloud-monitor`
- `minicloud-vault`
- `minicloud-runner`

Cada servicio es una utilidad en C enfocada al plano de control.
La integración se prueba con fixtures locales y scripts reproducibles.

## Estructura

- `minicloud-*/src/main.c`: lógica principal de cada componente.
- `minicloud-*/tests/`: pruebas unitarias por servicio.
- `tests/test_stack.sh`: prueba de integración del stack.

## Flujo recomendado

```bash
# Desde bloque12/proyecto-final
make test
```

## Estado del MVP

- Contratos de servicio validados (`registry`).
- Plan de rutas con backend health (`gateway`).
- Métricas/eventos y severidad operativa (`monitor`).
- Manifiesto de secretos y lookup de referencias (`vault`).
- Validación de jobs y planificación por capacidad (`runner`).
- Reporte final de readiness de integración (`tests/test_stack.sh`).
