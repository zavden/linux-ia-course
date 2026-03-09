# Ejercicio 3.5 — Magia Oscura: Crear un Demonio (Daemon)

## 🎯 Objetivo
Orquestar la transformación de un proceso normal de interfaz interactiva en un **Demonio Linux real**, logrando que se desvincule por completo de tu terminal interactiva para vivir permanentemente flotando y sirviendo en el fondo del Sistema Operativo como lo hacen `sshd`, `nginx` o `cron`.

## 📚 Teoría Mínima
¿Cómo se independiza un programa en C puro? Mediante 5 pasos canónicos:
1. **Primer `fork()` y asesinato del padre:** Se crea un hijo en *background* y obligas que el original termine (`exit`). Bash ya no bloquea ni sigue atado a esto.
2. **`setsid()`:** El programa ahora no tiene papa, pero aún pertenece a la misma "Sesión Terrminal" (`TTY`). Esto rompe relaciones creando una Sesión Independiente exclusiva del kernel y pidiéndose ser el Líder.
3. **Segundo `fork()` y asesinato del segundo padre:** `setsid` nos hizo independientes. ¡El peligro es que los dueños de sesión pueden aún pedirle "a prestado" PTYs interactivos de nuevo al Linux! Hacer un SEGUNDO fork garantiza al 100% que NO eres Session Leader y jamás de los jamases un Bash se te volverá a colgar ni inyectar teclados.
4. **Cierre ciego de FDs (`close`)**: Nadie nos mira, así que se exige destruir la Boca 0, 1 y 2 correspondientes al Stdin, Stdout, Stderr. Para evitar logs en la Matrix los redirigiremos a `/dev/null`. Tu único contacto para debug será hacia el File System montando logs como `/var/log/...`
5. **Máscaras y Path (`chdir("/")` y `umask(0)`)**: Nos movemos a la raíz, si dejamos al daemon corriendo en nuestra consola origen, Linux no nos dejaría ni expulsar USBs bloqueando los montes en los que estábamos antes.

## 📝 Instrucciones

1. En `src/main.c`, crea la función `daemonize()` agrupando los 5 pasos canónicos de arriba.
2. Añade un gestor de señales como en el Ejercicio anterior (`SIGHUP` servía históricamente para decirle al demonio que recargue configuraciones `.conf` usando un trigger. Lo usaremos como prueba. Y `SIGTERM` lo matará y romperá su condicional while para limpieza de Log).
3. Un Demonio no se ejecuta miles de veces a la vez. Protege su nacimiento obligándolo a escribir su propio ID a `/tmp/midemonio.pid`. Si ya existe otro con ese número funcionando leyendo del archivo, aborta de inmediato. 
4. Tu bucle infinito deberá abrir un simple `/tmp/midaemon.log` cada 2 segundos y volcar "Mantenimiento Ok - Seg N".

## ✅ Criterios de Éxito
- Lanzas `./app`, notas cómo instantáneamente regresa el bash libre a la línea `$`, haces un `ps -eaf | grep app` y ¡Magia! verás que su TTY (Consola) dice `?` en lugar de `pts/0`. Has cortado tu cordón umbilical exitosamente. Para que acabe deberás usar `kill` en él como lo harías desde tu computadora matriz con los servicios en Cloud Server remotos!
