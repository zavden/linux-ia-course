# THEORY.md — Bloque 01: Fundamentos sólidos de C en Linux

Este bloque no es "C básico" de academia. Es la base operativa para escribir utilidades tipo Unix que se comporten bien bajo errores, datos malformados, límites de memoria y uso real en terminal.

## Alcance del bloque

Temas que debes dominar antes de avanzar:

1. Contrato de ejecución de programas CLI en Linux (`argc/argv`, `getopt`, convenciones de salida).
2. Strings seguros y manejo explícito de buffers.
3. Estructuras dinámicas genéricas (`struct`, `void *`, ownership de memoria).
4. Manejo de errores POSIX (`errno`, retorno de syscalls, `goto cleanup`).
5. Aritmética de punteros y microbenchmarks realistas (`memcpy`/`memset`).

Material práctico previo de este bloque quedó archivado en `bloque01/ej_legacy/`.

---

## 1) Programas CLI en Linux: contrato real

## 1.1 Firma de `main` y semántica de argumentos

Formas válidas comunes:

```c
int main(void);
int main(int argc, char *argv[]);
```

Para herramientas CLI se usa la segunda.

- `argc` es la cantidad de tokens recibidos por el proceso.
- `argv` es un array de punteros a `char` terminado en `NULL` (`argv[argc] == NULL`).
- `argv[0]` suele contener el nombre/invocación del programa, pero no debe asumirse confiable para lógica crítica.

Observaciones importantes:

1. El shell hace tokenización y expansión antes de ejecutar tu binario.
2. Quoting (`"..."`, `'...'`) y escapes determinan qué llega a `argv`.
3. Tu programa no recibe "la línea completa", recibe tokens ya parseados por el shell.

## 1.2 Convenciones Unix de salida y errores

Convención mínima para utilidades:

1. `stdout`: salida normal de datos.
2. `stderr`: diagnósticos/errores.
3. `EXIT_SUCCESS` (0): ejecución correcta.
4. `EXIT_FAILURE` (!=0): fallo.

Esto permite pipear sin contaminar datos:

```bash
mi_tool --list 1>salida.txt 2>errores.log
```

## 1.3 Parseo robusto con `getopt` y `getopt_long`

`getopt` evita parseo manual frágil.

Headers:

```c
#include <unistd.h>   // getopt
#include <getopt.h>   // getopt_long
```

Variables globales implicadas:

1. `optarg`: valor de la opción actual (si aplica).
2. `optind`: índice del próximo argumento por procesar.
3. `optopt`: opción inválida encontrada.
4. `opterr`: controla impresión automática de errores.

## 1.4 `optstring` bien entendida

Ejemplo:

```c
"vo:n:h"
```

Significa:

- `v` y `h` no requieren argumento.
- `o` y `n` sí requieren argumento por `:`.

Casos relevantes:

1. Opción desconocida: retorno `?`.
2. Falta argumento requerido: retorno `?` o `:` (si `optstring` empieza con `:`).
3. `--`: marca fin de opciones; lo que sigue son posicionales.

## 1.5 `getopt_long` y opciones largas

Tabla típica:

```c
static struct option long_opts[] = {
    {"verbose", no_argument,       0, 'v'},
    {"output",  required_argument, 0, 'o'},
    {"number",  required_argument, 0, 'n'},
    {"help",    no_argument,       0, 'h'},
    {0,0,0,0}
};
```

Soporta:

```bash
./app -v -o out.txt -n 3
./app --verbose --output=out.txt --number=3
```

## 1.6 Validación de tipos: nunca usar `atoi`

Para enteros usa `strtol`:

```c
char *end = NULL;
errno = 0;
long val = strtol(optarg, &end, 10);
if (errno != 0 || *end != '\0' || val < 0 || val > INT_MAX) {
    // error de parseo/rango
}
```

`atoi` no reporta errores con precisión.

## 1.7 Patrón recomendado de parseo

1. Inicializar defaults.
2. Iterar `getopt_long`.
3. Validar cada opción en el `switch`.
4. Tras parseo, consumir posicionales desde `argv[optind]`.
5. Mantener función `print_usage(FILE *out, const char *prog)`.

## 1.8 Errores comunes en CLIs

