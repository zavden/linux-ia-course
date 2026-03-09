# 🚀 Proyecto 06 — `minicache`: Tu propio Memcached / Redis Asíncrono en C

## 🎯 Objetivo
Hacer converger el Sistema `epoll` del Ejercicio 6.3 con un Hashmap / Arrays de Memoria. Programaremos un **Storage Key-Value (Cache de datos En Memoria LRAM)** como Redis, completamente libre de Hilos (Puro Event-Loop 1 sólo Thread), que atienda a múltiples clientes por TCP permitiéndoles Guardar variables globalmente y Leerlas a la asombrosa velocidad de luz de C.

## 📋 Requerimientos Completos

### 1. The Key-Value Store (El Cerebro)
- Nuestro servidor en pura Memoria RAM usará, por simplicidad para la currícula, un Array estático Fijo GIGANTE de pares (Si quieres, hasta un Array normal `struct Par { char clave[32]; char valor[128]; int valid_flag; }`).
- Como estamos usando SÓLO 1 HILO (El `main()` Master), ¡**NO ocupamos para nada `pthread_mutex` ni rw_locks aquí!!**. El Multiverso se ha aplanado. Cero Race Conditions. Un lujo.

### 2. Sockets y Epoll 
- Implementa exactamente el motor del *Ejercicio 6.3*. 
- Pon a tu Server TCP Master socket escuchando tu Puerto C, digamos, `9595`.
- Usa `epoll_create1` y configuralo para atrapar y agregar (`EPOLL_CTL_ADD`) clientes de forma infinita `epoll_wait()`.

### 3. Parsear el Mini-Protocolo TCP
Cuando `epoll_wait` te regrese un Cliente TCP normal (un FD esclavo) al que tú le debes hacer `read()`, vendrá un String escrito por el usuario humano. 
Deberás parsearlo usando `sscanf()` o `strtok()` para armar un protocolo "Cache" de tres comandos básicos simples:
1. `SET <clave> <valor>`: El usuario quiere guardar "SET user pepe". Tú iterarás tu RAM buscando la `<clave>` o un hueco libre valid_flag y sobreescribirás el valor. Luego le mandarás por el FD un `write`: `"OK\n"`. 
2. `GET <clave>`: Iterarás la DB y si hallas la Clave se la imprimirás al FD TCP: `"VALUE: pepe\n"`. Si no: `"NULL\n"`.
3. `DEL <clave>`: Lo borras de la RAM y le dices `"OK\n"`.

### 4. El Ciclo de Vida del File Descriptor (Desconexiones)
- Evade Crashear. Si `read()` es igual a `0`. Significa Cliente Crtl-C (o se fue). Haz `close()`.
- Ojo: Debes usar Obligatoriamente el flag `O_NONBLOCK` con la función `fcntl()` tanto para el socket maestro Servidor, como para TODOS los sockets de los Clientes que entren (inmediatamente después del `accept()`), o un usuario malicioso podría colgar tu servidor asíncrono entero!.

## ✅ Criterios de Éxito
- Lanzas `minicache` en una terminal.
- Abres TRES terminales más diferentes y en cada una corres `.nc 127.0.0.1 9595`.
- En la Consola #1 pones: `SET nombre Juan` (Da OK). 
- En la Consola #2 pones: `GET nombre` (Debe asombréate devolviendo `VALUE: Juan`).
- Cierra las consolas rudamente. `minicache` seguirá Inmortal vivo a 0% CPU absorbiendo todo asíncronamente como un Servidor Redis Profesional de $1M Billones C.
