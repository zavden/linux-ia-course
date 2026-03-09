# C02 — storage health daemon

## Objetivo
Desarrollar daemon que vigile salud de almacenamiento y emita alertas.

## Qué debe hacer
- Muestreo periódico de capacidad por mountpoint.
- Lectura de cuotas y detección de umbrales críticos.
- Integración de eventos de filesystem (`inotify` o fallback).
- Salida a log estructurado (JSON lines).

## Pistas
- Arquitectura por módulos: collectors, rules, notifier.
- Incluye archivo de configuración con umbrales.
- Diseña pruebas con fixtures para no depender del host real.
