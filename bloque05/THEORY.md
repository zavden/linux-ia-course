# THEORY.md — Bloque 05: Hilos y Concurrencia POSIX

Este bloque cubre concurrencia **intra-proceso** con `pthread`.
El objetivo real no es solo "crear hilos", sino diseñar programas correctos bajo interleaving arbitrario del scheduler.

---

## 1. Modelo mental mínimo correcto

### 1.1 Concurrencia vs paralelismo
- **Concurrencia**: múltiples tareas activas con progreso intercalado.
- **Paralelismo**: múltiples tareas ejecutando al mismo tiempo en distintos núcleos.
- Un programa concurrente puede no paralelizar (ej: 1 núcleo), pero igual necesita sincronización.

### 1.2 Proceso vs hilo
- Un proceso tiene espacio de direcciones propio.
- Los hilos de un mismo proceso comparten:
  - heap
  - variables globales/estáticas
  - file descriptors
- Cada hilo tiene:
  - stack propio
  - registros de CPU propios
  - estado de scheduling propio

Implicación práctica: en hilos, compartir datos es barato; **coordinar acceso correcto es lo difícil**.

### 1.3 Qué es una race condition
Hay race condition cuando dos o más hilos acceden al mismo dato, al menos uno escribe, y no hay sincronización adecuada.
Eso produce resultados no deterministas y, en C, puede convertirse en comportamiento indefinido.

Ejemplo clásico:
```c
counter++; // no es atómico
```
`counter++` implica leer-modificar-escribir. Si dos hilos intercalan esa secuencia, se pierden incrementos.

---

## 2. Ciclo de vida de hilos (`pthread`)

### 2.1 Crear
Firma base:
```c
int pthread_create(pthread_t *thread,
                   const pthread_attr_t *attr,
                   void *(*start_routine)(void *),
                   void *arg);
```

Puntos críticos:
- Compilar y linkear con `-pthread`.
- `start_routine` recibe un `void *`.
- Si el hilo necesita múltiples argumentos, agrúpalos en un `struct`.

### 2.2 Esperar finalización (`join`)
```c
int pthread_join(pthread_t thread, void **retval);
```
- Sin `join` (o `detach`) dejas recursos de hilo sin recolectar.
- `join` también actúa como punto de sincronización: garantiza visibilidad de lo hecho por ese hilo antes de continuar.

### 2.3 `detach`
```c
pthread_detach(thread);
```
- Útil para hilos "fire-and-forget".
- Un hilo detached **no** se puede hacer `join`.
- Úsalo solo cuando realmente no necesitas resultado ni control de finalización explícito.

### 2.4 Retorno de resultados desde hilo
Patrones comunes:
1. Escribir en estructura compartida sincronizada.
2. Devolver puntero por `return` y recibirlo en `pthread_join`.
3. Cola thread-safe para publicar resultados.

Si devuelves memoria heap por `join`, quien hace `join` es responsable de `free`.

---

## 3. Sincronización: elegir la primitiva correcta

### 3.1 `pthread_mutex_t` (exclusión mutua)
Garantiza acceso exclusivo a sección crítica.

Patrón correcto:
```c
pthread_mutex_lock(&m);
/* sección crítica */
pthread_mutex_unlock(&m);
```

Reglas:
- Lock y unlock en todos los caminos de control.
- Minimiza tiempo dentro de sección crítica.
- Evita llamar funciones lentas/bloqueantes con lock tomado, salvo necesidad clara.

Errores típicos:
- olvidar unlock en un `return` temprano
- lock doble del mismo mutex sin atributo recursivo
- usar mutex para proteger datos distintos sin criterio (contención innecesaria)

### 3.2 `pthread_cond_t` (espera por condición)
La condvar **no reemplaza** al mutex; se usa junto con él.

Patrón obligatorio:
```c
pthread_mutex_lock(&m);
while (!condicion) {
    pthread_cond_wait(&cv, &m);
}
/* condición verdadera */
pthread_mutex_unlock(&m);
```

Por qué `while` y no `if`:
- wakeups espurios
- competencia entre múltiples consumidores al despertar

`pthread_cond_wait`:
- libera el mutex atómicamente al dormir
- lo readquiere antes de retornar

`signal` vs `broadcast`:
- `signal`: despierta uno
- `broadcast`: despierta todos (útil en cierre/shutdown)

### 3.3 `pthread_rwlock_t` (lectores/escritores)
- Permite múltiples lectores simultáneos.
- Escritor requiere exclusión total.

Cuándo usarlo:
- muchas lecturas, pocas escrituras
- datos relativamente pequeños y coherentes como snapshot

Cuándo no conviene:
- escritura frecuente
- secciones críticas muy cortas donde overhead supera beneficio

### 3.4 Semáforos (`sem_t`)
Semáforo contador: controla "capacidad" (N accesos simultáneos).

Operaciones:
- `sem_wait`: consume token o bloquea
- `sem_post`: devuelve token

Uso típico:
- rate limiting
- pool de recursos limitados (conexiones, slots, workers)

