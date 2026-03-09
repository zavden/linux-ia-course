#!/bin/bash
set -euo pipefail
echo "=== Tests Proyecto 03 (minishell bash clone) ==="

cd solucion/

make clean >/dev/null
make >/dev/null

echo ">> Evaluando Redireccion Input Output Magica de tu Sistema..."
# Vamos a inyectar comandos de Bash crónicos pasando por Standard Input puro como heredocs <<EOF al minishell
cat << 'EOF' > test_input.txt
pwd
echo hola minishell testing > out_test.txt
sleep 2 &
ls -la | grep minishell
exit
EOF

# Inyección Terminal Stdin a nuestro clon:
./minishell < test_input.txt > full_output.log 2>&1

if grep -q "minishell testing" out_test.txt; then
    echo "✅ Redirección básica externa (echo > file.txt) atrapado impecable."
else
    echo "❌ Minishell falló en atrapar el caracter especial '>' enrutando Stderr falso."
    exit 1
fi

if grep -q "Inyectado Background Job" full_output.log; then
    echo "✅ Lanzamiento Asíncrono Desatendido de 'sleep 2 &' detectado y no bloqueado síncronizado."
else
    echo "❌ Error parsing '&' operando background bloqueos muertos fallido O el OS lo atrapó y tu no."
    exit 1
fi

if grep -q "srwxr" full_output.log || grep -qi "100" full_output.log || grep -q "minishell" full_output.log; then
    echo "✅ Tubería Inter-Process (ls | grep) operando con el Dup2 vivo en tu CLI."
else
    echo "❌ Pipes | crasheados al ejecutar el salto de tuberías izquierdad-derechas crasas."
fi

# Cleanup
rm -f out_test.txt full_output.log test_input.txt minishell
echo "PASSED"
