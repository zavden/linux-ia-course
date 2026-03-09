# C02 - distribucion de secretos vault/runner (skeleton)

## Objetivo
Diseñar flujo seguro de distribucion y rotacion de secretos hacia jobs del runner.

## Que debe hacer
- Resolver secretos por scope y job identity.
- Entregar material secreto con TTL y politicas de renovacion.
- Revocar secretos al finalizar job o en incidente.
- Auditar cada acceso y rotacion.

## Pistas
- Separa metadatos de secreto y payload sensible.
- Evita logs de valores secretos: solo IDs y huellas.
- Implementa simulador de rotacion y expiracion para pruebas.
