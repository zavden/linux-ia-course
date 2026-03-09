# E10 — minitop mini

## Objetivo
Integrar parser de memoria + escaneo de PIDs estilo `/proc`.

## Qué hace
- Lee un `meminfo` desde ruta configurable.
- Escanea directorio tipo `proc_root` con subcarpetas numéricas.
- Imprime tabla simple de procesos desde `status` de cada PID.

## Ejecutar
```bash
make run
make test
```
