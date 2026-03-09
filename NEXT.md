# NEXT.md - 5 proyectos premium para portafolio (C + Linux)

Este documento define 5 proyectos para pasar de "curso tecnico fuerte" a "perfil que se ve profesional en entrevistas".
La idea no es solo programar: es entregar proyectos con arquitectura clara, pruebas serias, metrica, seguridad y demo reproducible.

-------------------------------------------------------------------------------
Barra minima de calidad para TODOS los proyectos
-------------------------------------------------------------------------------

1) Repositorio limpio y profesional
- `README.md` con problema, arquitectura, como correr, resultados y roadmap.
- `docs/` con diseno tecnico, ADRs (decisiones de arquitectura) y protocolo.
- `Makefile` con `all`, `clean`, `test`, `lint`, `run`, `docker`.
- `tests/` unitarios + integracion + carga + regresion.
- `scripts/` para setup, demo y benchmark.

2) Calidad de codigo
- Compilar con `-Wall -Wextra -Werror -pedantic -std=c17`.
- Sin fugas de memoria (valgrind limpio en paths criticos).
- Manejo de errores consistente (sin `exit()` silencioso).
- Logs estructurados con timestamp, nivel y request-id/correlation-id.

3) Evidencia de ingenieria
- CI (GitHub Actions o similar) ejecutando build y tests.
- Benchmarks reproducibles con scripts y resultados en `docs/benchmarks/`.
- Cobertura minima de pruebas en logica core (aunque sea manual + reportada).
- Checklist de seguridad y hardening por proyecto.

4) Entregable para entrevista
- Video demo de 5 a 8 minutos.
- "Runbook" para reproducir la demo en menos de 10 minutos.
- Seccion "What I would improve with 4 more weeks".

-------------------------------------------------------------------------------
Proyecto 1 - MiniCloud Pro (orquestador distribuido HTTP)
-------------------------------------------------------------------------------

Objetivo de portafolio
- Evolucionar tu base de `bloque14` a una plataforma pequena de orquestacion con control-plane y data-plane.
- Demostrar sockets, concurrencia, procesos, observabilidad, resiliencia y arquitectura distribuida.

Problema real que resuelve
- Recibir "jobs", enrutar a nodos disponibles, registrar estado, manejar fallos y exponer metricas.

Stack tecnico sugerido
- Lenguaje: C17.
- IPC/red: sockets TCP, HTTP/1.1 minimal.
- Seguridad: API keys (MVP), TLS con OpenSSL (fase avanzada).
- Infra: Docker Compose para todos los servicios.
- Testing: Bash + binarios C de prueba + carga con `hey` o `wrk`.

Arquitectura minima (6 servicios)
1. `api-gateway`: recibe requests del cliente y valida entrada.
2. `scheduler`: decide nodo destino (round-robin + capacidad).
3. `node-agent`: ejecuta jobs y reporta estado.
4. `registry`: descubre servicios y health de instancias.
5. `vault`: entrega secretos por nombre con control de acceso.
6. `monitor`: guarda eventos y genera reporte de estado.

Contrato API (MVP)
- `POST /jobs` -> crea job.
- `GET /jobs/<id>` -> estado del job.
- `GET /health` en todos los servicios.
- `GET /metrics` en scheduler, gateway y node-agent.

Modelo de datos recomendado
- `job`: `id`, `name`, `cpu`, `mem`, `secret_ref`, `status`, `created_at`, `updated_at`.
- `node`: `id`, `host`, `port`, `capacity_cpu`, `capacity_mem`, `used_cpu`, `used_mem`, `status`.
- `event`: `ts`, `service`, `severity`, `message`, `job_id`.

Roadmap por sprints (6 semanas, 1 sprint por semana)
1. Sprint 1: protocolo HTTP comun + libreria de utilidades (parse, log, errores).
2. Sprint 2: registry + monitor funcionales con tests de contrato.
3. Sprint 3: vault + scheduler basico (round-robin).
4. Sprint 4: node-agent con ejecucion real de procesos (`fork/exec`, timeouts).
5. Sprint 5: gateway integral + pruebas end-to-end + retries/backoff.
6. Sprint 6: hardening, benchmark, docs y demo final.

