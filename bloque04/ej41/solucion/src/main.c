/*
 * Ejercicio 4.1 — Vectores Dinámicos manuales (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Replicar el funcionamiento estructural interno de C++ `std::vector` o
 * de los `Array` y `Lists` de lenguajes como JS o Python en C crudo.
 * Demuestra el uso condicionado de expansion de RAM y mitigación de fugas 
 * listos para su posterior análisis implacable de Valgrind.
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *data;
    size_t len;
    size_t capacity;
} dyn_array_t;

/*
 * CONSTRUCTOR SEGURO EN EL HEAP
 */
void da_init(dyn_array_t *arr, size_t initial_cap) {
    // Usamos malloc. (Se le debe proveer un tamaño multiplicado por el tamaño literal bit de objeto que metemos o explotará corto)
    arr->data = malloc(initial_cap * sizeof(int));
    if (!arr->data) {
        perror("Error critico. Kernel OS Sin RAM disponble al invocar Array Cero.");
        exit(EXIT_FAILURE);
    }
    arr->len = 0;
    arr->capacity = initial_cap;
    printf("[Init] Array forjado con cap base = %lu int slots\n", (unsigned long)initial_cap);
}

/*
 * INSERCIÓN CON AUTO-CRECIMIENTO (AMORTIZADO)
 */
void da_push(dyn_array_t *arr, int value) {
    // 1. LIMITE FISICO ALCANZADO A TOPE
    if (arr->len == arr->capacity) {
        size_t new_cap = arr->capacity * 2; // Factor x2 de crecimiento amortizado clasico.
        printf(">> [Re-Alocación de Contexto] Capacidad topada (%lu). ¡Realizando Realloc X2 expansivo a %lu!...\n", 
               (unsigned long)arr->capacity, (unsigned long)new_cap);
        
        // 2. MAGIA DE REALLOC
        // Si hay espacio adelante del charco, el Kernel solo estira el limite y devuelve el mismo apuntador.
        // Si nuestro int* estaba trancado por otras variables pegadas fisicamente, 
        // El kernel mudará silenciosa y mágicamente estos bytes al infito de RAM lejano y nos dará 
        // un nuevo charco pointer limpio, por lo que DEBEMOS pisal nuestra variable arr->data actual.
        int *temp = realloc(arr->data, new_cap * sizeof(int));
        
        if (!temp) {
            perror("Catástrofe de Expansión. Imposible acatar Realloc nuevo limite.");
            // En librerias criticas, podrias hacer clean acá, por suerte temp=NULL si falla, protegiendo 
            // a nuestro viejo puntero sin corromperlo.
            exit(EXIT_FAILURE);
        }
        
        arr->data = temp;
        arr->capacity = new_cap;
    }

    // 3. INSERCIÓN PURA ASUMIDA (Espacio garantizado). 
    arr->data[arr->len] = value;
    arr->len++;
}

/*
 * DESTRUCTOR FINAL. 
 * ¡Obligatorio!. (De lo contrario el OOM killer del kernell te odiará hasta la sepultura local de cierre.)
 */
void da_free(dyn_array_t *arr) {
    // Retornamos el Pointer fisico al Custodio central y vaciamos el objeto basura struct
    free(arr->data);
    arr->data = NULL; // Proteccion defensiva si un becario hace Use-After-Free
    arr->capacity = 0;
    arr->len = 0;
}

int main(void) {
    dyn_array_t vec;
    
    puts("====== Iniciando Demostración Asignación Amortizante de Bloques ======");
    // Empieza diminuto para forzar relocalizaciones fuertes múltiples y exprimir realloc
    da_init(&vec, 2); 

    for (int i = 0; i < 20; i++) {
        // Multiplicamos por algo random simulando data y observando logs de saltos.
        da_push(&vec, i * 10);
    }

    printf("\n[Final] Insertado 20 items. Estado: Len=%lu / Cap=%lu\n", 
           (unsigned long)vec.len, (unsigned long)vec.capacity);

    // Iterador purificador final 
    printf("[Final] Mostrando Head->[0, 1, 2] = [%d, %d, %d...]\n", vec.data[0], vec.data[1], vec.data[2]);

    puts("Liberando estructura y retornando Heap RAM a la Glibc/Linux.");
    da_free(&vec);
    
    // VALGRIND LEAK CHECK A ESPERAS TENSAS....
    return EXIT_SUCCESS;
}
