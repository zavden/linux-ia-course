# Ejercicio 2.3 — Permisos, chmod, umask y ACLs

## 🎯 Objetivo
Entender cómo el Kernel de Linux gestiona la seguridad de los archivos a bajo nivel mediante bits de permisos (Modo Octal), Umask, e introducirse teóricamente en las ACLs (Access Control Lists).

## 📚 Teoría Mínima
- **Octal:** Los permisos `rwxr-xr--` (754) en C se escriben con un cero inicial: `0754`. 
  - `0400` = Read User, `0040` = Read Group, `0004` = Read Other
  - `0200` = Write User, `0020` = Write Group, `0002` = Write Other
  - `0100` = Exec User, `0010` = Exec Group, `0001` = Exec Other
- **Umask:** Es una máscara restadora del Kernel. Si tú en C pides crear un archivo con permisos `0666` (rw-rw-rw-), pero tu bash tiene un umask de `0022` (----w--w-), el Kernel hará una resta lógica bit-a-bit y el archivo final nacerá con `0644`.

## 📝 Instrucciones

1. Crea en `src/main.c` un programa que lea dos argumentos: `<archivo> <permisos_octales>` (Ej: `./app test.txt 0644`).
2. Usa la syscall `umask(0)` para forzar que nuestra aplicación ignore la máscara del shell y asigne los permisos **exactos** solicitados. (Guarda la umask vieja que retorna la syscall).
3. Usa la syscall `open` con `O_CREAT` o `chmod()` puro si el archivo ya existe para setear esos permisos convertidos desde texto octal a un un número. Pista: `strtol(arg, NULL, 8)` sirve para convertir "0644" en entero octal.
4. Restaura la `umask()` original al final.
5. Usa `system()` para llamar al binario oficial `getfacl <archivo>` y demostrarle al usuario los permisos extendidos (ACLs) que tiene recién aplicado.

## ✅ Criterios de Éxito
- Lanzas `./app demo.txt 0777` y al hacer `ls -l` ves `-rwxrwxrwx` exacto sin que la umask de bash interfiriese.
- Aprendiste que `chown` solo lo puede hacer el root (UID 0), así que nos conformamos con `chmod`.
