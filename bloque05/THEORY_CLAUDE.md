# THEORY_CLAUDE.md — Bloque 05: Hilos y Concurrencia POSIX

> Hasta ahora, cada programa que has escrito ejecuta una sola secuencia de instrucciones. En este bloque, tus programas ejecutan **múltiples secuencias simultáneas** sobre la misma memoria. Lo difícil no es crear hilos — lo difícil es que no corrompan los datos del otro. Este bloque enseña las primitivas de sincronización de POSIX y los patrones que evitan bugs que solo se manifiestan en producción bajo carga.

---

## Mapa del Bloque

```
Tema 1: Modelo mental        →  Concurrencia vs paralelismo, qué comparten los hilos
Tema 2: Ciclo de vida        →  pthread_create, join, detach, paso de argumentos
Tema 3: Mutex                →  Exclusión mutua, secciones críticas
Tema 4: Condvars             →  Espera eficiente, por qué while y no if
Tema 5: rwlock y semáforos   →  Lectores/escritores, control de capacidad
Tema 6: Patrones clásicos    →  Productor-consumidor, thread pool, pipeline
Tema 7: Problemas reales     →  Deadlock, starvation, livelock
Tema 8: Diagnóstico          →  TSan, GDB multihilo, testing
```

---

## Tema 1 — El modelo mental correcto

### Concurrencia ≠ Paralelismo

| Concepto | Definición | Ejemplo |
|----------|-----------|---------|
| **Concurrencia** | Múltiples tareas en progreso (intercaladas o simultáneas) | Un cocinero alternando entre 3 sartenes |
| **Paralelismo** | Múltiples tareas ejecutándose **al mismo tiempo** | 3 cocineros, cada uno con una sartén |

Un programa concurrente en un solo núcleo **necesita sincronización** aunque nunca haya dos instrucciones ejecutándose simultáneamente, porque el scheduler puede intercalar los hilos en cualquier punto.

### Qué comparten los hilos, qué no

```
Proceso
├── [COMPARTIDO entre todos los hilos]
│   ├── Heap (malloc/free)
│   ├── Variables globales y estáticas
│   ├── File descriptors
│   ├── Espacio de direcciones virtual
│   ├── Señales (dispositions, pendientes de proceso)
│   └── PID, UID, GID, cwd
│
└── [PRIVADO por hilo]
    ├── Stack (variables locales)
    ├── Registros de CPU
    ├── Thread ID (pthread_t)
    ├── errno (thread-local desde POSIX.1-2001)
    ├── Máscara de señales
    └── Thread-local storage (TLS)
```

> [!IMPORTANT]
> **Compartir heap es barato; coordinar acceso es lo difícil.** Dos hilos pueden leer `counter` sin problema. Pero si uno lo lee mientras el otro lo escribe, el resultado es indeterminado.

### Race condition: el bug que no siempre se reproduce

```c
int counter = 0;  // variable global compartida

void *worker(void *arg) {
    for (int i = 0; i < 1000000; i++)
        counter++;   // ← NO es atómico
    return NULL;
}
```

`counter++` se compila en **3 operaciones** (load, increment, store). Si dos hilos ejecutan esto intercaladamente:

```
Hilo A: load counter → reg = 5
                                    Hilo B: load counter → reg = 5
Hilo A: reg = 6
Hilo A: store counter = 6
                                    Hilo B: reg = 6
                                    Hilo B: store counter = 6
                                    
Resultado: counter = 6 (se perdió un incremento)
```

Con 2 hilos y 1M iteraciones, el resultado esperado es 2,000,000 pero típicamente obtienes ~1,200,000–1,800,000.

---

## Tema 2 — Ciclo de vida de hilos

### Crear un hilo

```c
#include <pthread.h>

void *worker(void *arg) {
    int id = *(int *)arg;
    printf("Hilo %d trabajando\n", id);
    return NULL;
}

int main(void) {
    pthread_t threads[4];
    int ids[4];

    for (int i = 0; i < 4; i++) {
        ids[i] = i;
        int err = pthread_create(&threads[i], NULL, worker, &ids[i]);
        if (err != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(err));
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}
```

> [!CAUTION]
> **Las funciones de pthreads NO usan `errno`.** Retornan directamente el código de error (ej: `EAGAIN`, `ENOMEM`). NO hagas `perror("pthread_create")` — usa `strerror(err)`.

