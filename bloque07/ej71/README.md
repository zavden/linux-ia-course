# Ejercicio 7.1 — Cacería de Fantasmas: Valgrind y Errores Invisibles

## 🎯 Objetivo
Hacer tronar intencionalmente el manejo de memoria en C de las tres peores y más comunes formas posibles: (1) Leaking mem, (2) Use-After-Free, y (3) Buffer Overflow de Arrays. Veremos cómo `Valgrind` te detecta los 3 sin necesidad de crashear el CPU.

## 📚 Teoría Mínima
- Las fugas de Memoria son silenciosas. No crashean, sólo engordan y saturan la RAM Libre hasta el Apocalipsis (OOM - Out of memory).
- Los Buffer-Overruns (`Array[10] = "X"`) arruinan otras variables, provocando Comportamientos estocásticos y rándoms imposibles de predecir.

## 📝 Instrucciones

Construye `src/main.c`.
1. Crea una Flag `int opcion = atoi(argv[1]);` 
2. **Caso 1: El Leaker Silencioso.** Haz un `malloc(1024)`. Fíngelo: Llénalo de una string cualquiera... y ¡No uses `free`! Haz `return 0` de inmediato.
3. **Caso 2: El Asesino Zombie (Use-After-Free).** Haz `char *p = malloc(10);`. Llénalo con algo. Dale `free(p);`. Y en la línea siguiente, ¡Imprime tu letra muerta (`printf("%c", p[0])`) o mútala `p[0] = 'X'`!.
4. **Caso 3: El Desbordamiento (Buffer Overflow).** `char *mem = malloc(5);`. Trata de meterle un charcter literal a la posición 6 u 7: (`mem[6] = 'Y'`).
5. (Para que todos pasen el gcc y tu OS Kernell lo trague en Secuencial, asegúrate de liberar los del 2 y 3 formalmente al terminar ! aunque causen el Bug durante su loop).

## ✅ Criterios de Éxito
- Has compilado un binario maléfico.
- Pasas por consola `valgrind ./errores 1` y observas con asombro cómo te marca una condena "1 blocks definitely lost". 
- Pasas el Opcion Variante 2 y 3 y observas cómo el Motor Memcheck lo detecta alertando "Invalid Write of Size X" atajando Vulnerabilidades C !.
