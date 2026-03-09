# THEORY.md - Bloque 13: MiniCloud Distribuido por Sockets (HTTP/TCP Real)

Este bloque extiende el proyecto final a ejecucion distribuida real.
La diferencia clave frente al bloque 12: aqui los componentes interactuan por red,
no solo por parseo de fixtures locales.

---

## 1. Objetivo tecnico

Construir un stack de control plane con servicios desacoplados por sockets:
- `registry`: discovery y resolucion de rutas
- `gateway`: orquestador de requests
- `vault`: resolucion de secretos
- `runner`: admision/ejecucion de jobs
- `monitor`: ingesta de eventos y reporte de salud

---

## 2. Contratos de protocolo

Cada servicio expone comandos de texto lineal simples:
- `HEALTH`
- comandos especificos (`RESOLVE`, `GET`, `RUN`, `EVENT`, `REPORT`)

Principios:
1. un request por conexion
2. respuesta en una linea
3. formato estricto y parseable

---

## 3. Patrones de robustez en sockets

Checklist minimo:
- `SO_REUSEADDR` en servidores
- validacion estricta de comandos
- timeouts/limites de tamano de linea
- cierre limpio de FDs tras cada request

---

## 4. Orquestacion entre servicios

Flujo principal de `gateway`:
1. resolver backend via `registry`
2. si backend requiere secreto, pedirlo a `vault`
3. ejecutar admision en `runner`
4. publicar evento a `monitor`

---

## 5. Criterios de readiness distribuido

Readiness ya no es local: depende de conectividad y contratos validos entre servicios.

Ejemplo:
- `gateway` vivo pero sin `registry` no es funcional
- `runner` vivo pero sin secreto de `vault` debe rechazar

---

## 6. Mapa del proyecto

El proyecto vive en `bloque13/proyecto13/` y se valida con:
- tests unitarios por servicio
- test de integracion del stack

---

## 7. Errores comunes

1. acoplar servicios por supuestos no versionados
2. no validar entradas de red
3. no emitir errores accionables
4. no medir eventos de fracaso

---

## 8. Resultado esperado del bloque

Un MVP distribuido, testeable localmente, que demuestra:
- discovery
- routing
- secret lookup
- admission control
- observabilidad basica
