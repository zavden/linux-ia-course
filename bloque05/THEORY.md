# 📖 THEORY.md — Bloque 05: Hilos y Concurrencia (La Matrix)

El multiprocesamiento que vimos en `fork()` divide y clona programas completos duplicando todos los gastos con variables aisladas. Pero, ¿Qué pasa si quieres que dos "Tareas" fluyan a la vez pero que ambas **lean y escriban las MISMAS variables globales** sin consumir el doble de RAM? 

La respuesta es la concurrencia ligera: Los **Hilos (Threads)**. El infame paraíso de la escalabilidad y las pesadillas lógicas de los Bugs asíncronos.

---

## 1. El estándar sagrado: POSIX Threads (`pthreads`)
- Un Proceso C arranca con **1** hilo principal (`main()`). 
- Llamando a `pthread_create()` el programa inyecta un sub-proceso fantasma: Un nuevo clon que comparte ABSOLUTAMENTE EL MISMO HEAP, variables globales, y archivos (`fd`), pero donde el SO le da un **Stack Propio** exclusivo para sus variables y llamados de funciones locales.
- Al igual que en `fork` necesitábamos un `wait`, en pthreads necesitamos un **`pthread_join()`** para fusionar el Hilo de vuelta, garantizando que el `main()` no termine prematuramente (ejecutar `return 0` en el main mata automáticamente y sin piedad a todos tus hilos vivos subyacentes).

---

## 2. Punteros `void *` (El Comodín de Argumentos C)
En `pthread_create`, tienes que decirle a C "¡Ejecuta esta función!". 
- Pero el prototipo está obligado por Kernell a tomar un solo argumento miserable `void *arg`. 
- Si quieres pasar 3 *ints* y 2 *strings*... la técnica universal en C es envolverlos en un mágico `struct`, y decirle a Pthreads que envíe la dirección cruda `&mi_struct_de_argumentos` y hacerle Type-Cast a `(void *)`. Dentro del Thread destino, tú lo revives con `(mi_struct *)arg`.

---

## 3. Condiciones de Carrera (Race Conditions)
Si Hilo 1 lee `x=5`, y lo sube `x=6`. E Hilo 2 llega en el milisegundo anterior a esa suma leyendo el `x=5` originario, él también sumará arrojando `x=6`. ¡Ambos Hilos mataron su información creyendo que insertaron `2` unidades produciendo una pérdida matemática fatal!
- Las condiciones de carrera causan que un Código sea "No-Thread-Safe". Resultan en Bugs fantasma imposibles de rastrear que rompen transacciones bancarias.

---

## 4. Remedios y Bloqueos de RAM
- **Mutex (`pthread_mutex_t`)**: Es un Candy-Lock o Semáforo Simple de 1 luz roja o verde. Antes de que cualquier hilo lea la variable global y la sume, debe llamar a `pthread_mutex_lock()`. Si el Lock está verde, se pone rojo instantáneamente y el hilo avanza y hace su matemática. Si otro Hilo choca queriendo modificarlo al mismo tiempo, el `lock()` detectará la luz roja, y en vez de retornar el programa, el Sistema Operativo congelará (**Bloqueará suspendiendo CPU**) a ese pobre competidor manteniéndolo pasmado hasta que el Hilo 1 haga `pthread_mutex_unlock()`.

- **Condition Variables (`pthread_cond_t`)**: ¡No confundir con los Mutex!. Una var de condición permite que un hilo (El Consumidor) se duerma en pausa pacífica a la espera de que ocurra un evento "Sigue habiendo 0 tareas, duérmete. Si hay una despertate". El Hilo Productor cuando meta una tarea activará a todos los durmientes lanzando un `pthread_cond_broadcast()`.

- **Read-Write Locks (`pthread_rwlock_t`)**: Un Mutex normal detiene a todo el mundo (Si 10 hilos van a LEER estadisticas sin modificarlas, es estúpido pararlos en fila de 1 a 1 perdiendo tiempo si todos sólo van a mirar). Un Lock ReadWrite permite que INFINITOS lectores avancen todos juntos, pero si alguien va a llamar un Bloqueo de Escritor... frena a los billones de Lectores esperando exclusividad total atómica.

- **Semáforos**: Similares al Mutex, pero en vez de dejar a 1 sola persona entrar... dejan a `N` personas (Por ejemplo: Tienes 4 carritos de supermercado - Semáforo de Counter 4).
