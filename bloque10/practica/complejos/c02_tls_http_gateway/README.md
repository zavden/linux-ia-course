# C02 - TLS HTTP gateway (skeleton)

## Objetivo
Construir un gateway HTTPS que termine TLS y enrute trafico HTTP a backends internos.

## Que debe hacer
- Listener TLS con certificados configurables y recarga controlada.
- Parseo HTTP robusto y forwarding a backend segun reglas.
- Timeouts, limites y sanitizacion de cabeceras.
- Logs estructurados con metrica de latencia y codigos de estado.

## Pistas
- Separa plano de datos (forwarding) y plano de control (config/health).
- Disena manejo de errores para handshake, read parcial y backend caido.
- Incluye pruebas de regresion de seguridad para cabeceras y rutas.
