# THEORY.md - Bloque 10: Servicios de Red en Linux/C

Este bloque conecta programacion de sistemas con operacion de servicios de red reales.
El foco no es solo abrir sockets, sino **entender protocolos, configuracion, seguridad y observabilidad**
de DNS, HTTPS, SMTP y comparticion de archivos (NFS/Samba), ademas de patrones de proxy inverso.

---

## 1. Panorama de servicios de red

En un host Linux de produccion, los servicios no viven aislados:
- DNS resuelve nombres
- HTTPS expone APIs
- SMTP enruta notificaciones
- NFS/Samba comparten archivos
- proxies conectan clientes con backends

Un fallo en uno impacta a los demas.
Por eso un ingeniero de sistemas en C debe poder:
1. parsear configuraciones
2. validar politicas
3. diagnosticar estados
4. automatizar chequeos

---

## 2. DNS operativo: modelo y riesgos

### 2.1 Zona DNS y tipos de registro
Registros clave:
- `SOA`: metadatos de autoridad (serial, refresh, retry, expire)
- `NS`: servidores autoritativos
- `A` / `AAAA`: direccion IPv4/IPv6
- `CNAME`: alias
- `MX`: enrutamiento de correo
- `TXT`: metadata libre (SPF, verificaciones, etc.)

### 2.2 Reglas semanticas importantes
1. un `CNAME` no debe coexistir con otros tipos en el mismo owner
2. target de `MX` debe resolver (idealmente A/AAAA)
3. serial del `SOA` debe ser monotono para replicas
4. TTLs extremos pueden causar latencia de convergencia o carga innecesaria

### 2.3 Parseo robusto de zona
Buenas practicas al parsear archivos BIND:
- ignorar comentarios `;`
- tolerar orden variable de campos (owner, ttl, class, type)
- validar tokenizacion sin asumir formato perfecto
- reportar errores con linea y causa

---

## 3. HTTPS/TLS: capas y decisiones

### 3.1 Separacion conceptual
- HTTP define semantica de requests/responses
- TLS encapsula canal cifrado y autenticado

Un gateway HTTPS en C necesita dominar ambos niveles.

### 3.2 Baseline TLS minimo
Politicas comunes actuales:
- permitir `TLSv1.2` y `TLSv1.3`
- deshabilitar `TLSv1.0`/`TLSv1.1`/SSL heredado
- evitar suites obsoletas (`RC4`, `3DES`, `NULL`, `MD5`)
- usar tamaños de clave y curvas modernas

### 3.3 Errores frecuentes
1. aceptar defaults inseguros del runtime sin auditar
2. no validar cadena de certificados al actuar como cliente
3. logs ambiguos de handshake (difícil troubleshooting)
4. mezclar errores HTTP y TLS en la misma capa

---

## 4. Parseo de URL y superficie de ataque

Toda URL externa es input no confiable.
Un parser seguro debe validar:
1. esquema esperado (`https`)
2. host permitido
3. puerto en rango valido
4. path bien formado

Riesgos al no validar:
- SSRF
- redirecciones internas inesperadas
- errores de ruteo a servicios sensibles

---

## 5. SMTP: flujo y estados

### 5.1 Secuencia basica
Sesion SMTP tipica:
1. `EHLO`/`HELO`
2. `MAIL FROM:`
3. `RCPT TO:` (uno o mas)
4. `DATA`
5. cuerpo del mensaje
6. linea `.` de cierre
7. `QUIT`

### 5.2 Respuestas multilinea
Servidor puede responder con:
- `250-...` (continuacion)
- `250 ...` (linea final)

Un parser correcto debe distinguir ambos casos.

### 5.3 Seguridad en cliente SMTP
- validar codigos esperados por estado
- evitar header injection
- controlar tamaño de mensaje y timeout
- preferir STARTTLS/TLS cuando aplique

---

## 6. NFS: modelo de exportacion

Archivo `/etc/exports` define:
- path exportado
- clientes permitidos
- opciones por cliente (`rw`, `ro`, `sync`, `root_squash`, etc.)

### 6.1 Opciones criticas
- `rw` vs `ro`
- `root_squash` (recomendado) vs `no_root_squash` (riesgo)
- `no_subtree_check` y `sync/async`

### 6.2 Riesgos operativos
1. exponer export a subredes muy amplias
2. usar `no_root_squash` sin necesidad real
3. mezclar politicas sin inventario centralizado

---

## 7. Samba/CIFS: shares y politicas

En `smb.conf`, cada seccion de share define controles como:
- `read only`
- `writable`
- `guest ok`
- `browseable`

Auditorias utiles:
- cuantas shares son escribibles
- cuantas permiten guest
- cuantas estan ocultas (`browseable = no`)

Problemas comunes:
- configuraciones contradictorias (`read only` vs `writable`)
- defaults implícitos no documentados
- permisos FS subyacentes incoherentes con Samba

---

## 8. Reverse proxy: idea de control-plane y data-plane

