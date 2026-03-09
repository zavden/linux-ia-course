# 📖 THEORY.md — Bloque 07: Debuggers, Make y Metaprogramación (Optimizando el Infierno)

El Código en C es tan rápido por una razón: **Te quita los estabilizantes de la bicicleta**. Si haces algo malo y pierdes Memoria, el CPU Linux crasheará con el infame Error "Segmentation Fault: Core Dumped". En este bloque, aprenderás a usar las utilidades y artes arcanas para sobrevivir a la guerra contra el Kernel.

---

## 1. El Ojo Que Todo Lo Ve: `gdb` (GNU Debugger)
Compilar con `gcc main.c` crea un ejecutable que no tiene nombres de funciones adentro (solo ceros y unos). Si crashea, Linux no te puede decir "¿Dónde fue?".
- **Bandera Milagrosa `-g`**: `gcc -g main.c` le inyecta "Símbolos de Debuggeo". Hace tu binario 20% más pesado, ¡pero contiene el Mapa fuente Original!
- **Arrancar `gdb ./mi_app`**:
   - `run` (r): Ejecuta tu programa. Si hay un Segfault, ¡Se detendrá EXÁCTAMENTE EN LA LÍNEA DE C QUE EXPLOTÓ!
   - `break main.c:15` (b): Pone un PARE invisible allí. Cuando corras el `run` se pausará.
   - `print mi_variable` (p): Muestra qué valores tiene tu array / variable justo en esa milésima de segundo Pausada!
   - `next` (n): Avanza "1 simple línea C" para que veas la ejecución lenta. `step` (s) se Mete adentro de la función actual.
   - `backtrace` (bt): Si crasheaste en una librería externa a 5 saltos... ¡El BT te muestra toda la jerarquía de Calls al revés para ver quién causó el mal!.

---

## 2. El Cirujano de Fugas: Valgrind / Memcheck
Incluso si tu Binario No Crashea (`EXIT_SUCCESS`)... ¡Podría haber perdido / leakeado toda la memoria si no usaste `free()`, causando que la Computadora servidora que lo corra se Apague a los 3 días!
- `valgrind --leak-check=full ./app`: Emula una CPU virtual lenta, vigila CADA SÓLO `malloc` y `free` ejecutado. 
- Te Reporta al final los "Orphans" `Definately Lost Bytes`. Si el contador no da *Zero InCorrections*, eres un Peligro C.
- También intercepta "Use-After-Free" (Ej. usar un puntero Cuyo Free ya mandaste ayer) previniendo Vulnerabilidades de Hackeo de Desbordamiento severas!.

---

## 3. El Herrero Automatizado: `make` y GNU Makefiles
Si en Bloque6 programáramos un server en C de 40 archivos C... ¡Escribiríamos `gcc f1.c f2.c ... f40.c` todos lo días!. Además, cada vez que modificaras el archivo #3... gcc volvería a re-compilar **LOS 40 ARCHIVOS** tardando 10 minutos inútilmente!
- **Makefile**: Es un script purista (Usa **TABULATIONS `\t`** no espacios, cuidado!) que funciona creando Grafos de Dependencias.
```makefile
mi_app: src.o dest.o  # <- Objetivo
	gcc src.o dest.o -o mi_app  # <- Instruccion (CON UN TAB '\t' AL INICIO, IMPORTANTISIMO)

src.o: src.c 
	gcc -c src.c
```
- `make` lee el archivo y verifica la 'Fecha de Modificación de Kernell'. Si cambiaste "src.c", make sólo corre `gcc -c src.c`, y reutiliza el viejo binario ya forjado de "dest.o", uniendo todo en `.1` segundos y salvándote la vida (y tu trabajo remoto).

---

## 4. Inyección de Librerías Estáticas `.a` y `.so` Dinámicas (Opcional)
- A veces no entregas `.c`, entregas un Librería Mágica de Paga. 
- `.a` (Archive/Estática): Al usar gcc, este mete/incrusta tu codigo literal en el Binario Resultante .exe final inflándolo de peso per haciéndolo "Portable" .
- `.so` (Shared Object/Dinámica): Linux solo anota en el Binario final "Hey! Ocupo que cuando esto se corra, el usuario tenga instalada en la RAM Linux la SO". (Esto lo usan las APIs Posix como `-pthread` o `-lm` Matemática). Ahorra peso de Binario.
