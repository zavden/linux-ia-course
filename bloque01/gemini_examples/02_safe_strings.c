#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Función copy segura y garantizada NULL-terminated
// char *strncpy(char *dest, const char *src, size_t n) NO garantiza el \0 si
// len(src) >= n.
size_t safe_strncpy(char *dest, const char *src, size_t dest_size) {
  if (dest_size == 0)
    return 0;

  size_t i;
  for (i = 0; i < dest_size - 1 && src[i] != '\0'; i++) {
    dest[i] = src[i];
  }
  dest[i] = '\0'; // Siempre null-terminated

  return i; // Bytes copiados (sin contar \0)
}

// Función concat segura y garantizada NULL-terminated
// char *strncat(char *dest, const char *src, size_t n) es engañosa con la "n".
size_t safe_strncat(char *dest, const char *src, size_t dest_size) {
  size_t dest_len = strlen(dest);
  if (dest_len >= dest_size)
    return 0; // El buffer ya estaba desbordado logicamente

  size_t space_left = dest_size - dest_len;
  return safe_strncpy(dest + dest_len, src, space_left);
}

int main(void) {
  char buffer[10];

  printf("=== safe_strncpy ===\n");
  // Test 1: Cabe perfecto
  safe_strncpy(buffer, "Hola", sizeof(buffer));
  printf("Caso 1: %s\n", buffer); // Hola

  // Test 2: Sobrecarga (len 13 > buffer 10)
  // strncpy estándar copiaría 10 chars sin \0 final. safe_strncpy copiará 9 y
  // pondrá \0.
  size_t copied = safe_strncpy(buffer, "Hola123456789", sizeof(buffer));
  printf("Caso 2: %s (Copiados: %zu)\n", buffer, copied); // Hola12345 (9 chars)

  printf("\n=== safe_strncat ===\n");
  // Test 3: Concatenar seguro
  safe_strncpy(buffer, "A", sizeof(buffer));
  safe_strncat(buffer, "B", sizeof(buffer));
  printf("Caso 3: %s\n", buffer); // AB

  // Test 4: Concatenar hasta el límite
  safe_strncat(buffer, "CDEFGHIJKLMN", sizeof(buffer));
  printf("Caso 4: %s\n", buffer); // ABCDEFGHI (9 chars máximo)

  return 0;
}
