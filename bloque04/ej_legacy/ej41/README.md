# Ejercicio 4.1 — Vectores Dinámicos manuales (`malloc`, `realloc` y `free`)

## 🎯 Objetivo
Entender cómo programar estructuras de datos que crecen "infinitamente" alojando su cuerpo en el **Heap**. Implementaremos un emulador de lista genérica (`DynArray`) estilo lenguaje de alto nivel. Aprenderemos el coste real de re-alojar memoria RAM para expandirse, y a domar los *Memory Leaks*.

## 📚 Teoría Mínima
- `malloc(size_t size)`: Pide `size` bytes exactos. Devuelve `void*` nulo si falla (Ej. La RAM de la laptop colpasó explotando). No limpia la RAM otorgada; heredarás la "basura" o ruinas y bytes randomizados que haya dejado otro programa ajeno segundos atrás.
- `calloc(size_t n_items, size_t size)`: Igual a `malloc`, pero muy gentilmente limpia con ceros `\0` todo el tamaño entero previniendo bugs de basurilla antigua.
- `realloc(void *ptr, size_t new_size)`: El arma estrella. Si tu array de 10 posiciones (40 bytes) está lleno... y quieres meter el elemento 11. Llamas `realloc(ptr, 80)`. Tratará de darte los siguientes 40 espacios contiguos. Pero, si resultara que otro programa había tomado ya esos espacios próximos en la RAM... `realloc` **copiará ciegamente** todo tu array viajándolo a otra ciudad vacía muy lejana de RAM con suficiente capacidad, matando y despojando automáticamente a la antigua! Esto lo devuelve siempre como un `ptr` nuevo distinto; por lo que siempre tienes que guardar su update. (Jamás pases o asumas que un pointer de un realloc conserva su dirección inicial física).

## 📝 Instrucciones

Construye `src/main.c`.
1. Fija una constante para el límite iterativo de números (`1000` inserciones teóricas).
2. Haz una estructura `dynarray_t`:
   ```c
   typedef struct {
       int *data;
       size_t len;
       size_t capacity;
   } dynarray_t;
   ```
3. Defíne una función de crear y inicialíza struct pasándole un `capacity = 2` espacios de origen (Haz uso de `malloc()` de tamañó `2 * sizeof(int)`).
4. Crea `void push(dynarray_t *arr, int value)`. Adentro... ¡Revisa tu límite físico lógico! Si `len == capacity` hemos chocado contra pared. Duplica el `capacity` por 2x usando `realloc(arr->data, capacity * 2 * sizeof(int))` y actualiza el apuntador y struct. Luego insértale el valor.
5. Imprime loggando cada que ocurra una re-alojación física real y pesada. 
6. Eyecta la matriz y librala al Kernell con `free()`.

## ✅ Criterios de Éxito
- Has creado un script Bash que compile C y **obligatoriamente pase por medio de `valgrind --leak-check=full`** este binario para asegurarnos que todos tus contadores o bloques devueltos sean exactamente `0 bytes leaked` sin huérfanos!.
