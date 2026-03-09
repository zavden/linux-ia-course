# C02 - cgroup controller manager (skeleton)

## Objetivo
Implementar un gestor de límites cgroup v2 para procesos y grupos de trabajo.

## Qué debe hacer
- Crear jerarquías cgroup y asignar procesos.
- Aplicar límites de CPU y memoria con validación de entradas.
- Exponer estado actual y detectar violaciones de política.
- Soportar actualización dinámica de límites.

## Pistas
- Usa capa de I/O para archivos cgroup desacoplada de la lógica de políticas.
- Diseña modelo declarativo (`desired state`) y reconciliación.
- Prioriza mensajes de error accionables por recurso.
