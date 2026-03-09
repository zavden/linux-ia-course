# C03 — laboratorio guiado de bugs de memoria

## Objetivo
Diseñar un laboratorio reproducible de errores de memoria para practicar depuración.

## Qué debe hacer
- Incluir escenarios: leak, use-after-free, overflow, double-free.
- Activar escenario por argumento CLI.
- Entregar guías de diagnóstico con `gdb`, sanitizers y (si disponible) valgrind.
- Ofrecer versión "corregida" para comparar resultados.

## Pistas
- Aísla cada bug en función independiente.
- Evita que todos los escenarios crasheen igual; cada uno debe enseñar algo distinto.
- Documenta expectativa de salida y herramientas recomendadas por caso.
