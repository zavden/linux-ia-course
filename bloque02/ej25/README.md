# Ejercicio 2.5 — Profundización en Inodos: Enlaces Duros y Simbólicos

## 🎯 Objetivo
Entender mediante C la diferencia entre un enlace duro (`link`) y un enlace simbólico (`symlink`), y comprender la magia de la desvinculación (`unlink`).

## 📚 Teoría Mínima
1. **Inodo (Inode):** Es el verdadero "archivo" en el disco duro. Contiene los datos, propietario y fecha. ¡El inodo NO contiene nombre!
2. **Entrada de Directorio (Hard Link):** Es simplemente un string (nombre) que apunta a un Número de Inodo. 
   - Puedes tener 50 "nombres" diferentes en tu disco apuntando al MISMO Inodo. 
   - El Inodo mantiene un contador (`st_nlink`). Si borras un archivo, solo destruyes *un nombre*. El inodo (y tus datos) *solo* se borra si el contador llega a `0`.
3. **Enlace Simbólico (Symlink):** Es un archivo real, distinto, con su Inodo propio, que por dentro solo guarda un string en texto con la ruta hacia de otro archivo. Si el objetivo se borra, el symlink queda "roto" apuntando a la nada.

**Syscalls:**
- `link(old, new)`: Crea un Hard Link equivalente a `ln target linkname`.
- `symlink(target, linkpath)`: Crea un enlace simbólico equivalente a `ln -s target linkpath`.
- `readlink(path, buf, size)`: Si le das un symlink, te escupe el texto de hacia dónde apunta.
- `unlink(path)`: Borra una entrada de directorio (un nombre).

## 📝 Instrucciones

Construye un programa en `src/main.c` que reciba 3 argumentos:
1. El archivo original a crear.
2. El nombre para su futuro hard link.
3. El nombre para su futuro symlink.

El programa debe:
1. Crear el primer archivo (o abrirlo). Escribirle `"Hola Mundo"`.
2. Usar `link()` para atarlo al hard link.
3. Usar `symlink()` para atarlo al enlace simbólico.
4. Llamar a `stat()` o `lstat()` y demostrar imprimiendo en pantalla que el archivo original y el hard link comparten **exactamente el mismo número de inodo** (`st_ino`), pero el symlink tiene un Inodo distinto.
5. Luego, `unlink()` el archivo original. Esto borra la entrada "A".
6. Leer de nuevo el hard link y demostrar que la data ("Hola Mundo") SIGUE VIVA en el Inodo y cuenta `st_nlink == 1`.
7. Usar `readlink()` en el symlink e imprimir hacia dónde apunta, a pesar de que eso (el archivo original) ya haya sido "borrado".

## ✅ Criterios de Éxito
- Has probado empíricamente cómo Linux maneja el conteo de referencias al borrar archivos, implementando un programa que orquesta todo el ciclo en C.
