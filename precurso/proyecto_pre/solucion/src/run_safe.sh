#!/bin/bash
# Solución del Proyecto Final Preparatorio (run_safe.sh)
set -eo pipefail

if [ $# -eq 0 ]; then
    echo "Uso: ./run_safe.sh <archivo.c>"
    exit 1
fi

FILE="$1"

if [ ! -f "$FILE" ]; then
    echo "❌ Error: El archivo '$FILE' no existe."
    exit 1
fi

# Eliminar Makefile y Dockerfile previos si los hay
rm -f Makefile Dockerfile

echo "Generando Makefile efímero..."
cat > Makefile << 'EOF'
CC = gcc
CFLAGS = -Wall -Wextra -std=c17
TARGET = app.exe

# El primer .c que encuentre (para simplificar, asumimos que solo pasamos uno principal o compila todos)
SRCS = $(wildcard *.c)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
EOF

echo "Generando Dockerfile efímero..."
cat > Dockerfile << 'EOF'
FROM fedora:latest
RUN dnf install -y gcc make && dnf clean all
CMD ["make", "all"]
EOF

echo "Construyendo imagen temporal..."
docker build -t comp-temp . >/dev/null

echo "Lanzando contenedor efímero para compilación..."
# Ejecutamos con --rm para que se borre. 
# Si el make falla, capturamos el error
set +e
docker run --rm -v "$PWD":/app -w /app comp-temp make all
EXIT_CODE=$?
set -e

# Limpiar restos
rm -f Dockerfile Makefile

if [ $EXIT_CODE -eq 0 ]; then
    echo "✅ Compilado con exito! Generado ./app.exe"
    # Opcional: Ejecutarlo localmente si el host tiene formato ELF compatible o en otro contenedor
    # docker run --rm -v "$PWD":/app -w /app fedora:latest ./app.exe
else
    echo "❌ Falló la compilación."
    exit $EXIT_CODE
fi
