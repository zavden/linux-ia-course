# Ejercicio D.4 — Docker Compose Básico

## 🎯 Objetivo
Hacer el salto del largo comando `docker run` al limpio y corto `docker-compose.yml`, que es el estándar de este curso completo.

## 📚 Teoría Mínima
Docker Compose es una herramienta oficial que lee configuraciones en un archivo `.yml` y lanza contenedores y redes usando esa receta en lugar de tener que recordar Flags horribles.

Un archivo `docker-compose.yml` equivalente a D.3:

```yaml
services:
  compilador:
    image: gcc:latest
    volumes:
      - ./src:/usr/src/miapp
    working_dir: /usr/src/miapp
    command: ["gcc", "-o", "hola", "hola.c"]
```

Comandos:
- `docker compose up`: Lanza todos los servicios. Vuelve a ejecutarlo si cambias los archivos (con `--build` si cambias el Dockerfile).

## 📝 Instrucciones

1. Crea exactamente el archivo `docker-compose.yml` descrito arriba en este directorio `ej04_compose`.
2. Como acabas de ver arriba, esta receta asume que dentro de una carpeta `src` hay un archivo llamado `hola.c`. Crea ambos y escribe un código C trivial (puedes copiar el de `ej03_volumenes`).
3. Ejecuta en tu terminal el orquestador: `docker compose up`
4. De igual forma, cuando termine, se habrá creado el ejecutable `./src/hola` pero esta vez no tuviste que memorizar ningún comando críptico para montarlo. 

## ✅ Criterios de Éxito
- Ejecutar `docker compose up` en este directorio debe compilar de vuelta el código `hola.c` y soltar el binario localmente sin mostrar errores en pantalla.
