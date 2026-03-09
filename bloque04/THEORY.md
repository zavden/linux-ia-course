# 📖 THEORY.md — Bloque 04: Memoria Avanzada

Este bloque cubre los secretos de cómo Linux reparte y organiza el tesoro más preciado de un ordenador después de su procesador: la Memoria RAM. Aquí ahondaremos en el Heap, la Memoria Virtual y los límites duros del SO.

---

## 1. El Allocator Interno (glibc) y el Kernel: `malloc` vs `brk`/`mmap`
- Cuando pides `malloc(100)` a la librería estándar (la `glibc`), ¡tu C rara vez molesta al Kernel de Linux! La librería pide un inmenso trozo ("chunk") de megabytes al Kernel y luego ella, como un contable meticuloso, te va prestando pequeños `mallocs` partiendo ese pastel gigante en RAM local.
- Si le pides a `malloc` cantidades colosales (por ejemplo, 1 GB), la glibc sí tendrá que pedirle soporte al Kernel llamando a `mmap()` (Mapeo de Memoria Anonimo puro de Kernell) u `brk` para expandir el borde del Heap de tu binario.
- `free(ptr)`: Le devuelve ese trocito a la glibc. Cuidado: La glibc retiene esa RAM un rato más en caché para futuros `mallocs` rápidos y no suele devolvérsela instantáneamente al Kernel (tu PC podría seguir reportando alto consumo de RAM en `htop` aunque hayas hecho free).

---

## 2. Archivos Mapeados (Memory-Mapped Files): `mmap()`
En UNIX hay dos formas de leer un archivo al disco: 
1. Moverlo pedacito a pedacito como baldes de agua (usando la syscall `read()` y un array local).
2. Hacer **Magia Absoluta**: Usar `mmap("tu_archivo.bin")`, que le pide a Linux que proyecte el archivo real entero directo en tu segmento virtual de RAM sin cargarlo todo de golpe. 
   - Linux usa su `Page Cache`.
   - Si devuelves un puntero `char *data = mmap(...)`, cuando accedas a `data[50000]`, Linux pausa imperceptiblemente el CPU (Page Fault), lee ese kilobyte pedazo físico de disco en ese bloque, y continúa tu app!.
   - Es abismalmente eficiente para saltos y lecturas aleatorias en archivos masivos de Gigabytes!.

---

## 3. Memoria Compartida (Shared Memory POSIX): `shm_open()`
El Bloque 03 nos enseñó a compartir String/Textos con Tuberías Pipes(`|`). Pero eso tiene límite. Si padre e hijo quieren leer una Matriz 4D flotante de OpenGL Gigantezca de 100 MB al instante a la vez... un PIPE crashearía de lento. 
- Necesitamos "Memoria Compartida", un bloque de RAM neutral donde el Hijo y Padre logren tener direcciones apuntando a literalmente el mismo pedazo silicón electrónico de hardware. Si el Hijo modifíca el `int x`, el Padre lo ve instantáneamente. 
- Se logra combinando `mmap` con el flag de Visibilidad Global (`MAP_SHARED`), o con la API POSIX moderna `shm_open()` que crea un "Archivo Fantasma en RAM" accesible desde la ruta `/dev/shm/...`.

---

## 4. Castigos de Recursos (`Resource Limits / ulimit`)
Un servidor que no ponga la correa a sus procesos colapsará asfixiado.
- La Syscall `getrlimit()` o `setrlimit()` en C puro dicta las fronteras lógicas por Proceso Individual: ¿Cuántos File descriptors (FDs) máximo puede abusar mi demonio simultáneamente abertos? (Límite por defecto común: 1024). ¿Cuántos Megabytes del Heap le autorizo antes de que el SO lo mate sin avisar (el temible OOM Killer - *Out Of Memory Killer*)?
- El Comando bash `ulimit` (o `prlimit` de systemctl) es su traducción equivalente a administradores (humanos). 
- Modificar los límites duros (*Hard Limits*) en ejecución requiere credenciales de súper usuario (`root`). Pero los procesos mortales pueden jugar disminuyéndose voluntariamente su *Soft Limit* o devolviéndolo asintóticamente a su frontera originaria `Hard`.
