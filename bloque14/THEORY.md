# THEORY.md - Bloque 14: MiniCloud HTTP/1.1 y Contratos API

Este bloque lleva el stack distribuido a un protocolo HTTP/1.1 minimo real.
La meta es operar servicios que hablen por APIs con status codes, rutas y payloads estructurados.

---

## 1. Objetivo del bloque

Migrar de comandos de texto por socket a HTTP:
- request line (`GET /path HTTP/1.1`)
- headers basicos
- status codes estandar
- body JSON simplificado

---

## 2. Contratos API

Cada servicio define endpoints claros:
- `GET /health`
- endpoints de dominio (`/resolve`, `/secret/<id>`, `/run`, `/event`, `/report`)

Contratos versionables evitan acoplamiento fragil entre componentes.

---

## 3. Parser HTTP minimo en C

Para MVP en C:
1. leer request line
2. extraer metodo/path
3. ignorar headers no usados (consumir hasta linea vacia)
4. responder con status + `Content-Length`

No es un parser RFC completo; es un parser defensivo para pruebas controladas.

---

## 4. Status codes operativos

Recomendacion por categoria:
- `200` exito
- `202` evento aceptado async
- `400` request invalido
- `404` recurso inexistente
- `409` conflicto de capacidad/politica
- `500` error interno

---

## 5. Integracion del control plane

Flujo tipico del gateway:
1. `registry`: resolver backend por path
2. `vault`: resolver secreto activo
3. `runner`: intentar admision/ejecucion
4. `monitor`: registrar evento y estado

---

## 6. Observabilidad API-first

Monitor debe exponer:
- resumen de errores
- latencia media/max
- severidad global (`OK/WARN/CRIT`)

Eso permite readiness y troubleshooting automatizable.

---

## 7. Riesgos frecuentes

1. ignorar validacion de query params
2. respuestas sin `Content-Length`
3. mezclar errores de dominio con `200 OK`
4. no testear degradaciones de dependencias

---

## 8. Resultado esperado

Un stack distribuido por sockets HTTP/1.1 con:
- tests unitarios por servicio
- test de integracion de todo el flujo
- contratos legibles y estables
