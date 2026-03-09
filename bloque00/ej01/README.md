# Ejercicio 0.1 — Hello Docker

## 🎯 Objetivo

Compilar y ejecutar tu primer programa en C dentro de contenedores Docker, verificando que funcione idénticamente en **Fedora** y **Debian**.

## 📚 Teoría Mínima

### Docker para desarrollo en C

Docker te permite crear **entornos aislados y reproducibles**. En este curso, cada ejercicio corre dentro de contenedores para:

1. **No ensuciar tu sistema** — instalar/desinstalar herramientas sin riesgo.
2. **Garantizar reproducibilidad** — el mismo Dockerfile produce el mismo entorno siempre.
3. **Testear en múltiples distros** — compilar en Fedora y Debian con un solo comando.

### Anatomía de un Dockerfile

```dockerfile
FROM fedora:latest          # Imagen base
RUN dnf install -y gcc      # Instalar dependencias
WORKDIR /app                # Directorio de trabajo
COPY src/ ./src/            # Copiar código fuente
RUN gcc -o hello src/main.c # Compilar
CMD ["./hello"]             # Ejecutar al hacer 'docker run'
```

### Detectar la distribución

El archivo `/etc/os-release` existe en todas las distros modernas y contiene variables como:

```
ID=fedora            # o "debian"
VERSION_ID=41        # versión
PRETTY_NAME="Fedora Linux 41"
```

Desde C puedes leerlo con `fopen()` y `fgets()`, o usar `getenv()` si defines variables en el Dockerfile.

## 📝 Instrucciones

1. **Escribe `src/main.c`** — Un programa que:
   - Abra y parsee `/etc/os-release`.
   - Extraiga el valor de `PRETTY_NAME`.
   - Imprima: `Hello from <PRETTY_NAME>`.
   - Si no puede abrir el archivo, imprima un error con `perror()` y retorne `EXIT_FAILURE`.

2. **Personaliza los Dockerfiles** — Edita `Dockerfile.fedora` y `Dockerfile.debian` para que:
   - Instalen `gcc` y `make`.
   - Copien tu código y lo compilen.
   - Ejecuten el binario al hacer `docker run`.

3. **Compila y ejecuta en ambas distros:**
   ```bash
   docker build -f Dockerfile.fedora -t ej01-fedora . && docker run --rm ej01-fedora
   docker build -f Dockerfile.debian -t ej01-debian . && docker run --rm ej01-debian
   ```

## ✅ Criterios de Éxito

- [ ] El programa compila sin warnings con `-Wall -Wextra -pedantic -std=c17`.
- [ ] En Fedora imprime algo como: `Hello from Fedora Linux 41`.
- [ ] En Debian imprime algo como: `Hello from Debian GNU/Linux 12 (bookworm)`.
- [ ] Si `/etc/os-release` no existe, el programa sale con código 1 y mensaje de error.
- [ ] Los tests automatizados pasan en ambas distros.

## 💡 Pistas

<details>
<summary>Pista 1 — Parsear os-release</summary>

Busca líneas que empiecen con `PRETTY_NAME=`. El valor puede estar entre comillas:
```c
// Si la línea es: PRETTY_NAME="Fedora Linux 41"
// Necesitas extraer: Fedora Linux 41 (sin comillas)
```

Funciones útiles: `strncmp()`, `strchr()`, `strlen()`.
</details>

<details>
<summary>Pista 2 — Leer línea por línea</summary>

```c
FILE *f = fopen("/etc/os-release", "r");
char line[256];
while (fgets(line, sizeof(line), f)) {
    // procesar cada línea
}
```
</details>

## 📖 Referencias

- `man 5 os-release`
- [Dockerfile reference](https://docs.docker.com/reference/dockerfile/)
- `man fopen`, `man fgets`, `man strncmp`