1. Aceptar silenciosamente entradas inválidas.
2. Mezclar salida de datos y errores en `stdout`.
3. No documentar defaults.
4. Ignorar `--help` y devolver código de error.

---

## 2) Strings seguros en C: disciplina de bytes

## 2.1 Modelo real de string en C

Un string es un array de bytes terminado en `\0`.

```c
char s[] = "Hola"; // {'H','o','l','a','\0'}
```

Toda API de string clásica (`strlen`, `printf("%s")`, etc.) depende de encontrar `\0`.

## 2.2 Riesgo central: no hay tamaño implícito

Un `char *` no conoce la capacidad del buffer al que apunta.
Por eso APIs como `strcpy` son inseguras cuando el tamaño real del destino no está garantizado.

## 2.3 `strncpy` no es `strlcpy`

`strncpy` fue diseñada para campos de longitud fija (históricos), no para strings "normales".
Problemas:

1. Puede no terminar en `\0` si se truncó.
2. Rellena con ceros hasta `n`, costoso e inesperado.

## 2.4 Contratos robustos para wrappers seguros

Diseña funciones con contrato explícito:

1. Reciben `dst_size` siempre.
2. Garantizan null-termination cuando `dst_size > 0`.
3. Devuelven longitud "deseada" para detectar truncamiento.

Patrón de retorno estilo `strlcpy`:

- retorno = longitud de `src`.
- hubo truncamiento si `ret >= dst_size`.

## 2.5 `safe_strcpy` correcto (semántica)

Requisitos:

1. Si `dst_size == 0`, no escribir nada.
2. Copiar hasta `dst_size - 1` bytes.
3. Escribir `dst[last] = '\0'`.
4. Retornar longitud original de `src`.

## 2.6 `safe_strcat` correcto

Requisitos:

1. Detectar longitud actual de `dst` sin salir de `dst_size`.
2. Si no hay `\0` dentro de `dst_size`, considerar buffer corrupto/truncado y no desbordar.
3. Concatenar solo bytes disponibles (`dst_size - dst_len - 1`).
4. Null-terminate y retornar longitud objetivo total (`dst_len + src_len`).

## 2.7 `safe_snprintf` y variádicos

Usar `va_list`, `va_start`, `va_end` y `vsnprintf`:

```c
int safe_snprintf(char *dst, size_t n, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vsnprintf(dst, n, fmt, ap);
    va_end(ap);
    return r;
}
```

Retornos de `snprintf`:

1. `r < 0`: error de formato/encoding.
2. `r >= n`: salida truncada.
3. `0 <= r < n`: éxito completo.

## 2.8 Reglas de oro en buffers

1. `sizeof(buffer)` solo funciona en arrays locales, no en punteros.
2. No asumir ASCII puro si hay UTF-8 multi-byte.
3. Evitar `sprintf` (sin límite).
4. Verificar truncamiento como parte normal del flujo.

## 2.9 Testing mínimo que sí detecta fallos

Para funciones seguras, cubre:

1. `dst_size = 0`.
2. `dst_size = 1`.
3. `src` vacío.
4. `src` mucho más largo que destino.
5. `dst` casi lleno antes de `safe_strcat`.

---

## 3) `struct`, `void *` y listas enlazadas genéricas

## 3.1 Por qué lista enlazada en este bloque

No es por rendimiento puro frente a arrays; es para practicar:

1. modelado con `struct`.
2. punteros y ownership.
3. asignación/liberación por nodo.
4. invariantes de estructura.

## 3.2 Layout, alineación y padding

`struct` puede tener bytes de relleno por alineación.
No serialices un `struct` directo a disco/red sin diseño explícito.

## 3.3 API de lista genérica

Modelo típico:

```c
typedef struct node {
    void *data;
    struct node *next;
} node_t;

typedef struct {
    node_t *head;
    int size;
} list_t;
```

Operaciones del ejercicio:

1. `create`.
2. `push`.
3. `pop`.
4. `destroy`.

## 3.4 Invariantes que no deben romperse

1. `size >= 0` siempre.
2. `size == 0` implica `head == NULL`.
3. `pop` en lista vacía debe devolver `NULL` sin crashear.

## 3.5 Ownership de `data` (decisión crítica)

