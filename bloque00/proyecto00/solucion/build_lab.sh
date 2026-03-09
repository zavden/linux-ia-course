#!/bin/bash
# ==============================================================================
# Proyecto 00 — "build-lab" (SOLUCIÓN DIDÁCTICA)
# 
# OBJETIVO DIDÁCTICO:
# Este script de shell orquesta directamente al demonio de Docker.
# Muestra cómo puedes crear herramientas de CI/CD (Continuous Integration) locales
# para testear código en múltiples contenedores aislados y reportar resultados.
# ==============================================================================

# `set -u` provoca que si intentamos leer una variable que no existe,
# el script aborte inmediatamente en lugar de usar un string vacío. 
# Es una práctica vital de seguridad y robustez en Bash.
set -u

# ------------------------------------------------------------------------------
# 1. VALIDACIÓN DEL ARGUMENTO DE ENTRADA
# ------------------------------------------------------------------------------
# `$#` almacena el número de argumentos que le pasaste al script.
if [ $# -lt 1 ]; then
    echo "Uso: $0 <directorio_ejercicio>"
    exit 1
fi

TARGET_DIR="$1"

# Verificamos si la cadena de texto proporcionada es realmente una ruta
# de directorio válida y existente en el host linux usando la bandera `-d`.
if [ ! -d "$TARGET_DIR" ]; then
    echo "Error: El directorio '$TARGET_DIR' no existe."
    exit 1
fi

# Intentamos ingresar a ese directorio. Si falla de permisos, salimos.
cd "$TARGET_DIR" || exit 1

# ------------------------------------------------------------------------------
# 2. DEFINICIÓN DEL ESCENARIO Y RECIPIENTE DE RESULTADOS
# ------------------------------------------------------------------------------
# Declaramos un vector tradicional con las distros a probar
DISTROS=("fedora" "debian")

# `declare -A` crea en Bash un Array Asociativo (diccionario / hash map).
# Aquí mapearemos: "nombre-distro" -> "Mensaje de resultado"
declare -A RESULTS

echo "[build-lab] Iniciando validación en $TARGET_DIR..."

# ------------------------------------------------------------------------------
# 3. EL BUCLE PRINCIPAL MULTI-DISTRO
# ------------------------------------------------------------------------------
# `${DISTROS[@]}` expande eficientemente todos los elementos del array
for distro in "${DISTROS[@]}"; do
    img_name="build-lab-${distro}-test"
    
    # Comprobamos que el repositorio tiene el Dockerfile para esta distro
    if [ ! -f "Dockerfile.$distro" ]; then
         RESULTS["$distro"]="⚠️ SKIP (No Dockerfile)"
         continue # Saltamos a la siguiente iteración del for
    fi

    # --------------------------------------------------------------------------
    # FASE A: Construcción de Imagen (Build)
    # --------------------------------------------------------------------------
    echo "  -> Construyendo imagen $distro..."
    # Ejecutamos docker build
    # Ocultamos toda la verborragia del log mandando sdout (1) y stderr (2) a la nada.
    # Si este comando falla (! docker build), guardamos la falla.
    if ! docker build -f "Dockerfile.$distro" -t "$img_name" . >/dev/null 2>&1; then
         RESULTS["$distro"]="❌ BUILD FAILED"
         continue
    fi

    # --------------------------------------------------------------------------
    # FASE B: Ejecución Interactiva y Captura de Códigos
    # --------------------------------------------------------------------------
    echo "  -> Ejecutando tests en $distro..."
    
    # Desactivamos set -e si estuviese prendido, ya que si Docker falla,
    # queremos capturar el código y continuar con la otra distro, no morir aquí.
    set +e
    
    # --entrypoint sobreescribe lo que sea que diga el CMD del Dockerfile.
    # Obligamos a ejecutar 'make test' dentro del contenedor en las fuentes ya fijadas.
    docker run --rm --entrypoint make "$img_name" test >/dev/null 2>&1
    
    # `$?` guarda el "Exit Code" del último comando (docker run en este caso).
    # Un Exit Code de 0 en Linux significa que todo salió perfecto.
    exit_code=$?
    
    # Reactivamos validaciones estrictas
    set -e

    if [ $exit_code -eq 0 ]; then
        RESULTS["$distro"]="✅ PASSED"
    else
        RESULTS["$distro"]="❌ FAILED"
    fi
done

# ------------------------------------------------------------------------------
# 4. REPORTE FINAL
# ------------------------------------------------------------------------------
echo ""
echo "[build-lab] Reporte de tests para: $TARGET_DIR"
for distro in "${DISTROS[@]}"; do
    # Muestra el valor en el diccionario RESULTS buscando la calve $distro.
    # El `:-UNKNOWN` significa "Si la clave no existe, muestra UNKNOWN".
    # El acento circunflejo (`^`) capitaliza la primera letra (fedora -> Fedora).
    echo "- ${distro^}: ${RESULTS[$distro]:-UNKNOWN}"
done