### El bug clásico del argumento por referencia

```c
// ❌ BUG: todos los hilos leen la misma variable 'i'
for (int i = 0; i < 4; i++) {
    pthread_create(&threads[i], NULL, worker, &i);
    // Cuando el hilo lee *arg, 'i' puede valer 1, 2, 3, 4...
    // o cualquier cosa — es una race condition
}

// ✅ CORRECTO: cada hilo tiene su propia copia
int ids[4];
for (int i = 0; i < 4; i++) {
    ids[i] = i;
    pthread_create(&threads[i], NULL, worker, &ids[i]);
}
```

### Argumentos complejos: usar un struct

```c
typedef struct {
    int    id;
    char  *filename;
    int   *result;
} thread_args_t;

void *worker(void *arg) {
    thread_args_t *a = arg;
    // usar a->id, a->filename, etc.
    *a->result = 42;
    return NULL;
}

// Crear:
thread_args_t args[N];
for (int i = 0; i < N; i++) {
    args[i] = (thread_args_t){ .id = i, .filename = files[i], .result = &results[i] };
    pthread_create(&threads[i], NULL, worker, &args[i]);
}
```

### `join` vs `detach`

| | `pthread_join` | `pthread_detach` |
|---|---|---|
| **Qué hace** | Bloquea hasta que el hilo termine, recoge su estado | Marca el hilo como "fire-and-forget" |
| **Recursos** | Liberados al hacer `join` | Liberados automáticamente al terminar |
| **Resultado** | Puedes obtener el valor de retorno | No puedes |
| **Si olvidas** | Recursos del hilo quedan sin liberar (similar a zombie) | — |
| **Cuándo usar** | Por defecto — siempre | Cuando realmente no necesitas esperar ni resultado |

```c
// Obtener resultado via join:
void *retval;
pthread_join(thread, &retval);
int *result = retval;  // result fue allocado con malloc en el hilo
printf("Resultado: %d\n", *result);
free(result);          // quien hace join es responsable del free
```

---

## Tema 3 — Mutex: exclusión mutua

### El contrato

```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_lock(&mutex);
// === SECCIÓN CRÍTICA ===
// Solo un hilo a la vez puede estar aquí
counter++;
// === FIN SECCIÓN CRÍTICA ===
pthread_mutex_unlock(&mutex);
```

### Reglas de supervivencia con mutexes

| Regla | Por qué |
|-------|---------|
| Lock y unlock en **todos** los caminos de control | Si haces `return` dentro de la sección crítica sin unlock, el mutex queda locked forever |
| Minimizar el código dentro del lock | Menos tiempo locked = menos contención = mejor rendimiento |
| No llamar funciones lentas/bloqueantes con lock tomado | Si tu hilo duerme con el lock, todos los demás esperan |
| Un mutex protege **un recurso específico**, documentarlo | Si usas el mismo mutex para proteger cosas distintas, tienes contención innecesaria |
| Nunca lockear dos veces el mismo mutex sin `PTHREAD_MUTEX_RECURSIVE` | Deadlock inmediato contra ti mismo |

### Ejemplo completo: contador thread-safe

```c
typedef struct {
    int value;
    pthread_mutex_t lock;
} safe_counter_t;

void counter_init(safe_counter_t *c) {
    c->value = 0;
    pthread_mutex_init(&c->lock, NULL);
}

void counter_increment(safe_counter_t *c) {
    pthread_mutex_lock(&c->lock);
    c->value++;
    pthread_mutex_unlock(&c->lock);
}

int counter_get(safe_counter_t *c) {
    pthread_mutex_lock(&c->lock);
    int val = c->value;
    pthread_mutex_unlock(&c->lock);
    return val;
}

void counter_destroy(safe_counter_t *c) {
    pthread_mutex_destroy(&c->lock);
}
```

---

## Tema 4 — Condition variables: esperar sin quemar CPU

### El problema que resuelven

Sin condvars, para esperar que un dato esté disponible, harías:

```c
// ❌ BUSY WAIT — quema CPU al 100%
while (!data_ready) {
    // spin... spin... spin...
}
```

Con condvars, el hilo **duerme** hasta que otro lo despierta:

```c
// ✅ ESPERA EFICIENTE — el hilo duerme, no consume CPU
pthread_mutex_lock(&mutex);
while (!data_ready) {
    pthread_cond_wait(&cond, &mutex);  // duerme y libera mutex atómicamente
}
// data_ready es true, mutex está locked
process_data();
pthread_mutex_unlock(&mutex);
```

