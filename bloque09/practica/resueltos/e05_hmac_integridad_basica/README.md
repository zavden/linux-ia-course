# E05 — HMAC-SHA256 para integridad

## Objetivo
Calcular MAC autenticado para detectar alteraciones de mensajes.

## Qué hace
- Implementa HMAC-SHA256 en C puro.
- Calcula MAC de un mensaje con clave compartida.
- Verifica que al mutar el mensaje cambie el MAC.

## Ejecutar
```bash
make run
make test
```
