# Ejercicio D.1 — El Mundo Efímero

## 🎯 Objetivo
Comprender la diferencia entre una **Imagen** y un **Contenedor**, y por qué los contenedores son efímeros por defecto.

## 📚 Teoría Mínima
- **Imagen:** Una plantilla inmutable (de solo lectura) que contiene un sistema base y aplicaciones preinstaladas. (Ejemplo: `ubuntu`, `fedora`). Se construyen con Dockerfiles.
- **Contenedor:** Una instancia *en ejecución* de una imagen. Tiene una capa superior de lectura/escritura muy ligera.
- **Efímero:** Si borras un contenedor, pierdes todos los cambios que hayas hecho dentro de él, a menos que uses Volúmenes.

Comandos Clave:
- `docker run -it ubuntu /bin/bash`: Descarga la imagen `ubuntu` (si no la tienes), crea un contenedor, inicia una terminal interactiva `-it`, y ejecuta `/bin/bash`.
- `docker ps`: Lista los contenedores en ejecución.
- `docker ps -a`: Lista también los detenidos.

## 📝 Instrucciones

Sigue estos pasos en tu terminal (este ejercicio no tiene script automatizado para crear):

1. Arranca un contenedor interactivo de debian:
   `docker run -it debian /bin/bash`
2. Una vez dentro del contenedor (el prompt cambiará), crea un archivo:
   `echo "Hola desde el interior" > /secreto.txt`
3. Verifica que existe:
   `cat /secreto.txt`
4. Sal del contenedor escribiendo `exit`. El contenedor se detendrá.
5. Inicia un *nuevo* contenedor de debian:
   `docker run -it debian /bin/bash`
6. Intenta leer el archivo:
   `cat /secreto.txt`
   *(Verás que te dice "No such file or directory". ¿Por qué? Porque es un contenedor totalmente nuevo creado desde la imagen base limpia).*

## ✅ Criterios de Éxito
- Entiendes por qué el archivo desapareció en el paso 6.
- Comprendes que todo lo generado dentro del contenedor (binarios, logs) se pierde si no se toman medidas extra (las cuales veremos en D.3).
