# E03 - linter de politica TLS

## Objetivo
Validar que una configuracion TLS cumpla baseline minimo de seguridad.

## Que hace
- Lee archivo `key=value` de politica TLS.
- Rechaza protocolos obsoletos y ciphers inseguros.
- Verifica tamano minimo de clave RSA.

## Ejecutar
```bash
make run
make test
```