Pruebas obligatorias
- Unit: parse HTTP, parse config, manejo de errores.
- Integracion: flujo completo create-job -> schedule -> run -> report.
- Resiliencia: matar un servicio en caliente y validar recovery.
- Carga: 100/500/1000 jobs con reporte de latencia p50/p95.
- Memoria: valgrind en paths de hot path y shutdown.

Metrica/SLO para mostrar
- SLO de disponibilidad del gateway: >= 99% en pruebas locales largas.
- Latencia `POST /jobs` p95 < 150 ms en entorno local controlado.
- Error rate < 1% bajo carga objetivo.

Entregables que "lucen" en CV
- Diagrama de arquitectura en `docs/architecture.md`.
- Tabla de benchmarks (latencia, throughput, error rate).
- Registro de incidentes y correcciones ("postmortems" cortos).
- Script `scripts/demo.sh` que monta todo y corre escenario de falla.

Titulo sugerido para CV/LinkedIn
- "Disene un orquestador distribuido en C con 6 microservicios HTTP, scheduling por capacidad, observabilidad y pruebas de resiliencia."

-------------------------------------------------------------------------------
Proyecto 2 - SafeBackup (backup incremental + deduplicacion + restore)
-------------------------------------------------------------------------------

Objetivo de portafolio
- Demostrar dominio de filesystem, hashing, formatos de almacenamiento, consistencia y recuperacion.

Problema real que resuelve
- Respaldar directorios de forma incremental, deduplicar contenido, restaurar versiones y verificar integridad.

Stack tecnico sugerido
- C17, POSIX file APIs (`open/read/write/lseek/fsync`), `mmap` opcional.
- Hash: SHA-256 (OpenSSL).
- Compresion: gzip/zstd opcional (fase avanzada).
- Cifrado opcional: AES-256-GCM para snapshots sensibles.

CLI objetivo
- `safebackup init <repo>`
- `safebackup snapshot <repo> <path>`
- `safebackup list <repo>`
- `safebackup restore <repo> <snapshot-id> <destino>`
- `safebackup verify <repo> [snapshot-id]`
- `safebackup prune <repo> --keep-last N`

Diseno interno del repositorio
- `repo/objects/<sha256>`: blobs deduplicados.
- `repo/snapshots/<id>.json`: metadatos de snapshot (archivos, permisos, hash, links).
- `repo/index/` para acelerar busquedas.
- `repo/logs/` para auditoria y debugging.

Fases del proyecto
1. MVP: snapshot completo sin dedupe.
2. Incremental: guardar solo cambios entre snapshots.
3. Dedupe por contenido: objetos por hash.
4. Restore parcial: por archivo/ruta.
5. Verify + fsck del repo (deteccion de corrupcion).
6. Prune y politicas de retencion.
7. Cifrado y compresion opcionales.

Casos complejos que debes soportar
- Archivos grandes (>= 2GB) sin romper memoria.
- Symlinks, hardlinks y permisos POSIX.
- Archivos borrados/renombrados entre snapshots.
- Interrupcion en mitad de snapshot (recover transaccional).

Pruebas obligatorias
- Integridad: hash del archivo original == restaurado.
- Repetibilidad: dos restores del mismo snapshot producen mismo estado.
- Stress: arbol con 100k archivos pequenos + algunos grandes.
- Fallos: energia simulada (kill -9 durante snapshot) y recuperacion.

Metrica para mostrar
- Tiempo de snapshot completo vs incremental.
- Ratio de deduplicacion (% de espacio ahorrado).
- Tiempo de restore por tamano de dataset.

Entregables de alto valor
- `docs/format.md` con formato on-disk.
- `docs/recovery.md` con estrategia ante corrupcion.
- Benchmark con datasets sinteticos y reales.

Titulo sugerido para CV/LinkedIn
- "Construí un motor de backup incremental en C con deduplicacion por hash, restore versionado y verificacion de integridad."

-------------------------------------------------------------------------------
Proyecto 3 - RedProxy (reverse proxy + balanceo + cache + rate-limit)
-------------------------------------------------------------------------------

Objetivo de portafolio
- Demostrar redes, rendimiento HTTP, algoritmos de balanceo y controles de seguridad de borde.

Problema real que resuelve
- Exponer varios servicios backend con una sola entrada, controlando carga y ataques basicos.

Funcionalidad objetivo
1. Reverse proxy por host/path.
2. Balanceo: round-robin y least-connections.
3. Health checks activos de backends.
4. Cache para `GET` (TTL + invalidez basica).
5. Rate limit por IP (token bucket).
6. Logs de acceso y errores.
7. Modo TLS terminacion (fase avanzada).

