# 🚀 Proyecto 05 — `miniserver`: Construyendo el NGINX/Apache C Nativo

## 🎯 Objetivo
Hacer converger el poder de los `Threads` con los conocimientos de un Sistema Operativo de Redes. Diseñarás un verdadero **Thread Pool** (Piscina de Hilos) que administre e intercepte peticiones usando Cond Variables y Mutex, implementándolo para crear un **Servidor HTTP 1.1 Multi-Cliente Inmortal**.

## 📋 Requerimientos Completos

### 1. The Thread Pool (Arquitectura General de Productor / Consumidor)
Imagina que te atacan 100,000 navegadores Web solicitando la FOTO. Hacer `100,000 pthread_create()` tumbará la PC con el Kernel crasheando por "Out of Limits Processes".
La única forma Enterprise/RealWorld es un Thread-Pool:
- Crea de golpe, AL ARRANCAR la CLI, unos **5 Hilos Estáticos inmortales** (`Workers`).
- Suéltalos en la Función Consumidora y de inmediato ponlos en un Loop `while(1)` infinito que se quede dormido a 0% CPU (`pthread_cond_wait`).  
- El **Hilo Padre Principal (Main)** escuchará la tarjeta WiFi. Cuando llegue un Intruso (Un File Descriptor de Socket TCP Conectado), el papá atrapará su ID y lo arrojará a un `Array/Buffer Tareas` y tocará la campaña `cond_broadcast`. Un Hilo worker de los tuyos se activará, le procesará su petición, le cerrará en su cara el `fd` TCP, ¡Y volverá sumiso al `wait` inicial!. 

### 2. Sockets y Tarjeta de Red 
- Tendrás que usar las SYS_CALLS rudimentarias C POSIX universales de Red:
  `int server_fd = socket(AF_INET, SOCK_STREAM, 0);`
- Ligar el servidor al puerto local (Ej: `8080`) a usar `bind()` y luego preparar tú antena radial OS kernell para llamadas entrantes ordenando: `listen(server_fd, ...)`
- El Main Padre lo único que hará toda la vida es trabarse (congelar CPU) ciegamente en base al Wait de Conexiones red pasiva: `int client_fd = accept(server_fd, ...)`.

### 3. Parsear el HTTP Request Textual Base
- Cuando un Worker acate y reciba su cliente, tendrá que hacer `read(..)` para leer los caracteres arcaicos y primitivos textuales que manda Google Chrome o Edge:
- Lucen así: `GET /index.html HTTP/1.1\r\n`
- Corta y aísla el método (es un `GET`?) y la ruta (`/archivo`). Extrae el primer gran String.

### 4. Entregando Bytes puros en Response a Tarjeta Red.
- Para responder exitosamente con un Web-Page desde C, no puedes solo hacer un printf. Chromium y Firefox exigen un Protocolo en Header estricto: Primero mandas: `HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n`
- Y exactamente detrás de esos 2 Enters crudos seguidos tuyos (`\r\n\r\n`), le haces un `open()` y otro `read()` al HTML verídico alojado en tu Linux Host (o al error 404.html si falló tu FileSystem), inyectándolo C caracter x byte usando `write()` hacia tu cliente tcp fd remoto local!.

## ✅ Criterios de Éxito
- Has compilado tu binario. Abres tu `Firefox` real, tucleas `http://localhost:8080`, y te recibe la cabecera pura HTML de C que le sembraste con un render de éxito asincrono donde si das 20 vueltas y spameas de F5 a tu App no crasheará y mantendrás 5 Pthreads en tu Memory Top vivos absorbiendolo sin demoras en filas.
