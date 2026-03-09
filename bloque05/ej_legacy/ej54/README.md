# Ejercicio 5.4 — Peaje Selectivo: Read-Write Locks (`pthread_rwlock_t`)

## 🎯 Objetivo
Entender el cuello de botella que introducen los Mutex tradicionales, y solucionarlo utilizando los Candados de Lectura/Escritura. Aprenderemos que si nadie va a destruir / modificar una variable RAM... "¡Dejadlos Pasar!" y leerla simultáneamente.

## 📚 Teoría Mínima
- Los Mutex normales detienen a **todos**: Si un hilo va a hacer un "Select" de tu array para solo imprimirlo, obligará a los demás a frenar por turno.
- Esto es fatal para estructuras como un Caché (90% consultas ciegas, 10% inserciones pesadas temporales).
- La solución brillante de `pthread_rwlock_t` divide la puerta en dos carriles:
   1. `pthread_rwlock_rdlock()`: El Candado de **Lectores**. Si 1 Lector toma este Lock, y llega otro..., ¡Pasan ambos! Linux les da permiso cediéndoles el RAM al mismo tiempo, total, "Si ambos sólo van a asomarse y mirar, nadie rompe nada". Y si llegan mil Lectores, los mil ven simultáneamente a 100% de CPU Paralleles!.
   2. `pthread_rwlock_wrlock()`: El Candado del **Escritor**. El Dios Muerte. Si un hilo solitario que necesita modificar su array llama a éste, **detendrá a todos y cada uno de los Lectores y también a los otros Escritores** quedándose asilado e ininterrumpido en modo Dios para destrozar o mutar cosas localmente antes del unlock.

## 📝 Instrucciones

Construye `src/main.c`.
1. Fija una var local de caché intencionada `var_cache = 0`.
2. Declara un `pthread_rwlock_t rw_lock`. E inicialízalo (`pthread_rwlock_init`).
3. Lanza **5 Hilos LECTORES**: Su misión es entrar a un bucle C infinito que corra unas 5 veces. 
   - Que invoquen a `pthread_rwlock_rdlock`.
   - Que digan "Lector %d viendo var: %d".
   - Duermen 1 seg con un `sleep()` intencional dentro de la sección de seguridad. 
   - Hacen un local unlock `pthread_rwlock_unlock`.
4. Lanza **2 Hilos ESCRITORES**: Misma dinámica bucle, **pero**.
   - Que llamen a `pthread_rwlock_wrlock`.
   - Que digan ¡Mutaando cache! (Súmale a la var + 10).
   - Duerman también 1 Segundo completo pesadamente...
   - Desbloqueen libre `pthread_rwlock_unlock`.
5. Junta todo usando el final feliz `pthread_join(..)`.
6. Enseña visualmente en print que los 5 Lectores de alguna manera no tardaron (5 Hilos X 1 Seg Sleep = 5 Segundos). Si corren a la vez paralizados sobre la misma variable **solo debió tardar 1 Segundo total** porque saltaron el Lock mutuo al mismo tiempo.

## ✅ Criterios de Éxito
- Comprobar que tu Consola te escupe líneas de `[Lector N]` agrupadas en un solo estallido instantáneo 5 al mismo tiempo a pesar de cada quien tener su lock, revelando un pasaje libre del Kernel (Thread-Safe read pass) frente al solitario demorado Writer!.
