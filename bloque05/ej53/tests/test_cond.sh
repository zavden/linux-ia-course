#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 5.3 (Cond Vars Prod-Cons Syncs) ==="

cd solucion/src
gcc -O2 -Wall -Wextra -std=c17 main.c -pthread -o cond_test || exit 1


echo "Iniciando Banquete con Temporización Asincrona... (Demora de Pseudo-Cocinas: 15 Platos * 0.5s = 7-8 segs aprox)"

# Para que el test NO SE NOS CONGELE INDEFINIDAMENTE C si hacen el bug fatal del Deadlock o falta el `broadcast final`
# le pondremos un comando de bash `timeout` cortandolo como asfixiado!
OUT=$(timeout 15s ./cond_test) || true 
echo "$OUT"

if echo "$OUT" | grep -q "SIMULACIÓN TERMINADA"; then
    echo "✅ Salida de Clean-Up de Cond_Broadcast Abandono Confirmada. Lograste despertar fantasmas y vaciar restaurante C Limpio."
else
    echo "❌ Error fatal o DeadLock Asíncrono Encontrado C. Los hilos quedaron trabados esperando y bash Timeout los quemó vivos abortando! (Usaste Unlocks?)"
    exit 1
fi

if echo "$OUT" | grep -q "0 Stock. Durmiéndome ciegamente cediendo Núcleo"; then
    echo "✅ Condición Sleep/Wait Efectuada detectada y ejecutada en CPU Kernell."
else
    echo "❌ Peligro: El wait Falló. O lograste producir todo en milisegundo antes quemado sin hacer esperar nunca consmidr."
    exit 1
fi


rm -f cond_test
echo "PASSED"
