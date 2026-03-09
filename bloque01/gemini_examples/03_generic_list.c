#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definir tipo de función para liberar los datos (polimorfismo en C)
typedef void (*free_func_t)(void *);

// Nodo de la lista genérica
typedef struct node {
  void *data;
  struct node *next;
} node_t;

// Estructura de control de la lista
typedef struct {
  node_t *head;
  node_t *tail;
  size_t size;
  free_func_t free_func; // Cómo liberar los elementos que aloja esta lista
} list_t;

// Crear e inicializar lista
list_t *list_create(free_func_t free_func) {
  list_t *list = malloc(sizeof(list_t));
  if (!list)
    return NULL;

  list->head = NULL;
  list->tail = NULL;
  list->size = 0;
  list->free_func = free_func;

  return list;
}

// Añadir al final (O(1) porque tenemos puntero al tail)
int list_append(list_t *list, void *data) {
  node_t *node = malloc(sizeof(node_t));
  if (!node)
    return -1;

  node->data = data;
  node->next = NULL;

  if (list->tail) {
    list->tail->next = node;
  } else {
    list->head = node;
  }
  list->tail = node;
  list->size++;

  return 0;
}

// Destruir toda la lista
void list_destroy(list_t *list) {
  if (!list)
    return;

  node_t *current = list->head;
  while (current) {
    node_t *next = current->next;
    // Solo llamamos la función si los datos son 'owned' (se proporcionó
    // función)
    if (list->free_func && current->data) {
      list->free_func(current->data);
    }
    free(current);
    current = next;
  }
  free(list);
}

// ----- EJEMPLO DE USO -----

// Un struct complejo para guardar en la lista
typedef struct {
  int id;
  char *name;
} user_t;

// Función específica para destruir mi tipo custom
void free_user(void *data) {
  user_t *u = (user_t *)data;
  printf("[Liberando User ID: %d]\n", u->id);
  free(u->name); // Liberar campos internos asignados dinámicamente
  free(u);       // Liberar el struct encapsulador
}

int main(void) {
  // 1. Crear la lista y decirle cómo liberar los datos ("Owned model")
  list_t *users = list_create(free_user);
  if (!users) {
    return EXIT_FAILURE;
  }

  // 2. Insertar elementos
  user_t *u1 = malloc(sizeof(user_t));
  u1->id = 1;
  u1->name = strdup("Alice");
  list_append(users, u1);

  user_t *u2 = malloc(sizeof(user_t));
  u2->id = 2;
  u2->name = strdup("Bob");
  list_append(users, u2);

  // 3. Iterar genéricamente
  printf("Usuarios registrados: %zu\n", users->size);
  for (node_t *curr = users->head; curr != NULL; curr = curr->next) {
    user_t *u = (user_t *)curr->data;
    printf("- User %d: %s\n", u->id, u->name);
  }

  // 4. Cleanup total en una llamada (sin memory leaks)
  printf("Destruyendo lista...\n");
  list_destroy(users);

  return EXIT_SUCCESS;
}