Hay dos modelos válidos:

1. Lista dueña del dato: lista hace `free(data)` en `destroy`.
2. Lista no dueña: lista solo libera nodos.

Si no se define explícitamente, aparecen leaks o double free.

Para una lista genérica reusable, conviene permitir callback destructor:

```c
typedef void (*free_fn)(void *);
void llist_destroy_ex(llist_t *list, free_fn fn);
```

## 3.6 `void *` sin confusiones

- `void *` permite almacenar cualquier dirección.
- No puedes desreferenciar `void *` sin cast.
- El cast debe reflejar el tipo real almacenado.

Errores comunes:

1. Guardar puntero a variable local y usarlo fuera de scope.
2. Cast incorrecto (`int *` vs `double *`) generando UB.

## 3.7 Complejidad temporal básica

Si `push` inserta al frente:

1. `push`: O(1)
2. `pop` del frente: O(1)
3. búsqueda sin índice: O(n)

## 3.8 Patron robusto de implementación

`push` seguro:

1. `malloc` nodo.
2. si falla, no modificar lista.
3. enlazar nuevo nodo a `head`.
4. incrementar `size`.

`pop` seguro:

1. si vacía, `NULL`.
2. guardar `head` temporal.
3. mover `head` a `next`.
4. decrementar `size`.
5. devolver `data` y liberar nodo.

## 3.9 Cómo validar memoria de la lista

Con Valgrind/ASan debes verificar:

1. 0 leaks tras `destroy`.
2. sin invalid read/write.
3. sin use-after-free al reusar punteros.

---

## 4) Manejo de errores POSIX y `goto cleanup`

## 4.1 Regla principal: cada llamada se verifica

En C de sistemas, nunca asumas éxito.

Tipos de fallo típicos:

1. syscalls y funciones POSIX: retorno `-1` y `errno`.
2. `malloc/calloc/realloc`: `NULL`.
3. `fopen`: `NULL`.
4. algunas APIs devuelven códigos propios (ej. pthreads) y no usan `errno` directo.

## 4.2 `errno` bien utilizado

Hechos clave:

1. `errno` solo es significativo si la llamada indicó error.
2. Puede cambiar en llamadas posteriores; captura pronto.
3. Es thread-local en implementaciones modernas.

Patrón:

```c
if (open(...) == -1) {
    int e = errno;
    fprintf(stderr, "open fallo: %s\n", strerror(e));
}
```

## 4.3 `perror`, `strerror`, `strerror_r`

- `perror("ctx")`: rápido para CLI.
- `strerror(errno)`: útil para formateo personalizado.
- `strerror_r`: para contextos thread-safe/estrictos según plataforma.

## 4.4 `goto cleanup` bien aplicado

Objetivo: un único bloque de salida que libere recursos en orden seguro.

Patrón canónico:

```c
int rc = -1;
FILE *f = NULL;
char *buf = NULL;

buf = malloc(1024);
if (!buf) goto cleanup;

f = fopen(path, "r");
if (!f) goto cleanup;

/* trabajo */
rc = 0;

cleanup:
if (f) fclose(f);
free(buf);
return rc;
```

Ventajas:

1. evita duplicación de cleanup.
2. reduce paths inconsistentes de error.
3. mejora mantenibilidad en funciones largas.

## 4.5 Macros de chequeo sin trampas

Macro segura tipo statement:

```c
#define CHECK_SYS(call) \
    do { \
        if ((call) == -1) { \
            fprintf(stderr, "[%s:%d] %s falló: %s\n", \
                    __FILE__, __LINE__, #call, strerror(errno)); \
            goto cleanup; \
        } \
    } while (0)
```

También necesitas variante para punteros (`NULL`) porque no todo falla con `-1`.

## 4.6 Errores que debes tratar explícitamente

1. `EINTR`: llamada interrumpida por señal; en algunas operaciones conviene reintentar.
2. `EACCES` / `EPERM`: permisos.
3. `ENOENT`: archivo/ruta inexistente.
4. `ENOSPC`: sin espacio en disco.

## 4.7 Contrato de salida de programas CLI

