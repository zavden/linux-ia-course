# THEORY.md - Bloque 09: Seguridad en Linux/C (Hardening, Integridad y Defensa)

Este bloque introduce seguridad aplicada a software de sistemas en C.
La meta no es solo que el programa "funcione", sino que sea **resistente a abuso**,
**auditable** y **operable en entornos reales**.

---

## 1. Modelo mental de seguridad en C/Linux

En C, cualquier error de validacion o memoria puede escalar a bug critico.
Por eso conviene pensar en capas:

1. **Superficie de ataque**: entradas CLI, archivos, sockets, entorno, syscalls.
2. **Controles preventivos**: validacion, minimo privilegio, limites de recursos.
3. **Controles detectivos**: logs, trazas, checks de integridad.
4. **Controles correctivos**: fail-safe, rollback, bloqueo de acciones riesgosas.

Principio central: cada dato externo es hostil hasta demostrar lo contrario.

---

## 2. Minimo privilegio y Linux capabilities

### 2.1 Por que capabilities y no root total
Tradicionalmente, `root` tiene privilegio global. Eso amplifica impacto de fallos.
Las capabilities dividen privilegios en bits finos (`CAP_NET_BIND_SERVICE`, `CAP_NET_RAW`, etc.).

Beneficio:
- proceso con privilegio minimo para su tarea
- menor blast radius si hay compromiso

### 2.2 Conjuntos relevantes
Aunque hay varios sets (effective/permitted/inheritable/ambient/bounding),
para diagnostico rapido suele mirarse `CapEff`.

Regla practica:
- auditar mascara actual
- conservar solo lo estrictamente necesario
- dropear el resto lo antes posible

### 2.3 Errores frecuentes
1. arrancar como root y nunca reducir privilegios
2. mantener `CAP_NET_RAW` sin necesidad
3. asumir que "funciona" implica "esta seguro"

---

## 3. Contexto de politicas: SELinux y AppArmor

Capabilities controlan privilegios por proceso, pero no sustituyen MAC.
SELinux/AppArmor agregan politicas de confinamiento obligatorias.

### 3.1 SELinux
Conceptos utiles:
- estado global (enabled/disabled)
- modo (`enforcing` vs `permissive`)
- contextos y tipos (dominio/objeto)

En produccion, `permissive` suele ser estado transitorio de ajuste.

### 3.2 AppArmor
Conceptos utiles:
- perfiles por binario
- modo `enforce` o `complain`
- alcance por rutas/recursos permitidos

### 3.3 Implicacion operativa
Un auditor serio debe leer estado de LSM y cruzarlo con privilegios del proceso.
No basta con revisar un solo control aislado.

---

## 4. Integridad criptografica: hash vs HMAC

### 4.1 Hash (SHA-256)
Propiedades esperadas:
- determinista
- sensible a cambios minimos en entrada
- resistente a colisiones para uso practico moderno

Uso tipico:
- checksum de artefactos
- huella de contenido

### 4.2 HMAC-SHA256
HMAC combina hash + clave secreta compartida.
No solo detecta cambios: tambien autentica origen (quien conoce la clave).

Regla:
- para integridad no autenticada, hash puede bastar
- para integridad autenticada, usar HMAC

### 4.3 Errores comunes
1. comparar MAC con `strcmp` y filtrar timing
2. mezclar claves en texto plano en codigo
3. truncar MAC sin politica clara
4. reutilizar misma clave para funciones distintas sin separacion

---

## 5. Comparacion de secretos y side channels de tiempo

Comparadores ingenuos suelen cortar al primer byte distinto.
Eso puede filtrar prefijos correctos por diferencia de tiempo acumulada.

Mitigacion:
- comparar siempre longitud maxima relevante
- acumular diferencias en variable intermedia
- decidir al final

Nota importante:
comparacion constante ayuda, pero no elimina todos los side channels.
Sigue siendo necesario hardening global del proceso.

---

## 6. Validacion estricta de entrada

Validar no es solo "que no crashee": es aplicar **politica explicita**.

Checklist minimo por campo:
1. formato permitido (charset/regex simple)
2. longitud minima y maxima
3. rango numerico
4. conversion sin residuos (`endptr` al final)
5. semantica del dominio (ej. puerto 1..65535)

Errores tipicos en C:
- usar `atoi` (sin deteccion robusta de error)
- aceptar basura al final (`"80abc"`)
- ignorar signos o overflows de conversion

---

## 7. Integer overflow y tamanos de memoria

Muchos bugs criticos nacen en calculos como:
- `bytes = n * elem_size`
- `offset = base + index * stride`

Si overflowea, el buffer asignado puede ser menor al esperado.
Luego aparecen escrituras fuera de limites.

Patron seguro:
1. validar parseo hacia tipo destino
2. chequear overflow antes de multiplicar/sumar
3. reservar memoria solo con tamano validado
4. mantener checks de limites en lecturas/escrituras

---

## 8. Path traversal y composicion segura de rutas

Riesgo clasico:
- entrada: `../../etc/shadow`
- codigo vulnerable: `root + "/" + input`

