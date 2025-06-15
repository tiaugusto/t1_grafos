# Trabalho de Algoritmos e Teoria dos Grafos

Este projeto implementa um analisador de grafos não direcionados com suporte às seguintes análises:

- Componentes conexas  
- Verificação de bipartição  
- Cálculo de diâmetro por componente  
- Identificação de vértices de corte (articulação)  
- Identificação de arestas de corte (pontes)  

 **Nota:** se o peso da aresta for omitido, assume-se **1** como sendo o valor padrão.  
 O cálculo do diâmetro **leva os pesos em conta**.

---

## 📥 Entrada de Exemplo

```text
// o nome do grafo
triângulo_com_vértice

// uma lista com três arestas e seus pesos
um -- dois 12
dois -- quatro 24
quatro -- um 41

// um vértice isolado
três
```

## 📤 Saída Correspondente
```text
grafo: triângulo_com_vértice
4 vertices
3 arestas
2 componentes
não bipartido
diâmetros: 0 36
vértices de corte:
arestas de corte:
```