Nota de portabilidad:
- en algunos entornos `sem_init` puede no estar disponible; considera fallback con `sem_open` (nombrado) o mutex+cond.

---

## 4. Diseños concurrentes recurrentes

### 4.1 Productor-consumidor con cola acotada
Componentes mínimos:
- cola circular
- mutex de estado de cola
- condvar `not_empty`
- condvar `not_full`

Beneficios:
- no hay busy-wait
- backpressure natural cuando la cola se llena

### 4.2 Thread pool fijo
Idea:
- N workers persistentes
- hilo(s) productor(es) encolan tareas
- workers duermen cuando no hay tareas

Ventajas:
- evita costo de crear/destruir hilo por tarea
- controla concurrencia máxima
- arquitectura base de servidores reales

### 4.3 Pipeline por etapas
- cada etapa transforma datos y publica a la siguiente
- cada frontera entre etapas suele ser una cola thread-safe
- para finalizar limpio, usa sentinels o flags + broadcast

---

## 5. Deadlocks, starvation y otros problemas reales

### 5.1 Deadlock
Se da cuando hay ciclo de espera entre locks.

Prevención práctica:
- orden global de adquisición de locks
- mantener secciones críticas cortas
- evitar lock anidado si no es imprescindible
- usar `trylock` + rollback en diseños complejos

### 5.2 Starvation
Un hilo progresa poco o nada porque otros monopolizan recurso.
Soluciones dependen del caso:
- colas FIFO
- fairness en scheduler de tareas
- dividir lock global en locks por shard

### 5.3 Livelock
Hilos no se bloquean, pero tampoco avanzan (ej: reintentos infinitos coordinados mal).
Mitigación:
- backoff aleatorio
- límites de reintentos
- protocolos más simples

---

## 6. Memoria y visibilidad (sin entrar a formalismo pesado)

En C concurrente, no basta "escribir" una variable; importa cuándo y cómo otro hilo la ve.
Primitivas POSIX (`mutex`, `condvar`, `rwlock`, `join`) establecen puntos de sincronización que ordenan visibilidad.

Regla práctica segura para este curso:
- Todo dato mutable compartido se accede bajo la misma disciplina de sincronización.
- Si no hay disciplina clara, hay bug potencial aunque "parezca funcionar".

---

## 7. Compilación, diagnóstico y pruebas

### 7.1 Flags de compilación
- `-pthread` obligatorio para `pthread`/semafóricos POSIX en este bloque.
- Mantén `-Wall -Wextra -Werror` para detectar errores temprano.

### 7.2 Herramientas útiles
- `gdb`: depurar bloqueos y estados de hilos.
- Sanitizers (`-fsanitize=thread` cuando esté disponible): detectar races.
- `strace`/`dtruss` (según plataforma): observar bloqueos y syscalls.

### 7.3 Testing de concurrencia
Pruebas robustas:
- evitan depender de timings exactos
- validan invariantes (`produced == consumed + queued`, etc.)
- incluyen escenarios de cierre/apagado

---

## 8. Checklist de diseño concurrente

Antes de cerrar un ejercicio/proyecto:
1. ¿Qué datos son compartidos?
2. ¿Con qué primitiva se protege cada dato?
3. ¿Existe un protocolo de apagado sin hilos colgados?
4. ¿Hay riesgo de deadlock por orden de locks?
5. ¿El test valida invariantes y no solo logs?

---

## 9. Mapa del bloque (práctica sugerida)

### Resueltos
- `e01_pthread_create_join`: creación/join y partición de trabajo.
- `e02_args_struct_y_join`: argumentos complejos y retorno por `join`.
- `e03_race_condition_demo`: pérdida de updates sin sincronización.
- `e04_mutex_contador_seguro`: corrección con mutex.
- `e05_condvar_productor_consumidor`: espera eficiente sin busy-wait.
- `e06_condvar_queue_cierre`: multi-productor/multi-consumidor con cierre.
- `e07_rwlock_cache_basica`: lectura concurrente y escritura exclusiva.
- `e08_semaforo_rate_limiter`: límite de concurrencia por tokens.
- `e09_thread_pool_minimo`: workers persistentes + cola de tareas.
- `e10_pipeline_multietapa`: procesamiento por etapas con sentinels.

### Complejos
- `c01_thread_pool_prioridades`: pool con prioridad/fairness.
- `c02_miniserver_pool_http`: servidor TCP/HTTP con workers.
- `c03_benchmark_lock_contention`: comparación empírica de estrategias.

---

## 10. Errores frecuentes que debes detectar tú mismo

- Pasar a `pthread_create` dirección de variable local de loop sin copiarla por hilo.
- Usar `if` en lugar de `while` alrededor de `pthread_cond_wait`.
- Modificar estado compartido fuera de lock.
- No despertar (`signal/broadcast`) al cambiar condición esperada por otros.
- Cerrar programa sin unir o detener hilos de fondo.
- Basar corrección en `sleep()` en vez de sincronización real.

Si mantienes estas reglas, puedes escalar a proyectos reales de IO concurrente sin depender de suerte del scheduler.
