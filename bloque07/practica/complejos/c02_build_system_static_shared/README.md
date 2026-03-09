# C02 — build system estática + compartida

## Objetivo
Diseñar un sistema de build que genere binario principal, librería estática y librería compartida.

## Qué debe hacer
- Generar `libX.a` y `libX.so`/`libX.dylib` según plataforma.
- Linkear un ejecutable contra una de las variantes.
- Exponer objetivos `all`, `clean`, `debug`, `release`, `test`.
- Documentar diferencias de carga dinámica por SO.

## Pistas
- Usa variables de Make para evitar duplicación.
- Separa compilación de objetos PIC para shared library.
- Agrega validación con `nm`/`otool`/`ldd` según entorno.
