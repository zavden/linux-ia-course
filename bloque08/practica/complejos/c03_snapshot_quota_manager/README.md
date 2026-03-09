# C03 — snapshot + quota manager

## Objetivo
Diseñar herramienta que coordine snapshots y políticas de cuota por usuario/proyecto.

## Qué debe hacer
- Crear política declarativa (retención, tamaño máximo, frecuencia).
- Simular/ejecutar snapshots y limpieza por antigüedad.
- Ajustar quotas en función de políticas y uso observado.
- Generar reporte final de cumplimiento.

## Pistas
- Empieza con backend “simulado” sobre archivos de estado.
- Implementa planificación temporal con tolerancia a fallos parciales.
- Asegura idempotencia para reintentos seguros.
