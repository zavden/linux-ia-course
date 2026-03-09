/*
 * Ejercicio 4.5 — Custom Arena Pool Allocator (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Escribir tu propio Manejador de RAM customizado sobre la Marcha (Arena Allocator).
 * Esta técnica está alojada detrás de motores de consolas y videojuegos, evita
 * defragmentaciones que ralentizan mallocs por O(N) logrando allocaciones 
 * atómicas O(1) con un Garbage Destructor manual lineal masivo.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>    // CLOCKS_PER_SEC
#include <string.h>

/* ESTRUCTURAS ADMINISTRADORAS */
typedef struct {
    void *base;       // Dirección cruda maestra OS. (Big Chunk pointer)
    size_t capacity;  // Tamaño máximo del barril (Gigabytes / Kilobytes)
    size_t offset;    // Cuántos bloques o bytes ya he vendido / repartido hoy?
} arena_pool_t;


/* 
 * FASE PREVENCION: ALLOCATING THE MOTHERSHIP
 */
arena_pool_t* pool_create(size_t huge_bytes) {
    arena_pool_t *pool = malloc(sizeof(arena_pool_t));
    if (!pool) return NULL;
    
    // Le pedimos de GOLPE el trozo enorme a Malloc al OS para revenderlo después pedazo por pedazo.
    pool->base = malloc(huge_bytes);
    if (!pool->base) {
        free(pool);
        return NULL;
    }
    
    pool->capacity = huge_bytes;
    pool->offset = 0;
    return pool;
}

/* 
 * MAGIA DE LA DISTRIBUCIÓN O(1) DE C++
 */
void* pool_alloc(arena_pool_t *pool, size_t pedazo_demandado) {
    // Nos cercionamos de que este pedacito encaja aun en lo que sobra del barril entero?
    if (pool->offset + pedazo_demandado > pool->capacity) {
        // Fracaso sin crashear!. Nuestro OOM-Killer Interno propio se activa y arroja NULL.
        return NULL; 
    }
    
    // Algebra de Punteros C:
    // Tenemos que convertir todo a `char *` para que el compilador no explote ni haga cast random
    // sabiendo que 1 char = 1 Byte exacto y sumar el OFFSET literal byte a byte!.
    void *ptr_vendido = (char*)pool->base + pool->offset;

    // Actualizamos el Contador interno avanzando el puntero para que el pròximo comprador no 
    // reciba este pedazo ni lo sobreescriba.
    pool->offset += pedazo_demandado;
    
    return ptr_vendido;
}

/* 
 * EL DESTRUCTOR / PURGADOR GLOBAL. (No existe concepto de "free" individual en Arena Pools)
 */
void pool_destroy(arena_pool_t *pool) {
    // Libre de pecado, con 1 SOLO llamado de SYS_CALL limpia millones de variables repartidas
    free(pool->base); 
    free(pool);
}


#define NUM_OBJETOS 10000000 // 10 Millones!!! 
#define SIZE_NODO_BYTE 16    // Variables diminutas de 16 Bytes (Vectores 3D quiza)

int main(void) {
    printf("=============== BENCHMARK ARENA ALLOCATOR VS GLIBC MALLOC ===============\n");

    // Calculo: 10 millones objects * 16 bytes = 160 Megabytes continuos de RAM!. 
    size_t mega_barril_size = (size_t)NUM_OBJETOS * SIZE_NODO_BYTE;
    
    printf(">> 1. Solicitando El Barril Base C++ (Area Gigante Continua = %lu MB)...\n", 
           (unsigned long)(mega_barril_size / 1024 / 1024));
           
    arena_pool_t *my_arena = pool_create(mega_barril_size);
    if (!my_arena) {
        perror("Fallo fatal OS Malloc Principal Inicial."); return EXIT_FAILURE; 
    }

    /* -------------------------------------------------------------
     * TEST 1: REPARTO EXTREMO DESDE NUESTRA PROPIA ARENA C
     * ------------------------------------------------------------- */
    printf("\n>> 2. Disparando %d de 'mallocs' diminutos Custom-Pool O(1)...\n", NUM_OBJETOS);
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OBJETOS; i++) {
        // Notase que si esto fuera un struct 3D Node "Node *n = pool_alloc(...)", le ahorramos la vida entera a la PC
        void *nodo_ficticio = pool_alloc(my_arena, SIZE_NODO_BYTE);
        if (!nodo_ficticio) {
            printf("Error C OOM: Te llenaste en el objeto Nº %d\n", i);
            break; 
        }
    }
    
    clock_t end = clock();
    double time_mi_arena = (double)(end - start) / CLOCKS_PER_SEC;
    printf("\t [!] Arena Custom Completada en >>>>  %f SEGUNDOS  <<<<\n", time_mi_arena);


    /* -------------------------------------------------------------
     * TEST 2: REPARTO ESTRESANTE TRADICIONAL (MALLOC() 10 MILLONES DE VECES EN GLIBC Y KERNEL)
     * ------------------------------------------------------------- */
    printf("\n>> 3. Disparando %d 'mallocs' tradicionales Glibc Nivel OS... (Cuidado, Puede Asfixiar a Top/Htop)\n", NUM_OBJETOS);
    
    void **basura_tradicional_guarda = malloc(sizeof(void *) * NUM_OBJETOS); // Solo Guardar array para el free despues.

    start = clock();
    for (int i = 0; i < NUM_OBJETOS; i++) {
        void *n = malloc(SIZE_NODO_BYTE);
        basura_tradicional_guarda[i] = n;
        if (!n) { printf("Linux te abortó malloc tradicional al # %d !!!\n", i); break;}
    }
    end = clock();
    
    double time_malloc = (double)(end - start) / CLOCKS_PER_SEC;
    printf("\t [!] OS C_Malloc Completado en   >>>>  %f SEGUNDOS  <<<<\n", time_malloc);
    

    // Muestra final del veredicto aplastador
    if (time_mi_arena < time_malloc) {
        printf("\n🔥 [VEREDICTO] ¡Tu Allocator POOL Custom barrió el piso frente a System Malloc, aplastándolo fuertemente por fragmentación C!\n");
    }

    // ============================================
    // DESTRUCTOR DE GALAXIAS MASSIVO (LIMPIEZA OS)
    // ============================================
    printf("\n>> 4. Destruyendo memoria. Arena Pool (1 sola Syscall O(1)) VS Malloc Loop (10 Millones Syscalls O(N)) ...\n");
    
    // Free de nuestra arena (Instasteneo)
    pool_destroy(my_arena);
    
    // Free el Malloc Tradicional asfixiante tortura:
    for (int i = 0; i < NUM_OBJETOS; i++) {
        free(basura_tradicional_guarda[i]);
    }
    free(basura_tradicional_guarda);

    printf(">> Apagón Valido y Limpio general en Kernell Sin Leaks.\n");

    return EXIT_SUCCESS;
}
