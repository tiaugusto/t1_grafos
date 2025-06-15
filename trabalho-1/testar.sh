#!/bin/bash

EXEMPLOS_DIR="exemplos"
EXEC="./teste"  # Substitua pelo seu binário

passou=0
falhou=0

for in_file in "$EXEMPLOS_DIR"/*.in; do
    base=$(basename "$in_file" .in)
    out_file="$EXEMPLOS_DIR/$base.out"

    if [[ ! -f "$out_file" ]]; then
        echo "[AVISO] Arquivo .out não encontrado para $base"
        continue
    fi

    echo -n "Testando $base... "

    # Executa o programa com entrada do .in e salva a saída temporária
    $EXEC < "$in_file" > saida.tmp

    # Compara saída gerada com saída esperada
    if diff -q saida.tmp "$out_file" > /dev/null; then
        echo "✅ OK"
        ((passou++))
    else
        echo "❌ ERRO"
        echo "Diferenças:"
        diff -u "$out_file" saida.tmp
        ((falhou++))
    fi
done

rm -f saida.tmp

echo
echo "✔️  $passou testes passaram"
echo "❌  $falhou testes falharam"
