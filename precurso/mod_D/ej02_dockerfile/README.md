# Ejercicio D.2 — Escribiendo un Dockerfile

## 🎯 Objetivo
Crear tu propia imagen de Docker usando un `Dockerfile`.

## 📚 Teoría Mínima
Un **Dockerfile** es una receta paso a paso para crear una Imagen.

Instrucciones comunes:
- `FROM imagen:etiqueta`: De qué imagen base partes. (Ej: `FROM alpine:latest`). Siempre es la primera línea.
- `RUN comando`: Ejecuta un comando *durante* la construcción de la imagen (Ej: instalar dependencias con `apt-get` o `apk`).
- `COPY local contenedor`: Copia un archivo de tu PC local hacia dentro de la imagen.
- `CMD ["ejecutable", "arg1"]`: Define el comando por defecto que correrá cuando alguien haga `docker run`.

Comandos de Docker:
- `docker build -t mi-imagen .` : Construye una imagen y le pone la etiqueta `-t` "mi-imagen". El punto `.` indica que el Dockerfile está en el directorio actual.

## 📝 Instrucciones

En la carpeta actual (ej02_dockerfile):

1. Crea un script ejecutable simple llamado `script.sh` en `src/` que imprima "Hola desde mi propia imagen!".
2. Crea un archivo llamado `Dockerfile` en esta misma carpeta.
3. El `Dockerfile` debe:
   - Partir de `alpine` (`FROM alpine:latest`). Alpine es un Linux muy ligero.
   - Usar `RUN` para instalar curl: `RUN apk add --no-cache curl`
   - Copiar tu script hacia dentro de la imagen: `COPY src/script.sh /script.sh`
   - Darle permisos: `RUN chmod +x /script.sh`
   - Ejecutar el script por defecto: `CMD ["/script.sh"]`
4. Construye tu imagen:
   `docker build -t mi-primera-imagen .`
5. Corre tu imagen:
   `docker run --rm mi-primera-imagen`

## ✅ Criterios de Éxito
- Ejecutar el comando del paso 5 debe imprimir "Hola desde mi propia imagen!".
- El flag `--rm` causó que el contenedor se autodestruyera limpiamente al terminar.
