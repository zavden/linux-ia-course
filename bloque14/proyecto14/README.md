# Proyecto 14 - MiniCloud HTTP/1.1

Version del stack distribuido usando HTTP/1.1 minimo real entre servicios.

## Servicios

- `registry`: `GET /health`, `GET /resolve?path=...`, `GET /snapshot`
- `vault`: `GET /health`, `GET /secret/<name>`, `GET /stats`
- `runner`: `GET /health`, `GET /run?...`, `GET /stats`
- `monitor`: `GET /health`, `GET /event?...`, `GET /report`
- `gateway`: cliente-orquestador del flujo distribuido

## Ejecutar validacion

```bash
cd bloque14/proyecto14
make test
```
