# C01 — shell con job control (guiado)

## Objetivo
Implementar control de jobs foreground/background en un mini-shell.

## Qué debe hacer
- Soportar `&` para background.
- Recolectar hijos con `SIGCHLD + waitpid(..., WNOHANG)`.
- Mantener lista básica de jobs.
- No bloquear shell al lanzar background.

## Estado
Plantilla guiada (sin resolver completamente).
