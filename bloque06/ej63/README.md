# Ejercicio 6.3 — La Magia de NGINX: La API `epoll` 

## 🎯 Objetivo
Crear el corazón de un Servidor Web Masivo que pueda manejar a 5 Pestañas de Telnet/Netcat a la vez ¡sin usar un solo `pthread` o `fork` adiconal! Usaremos la API asíncrona definitiva de Linux: **`epoll`**.

## 📚 Teoría Mínima
- `epoll` permite "Observar" miles de File Descriptors (Sockets) de golpe, dándote un Resumen instantáneo O(1) de Cuales están "Listos para lectura (Con datos pendientes)" para que no vayas preguntándoles de a 1 en 1 gastando el CPU.
- **Paso 1: `epoll_create1(0);`**. Esto te devuelve un File Descriptor Central Maestro del Kernel. (Digamos, un Archivo Mágico C).
- **Paso 2: Agregar Vigilantes `epoll_ctl()`**. Si llega un Socket nuevo (`accept()` en el master), le pasas su FD al epoll diciéndole "Mantenle un ojo en modo lectura (`EPOLLIN`) a este Cliente #4".
- **Paso 3: Bloquear Inteligentemente `epoll_wait()`**. Lanzas esto en tu Event-Loop eterno. Esta Función Congelará C a 0% CPU, y regresará atómicamente cuando ¡Siquiera 1! de los miles de observados tenga datos nuevos o haya hecho Crtl-C. "Me regresa diciendo: El cliente 4 y el Master TCP tienen Eventos".

## 📝 Instrucciones

Construye `src/main.c`. 
1. Re-usa tu Código de TCP Básico del *Ejercicio 6.1*: ( `socket`, `bind`, `listen` ).
2. IMPORTANTE: Pon al Master TCP Socket Server en Modo No-Bloqueante (Usa el mismo Código `fcntl` del 6.2). ¡De lo contrario un `accept` lento arruinará el `epoll`!.
3. Crea la Central Observadora: `int ef_master = epoll_create1(0);`
4. Prepara el Struct de Registro: `struct epoll_event ev_nueva; ev_nueva.events = EPOLLIN; ev_nueva.data.fd = server_socket;`
5. Añádelo a la lista del Kernell OS: `epoll_ctl(ef_master, EPOLL_CTL_ADD, server_socket, &ev_nueva);`
6. Abre una matriz vacía para recibir eventos resueltos: `struct epoll_event eventos_kernel_devueltos[MAX_EVEN_ARRAY];`
7. Bucle Asíncrono Eterno `while(1) {`
8. `int n = epoll_wait(ef_master, eventos_kernel_devueltos, 10, -1); // El -1 significa "Pausa Inifinita CPU O%!"`
9. Haz un Bucle interno desde `0` hasta `n eventos resueltos`:
10. Saca el `int actual_socket = eventos_kernel_devueltos[i].data.fd;`
11. **El Enrutador**: 
    - a) ¿`actual_socket` ES el Server TCP Master Principal ?: ¡Milagro, hay una Llama o nuevo Usuario tocando la puerta `accept`!. Añádelo al club invocando de nuevo los pasos `fcntl` (No Bloqueante) y el paso `epoll_ctl ADD` como hiciste arriba pero ahora para él nuevo FD.
    - b) ¿No es el Master? Sino que... ES un Cliente Normal (Ejem, Usuario 8 que ya teníamos años atrapado en el radar): ¡Milagro! ¡Ha escrito un Char!. Hazle `read()`. Si read da 0 (Se Desconectó), ciérralo. El O.S Kernel es tan listo que automáticamente al cerrarlo lo Borrará para siempre de la lista secreta `epoll_ctl`, tú no tienes ni que decirle que lo borre.

## ✅ Criterios de Éxito
- Has dominado lo que muy pocos mortales en Computación logran C: Una máquina multiplexadora Nativa de Servidor Concurrente usando 1 solo sub-proceso, atestiguando cómo atiendes simultáneamente 5 pestañas abieretas de `nc 127.0.0.1 8080` de manera fluida y concurrente.
