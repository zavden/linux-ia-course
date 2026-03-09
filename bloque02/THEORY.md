# 📖 THEORY.md — Bloque 02: Archivos y Subsistema I/O

El Bloque 02 está diseñado de forma transversal centrado bajo un viejo refrán famoso de los sistemas operativos UNIX y todo OS descendiente (OSX, Linux, BSD..): *"En Unix, absolutamente TODO es un archivo"*.

---

## 1. El Costo de un "Context Switch"

Linux tiene dos mundos físicos fuertísimos y amurallados: 
- El "**User Space**" (Modo Usuario): Tu compilador, visual studio code, y tu C cutre corriendo.
- El "**Kernel Space**" (Modo Kernel): El alma misma del hardware, capaz de destruir y leer el disco duro físico con privilegios absolutos `Ring 0`.

Tus aplicaciones NUNCA tocan el hardware bajo Linux, por suerte. Cuando tu código choca con las **Llamadas al Sistema** (*System Calls* como `read`, `write`, `open`, `fork`). Tienes que ceder brutalmente todo control al SO: Tu app se suspende en tiempo real, se ejecuta un cambio arquitectónico de privilegios (Context Switch) extremadamente caro en términos de nanosegundos y ciclos de CPU, el SO atiende por tú tu solicitud, y devuelve los datos desescalando privilegios otra vez, entregándotelos despacito.

### Syscalls I/O puras (Bajo Nivel): Unbuffered

Viven de usar **File Descriptors** (`ints` fríos al estilo 3, 4, o 5 devueltos por `open`), sin mediar capas. Llamar 10 millones de `read()` buscando solo "1 byte", someterá brutalmente al kernel con 10 millones de context switches a tu hardware matando el rendimiento.

### Stdio I/O Bufferizadas (Alto Nivel) : Buffered

Es el envoltorio de oro brillante (`glibc - stdio.h`) originado para suavizar este problema.
Usa **File Streams** (`FILE *` al estilo `fopen`). Cuando tú le ordenas a stdio un simple byte con `fgetc`, en realidad, tu libreria de C efectua en la sombra la trampa de un `read_systemcall()` muy robusto y gordo por ejemplo de 4096 bytes de tu disco, robándoselos de cantazo... te entrega tu solicitado byte y esconde y traga en RAM los espantosísimos 4095 restántes en lo llamado un "Búfer".
Por eso stdio es la clave inconfundible de todas las Utilidades C cuando manipulan textos en vez de bases de datos binarias crudas gigantes puras, evitando asfixiarte durante Context Switches letales al OS en los for-loops.

---

## 2. Inodes, Hard Links y Symlinks (La desmitificación de un archivo real)

Piensa en los archivos como en ti y de cómo un sistema estatal gubernamental lleva y controla verdaderamente tus datos demográficos.
La realidad secreta para cualquier informático a este nivel es que... **Los Archivos no tienen nombre en el subsistema de directorios**.

1. **El verdadero individuo (`Inodo / INODE`)**: Un bloque de registros base crudo.
   Adentro contiene tu peso en discos (`size`), a quién pertenece (`UID / dueño`), y una fecha críptica (`mtime`). Es la persona biológica real con la metadata.

2. **Dando y Asignando el Identificador (`Hard Links` o Enlaces crudos o Entradas nativas de Direcciones)**: El estado te da un DNI. Un "Directorio de Linux" es un simple documento feo de papel que tiene dos columnas escritas a lápiz: tu número social/inodo (`#982463255`) y el apodo string puro (`Juan_Documento.txt`). 
   Puedes sacarte infinitos DNIs de países... Puedes tener los nombres que te apetezcan (alias de Hard Link) viviendo en distintas carpetas amarrados a este ID tuyo, y aunque el SO queme todos tus IDs, tú como INODE subyacente de datos verdaderos sigilososo **no serás eliminado del disco duro físico hasta que tu contador personal de enlaces (referencias al Inodo) caiga finalmente a cero 0 rotundo.** (`unlink` es lo que de hecho ocurre siempre con `rm`.) 

3. **Cadenas Ficticias Simples (`Symlinks` o Soft Links)**: A diferencia de los Dnis crudos, éste tipo nació creando a una perona biológica separada y completa (con su propio ID propio al mundo). Salvo de que todos sus registros biológicos adentros no ocultan fotos ni datos reales valiosos, tienen guardados adentro de sí un pedazo tonto de cuerda textual apuntando la ruta por texto como `../../../otra_ubicacion.doc`. Cando un System Call lo encuentra y lo quiere desenredar le sigue la pita ciegamente y se estrella y reporta un fallo de "Broken Link" si de casualidad la persona biológica meta fue verdaderamente destrozada y desvaneció.

---

## 3. Dirents y STAT API (Abriendo Directorios a patadas manuales)

Linux nos expone APIs brutales sin usar herramientas externas tontas para escupir recursividad: `opendir()`, `readdir()` nos sueltan la estructura `dirent` (la tabla de IDs y nombretes recién explicadas en papel). Y luego combinándolo y abofeteando archivos individuales y preguntándole al kernel sus datos reales privados directmente arrojados por una Invocativa gorda de metadatos conocida como `stat`.

Usamos herramientas matemáticas de Bits exclusivas originadas por Linux que desenvuelven y nos entregan con magia booleana el veredicto final si topamos con alguna monstruosidad atípica: (La macro unida y fundida con el campo integer st_mode (`S_ISDIR`, `S_ISREG`, `S_ISLNK`)). Es mandatorio usar la variante `lstat` en utilidades iterando que no querramos que los saltos de symlink exploten en la cara uniendo recursividades imposibles y crasheen (como un perro trantando de morderse su cola para siempre).
