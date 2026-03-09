# Ejercicio 0.3 — Docker Compose Multi-Distro

## 🎯 Objetivo

Orquestar entornos multi-distro usando **Docker Compose**. Lograr que un solo comando compile y ejecute tu código fuente en **Fedora** y **Debian** simultáneamente.

## 📚 Teoría Mínima

### ¿Por qué Docker Compose?

Hasta ahora hemos usado `docker build` y `docker run` manualmente para cada distro. Esto es tedioso. `docker-compose` permite definir una infraestructura entera en un archivo YAML y levantarla con un comando.

### Tu primer `docker-compose.yml`

```yaml
services:
  fedora:
    build:
      context: .
      dockerfile: Dockerfile.fedora
    volumes:
      - ./src:/app/src:ro    # Montar tu código local dentro del contenedor

  debian:
    build:
      context: .
      dockerfile: Dockerfile.debian
    volumes:
      - ./src:/app/src:ro
```

**Beneficios clave del montaje (volumes):**
1. Escribes código en tu host con tu editor favorito.
2. Al ejecutar `docker compose up --build`, ambos contendores compilan la versión más reciente de tus archivos `src/`.
3. No hay que copiar archivos manualmente en cada test.

## 📝 Instrucciones

1. **Escribe `src/main.c`:**
   Crea un programa que imprima en qué compilador se está ejecutando usando la macro `__VERSION__`.
   Ejemplo: `[Multi-Distro Test] Compilado con GCC <version>`

2. **Personaliza `docker-compose.yml`:**
   Edita el archivo `docker-compose.yml` de este directorio para que defina los dos servicios requeridos: `fedora` y `debian`.
   Utiliza los Dockerfiles ya presentes. Modifica los containers para que monten `./src` local en el `/app/src` del contenedor en modo Read-Only (`ro`).

3. **Ejecuta Docker Compose:**
   ```bash
   docker compose up --build
   ```

4. **Analiza el output:**
   Notarás que las versiones de GCC pueden diferir entre Fedora (más reciente) y Debian (más estable). Esta es la razón de probar en ambas.

## ✅ Criterios de Éxito

- [ ] `docker compose up --build` funciona y no muestra errores.
- [ ] El log en pantalla intercala mensajes de `fedora-1` y `debian-1`.
- [ ] El programa base se compila exitosamente en ambos bajo C17.

## 💡 Pistas

<details>
<summary>Pista 1</summary>

El template base provee un `docker-compose.yml` que ya hace casi todo. Asegúrate de entender qué hace la directiva `volumes`.
</details>

## 📖 Referencias

- [Docker Compose Overview](https://docs.docker.com/compose/)
