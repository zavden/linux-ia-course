# EJERCICIOS.md - Bloque 09 (Indice)

Este archivo es solo mapa de ejercicios.
No incluye enunciados largos ni solucion embebida.

Practica completa en `bloque09/practica/`.

---

## Resueltos (10)

## E01 - parser de capabilities en status
- Objetivo: leer mascara efectiva de capabilities y detectar bits criticos.
- Que hace: parsea `CapEff` en hex y reporta capacidades seleccionadas.
- Ruta: `bloque09/practica/resueltos/e01_capability_status_parser`

## E02 - planner de drop de capabilities
- Objetivo: aplicar principio de minimo privilegio sobre mascara actual.
- Que hace: calcula conjuntos `keep/drop` y cuenta bits en cada uno.
- Ruta: `bloque09/practica/resueltos/e02_capability_drop_planner`

## E03 - parser SELinux/AppArmor
- Objetivo: unificar lectura de estado de controles MAC.
- Que hace: extrae modo SELinux y conteos de perfiles AppArmor.
- Ruta: `bloque09/practica/resueltos/e03_selinux_apparmor_parser`

## E04 - SHA-256 basico en C
- Objetivo: entender integridad criptografica implementando hash.
- Que hace: calcula digest SHA-256 y valida vector conocido.
- Ruta: `bloque09/practica/resueltos/e04_sha256_basico`

## E05 - HMAC-SHA256 de integridad
- Objetivo: autenticar integridad con clave compartida.
- Que hace: calcula HMAC y demuestra deteccion de alteracion de mensaje.
- Ruta: `bloque09/practica/resueltos/e05_hmac_integridad_basica`

## E06 - validacion de entrada segura
- Objetivo: aplicar validaciones estrictas de formato y rango.
- Que hace: valida usuario y puerto con parseo numerico robusto.
- Ruta: `bloque09/practica/resueltos/e06_secure_input_validation`

## E07 - asignacion con chequeo de overflow
- Objetivo: prevenir desbordes al calcular tamanos de buffer.
- Que hace: usa multiplicacion protegida antes de reservar memoria.
- Ruta: `bloque09/practica/resueltos/e07_checked_allocation_overflow`

## E08 - guardia contra path traversal
- Objetivo: impedir escapes de ruta fuera de raiz logica.
- Que hace: normaliza ruta relativa y rechaza segmentos inseguros.
- Ruta: `bloque09/practica/resueltos/e08_path_traversal_guard`

## E09 - comparacion de token en tiempo constante
- Objetivo: reducir filtracion por timing en comparacion de secretos.
- Que hace: compara token con estrategia constante y salida verificable.
- Ruta: `bloque09/practica/resueltos/e09_constant_time_token_check`

## E10 - parser de registros minivault
- Objetivo: validar formato de metadatos de secretos.
- Que hace: procesa lineas `user|salt|mac|flags` y cuenta validos/invalidos.
- Ruta: `bloque09/practica/resueltos/e10_minivault_record_parser`

---

## Complejos (3)

## C01 - minivault CLI secure storage
- Objetivo: construir CLI de secretos con integridad y controles de acceso.
- Que debe hacer: comandos de vault, politicas y trazabilidad de seguridad.
- Ruta: `bloque09/practica/complejos/c01_minivault_cli_secure_storage`

## C02 - PAM auth gateway
- Objetivo: delegar autenticacion al sistema con defensas adicionales.
- Que debe hacer: auth PAM + rate-limit + lockout + logging seguro.
- Ruta: `bloque09/practica/complejos/c02_pam_auth_gateway`

## C03 - security policy auditor
- Objetivo: auditar postura de seguridad de host Linux.
- Que debe hacer: collectors, reglas de severidad y reporte accionable.
- Ruta: `bloque09/practica/complejos/c03_security_policy_auditor`

---

## Notas

1. Cada ejercicio incluye `README.md`, `src/`, `tests/`, `Makefile`.
2. Los resueltos incluyen comentarios explicativos en el codigo.
3. Los complejos son plantillas guiadas para implementacion propia.
