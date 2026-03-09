# Ejercicio A.3 — Redirecciones y Pipes

## 🎯 Objetivo
Aprender a conectar comandos encadenando sus salidas (pipes) y redirigir salidas a archivos (redirection).

## 📚 Teoría Mínima
- `>`  : Redirige salida y sobreescribe el archivo. (`echo "hola" > a.txt`)
- `>>` : Redirige salida al final del archivo (append). (`echo "mundo" >> a.txt`)
- `<`  : Redirige un archivo hacia la entrada del comando.
- `2>` : Redirige los errores (stderr). (`2> /dev/null` = ocultar errores).
- `|`  : **Pipe**. Pasa el output del comando 1 como input al comando 2. (`ls | grep txt`).
- `grep "palabra"`: Filtra las líneas que contienen 'palabra'.
- `wc -l`: Cuenta las líneas que le llegan por input.

## 📝 Instrucciones

Imagina que tienes un archivo pesado de logs. He dejado un script falso `generar_logs.sh` en la carpeta `src` que imprime líneas simulando un log al usarlo: `./generar_logs.sh`.

Crea un script `src/filtrar.sh` que haga lo siguiente en una sola línea conectando pipes y redirecciones:
1. Ejecute `./generar_logs.sh`.
2. Filtre **únicamente** las líneas que contengan la palabra `ERROR`.
3. Guarde el resultado filtrado en un archivo `src/errores.log` **sobrescribiéndolo**.

## ✅ Criterios de Éxito
- Al correr `src/filtrar.sh`, debe aparecer un `src/errores.log` que contenga solo las líneas con `ERROR`.
- `wc -l src/errores.log` debería dar 3.
