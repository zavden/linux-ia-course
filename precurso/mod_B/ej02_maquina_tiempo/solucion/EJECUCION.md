# Solución del Ejercicio B.2

Si rompiste un archivo intencionalente y necesitas restaurarlo:

```bash
# Opción moderna:
git restore tu_archivo_roto.txt

# Opción clásica:
git checkout -- tu_archivo_roto.txt
```

Si habías hecho `git add` pero quieres deshacer ese stageado antes de restaurar:
```bash
git restore --staged tu_archivo_roto.txt
git restore tu_archivo_roto.txt
```
