# C01 - minidns zone compiler (skeleton)

## Objetivo
Diseñar una herramienta que valide, normalice y compile zonas DNS para despliegue seguro.

## Que debe hacer
- Leer zonas BIND y validar sintaxis/semantica de registros.
- Detectar inconsistencias (MX sin A/AAAA, CNAME conflictivo, serial no monotono).
- Generar salida canonical y reporte de errores por severidad.
- Preparar artefacto para recarga atomica de servicio DNS.

## Pistas
- Separa parser lexical, parser semantico y fase de validacion.
- Implementa modo strict y modo warn para distintos entornos.
- Mantén mensajes de error con linea/columna para depuracion rapida.