Defensa basica:
1. exigir ruta relativa para input
2. normalizar `.` y `..`
3. rechazar escapes por encima de raiz logica
4. filtrar segmentos invalidos

Defensa avanzada (produccion):
- usar `openat` con directorio base
- considerar `O_NOFOLLOW` para reducir riesgo de symlink attacks
- controlar TOCTOU (no verificar y usar en pasos separados inseguros)

---

## 9. Autenticacion en Linux con PAM (vision general)

PAM separa politica de autenticacion del codigo de aplicacion.
Tu binario puede delegar autenticacion al stack del sistema.

Buenas practicas:
- encapsular capa PAM detras de interfaz propia (testable)
- no exponer mensajes sensibles al cliente final
- agregar rate-limit/lockout para fuerza bruta
- auditar intentos con contexto util (sin volcar secretos)

---

## 10. Hardening de compilacion y enlazado

Flags frecuentes para elevar baseline defensivo:
- `-Wall -Wextra -Werror -pedantic`
- `-fstack-protector-strong`
- `-D_FORTIFY_SOURCE=2` (con optimizacion adecuada)
- `-fPIE` + `-pie`
- `-Wl,-z,relro,-z,now` (en Linux ELF)

Y en desarrollo:
- sanitizers (`-fsanitize=address,undefined`)
- builds separadas debug/release

Objetivo: detectar antes en desarrollo lo que en produccion seria incidente.

---

## 11. Manejo de secretos en memoria y logs

Reglas practicas:
1. no loggear secretos ni tokens completos
2. minimizar tiempo de vida de secretos en memoria
3. limpiar buffers sensibles cuando ya no se usan
4. evitar copias innecesarias
5. separar claves de datos de usuario

En cursos iniciales esto se simplifica, pero en produccion es obligatorio.

---

## 12. Diseño de auditor de seguridad

Arquitectura recomendada:
1. **Collectors**: capabilities, LSM, config local, estado runtime
2. **Rules**: politicas y umbrales por entorno
3. **Reporter**: evidencia + severidad (`OK/WARN/CRIT`)

Criterios de calidad:
- tolera fuentes faltantes sin caer
- salida estable para automatizacion
- evidencia clara por hallazgo
- versionado de formato de salida

---

## 13. Errores recurrentes en proyectos C de seguridad

1. confundir "sin crash" con "seguro"
2. parsear sin validar longitudes
3. tratar strings como confiables por venir de archivo local
4. usar comparadores no constantes para secretos
5. no revisar overflows al calcular buffers
6. mezclar log de negocio con informacion sensible
7. carecer de tests negativos (solo happy-path)

---

## 14. Estrategia de testing para seguridad

El bloque debe testear al menos:
- casos validos (comportamiento esperado)
- casos invalidos controlados (fail-safe)
- limites (longitudes, rangos, valores extremos)
- formatos corruptos o truncados

Enfasis pedagogico:
si una validacion no tiene test negativo, aun no esta cerrada.

---

## 15. Checklist previo a entrega

1. ¿Las entradas externas se validan con politica explicita?
2. ¿Los calculos de tamano tienen checks de overflow?
3. ¿Se evita path traversal al componer rutas?
4. ¿Se comparan secretos en tiempo constante?
5. ¿Se minimizan privilegios y se audita estado de seguridad?
6. ¿Los logs son utiles sin filtrar secretos?
7. ¿Hay tests de error, no solo de exito?

---

## 16. Mapa de practica del bloque

### Resueltos
- `e01_capability_status_parser`: parseo de `CapEff` y bits relevantes.
- `e02_capability_drop_planner`: plan de keep/drop de capabilities.
- `e03_selinux_apparmor_parser`: lectura de estado LSM.
- `e04_sha256_basico`: hash SHA-256 en C puro.
- `e05_hmac_integridad_basica`: HMAC-SHA256 para integridad autenticada.
- `e06_secure_input_validation`: validacion estricta de usuario/puerto.
- `e07_checked_allocation_overflow`: multiplicacion segura para asignaciones.
- `e08_path_traversal_guard`: normalizacion y bloqueo de traversal.
- `e09_constant_time_token_check`: comparacion de token sin early-return.
- `e10_minivault_record_parser`: parser robusto de metadatos seguros.

### Complejos
- `c01_minivault_cli_secure_storage`: CLI de secretos con integridad y politicas.
- `c02_pam_auth_gateway`: gateway de autenticacion + controles defensivos.
- `c03_security_policy_auditor`: auditor de postura de seguridad por reglas.

---

## 17. Relacion con operacion real

Este bloque conecta directo con tareas reales de SysAdmin/SRE/SecOps:
- reducir privilegios de procesos de servicio
- verificar postura de host (LSM + politicas)
- detectar alteraciones de datos con HMAC
- cerrar rutas comunes de explotacion en C
- construir herramientas auditables para cumplimiento y respuesta a incidentes

La idea final es simple: en C/Linux, seguridad no es un modulo aparte;
es una propiedad de diseño que debe estar presente desde la primera linea.
