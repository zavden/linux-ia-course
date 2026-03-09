# Ejercicio A.1 — Navegación y Archivos

## 🎯 Objetivo
Aprender a moverte por el sistema de ficheros, crear directorios y archivos, y usar comodines.

## 📚 Teoría Mínima
- `pwd`: Print Working Directory (¿Dónde estoy?)
- `cd <dir>`: Change Directory. `cd ..` sube un nivel. `cd ~` va a tu home.
- `ls -la`: Listar todo (incluidos ficheros ocultos que empiezan por `.`) con detalles.
- `mkdir -p a/b/c`: Crear ruta de directorios completa si no existe.
- `touch archivo.txt`: Crear un archivo vacío (o actualizar su fecha si ya existe).
- `cp origen destino`: Copiar. Usa `-r` para carpetas.
- `mv origen destino`: Mover o renombrar.
- `rm archivo`: Borrar. `-r` para carpetas, `-f` para forzar. Comodines: `*.txt` aplica a todos los txt.

## 📝 Instrucciones

Crea un script de bash llamado `src/ejecutar.sh` que haga exactamente lo siguiente:
1. Cree una estructura de carpetas `proyecto/src` y `proyecto/docs`.
2. Dentro de `proyecto/src`, cree 3 archivos de código vacío: `main.c`, `utils.c`, `math.c`.
3. Dentro de `proyecto/docs`, cree un archivo `readme.txt` y `changelog.txt`.
4. Mueva todos los archivos `.txt` desde `proyecto/docs` hacia la raíz de `proyecto/`.

Asegúrate de darle permisos de ejecución a tu script: `chmod +x src/ejecutar.sh`.

## ✅ Criterios de Éxito
- Al correr tu script, la carpeta `proyecto` debe tener la estructura exacta solicitada.
- El test de validación pasará cuando ejecutes `make test` (usaremos un script para validar en lugar de Make por ahora, ejecuta `./tests/test.sh`).

## 💡 Pistas
No cambies de directorio en el script con `cd`. Usa rutas relativas (ej. `mkdir -p proyecto/src`).
