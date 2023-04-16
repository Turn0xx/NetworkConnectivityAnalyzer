# Projet : Parallélisation de l'algorithme de fermeture transitive

## Étudiants
- Mazlani Khalil  21908958
- El Hilali Mouad 21904365

## Introduction

Dans ce projet, nous avons travaillé sur la parallélisation de l'algorithme de fermeture transitive pour le calcul des composantes connexes d'un graphe. Les composantes connexes sont utiles pour identifier des communautés distinctes dans les données issues de l'internet, comme les relations entre utilisateurs de Twitter ou les liens entre les pages web. Nous avons utilisé MPI et OpenMP pour paralléliser l'algorithme de Warshall qui calcule la fermeture transitive d'un graphe.

## Difficultés rencontrées et idées de parallélisation

La principale difficulté rencontrée lors de la parallélisation était la dépendance des données dans l'algorithme de Warshall. Pour résoudre ce problème, nous nous sommes inspirés de l'article [1] pour diviser la matrice d'adjacence en sous-matrices et distribuer les calculs aux différents processeurs. De plus, nous avons utilisé OpenMP pour paralléliser les boucles internes de l'algorithme de Warshall, permettant ainsi un gain de performance supplémentaire.

# Mesures de performance

Nous avons effectué des tests de performance sur une machine équipée de 6 cœurs et 12 threads, avec un cache L2 de 250 Mo. Les tests ont été réalisés sur des matrices de tailles 1000x1000, 3000x3000 et 10000x10000. Nous avons utilisé `mpirun -n 4` pour MPI et `omp_set_num_threads(3)` pour OpenMP car on a tester avec plusieurs valeur pour openmp et 3 était la plus performantes.

## Plateforme matérielle

* CPU : 6 cœurs, 12 threads
* Cache L2 : 250 Mo

## Utilisation
 Pour effectuer des tests sur les différentes tailles de matrices, utilisez les cibles make fournies dans le Makefile. Voici les cibles disponibles et leur fonction :

* `make test-par-1000` : Exécute les tests séquentiels et parallèles pour une matrice 1000x1000 et compare les résultats.
* `make test-par-3000` : Exécute les tests séquentiels et parallèles pour une matrice 3000x3000 et compare les résultats.
* `make test-par-10000` : Exécute les tests séquentiels et parallèles pour une matrice 10000x10000 et compare les résultats.


## Résultats

| Taille de la matrice | Temps séquentiel | Temps parallélisé | Accélération | Efficacité |
| -------------------- | ---------------- | ----------------- | ------------ | ---------- |
| 10000x10000          | 700 s            | 98 s              | 7.14         | 1,785      |
| 3000x3000            | 19 s             | 2.80 s            | 6.79         | 1,698      |
| 1000x1000            | 0.70 s           | 0.28 s            | 2.50         | 0,625      |

Les résultats montrent que notre implémentation parallélisée offre une accélération significative par rapport à l'approche séquentielle pour les matrices de grande taille, avec une efficacité relativement élevée pour les matrices 10000x10000 et 3000x3000. Cependant, pour les matrices de taille plus petite (1000x1000), l'efficacité est moindre. Cela suggère que des améliorations supplémentaires pourraient être apportées à notre approche pour mieux gérer les graphes de différentes tailles et densités.

## Optimisation
Il est possible que la modification du `#define BLOCK_SIZE 10` puisse influencer les performances. Changer cette valeur peut entraîner une meilleure utilisation de la mémoire cache et une réduction des accès mémoire, ce qui pourrait améliorer les performances globales. Cependant, il est essentiel d'effectuer des tests supplémentaires pour déterminer la valeur optimale de BLOCK_SIZE, car cela dépendra de l'architecture matérielle et de la taille de la matrice.


## Conclusion

Notre travail sur la parallélisation de l'algorithme de fermeture transitive pour le calcul des composantes connexes d'un graphe a abouti à une implémentation MPI et OpenMP efficace et performante. Les résultats obtenus montrent une accélération significative par rapport à l'implémentation séquentielle, en particulier pour les matrices de grande taille.

Les principales difficultés rencontrées lors de la parallélisation étaient liées à la dépendance des données dans l'algorithme de Warshall. En utilisant l'article [1] comme référence, nous avons réussi à diviser la matrice d'adjacence en sous-matrices et à distribuer les calculs aux différents processeurs. L'utilisation d'OpenMP pour paralléliser les boucles internes a également contribué à améliorer les performances de notre implémentation.

Dans l'ensemble, notre approche parallélisée a montré des résultats prometteurs et a démontré l'efficacité de la combinaison de MPI et OpenMP pour résoudre le problème de calcul des composantes connexes d'un graphe. Dans les travaux futurs, nous pourrions explorer d'autres techniques de parallélisation pour améliorer encore les performances de l'algorithme et adapter notre approche à des graphes de tailles et de densités variables.
