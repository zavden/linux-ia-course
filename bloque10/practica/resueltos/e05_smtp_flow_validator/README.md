# E05 - validador de flujo SMTP

## Objetivo
Comprobar que una sesion SMTP siga orden correcto de comandos.

## Que hace
- Procesa script de comandos cliente SMTP.
- Verifica secuencia `EHLO/HELO -> MAIL -> RCPT+ -> DATA -> . -> QUIT`.
- Reporta validez, numero de destinatarios y lineas de cuerpo.

## Ejecutar
```bash
make run
make test
```
