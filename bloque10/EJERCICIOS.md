# EJERCICIOS.md - Bloque 10 (Indice)

Este archivo es solo mapa de ejercicios.
No incluye enunciados largos ni solucion embebida.

Practica completa en `bloque10/practica/`.

---

## Resueltos (10)

## E01 - parser basico de zona DNS
- Objetivo: extraer inventario de registros desde formato tipo BIND.
- Que hace: cuenta `A`, `AAAA`, `CNAME`, `MX`, `NS`, `SOA`, `TXT`.
- Ruta: `bloque10/practica/resueltos/e01_dns_zone_parser_basico`

## E02 - selector de MX preferido
- Objetivo: elegir host de correo con menor preferencia MX.
- Que hace: filtra registros de una zona y selecciona prioridad minima.
- Ruta: `bloque10/practica/resueltos/e02_dns_mx_selector`

## E03 - linter de politica TLS
- Objetivo: validar baseline de seguridad para HTTPS.
- Que hace: revisa protocolos, ciphers y tamaño minimo de clave RSA.
- Ruta: `bloque10/practica/resueltos/e03_tls_policy_linter`

## E04 - parser de URL HTTPS
- Objetivo: descomponer URL segura en componentes operativos.
- Que hace: valida esquema y extrae host, puerto y path.
- Ruta: `bloque10/practica/resueltos/e04_https_url_parser`

## E05 - validador de flujo SMTP
- Objetivo: verificar orden correcto de comandos en sesion SMTP.
- Que hace: aplica maquina de estados para `EHLO/MAIL/RCPT/DATA/QUIT`.
- Ruta: `bloque10/practica/resueltos/e05_smtp_flow_validator`

## E06 - parser de respuestas SMTP multilinea
- Objetivo: interpretar codigos SMTP con lineas de continuacion.
- Que hace: detecta saludo, resultado final y capacidades anunciadas.
- Ruta: `bloque10/practica/resueltos/e06_smtp_reply_multiline_parser`

## E07 - parser de exports NFS
- Objetivo: auditar reglas de comparticion en `/etc/exports`.
- Que hace: cuenta hosts `rw/ro` y detecta `no_root_squash`.
- Ruta: `bloque10/practica/resueltos/e07_nfs_exports_parser`

## E08 - parser de smb.conf Samba
- Objetivo: auditar postura de shares Samba.
- Que hace: cuenta shares escribibles, guest y no browseables.
- Ruta: `bloque10/practica/resueltos/e08_samba_smbconf_parser`

## E09 - matcher de rutas de reverse proxy
- Objetivo: mapear path entrante a backend usando prefijos.
- Que hace: aplica longest-prefix-match y resuelve `target host:port`.
- Ruta: `bloque10/practica/resueltos/e09_reverse_proxy_route_matcher`

## E10 - agregador de salud de servicios
- Objetivo: consolidar estado operativo de servicios de red.
- Que hace: resume `ok/warn/crit` y calcula severidad global.
- Ruta: `bloque10/practica/resueltos/e10_service_health_aggregator`

---

## Complejos (3)

## C01 - minidns zone compiler
- Objetivo: compilar y validar zonas DNS con chequeos semanticos.
- Que debe hacer: parser robusto, normalizacion y reporte de errores.
- Ruta: `bloque10/practica/complejos/c01_minidns_zone_compiler`

## C02 - TLS HTTP gateway
- Objetivo: terminar TLS y enrutar trafico HTTP de forma segura.
- Que debe hacer: listener HTTPS, forwarding, limites y observabilidad.
- Ruta: `bloque10/practica/complejos/c02_tls_http_gateway`

## C03 - miniproxy reverse router
- Objetivo: resolver rutas y backends con health checks activos.
- Que debe hacer: config valida, seleccion de backend y reporte de estado.
- Ruta: `bloque10/practica/complejos/c03_miniproxy_reverse_router`

---

## Notas

1. Cada ejercicio incluye `README.md`, `src/`, `tests/`, `Makefile`.
2. Los resueltos incluyen comentarios explicativos en el codigo.
3. Los complejos son plantillas guiadas para implementacion propia.
