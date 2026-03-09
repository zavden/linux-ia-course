# C03 - miniproxy reverse router (skeleton)

## Objetivo
Desarrollar el nucleo de un reverse proxy con health checks y balanceo simple.

## Que debe hacer
- Cargar configuracion de backends/rutas y validarla.
- Resolver ruta por prefijo y seleccionar backend saludable.
- Ejecutar health checks periodicos con degradacion controlada.
- Exportar estado de salud y decisiones de ruteo para observabilidad.

## Pistas
- Implementa estrategia de seleccion (round-robin o least-latency) desacoplada.
- Mantén cache de config + recarga segura sin perder solicitudes en curso.
- Diseña tests para fallos de backend, timeouts y cambios de configuracion.