### Anatomía de `pthread_cond_wait`

```
pthread_cond_wait(&cond, &mutex):
  1. ATÓMICAMENTE: libera mutex + se duerme
  2. [duerme hasta que alguien hace signal/broadcast]
  3. Readquiere mutex
  4. Retorna
```

Lo atómico del paso 1 es crítico: evita la ventana donde sueltas el mutex y alguien hace `signal` antes de que te duermas (lost wakeup).

### `while`, nunca `if`

```c
// ❌ INCORRECTO:
if (!data_ready) {
    pthread_cond_wait(&cond, &mutex);
}
// Problema: spurious wakeup o competencia con otro consumidor
// data_ready podría seguir siendo false al despertar

// ✅ CORRECTO:
while (!data_ready) {
    pthread_cond_wait(&cond, &mutex);
}
// Garantizado: data_ready es true
```

**Spurious wakeups** son wakeups sin `signal`/`broadcast` — el estándar POSIX los permite. Además, si hay múltiples consumidores, uno puede consumir el dato antes de que el otro se despierte y readquiera el mutex.

### `signal` vs `broadcast`

| | `pthread_cond_signal` | `pthread_cond_broadcast` |
|---|---|---|
| Despierta | Un hilo (cuál, no definido) | Todos los hilos esperando |
| Cuándo usar | Una tarea disponible, un consumidor | Cambio de estado global (ej: shutdown) |
| Riesgo de `signal` | Si despierta al hilo "equivocado" que no puede proceder, nadie más despierta | Ninguno (todos revisan la condición) |

> [!TIP]
> **Si dudas, usa `broadcast`.** Es ligeramente menos eficiente (despierta hilos que volverán a dormir) pero siempre correcto. `signal` es una optimización que requiere más cuidado.

---

## Tema 5 — rwlock y semáforos

### `pthread_rwlock_t`: muchos lectores, un escritor

```c
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

// Lectores (pueden ser simultáneos):
pthread_rwlock_rdlock(&rwlock);
value = shared_data;  // lectura segura
pthread_rwlock_unlock(&rwlock);

// Escritor (exclusión total):
pthread_rwlock_wrlock(&rwlock);
shared_data = new_value;  // escritura exclusiva
pthread_rwlock_unlock(&rwlock);
```

| Escenario | Mutex | rwlock |
|-----------|-------|--------|
| 90% lecturas, 10% escrituras | Serializa todo innecesariamente | Los lectores avanzan en paralelo |
| 50% lecturas, 50% escrituras | Simple y eficiente | Overhead extra sin beneficio |
| Sección crítica < 1 µs | Simple y rápido | Overhead de rwlock no compensa |

### Semáforos POSIX: control de capacidad

Un semáforo es un **contador atómico** que bloquea cuando llega a cero:

```c
#include <semaphore.h>

sem_t sem;
sem_init(&sem, 0, 3);  // 0 = intra-proceso, 3 = valor inicial (3 slots)

// Consumir un slot (bloquea si el contador es 0):
sem_wait(&sem);
// ... usar recurso limitado ...
// Devolver el slot:
sem_post(&sem);

sem_destroy(&sem);
```

**Caso de uso: rate limiter**

```c
// Máximo 5 conexiones simultáneas:
sem_t conn_limit;
sem_init(&conn_limit, 0, 5);

void *handle_connection(void *arg) {
    sem_wait(&conn_limit);    // esperar slot
    process_request(arg);     // máximo 5 hilos aquí simultáneamente
    sem_post(&conn_limit);    // liberar slot
    return NULL;
}
```

> [!NOTE]
> **Semáforos nombrados (`sem_open`)** funcionan entre procesos independientes (similar a `shm_open`). Útiles cuando combinas shared memory (Bloque 4) con sincronización entre procesos.

---

## Tema 6 — Patrones clásicos de concurrencia

### Productor-consumidor con cola acotada

Este es **el patrón fundamental** de la concurrencia. Casi todo sistema concurrent lo usa: servidores web, pipelines de datos, message queues.

