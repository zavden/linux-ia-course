# C01 - namespace sandbox launcher (skeleton)

## Objetivo
Construir un lanzador de procesos aislados por namespaces con controles explícitos.

## Qué debe hacer
- Crear proceso con namespaces seleccionables (`pid`, `mnt`, `net`, `uts`, `ipc`, `user`).
- Configurar rootfs/chroot y entorno mínimo de ejecución.
- Aplicar drop de capacidades y límites básicos.
- Reportar estado y errores de aislamiento con trazabilidad.

## Pistas
- Separa parseo de flags, creación de sandbox y ejecución de comando.
- Diseña modo dry-run para validar configuración sin ejecutar.
- Maneja errores por etapa para troubleshooting rápido.
