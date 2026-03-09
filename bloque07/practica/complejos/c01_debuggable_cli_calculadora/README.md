# C01 — calculadora CLI depurable

## Objetivo
Crear una calculadora modular lista para depurar con `gdb`, con manejo robusto de errores.

## Qué debe hacer
- Parser de expresiones simples (`sum`, `sub`, `mul`, `div`).
- Separación clara por módulos (`parser`, `ops`, `cli`).
- Salidas de error con contexto (`errno` + mensaje humano).
- Objetivo `debug` en Makefile con símbolos (`-g`) y sanitizers opcionales.

## Pistas
- Añade modo verbose por flag (`--trace`).
- Incluye pruebas de caja negra para entradas inválidas.
- Mantén funciones pequeñas para facilitar `step` en `gdb`.
