# C01 - pipeline registry/gateway/monitor (skeleton)

## Objetivo
Diseñar flujo de control entre discovery, routing y observabilidad.

## Que debe hacer
- Consumir snapshot del registry y construir tabla de rutas activa.
- Propagar cambios al gateway sin interrumpir trafico en curso.
- Publicar metricas para monitor con etiquetas por servicio/ruta.
- Implementar fallback cuando registry este degradado.

## Pistas
- Separa plano de control (config) y plano de datos (requests).
- Define formato de snapshot versionado y valida compatibilidad.
- Agrega estrategia de rollback ante update invalido.
