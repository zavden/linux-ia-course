# 🚀 Proyecto 04 — `minitop`: Leyendo el Alma del Sistema (`/proc`)

## 🎯 Objetivo
Construir un Monitor de Procesos y Memoria (un clon de la mítica herramienta `top` o `htop`). Descubriremos el mayor de los secretos de UNIX: **Absolutamente la gran mayoría de la información vital del Kernel no es extraída con mágicas funciones complicadas, sino leyendo simples archivos de Texto plano escondidos en el directorio `/proc`**.

## 📋 Requerimientos Completos

### 1. Extracción de RAM Global (`/proc/meminfo`)
- El sistema Linux expone su salud general escribiendo un archivo virtual en RAM llamado `/proc/meminfo`.
- A tu binario `minitop` le exigiremos que, usando `fopen`, `fgets` (Buffer de librería `stdio` amortizando syscalls read), busque y extraiga las líneas:
  - `MemTotal:`
  - `MemFree:`
  - `Buffers:`
  - `Cached:`
- **Cálcalo**: Muestra un porcentaje de memoria ocupada = `(Total - Free - Buffers - Cached) / Total`.

### 2. Extracción de Carga de CPU Global (`/proc/stat` o `/proc/loadavg`)
- Lee `/proc/loadavg`.
- Muestra el promedio de carga a los _1 minuto_ y la cantidad de Tareas Totales listadas ahí.

### 3. Escaneo del Abismo de PIDs (`opendir /proc`)
- Cada proceso vivo tiene su propia carpeta bautizada numéricamente con su `PID` dentro de `/proc` (ej: `/proc/145/`).
- Itera el directorio base de `/proc` (Usa el código del *Ejercicio 2.4* con `opendir`/`readdir`).
- Descarta toda carpeta que no sea estrictamente y exclusivamente números. (El usuario o `ctype.h` `isdigit()` te ayudará a evadir carpetitas ignorables como `/proc/sys`).

### 4. Lectura a fondo de Proceso (`/proc/[pid]/stat` y `status`)
Por cada carpeta PID que halles:
- Abre `/proc/[pid]/stat`.
- **Extrae lo vital**: La segunda columna encasillada de paréntesis suele ser el Nombre del Ejecutable (ej. `(bash)` o `(sshd)`). La tercera letruca es su estado: `R` (Corriendo), `S` (Durmiendo), `Z` (Zombie!).
- Saca su "Tamaño en Memoria Virutal" desde `/proc/[pid]/status` buscando la línea `VmSize:` o `VmRSS`. ¡Imprime todos estos chistes agrupados en filas de tabla!

### 5. Loop (Refresh)
- Encapsula esta inmensa locura en un `while(1)`.
- ¡Limpia la terminal!. En Linux/Bash el comando mágico para "Barrido de pantalla Consola (`clear`)" e ir al tope arriba de la consola es hacer `printf("\033[2J\033[H");`.
- Descansa y duerme con `sleep(2)` o `sleep(3)` para no achicharrar y comer tu propio CPU repitiendo estos miles de archivos de texto virtual por segundo al leer `/proc`.

## ✅ Criterios de Éxito
- Lanzas `./minitop`, y te mostrará una interfaz verde tipo Cyberpunk, refrescándose segundo a segundo, mostrando tu carga CPU general, seguido del listado tabular ordenado y detallado de cada ser vivo respirando en el Corazón Central de tu Distribución Linux. ¡Habrás replicado una de las utilidades más veneradas del Open Source System Administration pura desde C!.
