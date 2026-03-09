# TODO — Bloque 06

> Estado: postergado temporalmente mientras refinamos Bloques 00–05.

## 1) Cerrar estructura mínima del bloque
- [ ] Crear `README.md` en `ej64`.
- [ ] Crear `README.md` en `ej65`.
- [ ] Definir objetivos, criterios de éxito y comandos de prueba para ambos.

## 2) Alinear alcance del proyecto 06
- [ ] Decidir nombre/alcance final: `minicurl` (MAIN) vs `minicache` (README de proyecto).
- [ ] Unificar nombre y descripción en:
  - [ ] `MAIN.md`
  - [ ] `bloque06/proyecto06/README.md`
  - [ ] tests asociados

## 3) Corregir estrategia de tests de aprendizaje
- [ ] Evitar que `make test` valide solo `solucion/`.
- [ ] Hacer que `make test` evalúe implementación en `src/`.
- [ ] Dejar tests de referencia separados (por ejemplo `make test-solution`).

## 4) Validación dual-distro real
- [ ] Verificar build/test en Fedora y Debian para `ej61`–`ej65` y `proyecto06`.
- [ ] Ajustar dependencias/headers Linux específicos según distro si aplica.

## 5) Refinar teoría del bloque
- [ ] Ampliar `THEORY.md` con nivel intermedio/avanzado.
- [ ] Incluir: `SO_REUSEADDR`, non-blocking robusto, edge vs level trigger, backpressure, manejo de `EAGAIN/EWOULDBLOCK`, cierre ordenado de sockets.

## 6) Criterio de “Bloque 06 listo”
- [ ] 5 ejercicios + 1 proyecto documentados.
- [ ] `README` completo en cada unidad.
- [ ] Tests útiles para alumno (no solo solución).
- [ ] Coherencia total entre `MAIN`, teoría y enunciados.
