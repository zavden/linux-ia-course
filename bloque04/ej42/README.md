# Ejercicio 4.2 — Archivos Volátiles: `mmap` y Magia de Memoria Virtual

## 🎯 Objetivo
Hacer desaparecer la barrera entre Disco y RAM. Usar la syscall `mmap()` para hacer que Linux te entregue un simple array de C (`char *`) que, milagrosamente por detrás, esté atado directamente a un archivo masivo en tu disco duro físico.

## 📚 Teoría Mínima
- Tradicionalmente asumiéramos: abro archivo -> creo un buffer local `[1024]` -> leo un un poco (`read`) -> lo reviso. Y repito.
- Con `mmap`: Abres archivo -> le pides `mmap()` al Kernel -> ¡Te devuelve un puntero único `char *data`! 
   - No has leído *nada* aún en RAM y tu huella de memoria es de cero bytes; pero si el archivo mide 5 Gigabytes, tu C puede, si da la gana, revisar de un salto la posición mágica `data[4000000000]`.
   - Cuando tu CPU choque intentando mirar esa posición de memoria que en teoría aún no existe, **la electrónica del Hardware detendrá tu proceso en picosegundos (Page Fault)**, le avisará a Linux, Linux cargará exactamente un pedacito diminuto (Página de 4KB usualmente) de esa parte recóndita del disco en la RAM subyacente, y le devolverá el CPU a tu App de C, que seguirá viviendo creyendo que la RAM entera siempre estuvo dispuesta para él como magia pura.

## 📝 Instrucciones

Construye `src/main.c`.
1. Toma un `<nombre_de_archivo>` gigante por argumento (ej. archivo.bin).
2. Usa `open()` para obtener el FD local (`O_RDWR`).
3. Saca su "Tamaño Exacto": Puedes usar la vieja conocida syscall `stat(archivo, &st)` o hacer `lseek` saltando al final con offset 0 para descubrir su bytesize límite y luego regresando al fondo.
4. Llama a la mítica función: 
   ```c
   void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
   // addr = NULL (Deja que el SO decida adonde poner la RAM)
   // length = Tu size exacto averiguado
   // prot = PROT_READ | PROT_WRITE (para decirle que queremos poder ver y escrbir en su bytes)
   // flags = MAP_SHARED (Para que si escribimos en C, ¡El FileSystem Disco Duro reaccione guardando los verdaderos cambios permanentemente!)
   ```
5. Si `mmap` fracasa (da error `MAP_FAILED`), muestra `perror` de forma formal.
6. Demuestra que se ha mapeado leyendo el 1er carácter: `char *data = mapped_ptr; printf("%c", data[0]);`.
7. **Modifícalo Mágicamente**: Seteale `data[0] = 'X';`. 
8. ¡Ya está! Para sincronizar formalmente esta inyección tuya al disco (que sino Linux tal vez se la guardaba en caché hasta apagarse la PC), haz un `msync(data, length, MS_SYNC)`.
9. Libera tu monstruosidad mapeada pidiéndole `munmap(data, length)`.
10. Cierra tu `fd` de socket y termina con tu main de forma limpia.

## ✅ Criterios de Éxito
- Has creado un binario que muta la letra de un archivo usando ¡Una asignación ciega a un array de memoria en vez de usar la rústica funcion `write()`.
- Validarás corriendo `cat <archivo> | head` para atestiguar que en serio el disco duro recibió tu 'X'.
