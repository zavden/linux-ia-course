# E06 - parser de respuestas SMTP multilinea

## Objetivo
Interpretar respuestas del servidor SMTP con formato `code-` y `code `.

## Que hace
- Lee transcript de respuestas SMTP.
- Detecta codigo de saludo y codigo final de la transaccion.
- Cuenta capacidades anunciadas y presencia de `STARTTLS`.

## Ejecutar
```bash
make run
make test
```
