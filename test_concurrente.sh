#!/bin/bash
# Prueba de concurrencia: lanza 5 clientes a la vez, cada uno con su propia clave

export IP_TUPLAS=localhost

echo "Lanzando 5 clientes en paralelo"

for i in 1 2 3 4 5; do
    ./app-cliente-concurrente $i > resultados_cliente_$i.txt 2>&1 &
done

wait
echo "Todos han terminado"

for i in 1 2 3 4 5; do
    echo ""
    echo "=== Cliente $i ==="
    cat resultados_cliente_$i.txt
done