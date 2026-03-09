# Ejercicio 5.5 — El Pase VIP: Semáforos POSIX (`sem_t`)

## 🎯 Objetivo
Entender el último elemento mágico de la sincronización de sistemas: El Semáforo. A diferencia de un Mutex (Que solo es 1 para 1), el Semáforo es un contador matemático atómico: "Tengo pase para `N` entidades simultáneas".

## 📚 Teoría Mínima
- Imagina un Boliche o Club Nocturno con C.A. (Capacidad Aprobada) de 50 Personas, pero afuera hay una fila de 1000.
- El Guardián es un `sem_t`. Al inicializar le dices `sem_init(&guardia, 0, 50)`.
- Cada persona nueva (Thread) llama a `sem_wait(&guardia)`. 
   - El Guardía le quita 1 al Ticket de entrada: "(50 - 1 = 49 Libres). ¡Pasá master!".
   - Si otro intenta entrar y la capacidad estaba marcando `0 Libres`, el `wait` **lo pausa y congela el CPU** tal cual el club lo haría obligándolo a esperar afuera horas.
- Cuando alguien adentró se emborrachó y se quiere ir, llama a `sem_post(&guardia)`.
   - El Guardía le suma a su cuenta `(0 + 1 = 1)`. Total libres 1 ! Y acto seguido levanta instantáneamente la vara dejando pasar cediéndole núcleo al primero que llevaba dormido 2 horas afuera de la rumba.

## 📝 Instrucciones

Construye `src/main.c`.
1. `#include <semaphore.h>`.
2. Declara un Semáforo global: `sem_t licencias_api;`
3. Inicialízalo en tu Main: `sem_init(&licencias_api, 0, 3);` (¡Sólo permites 3 Conexiones API Simultáneas!).
4. Dispara en un Tsunami atómico **10 Hilos de Peticiones** (Simulan 10 usuarios dando F5 refrescando tu Web al mismo segundo!).
5. La función Hija de peticiones invocará primero y antes que nada a `sem_wait(&licencias_api);`.
6. Si pasaron: ¡Están navegando tu web! Impríme "Petición %d Procesándose En Servidor (Restando API Libre)..." y usa `sleep(2)` pesado.
7. Al terminar liberan pase `sem_post(&licencias_api);` y retornan a morir.
8. En tu test de prueba, deberás ver a ojo limpio cómo de los 10 creados... ¡Solo entran TRES simultáneos!, se callan por 2 segundos esperando... luego ¡Pum un grupo de tres nuevos pasan ocupando el hueco! Control Mágico de Flujos Asíncronos Puros C++. Destrúyelo con `sem_destroy(&licencias_api)`.

## ✅ Criterios de Éxito
- Lograste domar el asalto cibernético conteniendo a la bestia a que tu C pase por el estrecho de las termópilas de 3 en 3 sincronizados con Kernel Waits optimizados.
