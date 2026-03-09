# Ejercicio 5.3 — Variables de Condición: Dormir y Despertar Multihilo

## 🎯 Objetivo
Resolver el infame problema de concurrencia: **"El Productor y el Consumidor"**. Evitaremos que el CPU se queme al 100% haciendo un "bucle infinito mirando si hay cosas `while(tareas == 0) {}`" (Busy-Wait), y en su lugar, dormiremos la rama atómicamente hasta que otro Hilo avise con una Señal Posix *Condition* ("Pthread Cond Signal").

## 📚 Teoría Mínima
- Un `Mutex` sólo protege simultaneidad.
- Una Variable de Condición `pthread_cond_t` **Paúsa a 0% CPU el Hilo** ligándolo al SO, pidiéndole que lo reactive ÚNICAMENTE CUANDO alguien llame a `pthread_cond_signal()` o `pthread_cond_broadcast()`.
- Un Consumidor SIEMPRE debe usar todo esto con un `while`: 
  ```c
  pthread_mutex_lock(&m);
  while( buffer_esta_vacio ) {   // CUIDADO: Un `if` puede arrojar un "Despertar fantasma" Espúreo de OS!
      pthread_cond_wait(&condicion_de_llenado, &m); // ¡MÁGIA! ¡Esto suelta el Mutex rojo auto-magicamente y Duerne al Hilo a la vez!
  }
  // Al despertar abajo de aqui... cond_wait VOLVIO a asegurar el Mutex cerrandolo automatico :)
  // -> SACAR DATO...
  pthread_mutex_unlock(&m);
  ```

## 📝 Instrucciones

Construye `src/main.c`.
1. Crea un Buffer Global Compartido (Lista o variable `tareas=0`) como stock. 
2. Crea `pthread_mutex_t m` y `pthread_cond_t cv`.
3. Lanza **1 Hilo Productor** y **3 Hilos Consumidores**
4. **Productor**: Entra a un `for (1..20)`. Lock Mutex -> Suma tarea `tareas++` -> `pthread_cond_broadcast(&cv)` diciéndole a toda los que duermen "Ey!, hay comidita!, despertad" -> Unlock Mutex -> sleep random (ej: usar `sleep(1)`) para dejarlos agonizar.
5. **Consumidores**: Bucle infinito. Lock Mutex -> `while(tareas == 0)` dormirnos ciegos `pthread_cond_wait(&cv, &m)`.  -> Despertamos.. comprobamos stock, comemos 1 -> Tareas `--` -> Unlock Mutex.
6. Demuestra en prints (loggings) cómo los consumidores despiertan peleando por comida en ráfagas.

## ✅ Criterios de Éxito
- Has resuelto uno de los 5 grandes teoremas de ciencias de computación clásicos usando código de sincronía de interbloqueos pacíficos.
- Verificaste que tus tres hilos consumidores no estén a 100% locamente iterando cuando hay `tareas = 0` sino suspendidos cediendo memoria e infra.
