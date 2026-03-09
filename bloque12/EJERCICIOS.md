# EJERCICIOS.md - Bloque 12 (Indice)

Este archivo es solo mapa de ejercicios.
No incluye enunciados largos ni solucion embebida.

Practica completa en `bloque12/practica/`.

---

## Resueltos (10)

## E01 - parser de contratos de servicio
- Objetivo: validar definiciones base de servicios del stack.
- Que hace: parsea `service|port|proto|health_path` y cuenta validos/invalidos.
- Ruta: `bloque12/practica/resueltos/e01_service_contract_parser`

## E02 - constructor de snapshot de registry
- Objetivo: sintetizar estado de discovery para consumo del control plane.
- Que hace: resume servicios `up/down` en salida JSON compacta.
- Ruta: `bloque12/practica/resueltos/e02_registry_snapshot_builder`

## E03 - planificador de rutas para gateway
- Objetivo: resolver ruteo consistente segun prefijos y salud de backend.
- Que hace: aplica longest-prefix-match y evita backends `down`.
- Ruta: `bloque12/practica/resueltos/e03_gateway_route_planner`

## E04 - reductor de estado de salud
- Objetivo: consolidar señales de salud por severidad.
- Que hace: reduce `ok/warn/crit` a estado global unico.
- Ruta: `bloque12/practica/resueltos/e04_health_status_reducer`

## E05 - parser de line protocol de metricas
- Objetivo: extraer indicadores operativos de eventos de request.
- Que hace: calcula latencia promedio y porcentaje de error.
- Ruta: `bloque12/practica/resueltos/e05_metrics_line_protocol_parser`

## E06 - validador de manifest de secretos
- Objetivo: verificar metadatos de secretos antes de distribuir.
- Que hace: valida nombre/scope/TTL/rotacion y resume riesgo.
- Ruta: `bloque12/practica/resueltos/e06_secret_manifest_validator`

## E07 - validador de job spec del runner
- Objetivo: evitar ejecucion de workloads con parametros invalidos.
- Que hace: valida limites de CPU/memoria/timeout por job.
- Ruta: `bloque12/practica/resueltos/e07_runner_job_spec_validator`

## E08 - parser de logs de event bus
- Objetivo: detectar senales de integracion y degradacion entre componentes.
- Que hace: cuenta tipos de evento, errores y latencia maxima.
- Ruta: `bloque12/practica/resueltos/e08_event_bus_log_parser`

## E09 - auditor de politicas minicloud
- Objetivo: evaluar baseline de seguridad y operacion del stack.
- Que hace: aplica checks de politica y emite `OK/WARN/CRIT`.
- Ruta: `bloque12/practica/resueltos/e09_minicloud_policy_auditor`

## E10 - reporte de readiness de integracion
- Objetivo: decidir si la plataforma completa esta lista.
- Que hace: consolida checklist por componente en `ready=0/1`.
- Ruta: `bloque12/practica/resueltos/e10_integration_readiness_report`

---

## Complejos (3)

## C01 - pipeline registry/gateway/monitor
- Objetivo: coordinar discovery, ruteo y observabilidad en flujo estable.
- Que debe hacer: reconciliacion de rutas, metricas y fallback.
- Ruta: `bloque12/practica/complejos/c01_registry_gateway_monitor_pipeline`

## C02 - distribucion de secretos vault/runner
- Objetivo: entregar secretos de forma segura durante ejecucion de jobs.
- Que debe hacer: autorizacion por scope, TTL, rotacion y auditoria.
- Ruta: `bloque12/practica/complejos/c02_vault_runner_secret_distribution`

## C03 - control plane minicloud
- Objetivo: orquestar ciclo de vida y readiness global del stack.
- Que debe hacer: validar config, reconciliar estado y reportar drift.
- Ruta: `bloque12/practica/complejos/c03_minicloud_control_plane_skeleton`

---

## Notas

1. Cada ejercicio incluye `README.md`, `src/`, `tests/`, `Makefile`.
2. Los resueltos incluyen comentarios explicativos en el codigo.
3. Los complejos son plantillas guiadas para implementacion propia.
