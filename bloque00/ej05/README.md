# Ejercicio 0.5 — Variables de Entorno y Configuración

## 🎯 Objetivo

Aprender a configurar aplicaciones leyendo **Variables de Entorno** desde C, una práctica estándar en la metodología *Twelve-Factor App* (muy usada en Docker y Kubernetes).

## 📚 Teoría Mínima

### ¿Qué es una variable de entorno?

Son pares clave-valor dinámicos disponibles para un proceso en ejecución.
Ejemplos en Linux: `PATH`, `USER`, `HOME`.

En Docker, inyectamos variables con el flag `-e`:
```bash
docker run -e MY_VAR="valor" mi-imagen
```
O en Docker Compose `environment:`:
```yaml
services:
  app:
    environment:
      - MY_VAR=valor
```

### Leerlas desde C

C estándar ofrece `getenv()` en `<stdlib.h>`:

```c
#include <stdlib.h>
#include <stdio.h>

int main() {
    char *user = getenv("USER");
    if (user != NULL) {
        printf("Usuario: %s\n", user);
    } else {
        printf("Usuario desconocido.\n");
    }
    return 0;
}
```

> [!WARNING]
> Nunca modifiques la cadena devuelta por `getenv()`. Es un puntero al entorno del proceso y modificarla causa comportamiento indefinido. Guarda una copia con `strdup()` si necesitas alterarla.

### Patrón de Fallback (Default values)

Es una buena práctica tener un valor por defecto si la variable no está configurada.

```c
const char *port_str = getenv("PORT");
int port = 8080; // Valor por defecto

if (port_str) {
    port = atoi(port_str); // Convertir string a entero
}
```

## 📝 Instrucciones

1. **Implementa `src/main.c`:**
   Crea un programa que lea dos variables de entorno:
   - `APP_PORT` (entero): si no existe, por defecto es `8080`.
   - `APP_ENV` (cadena): si no existe, por defecto es `"development"`.

2. **Requisitos de salida (stdout):**
   El programa debe imprimir estas líneas exactamente:
   ```
   Entorno actual: <valor de APP_ENV>
   Escuchando en puerto: <valor de APP_PORT>
   ```

3. **Pruebas en Docker:**
   Modifica tu forma de ejecutar el contenedor para pasar variables, por ejemplo:
   ```bash
   docker run --rm -e APP_PORT=9090 -e APP_ENV=production ej05-fedora
   ```

## ✅ Criterios de Éxito

- [ ] Sin variables de entorno, imprime "development" y "8080".
- [ ] Inyectando variables con `docker run -e`, el programa las respeta y las convierte adecuadamente.
- [ ] El script automatizado `make test` pasa todas las validaciones.

## 📖 Referencias

- `man getenv`
- `man atoi`
- [The Twelve-Factor App - Config](https://12factor.net/config)