Arquitectura
- `listener`: acepta conexiones y parsea request.
- `router`: elige upstream segun reglas.
- `balancer`: selecciona backend segun algoritmo.
- `proxy core`: reenvia request/respuesta.
- `cache`: memoria + opcion de disco.
- `guard`: rate-limit + reglas basicas anti abuso.

Formato de config recomendado
- `proxy.conf` en formato INI/simple:
  - `listen = 0.0.0.0:8080`
  - `route /api -> backend_pool_api`
  - `rate_limit default = 50rps`
  - `cache /static ttl=60`

Roadmap por fases
1. HTTP proxy minimo sin cache.
2. Multiples backends + round-robin.
3. Least-connections + health checks.
4. Cache GET con TTL.
5. Rate-limit por IP con ventanas y burst.
6. TLS y hardening.
7. Benchmark y tuning.

Pruebas obligatorias
- Correctitud: comparar respuestas directas vs proxied.
- Concurrencia: multiples clientes simultaneos.
- Resiliencia: backend caido, timeout y retry.
- Seguridad: requests malformadas, header flooding, payload raro.
- Rendimiento: throughput y latencia con y sin cache.

Metrica que debe salir en README
- Requests/s con 1, 10, 100 conexiones concurrentes.
- Hit ratio de cache.
- Latencia p50/p95/p99.
- Impacto de rate-limit bajo burst.

Entregables que destacan
- Grafica de mejora con cache activado.
- Tabla comparativa de algoritmos de balanceo.
- Escenario reproducible con `docker compose up` + script de carga.

Titulo sugerido para CV/LinkedIn
- "Implementé un reverse proxy HTTP en C con balanceo inteligente, cache TTL, rate limiting y pruebas de rendimiento."

-------------------------------------------------------------------------------
Proyecto 4 - Sentinel (mini service manager + sandbox de procesos)
-------------------------------------------------------------------------------

Objetivo de portafolio
- Demostrar control de procesos Linux a nivel profesional: daemonizacion, restart policies, aislamiento y limites.

Problema real que resuelve
- Ejecutar servicios de forma supervisada con politicas de reinicio, recursos y seguridad.

Componentes
1. `sentineld`: daemon principal (supervisor).
2. `sentctl`: CLI para operar servicios.
3. `unit parser`: lee archivos de unidad tipo INI.
4. `log engine`: captura stdout/stderr con rotacion.
5. `sandbox layer`: aplica aislamiento y limites.

Features MVP
- Start/stop/restart/status de servicios.
- Politicas `Restart=always|on-failure|never`.
- `ExecStart`, `ExecStop`, `WorkingDirectory`, `Environment`.
- PID tracking y recoleccion correcta de hijos.

Features avanzadas (para portafolio fuerte)
- Limites `RLIMIT_*` por servicio.
- Drop de privilegios (`setuid/setgid`).
- Capabilities minimas.
- Seccomp profile (allowlist de syscalls).
- Chroot y namespaces (si el entorno permite).

Formato de unidad ejemplo
- `units/api.unit`
  - `[Service]`
  - `Name=api`
  - `ExecStart=/opt/api/bin/server --port 9000`
  - `Restart=on-failure`
  - `RestartSec=2`
  - `User=app`
  - `LimitNOFILE=4096`

Roadmap recomendado (7 fases)
1. Parser de units + validacion robusta.
2. Supervisor basico (fork/exec + waitpid loop).
3. CLI y socket de control local.
4. Logs y rotacion.
5. Restart policies y backoff.
6. Sandbox y limites.
7. Testing de caos + docs.

Pruebas clave
- Servicio que falla al inicio (retry correcto).
- Servicio que se cuelga (timeout y reinicio).
- Servicio ruidoso (log rotation sin perder lineas).
- Senales (`SIGTERM`, `SIGHUP`) y shutdown limpio.
- Seguridad: confirmar que limites y usuario aplican de verdad.

Metrica y evidencia
- Mean time to recovery de un servicio fallando.
- Cantidad de reinicios por ventana.
- Overhead del supervisor.

Entregables premium
- `docs/runbook.md` con operacion diaria.
- `docs/security-hardening.md` con decisiones de sandbox.
- Demo "rompo servicio y se recupera solo" en video.

