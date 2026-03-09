# C03 - minicloud control plane (skeleton)

## Objetivo
Definir esqueleto de control plane para orquestar componentes del proyecto final.

## Que debe hacer
- Validar configuracion global y dependencias entre servicios.
- Coordinar lifecycle (start, reload, stop) por componente.
- Evaluar readiness global antes de aceptar trafico.
- Exportar reporte de estado y drift de configuracion.

## Pistas
- Usa arquitectura event-driven para desacoplar componentes.
- Mantiene contrato de eventos estable y versionado.
- Diseña estrategia de reconciliacion idempotente.
