# E09 - auditor de politicas minicloud

## Objetivo
Evaluar reglas de seguridad y operacion sobre configuracion consolidada.

## Que hace
- Lee archivo de politicas `key=value`.
- Verifica checks obligatorios de hardening y resiliencia.
- Emite severidad final `OK/WARN/CRIT`.

## Ejecutar
```bash
make run
make test
```
