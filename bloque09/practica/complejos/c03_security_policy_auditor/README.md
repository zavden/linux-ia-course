# C03 - security policy auditor (skeleton)

## Objetivo
Crear un auditor de postura de seguridad para hosts Linux con reporte de hallazgos accionables.

## Que debe hacer
- Recolectar estado de capabilities, SELinux/AppArmor y configuraciones base.
- Evaluar reglas (OK/WARN/CRIT) con severidad y evidencia.
- Generar salida para consola y formato consumible por automatizacion.
- Permitir politicas por perfil (dev, staging, prod).

## Pistas
- Diseña arquitectura collector -> rules -> reporter.
- Prioriza tolerancia a fuentes faltantes sin romper ejecucion.
- Mantén salida estable para comparacion entre corridas.
