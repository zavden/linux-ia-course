# Ejercicio 3.1 — Duplicacion de Memoria Viva: `fork` y `wait`

## 🎯 Objetivo
Hacer el tutorial clásico: Crear un proceso hijo e interactuar con la RAM del otro. Aprender cómo esperar procesos de forma síncrona, y aprender, mediante error a evitar los malvados procesos Zombie (`<defunct>`).

## 📚 Teoría Mínima
- `fork()` devuelve `0` al proceso hijo y devuelve el `PID` (el numero id del hijo) al proceso padre.
- Todo lo que el padre tenía en RAM (punteros, descritores) ha sido calcado de forma profunda. Modificar `x=1` dentro del contexto `if(pid == 0)` no perturbará al valor original `x` del espacio del padre o viceversa, ahora corren aislados el uno del otro.
- `waitpid(pid, &status, opciones)`: Espera que un hijo exacto sufra un cambio de estado fatal y recoge sus restos mortales (su status code, y retira el objeto Zombie de la lista `top`/`ps`).
- Usa la macro `WIFEXITED(status)` para validar que el hijo murió pasíficamente mediante un `exit(status)` o un final de main, en vez de haber sido acuchillado por una señal como un Segfault malicioso.
- Seguido de aquello descifras exactamente QUÉ devolvió el zombie (`return EXIT_STATUS` o `exit(NUM)`) utilizando una macro que extrae el byte exacto llamada `WEXITSTATUS(status)`. 

## 📝 Instrucciones

1. Crea en `src/main.c` un ciclo que haga `fork()` **N = 3 veces**.
2. Al momento que el hijo nazca (cuando detectas que el retorno dio `0`), debe: 
   - Imprimir su PID con `getpid()` y el PID del padre que lo arrojó `getppid()`.
   - Modificar una variable compartida global y probar luego que nada le paso en el proceso principal.
   - Detenerse 1 o 2 segundos aleatorios usando `sleep(num)`.
   - Morir usando un `exit(i + 10)` peculiar y diferente por cada hijo usando el índice para generar una singularidad.
3. El proceso padre (o tu thread inicial C puro) NO debe morir y acabarse de un golpe sin más. Deberá esperarlos amablemente recolectando el estado con el combo  `waitpid()` + comprobación profunda con las dos Macros mencionadas arriba. 

**Demo Zombie Opcional:** Comenta o quita de momento las rutinas `waitpid`, crúzate de brazos por `sleep(10)` y ve a la terminal o usa una funcion C ejecutora `system("ps -eaf | grep defunct")`. Observarás qué ocurre de veras si no les haces una sepultura digna.

## ✅ Criterios de Éxito
- Tendrás un binario que escupe las 3 fotocopias de sí mismo en el CPU demostrando el multithreading pesado.
- Valdrán retornos asimétricos valiosísimos rescatados intactos por su padre al cerrar la espera.