```c
typedef struct {
    int    *buffer;
    int     capacity;
    int     head, tail, count;
    pthread_mutex_t mutex;
    pthread_cond_t  not_full;
    pthread_cond_t  not_empty;
    int     shutdown;          // flag de cierre
} bounded_queue_t;

void queue_push(bounded_queue_t *q, int item) {
    pthread_mutex_lock(&q->mutex);
    while (q->count == q->capacity && !q->shutdown) {
        pthread_cond_wait(&q->not_full, &q->mutex);  // esperar espacio
    }
    if (q->shutdown) { pthread_mutex_unlock(&q->mutex); return; }

    q->buffer[q->tail] = item;
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;
    pthread_cond_signal(&q->not_empty);  // avisar al consumidor
    pthread_mutex_unlock(&q->mutex);
}

int queue_pop(bounded_queue_t *q, int *item) {
    pthread_mutex_lock(&q->mutex);
    while (q->count == 0 && !q->shutdown) {
        pthread_cond_wait(&q->not_empty, &q->mutex);  // esperar dato
    }
    if (q->count == 0 && q->shutdown) {
        pthread_mutex_unlock(&q->mutex);
        return -1;  // fin
    }

    *item = q->buffer[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->count--;
    pthread_cond_signal(&q->not_full);  // avisar al productor
    pthread_mutex_unlock(&q->mutex);
    return 0;
}

void queue_shutdown(bounded_queue_t *q) {
    pthread_mutex_lock(&q->mutex);
    q->shutdown = 1;
    pthread_cond_broadcast(&q->not_full);   // despertar a TODOS
    pthread_cond_broadcast(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
}
```

> [!IMPORTANT]
> **El shutdown usa `broadcast`, no `signal`.** Si solo despiertas uno, los demás hilos quedan dormidos eternamente. Al cerrar, todos deben despertarse para comprobar el flag y salir.

### Thread pool

```
main() ────▶ encolar tarea ────▶ ┌─────────────────┐
main() ────▶ encolar tarea ────▶ │  bounded_queue   │
                                 └────────┬────────┘
                                          │
                    ┌─────────────────────┼─────────────────────┐
                    ▼                     ▼                     ▼
              Worker 0              Worker 1              Worker 2
              (duerme si            (duerme si            (duerme si
               cola vacía)          cola vacía)           cola vacía)
```

El thread pool combina la cola acotada con N hilos persistentes:

```c
void *pool_worker(void *arg) {
    bounded_queue_t *q = arg;
    task_t task;

    while (queue_pop(q, &task) == 0) {
        task.function(task.arg);  // ejecutar tarea
    }
    return NULL;  // shutdown → salir limpiamente
}
```

Ventajas:
- Evita crear/destruir hilos por tarea (caro: ~50-100µs por creación).
- Controla la concurrencia máxima (no puedes crear 10,000 hilos).
- Es la arquitectura base de servidores reales (nginx, Apache).

---

## Tema 7 — Deadlocks, starvation y livelock

### Deadlock: el abrazo mortal

```c
// Hilo A:                    // Hilo B:
pthread_mutex_lock(&m1);      pthread_mutex_lock(&m2);
pthread_mutex_lock(&m2);      pthread_mutex_lock(&m1);  // ← DEADLOCK
//                            // A tiene m1, espera m2
//                            // B tiene m2, espera m1
//                            // Ninguno puede avanzar
```

**Las 4 condiciones necesarias** (Coffman):
1. **Exclusión mutua:** al menos un recurso es no-compartible.
2. **Retención y espera:** un hilo retiene un recurso mientras espera otro.
3. **No preemption:** los recursos no se quitan a la fuerza.
4. **Espera circular:** A espera a B, B espera a A.

**Prevención práctica:**

| Estrategia | Cómo | Ejemplo |
|------------|------|---------|
| Orden global de locks | Siempre adquirir en el mismo orden | Si necesitas m1 y m2, siempre lock m1 primero |
| Timeout + retry | `pthread_mutex_timedlock` | Si no consigues el lock en 100ms, retrocede y reintenta |
| Lock jerárquico | Nunca lockear un mutex de nivel inferior teniendo uno superior | Documentar niveles en el código |
| Reducir locks | Usar un solo mutex para datos relacionados | Menos locks = menos posibilidades de ciclo |

### Starvation

Un hilo progresa poco o nada porque otros monopolizan el recurso:

```
Hilo A: lock → trabajo largo → unlock → lock → trabajo largo → ...
Hilo B: intentando lock... intentando lock... (nunca lo consigue)
```