Un reverse proxy serio separa:
- **control-plane**: config, health-check, routing table
- **data-plane**: forwarding de trafico

### 8.1 Longest Prefix Match
Regla clasica en ruteo HTTP:
- elegir prefijo de ruta mas especifico

Ejemplo:
- `/api/v1` gana sobre `/api`
- `/api` gana sobre `/`

### 8.2 Seleccion de backend
Opciones comunes:
- round-robin
- least-connections
- least-latency

Siempre condicionada por estado de salud del backend.

---

## 9. Health checks y degradacion controlada

Sin health checks, el proxy envia trafico a backends caidos.

Patron recomendado:
1. chequeo periodico activo (TCP/HTTP)
2. umbral de fallos antes de marcar DOWN
3. umbral de exitos para recuperar
4. output de estado estable para monitoreo

Estados utiles:
- `OK`
- `WARN`
- `CRIT`

---

## 10. Timeouts y limites defensivos

Todo servicio de red necesita limites para no morir por abuso:
- timeout de handshake
- timeout de lectura/escritura
- tamaño maximo de headers/cuerpo
- limite de conexiones simultaneas

Sin limites, un cliente lento o malicioso puede agotar recursos.

---

## 11. Validacion de configuracion antes de aplicar

Configuracion invalida en produccion suele ser incidente.

Checklist de aplicacion segura:
1. parsear y validar en memoria
2. verificar referencias cruzadas (ruta->backend existente)
3. rechazar config parcial o inconsistente
4. aplicar de forma atomica (swap de estructura validada)

---

## 12. Logging y trazabilidad en servicios de red

Logs minimos por request/sesion:
- timestamp
- origen
- metodo/comando
- destino/backend
- codigo resultado
- latencia

Errores de logging comunes:
1. logs demasiado verbosos sin estructura
2. no incluir contexto de fallo
3. loggear secretos o payload sensible
4. mezclar mensajes de control y datos sin etiqueta

---

## 13. Diseño de parser en C para protocolos/config

Patron robusto:
1. normalizar linea
2. tokenizar con limites
3. validar cardinalidad de campos
4. validar formato y rango
5. acumular en estructura interna
6. evaluar reglas semanticas

No confundir parser sintactico con validador semantico.
Ambos son necesarios.

---

## 14. Pruebas en servicios de red

Cobertura recomendada:
- casos validos basicos
- casos invalidos por orden/protocolo
- edge cases (lineas vacias, comentarios, limites)
- referencias rotas (backend faltante, registro inexistente)

Para ejercicios del curso:
- fixtures textuales reproducibles
- salidas estables para `grep`/CI
- separacion clara entre codigo y datos de prueba

---

## 15. Errores recurrentes en este bloque

1. asumir formato fijo de archivos de servicio
2. no validar transiciones de estado en protocolos (SMTP)
3. ignorar configuraciones inseguras heredadas (TLS/NFS/Samba)
4. resolver rutas con primer match en vez de longest-prefix
5. no diferenciar health local vs salud global

---

## 16. Checklist antes de cerrar una herramienta de servicios

1. ¿El parser tolera entradas reales con variaciones razonables?
2. ¿Hay validaciones semanticas, no solo de forma?
3. ¿Se identifican configuraciones de alto riesgo?
4. ¿La salida es automatizable (estable y legible)?
5. ¿Se cubren tests negativos y positivos?

---

## 17. Mapa de practica del bloque

### Resueltos
- `e01_dns_zone_parser_basico`: conteo de registros DNS de zona.
- `e02_dns_mx_selector`: seleccion de MX preferido por prioridad.
- `e03_tls_policy_linter`: validacion de baseline TLS.
- `e04_https_url_parser`: parser y validacion de URL HTTPS.
- `e05_smtp_flow_validator`: maquina de estados para sesion SMTP.
- `e06_smtp_reply_multiline_parser`: parseo de respuestas multilinea SMTP.
- `e07_nfs_exports_parser`: auditoria basica de `exports` NFS.
- `e08_samba_smbconf_parser`: auditoria basica de shares Samba.
- `e09_reverse_proxy_route_matcher`: longest-prefix match para proxy.
- `e10_service_health_aggregator`: consolidacion de estado de servicios.

### Complejos
- `c01_minidns_zone_compiler`: compilador/validador semantico de zonas.
- `c02_tls_http_gateway`: gateway HTTPS con ruteo y hardening.
- `c03_miniproxy_reverse_router`: proxy inverso con health checks.

---

## 18. Relacion con operacion real

Este bloque prepara tareas practicas de SysAdmin/SRE/DevOps:
- revisar postura de configuracion DNS/TLS/SMTP/NFS/Samba
- automatizar chequeos previos a despliegues
- detectar drift de configuracion y riesgos operativos
- construir herramientas de soporte para troubleshooting en C

Cuando estos fundamentos estan bien hechos, los incidentes de red pasan de
"misteriosos" a "diagnosticables y corregibles".
