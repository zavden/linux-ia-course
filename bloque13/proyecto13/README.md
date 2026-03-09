# Proyecto 13 - MiniCloud Distribuido (Sockets Reales)

Implementacion distribuida de control plane con 5 servicios TCP.

## Servicios

- `minicloud-registry`: `HEALTH`, `RESOLVE <path>`, `SNAPSHOT`
- `minicloud-vault`: `HEALTH`, `GET <secret>`, `STATS`
- `minicloud-runner`: `HEALTH`, `RUN ...`, `STATS`
- `minicloud-monitor`: `HEALTH`, `EVENT ...`, `REPORT`
- `minicloud-gateway`: cliente-orquestador que coordina a los demas

## Ejecutar pruebas

```bash
cd bloque13/proyecto13
make test
```

## Notas

- Cada request usa conexion corta (un comando, una respuesta).
- El test de integracion levanta servicios en puertos locales temporales.
