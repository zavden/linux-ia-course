#include <stdio.h>
#include <string.h>

int main(void) {
  // 1. Caso de solapamiento en el que memcpy PODRÍA fallar.
  char str1[] = "1234567890"; // String modificable

  printf("Buffer original: %s\n", str1);

  // Queremos copiar "1234" desde str1[0] hacia str1[2], solapando.
  // MemMove es SEGURO con solapamientos (Overlap)
  memmove(str1 + 2, str1, 4);
  printf("Tras memmove:    %s\n", str1);
  // Resultado correcto garantizado: "1212347890"

  // IMPORTANTE:
  // memcpy(str2 + 2, str2, 4);
  // Hacer esto invocará UNDEFINED BEHAVIOR en C.
  // Podría resultar en "1212127890" porque copió str[0] en str[2],
  // luego trata de copiar str[2] originial, pero ya lo sobreescribió con el
  // '1'.

  // Lo comento porque algunos compiladores modernos incluyen chequeos en
  // memcpy u optimizan haciendo alias interno de memmove, pero NUNCA confíes en
  // esto.

  // Demostrando conceptualmente el bug de memcpy naive en overlap "forward":
  printf("Por qué NO usar memcpy naive conceptualmente:\n");
  char str3[] = "1234567890";
  char *dest = str3 + 2;
  char *src = str3;
  printf("  Original:      %s\n", str3);

  // Copy naive byte por byte:
  for (int i = 0; i < 4; i++) {
    dest[i] = src[i];
  }
  printf("  Error logico:  %s\n", str3);
  // Imprime "1212127890". Vemos que perdimos el '3' y el '4'.

  // memmove internamente haría el for() en reversa si dest > src.

  return 0;
}