Titulo sugerido para CV/LinkedIn
- "Desarrollé un service manager en C con supervision de procesos, politicas de reinicio y sandbox de seguridad."

-------------------------------------------------------------------------------
Proyecto 5 - OpsRadar (observabilidad + alertas + dashboard TUI)
-------------------------------------------------------------------------------

Objetivo de portafolio
- Demostrar observabilidad end-to-end: recoleccion, almacenamiento, reglas, alertado y visualizacion.

Problema real que resuelve
- Centralizar metricas/logs de varios servicios y detectar incidentes automaticamente.

Componentes
1. `agent`: recolecta metricas del host y servicios.
2. `collector`: recibe eventos via TCP/HTTP y normaliza.
3. `store`: guarda series temporales y eventos.
4. `rule-engine`: evalua umbrales y reglas compuestas.
5. `opsradar-tui`: panel en terminal con ncurses.
6. `alerter`: envia notificaciones (archivo, webhook, email opcional).

Metricas base recomendadas
- CPU uso total y por proceso.
- Memoria RSS/VSZ por servicio.
- Disco: uso, inodos, I/O wait basico.
- Red: conexiones activas, errores de socket.
- Aplicacion: requests/s, error rate, latencia p95.

Formato de eventos sugerido (line-oriented JSON)
- `{ "ts": "...", "service": "gateway", "level": "error", "msg": "...", "trace_id": "..." }`

Reglas de alerta (ejemplos)
- `cpu > 85% durante 5m`.
- `error_rate > 2% durante 3 ventanas`.
- `lat_p95 > 300ms y requests_s > 50`.
- `service_down > 30s`.

Roadmap por sprints
1. Agent + collector con protocolo simple.
2. Store append-only + consultas basicas.
3. Rule-engine con reglas de umbral.
4. Dashboard TUI (top servicios, alertas activas, timeline).
5. Correlacion basica de eventos por trace-id.
6. Export de reportes de incidente en markdown.

Pruebas
- Correctitud de parser y ventana temporal.
- Carga de eventos sostenida (no perder mensajes).
- Reglas con datos sinteticos (evitar falsos positivos obvios).
- Recovery tras reinicio del collector/store.

Demo potente para entrevista
- Levantar stack de ejemplo.
- Inyectar carga artificial en un servicio.
- Ver alerta en TUI y reporte generado automaticamente.
- Mostrar MTTR y pasos de mitigacion en runbook.

Entregables que te hacen destacar
- `docs/incident-playbook.md`.
- `docs/data-retention.md`.
- Benchmark de ingestion (events/s) y latencia de alerta.

Titulo sugerido para CV/LinkedIn
- "Creé una plataforma de observabilidad en C con recoleccion de metricas, motor de alertas y dashboard TUI para incident response."

-------------------------------------------------------------------------------
Orden recomendado para construirlos (maximo impacto)
-------------------------------------------------------------------------------

1. MiniCloud Pro
- Aprovecha trabajo previo y sube complejidad distribuida rapidamente.

2. RedProxy
- Te posiciona fuerte en redes y rendimiento.

3. SafeBackup
- Muestra profundidad de filesystem e integridad de datos.

4. Sentinel
- Refuerza Linux de bajo nivel y operacion real.

5. OpsRadar
- Cierra el ciclo con observabilidad y operacion profesional.

-------------------------------------------------------------------------------
Plan sugerido de 20 semanas (realista)
-------------------------------------------------------------------------------

- Semanas 1-4: MiniCloud Pro (MVP + resiliencia).
- Semanas 5-8: RedProxy (MVP + benchmark serio).
- Semanas 9-12: SafeBackup (snapshot, dedupe, restore).
- Semanas 13-16: Sentinel (supervisor + sandbox).
- Semanas 17-20: OpsRadar (agent, reglas, TUI, reporte).

-------------------------------------------------------------------------------
Checklist final para "presumir" de verdad
-------------------------------------------------------------------------------

- Cada proyecto tiene benchmark y reporte reproducible.
- Cada proyecto tiene pruebas de fallo (no solo happy path).
- Cada proyecto tiene una decision de seguridad explicita.
- Cada proyecto tiene demo grabada y script automatizado.
- Cada README tiene seccion "tradeoffs y deuda tecnica".
- Hay consistencia de estilo entre los 5 repos.
