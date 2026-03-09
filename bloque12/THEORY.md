# THEORY.md - Bloque 12: Proyecto Final Integrador (MiniCloud)

Este bloque une todo lo construido en el curso: C de sistemas, Linux, red, seguridad,
almacenamiento, observabilidad y runtime de bajo nivel. La meta no es un demo aislado,
sino un sistema coherente con varios servicios colaborando.

---

## 1. Vision del bloque final

El proyecto final representa una plataforma minima de microservicios en C:
- `registry`: discovery de servicios
- `gateway`: entrada de trafico y ruteo
- `monitor`: metricas y salud operacional
- `vault`: gestion de secretos
- `runner`: ejecucion de workloads

Valor pedagogico:
1. integrar componentes con contratos claros
2. operar en condiciones degradadas
3. auditar estado global de la plataforma

---

## 2. Arquitectura por planos

Para evitar acoplamiento excesivo, separa dos planos:

1. **Data plane**
- trafico de requests reales
- latencia y throughput como foco principal

2. **Control plane**
- configuracion, discovery, health, politicas
- consistencia y trazabilidad como foco principal

Error comun: mezclar ambos planos en el mismo flujo sin limites claros.

---

## 3. Contratos entre servicios

Cada servicio debe exponer contratos explicitos:
- esquema de request/response
- codigos de estado
- timeouts esperados
- version de protocolo

Sin contrato versionado, cualquier cambio rompe integracion.

Buenas practicas:
- validar entrada en frontera
- no asumir defaults silenciosos
- fallar temprano con error accionable

---

## 4. Service discovery (registry)

`registry` mantiene mapa de instancias activas.
Datos minimos:
- servicio
- endpoint
- estado
- timestamp de ultimo heartbeat

Riesgos:
1. aceptar heartbeats sin validar identidad
2. no expirar entradas obsoletas
3. snapshot inconsistente en lecturas concurrentes

Patron recomendado:
- estado inmutable por version
- snapshot atomico para consumidores

---

## 5. Gateway y ruteo

`gateway` aplica reglas de ruteo de entrada a backend.

Puntos clave:
- longest-prefix-match para rutas
- rechazar backend no saludable
- timeout y retry con limites
- trazabilidad por request (request-id)

Riesgos:
- ruteo ambiguo por reglas solapadas
- loops de retry sin budget
- latencias no acotadas en fallos parciales

---

## 6. Observabilidad y monitor

`monitor` no solo "muestra metricas"; habilita decisiones operativas.

Señales esenciales:
- latencia p50/p95/p99
- error rate
- saturacion (CPU, memoria, colas)
- disponibilidad por componente

Principio:
si no puedes medirlo de forma estable, no puedes operarlo con confianza.

---

## 7. Secretos y vault

`vault` gestiona material sensible (tokens, claves, credenciales).

Reglas base:
- minimo privilegio por consumidor
- TTL de secretos y rotacion
- auditoria de acceso sin filtrar valores
- separacion metadata vs payload

Errores criticos comunes:
1. loggear secretos por debug
2. secretos sin expiracion
3. reutilizar credenciales globales para todos los jobs

---

## 8. Runner y ejecucion controlada

`runner` ejecuta trabajos en entorno aislado.
Debe validar job specs antes de correr:
- imagen/comando
- limites CPU/memoria
- timeout
- secretos permitidos

Integracion con bloque 11:
- namespaces para aislamiento
- cgroups para limites
- seccomp/capabilities para hardening

---

## 9. Readiness global vs salud local

Un servicio puede estar "up" localmente y aun asi no estar listo para produccion.

Distinguir:
- **liveness**: proceso vive
- **readiness**: proceso puede atender correctamente
- **global readiness**: plataforma completa lista

Ejemplo:
`gateway` vivo pero `registry` inconsistente => globalmente no listo.

---

## 10. Politicas y auditoria continua

El control plane debe evaluar politicas tecnicas de forma automatica:
- TLS obligatorio
- mTLS interno (si aplica)
- rotacion de secretos
- backups activos
- intervalos de salud razonables

Salida esperada:
- `OK`, `WARN`, `CRIT`
- evidencia por check
- accion sugerida

---

## 11. Gestión de configuracion

Config en sistemas distribuidos debe ser:
- declarativa
- validada antes de aplicar
- versionada
- aplicable en forma atomica

Patron de despliegue seguro:
1. parse/validate
2. build snapshot nuevo
3. swap atomico
4. rollback si falla

---

## 12. Manejo de fallos y degradacion

No diseñes solo para happy-path.
Escenarios base:
- backend caido
- timeout en dependencia
- config invalida
- secretos expirados
- metricas incompletas

El sistema robusto degrada de forma controlada y explicable.

---

## 13. Concurrencia y consistencia

Aunque cada componente sea pequeno, la concurrencia aparece rapido:
- updates de registry mientras gateway enruta
- monitor leyendo metricas durante cambios de config
- rotacion de secretos en jobs activos

Buenas practicas:
- estructuras inmutables por version
- ownership claro de memoria
- puntos de sincronizacion acotados

---

## 14. Pruebas del integrador

Tres capas de testing recomendadas:
1. unitarias de parser/reglas
2. integracion por componente
3. end-to-end multi-servicio

En bloque 12, prioriza:
- fixtures reproducibles
- contratos estables
- pruebas de degradacion

---

## 15. Errores recurrentes en proyectos finales

1. implementar componentes sin contrato definido
2. acoplar todo por llamadas directas sin frontera clara
3. no versionar formatos de intercambio
4. no medir latencia/errores desde el inicio
5. ignorar limpieza y rollback en fallos parciales

---

## 16. Checklist de entrega final

1. ¿Cada componente tiene contrato y validacion de entrada?
2. ¿Existe ruta de diagnostico para fallos comunes?
3. ¿Hay metricas suficientes para operar el sistema?
4. ¿Se gestionan secretos con TTL y auditoria?
5. ¿El readiness global refleja dependencias reales?
6. ¿La configuracion se aplica de forma segura y reversible?

---

## 17. Mapa de practica del bloque

### Resueltos
- `e01_service_contract_parser`: validacion de contratos de servicio.
- `e02_registry_snapshot_builder`: snapshot compacto de discovery.
- `e03_gateway_route_planner`: plan de ruteo con estado de backends.
- `e04_health_status_reducer`: reduccion de salud por severidad.
- `e05_metrics_line_protocol_parser`: resumen de latencia/error rate.
- `e06_secret_manifest_validator`: validacion de metadatos de secretos.
- `e07_runner_job_spec_validator`: validacion de job specs del runner.
- `e08_event_bus_log_parser`: señales operativas desde eventos.
- `e09_minicloud_policy_auditor`: auditoria de politicas globales.
- `e10_integration_readiness_report`: decision de readiness integral.

### Complejos
- `c01_registry_gateway_monitor_pipeline`: pipeline control/data plane.
- `c02_vault_runner_secret_distribution`: distribucion segura de secretos.
- `c03_minicloud_control_plane_skeleton`: esqueleto de control plane final.

---

## 18. Relacion con el proyecto-final del bloque

La carpeta `bloque12/proyecto-final/` es el destino natural de este bloque.
Los ejercicios de `practica/` sirven como piezas base para acelerar ese proyecto:
- parsers y validadores como librerias internas
- reglas de auditoria reutilizables
- criterios de readiness listos para integracion

Objetivo final: pasar de utilidades sueltas a una plataforma coherente y operable.
