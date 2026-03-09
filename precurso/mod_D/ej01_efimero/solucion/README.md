# Solución del Ejercicio D.1

El comando que usaste para el punto 5 fue:
`docker run -it debian /bin/bash`

Cada vez que usas `docker run` sin argumentos extra apuntando a recursos persistentes, inicias un contenedor **nuevo** e **independiente** clonado de la imagen base de solo lectura, con una capa de escritura temporal totalmente limpia. El contenedor anterior (y el archivo secreto que creaste dentro) fue detenido y dejado en el olvido, no reutilizado.