Soluciones:
- Usar colas FIFO para el acceso (algunos mutex implementations ya lo hacen).
- Limitar el tiempo que un hilo mantiene el lock.
- Dividir un lock global en locks por shard (stripe locking).

### Livelock

Los hilos no se bloquean pero no avanzan:

```
Hilo A: trylock(m1) → OK → trylock(m2) → falla → unlock(m1) → retry
Hilo B: trylock(m2) → OK → trylock(m1) → falla → unlock(m2) → retry
// Ambos reintentan infinitamente sin avanzar
```

Solución: **backoff aleatorio** — esperar un tiempo random antes de reintentar.

---

## Tema 8 — Diagnóstico y testing de concurrencia

### Thread Sanitizer (TSan)

```bash
gcc -g -O1 -fsanitize=thread -pthread programa.c -o programa
./programa
```

TSan detecta data races en runtime con un overhead de ~5-15x:

```
WARNING: ThreadSanitizer: data race (pid=1234)
  Write of size 4 at 0x... by thread T2:
    #0 worker main.c:15
  Previous read of size 4 at 0x... by thread T1:
    #0 worker main.c:15
```

> [!WARNING]
> **TSan no es compatible con ASan.** No puedes compilar con `-fsanitize=thread,address` al mismo tiempo. Usa TSan para bugs de concurrencia y ASan para bugs de memoria, en ejecuciones separadas.

### GDB con múltiples hilos

```bash
gdb ./programa

(gdb) info threads          # listar todos los hilos
(gdb) thread 3              # cambiar al hilo 3
(gdb) bt                    # backtrace del hilo actual
(gdb) thread apply all bt   # backtrace de TODOS los hilos (para diagnosticar deadlocks)
```

### Testing de concurrencia: reglas

| Regla | Por qué |
|-------|---------|
| No basar la corrección en `sleep` | Un `sleep(1)` que "funciona" en tu máquina falla bajo carga |
| Validar invariantes, no orden | Verificar `produced == consumed + queued`, no "hilo 1 imprime antes que hilo 2" |
| Testear shutdown limpio | Muchos bugs de concurrencia se manifiestan solo al cerrar |
| Ejecutar múltiples veces | Un test que pasa 1 vez no prueba nada; pásalo 1000 veces |
| Variar número de hilos | Bugs pueden manifestarse solo con N>4 hilos |

```bash
# Ejecutar 1000 veces buscando fallos:
for i in $(seq 1000); do
    ./test_concurrent || { echo "FALLO en iteración $i"; break; }
done
```

### Compilación

```bash
# Todas las flags del bloque:
gcc -Wall -Wextra -Werror -pedantic -std=c17 \
    -pthread \                    # ← obligatorio para pthreads
    -g -O0 -fsanitize=thread \   # ← para desarrollo con TSan
    programa.c -o programa
```

---

## Checklist de salida del Bloque 05

- [ ] Crear N hilos, pasar argumentos via struct, y recogerlos con `join`
- [ ] Demostrar una race condition con `counter++` y corregirla con mutex
- [ ] Implementar productor-consumidor con cola acotada usando mutex + condvars
- [ ] El productor-consumidor tiene shutdown limpio sin hilos colgados
- [ ] Explicar por qué `while` y no `if` alrededor de `pthread_cond_wait`
- [ ] Usar rwlock para un caché con muchas lecturas y pocas escrituras
- [ ] Usar semáforos para limitar concurrencia (rate limiter)
- [ ] Implementar un thread pool mínimo con N workers
- [ ] Detectar un deadlock y corregirlo con orden de locks
- [ ] Pasar TSan sin warnings

---

## Referencias

| Recurso | Comando |
|---------|---------|
| Creación de hilos | `man 3 pthread_create`, `man 3 pthread_join` |
| Mutex | `man 3 pthread_mutex_lock`, `man 3 pthread_mutex_init` |
| Condvars | `man 3 pthread_cond_wait`, `man 3 pthread_cond_signal` |
| rwlock | `man 3 pthread_rwlock_rdlock`, `man 3 pthread_rwlock_wrlock` |
| Semáforos | `man 3 sem_init`, `man 3 sem_wait`, `man 3 sem_post` |
| Thread safety | `man 7 pthreads`, `man 7 signal-safety` |
| Thread Sanitizer | [TSan documentation](https://clang.llvm.org/docs/ThreadSanitizer.html) |