1. Error de uso (`--flag` inválida): salida no-cero + ayuda corta en `stderr`.
2. Error operacional (archivo no abre): no-cero + detalle de sistema.
3. Éxito parcial en utilidades tipo `cat` multiarchivo: política explícita y documentada.

---

## 5) Aritmética de punteros y benchmarking de memoria

## 5.1 Byte-level correcto

Para operaciones de memoria usa punteros a bytes:

```c
unsigned char *d = dest;
const unsigned char *s = src;
```

`unsigned char` evita ambigüedades por signo de `char`.

## 5.2 Reimplementar `memcpy` (didáctico)

Implementación ingenua:

```c
for (size_t i = 0; i < n; ++i) d[i] = s[i];
```

Límites importantes:

1. Si regiones se solapan, `memcpy` tiene comportamiento indefinido.
2. Para solapamiento la función correcta es `memmove`.

## 5.3 `memset` correcto

```c
for (size_t i = 0; i < n; ++i) d[i] = (unsigned char)c;
```

## 5.4 `memmove`: por qué importa

Si `dest > src` y hay solapamiento, copia hacia atrás.
Si no, copia hacia adelante.

Ese detalle evita corromper los datos fuente durante la copia.

## 5.5 Benchmark sin autoengaño

Checklist mínimo:

1. usar `clock_gettime(CLOCK_MONOTONIC, ...)`.
2. ejecutar varias iteraciones (no una sola).
3. hacer warm-up.
4. evitar que el compilador elimine trabajo (usar checksum/volatile).
5. comparar mismo tamaño, misma alineación, mismas condiciones.

## 5.6 Por qué libc gana por mucho

`memcpy` de libc suele incluir:

1. rutas vectorizadas SIMD (SSE/AVX/NEON).
2. selección por CPU en runtime (IFUNC en glibc).
3. optimizaciones por alineación y tamaños.
4. casos especiales micro-optimizados en ensamblador.

Conclusión operativa: reimplementa para aprender, usa libc en producción salvo necesidad muy específica.

## 5.7 Pitfalls en benchmarks

1. medir con `time()` de segundos.
2. mezclar debug (`-O0`) contra release (`-O2`) y comparar.
3. correr en sistema con carga alta y sacar conclusiones absolutas.
4. olvidar que caché de CPU distorsiona pruebas cortas.

---

## 6) Integración de temas en utilidades reales (`miniecho`/`minicat`)

Aunque el proyecto esté fuera de `ej_legacy`, este bloque culmina aquí.

## 6.1 `miniecho`

Debes combinar:

1. parseo de flags (`-n`, `-e`).
2. secuencias de escape (`\n`, `\t`, `\\`, `\c`).
3. comportamiento compatible con utilidades Unix.

Punto fino:

- `\c` implica terminar salida inmediatamente (incluye no imprimir `\n` final).

## 6.2 `minicat`

Debes combinar:

1. lectura de múltiples archivos.
2. soporte de `-` como stdin.
3. diagnóstico por archivo fallido sin detener procesamiento completo.
4. código de salida final coherente si hubo errores.

## 6.3 Filosofía Unix aplicada

1. herramientas pequeñas y predecibles.
2. salida apta para pipes.
3. errores explícitos y no silenciosos.
4. contrato estable de argumentos y retornos.

---

## 7) Criterios de dominio del Bloque 01

Debes ser capaz de:

1. escribir un parser CLI robusto con cortas y largas.
2. diseñar funciones de string que no desborden buffers.
3. implementar una lista enlazada genérica sin leaks.
4. manejar fallos con `errno` y cleanup unificado.
5. medir correctamente diferencias entre implementación ingenua y libc.

Si cualquiera de estos puntos falla, en bloques siguientes aparecerán bugs difíciles de rastrear.

---

## Referencias técnicas recomendadas

1. `man 3 getopt`
2. `man 3 strtol`
3. `man 3 snprintf`
4. `man 3 malloc`, `man 3 free`
5. `man 3 errno`, `man 3 perror`, `man 3 strerror`
6. `man 3 memcpy`, `man 3 memmove`, `man 3 memset`
7. `man 3 clock_gettime`

---

## Práctica del bloque

Los ejercicios nuevos del bloque (10 resueltos pedagógicos + 3 complejos) están en:

- `bloque01/EJERCICIOS.md`
