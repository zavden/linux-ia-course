# E01 - parser de contratos de servicio

## Objetivo
Validar contratos basicos de microservicios antes de integrarlos en el plano de control.

## Que hace
- Parsea lineas `service|port|proto|health_path`.
- Valida formato de nombre, puerto, protocolo y health path.
- Cuenta contratos validos e invalidos.

## Ejecutar
```bash
make run
make test
```
