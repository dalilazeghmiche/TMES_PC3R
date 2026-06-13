
## Auteurs

- Dalila Zeghmiche 21509990
- Feriel Benatsi 21509302

Master 1 – Sciences et Technologies du Logiciel  
Sorbonne Université  
PC3R – TME2

1. Objectif du TME

L’objectif de ce TME est de manipuler les threads coopératifs via la bibliothèque FairThreads (ft_v1.1).

Nous avons implémenté un système concurrent composé de :
n threads producteurs
m threads consommateurs
p threads messagers
deux ordonnanceurs (production / consommation)
deux tapis (files bornées)
trois journaux (production, consommation, voyage)


2.Ce TME nous a permis de comprendre :

Le modèle coopératif des threads
La gestion d’ordonnanceurs multiples
La synchronisation par événements (await, broadcast)
Les files bornées (tapis)
Les problèmes de compatibilité entre anciennes bibliothèques et compilateurs modernes
Les erreurs liées aux appels incorrects du scheduler
Nous avons également appris à :
Corriger des erreurs de linkage
Adapter du code à une ancienne API


difficulté est :
 Synchroniser correctement deux ordonnanceurs indépendants
 Gérer des threads qui migrent entre schedulers
 Éviter d’appeler ft_scheduler_start plusieurs fois
 Éviter de libérer la mémoire avant la fin réelle des threads
