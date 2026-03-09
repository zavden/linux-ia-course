# Solución del Ejercicio B.1

Para trackear tu proyecto y comitearlo:

```bash
# 1. Inicializar repositorio
git init

# 2. Ignorar binarios (Crea un .gitignore)
echo "*.o" > .gitignore
echo "build/" >> .gitignore
echo "*.exe" >> .gitignore
echo "*.out" >> .gitignore

# 3. Añadir todos los archivos
git add .

# 4. Crear el commit
git commit -m "Solución inicial del ejercicio"
```
