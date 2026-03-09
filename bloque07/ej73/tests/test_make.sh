#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 7.3 (Modular GNUMake Compilation) ==="

cd solucion/

# Limpiamos todo el basurero
make clean > /dev/null 2>&1

echo "[*] Invocación 1 GNU_MAKE: Compilación Original desde Cero..."
# Ejecutamos mi make Magno que lanzará el comando "all:" -> Llama a los .o -> Llama al main -> Arroja C Binario en 3 gcc's!
MAKEOUT_1=$(make 2>&1)
echo "$MAKEOUT_1"

if echo "$MAKEOUT_1" | grep -q 'gcc.*-c src/main.c'; then
    echo "✅ [1] Make forjó main.o unitariamente."
else
    echo "❌ Error De Reglas, no detectó o forjo Main_Object en Compilada Separadas"
    exit 1
fi
if echo "$MAKEOUT_1" | grep -q 'gcc.*main.o mates.o'; then
     echo "✅ [2] Make ensambló y Linkeó objetos aislados hacia un Binary Formal Kernell."
fi

# El Test Maestro De Reutilizacion (El Gran Secreto de Make C)!!
echo "[*] Invocación 2 GNU_MAKE: (Sin haber mutado nada). Make NO DEBERIA COMPILAR NADA, porque los .o C_OBJ son más recientes que el Text.c Source!"
MAKEOUT_2=$(make 2>&1)
echo "$MAKEOUT_2"

# "calculos_make is up to date" (Texto generico Make GNU Universal C)
# GNU Make no lanza los scripts bash si stat() size timestamp kernel C Files coinciden O(1) evitando miles C gcc!. 
if echo "$MAKEOUT_2" | grep -q 'up to date'; then
    echo "✅ [3] Optimizador Make Time Triunfante. Reutilizo el cache OS."
else
    echo "❌ Peligro: El Make está asquerosamente re-forjando TODO tu archivo pesadamente perdiendote tu Tiempo (Quizas te faltaron dependencias correctas file c?)."
    exit 1
fi

./calculos_make > exec_test 

if grep -q "10 + 50 = 60" exec_test; then
     echo "✅ Linkeo Binario Correcto Corriente a nivel Matemático y Modular."
fi

# Cleanup
make clean > /dev/null 2>&1
rm -f exec_test
echo "PASSED"
