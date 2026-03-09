# C02 - PAM auth gateway (skeleton)

## Objetivo
Diseñar un gateway de autenticacion en C que delegue identidad al sistema (PAM) y aplique politicas locales.

## Que debe hacer
- Autenticar usuarios con backend PAM.
- Aplicar controles locales (rate-limit, lockout, allowlist).
- Emitir logs auditables sin filtrar secretos.
- Exponer resultado en formato estable para integracion.

## Pistas
- Aisla capa PAM de la capa de negocio para testear sin PAM real.
- Define codigos de estado claros (AUTH_OK, AUTH_DENY, AUTH_ERROR).
- No mezcles mensajes de usuario con logs internos sensibles.
