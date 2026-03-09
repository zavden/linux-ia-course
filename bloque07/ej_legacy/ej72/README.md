# Ejercicio 7.2 — Autopsia Post-Mortem: GDB y Core Dumps

## 🎯 Objetivo
Perder el miedo al "Segmentation Fault (core dumped)". Aprender a configurar Linux de manera que, cuando un servidor crashee a las 4 de la mañana, deje un cadáver intacto en el disco duro (Core Dump) que tú puedas revisar a las 9 AM con el Debugger GDB y saber **qué línea exacta de código mató** el programa.

## 📚 Teoría Mínima
- Las distribuciones Linux suelen desactivar los *Core Dumps* por defecto para no llenar el disco de GB de archivos de error.
- Primero, tienes que pedir que lo habiliten usando `ulimit -c unlimited`.
- Te aseguras compilando con la `-g` para inyectar símbolos `gcc -g main.c`.
- Cuando programas una bomba lógica y falla, el terminal imprimirá `Segmentation fault (core dumped)`, y mágicamente creará un archivo monstruoso `core`, `core.XXXX` (donde XXXX es el PID), u otra ubicación que asigne el OS (`/var/lib/systemd/coredump`).
- **Resucitando a los muertos**: Ejecutas la CLI magica: `gdb ./mi_programa ./core`
- GDB leerá el ADN muerto, y con sus Comandos `bt` (backtrace), `print <var>` y `list`, desenterrará los valores de array o punteros rotos ¡Justo en el milisegundo final antes de la explosión!

## 📝 Instrucciones

Construye `src/main.c`. 
Vamos a programar el peor y más tonto de los crasheos posibles para asegurarnos de que muera sin piedad.
1. Crea una función `int dividir_seguro(int a, int b) { return a / b; }`.
2. Crea otra función inofensiva `void matar_rey() { int x = dividir_seguro(10, 0); }`.
3. Y quizás otra para desbordar la pila de llamadas (Recursividad infninita) o tocar memoria intocable `void apuñalar_ram() { char *nada = NULL; *nada = 'K'; }` (Deref NULL Pointer!!).
4. En tu `main`, hazte un menú interactivo con `printf` y `scanf` o Argumentos `argv[1]` que ofrezca esos 2 métodos de suicidio.
5. Usa tus conociemientos y pon manos a la obra: Correlo, que explote... y abre el `gdb` sobre el Dump!.

## ✅ Criterios de Éxito
- Tendrás un binario inestable que obligatoriamente desencadene dos Fatal Signals OS Distintos: (1) `SIGFPE` (Floating point exception - División x 0). y (2) `SIGSEGV` (Segmentation Violation - Null PTR).
- Comprobarás con Log Test Que bash ulimit te extrae los Core Files físicos y el parser de GDB de comandos atestigua exitoso la línea `dividir_seguro` como origen.
