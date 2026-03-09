# C03 - minicontainer runtime (skeleton)

## Objetivo
Diseñar runtime minimal de contenedores combinando namespaces, cgroups y políticas de seguridad.

## Qué debe hacer
- Validar configuración de contenedor antes de ejecutar.
- Crear entorno aislado (namespaces + rootfs) y aplicar cgroups.
- Ejecutar comando objetivo con manejo de ciclo de vida.
- Exponer estado final y eventos de ejecución.

## Pistas
- Implementa arquitectura por etapas: validate -> prepare -> isolate -> run -> cleanup.
- Aísla componentes para poder testear con mocks en entorno no privilegiado.
- Planifica manejo de fallos parciales con limpieza garantizada.
