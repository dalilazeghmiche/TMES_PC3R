# TMES_PC3R

Travaux pratiques (TMEs) du cours **PC3R** (Programmation Concurrente, Réactive, Répartie et Réticulaire) — Master 1 STL, Sorbonne Université.

**Auteurs :** Dalila Zeghmiche & Feriel Benatsi

## Structure du dépôt

```
TMES_PC3R/
├── TME1/   # Producteurs/consommateurs (C, Java, Rust)
├── TME2/   # Threads coopératifs avec FairThreads (ft_v1.1)
├── TME3/   # Introduction à Go / MapReduce
├── TME4/   # Système client/serveur en Go (canaux, anti-famine)
└── TME5/   # Vérification de modèles avec Promela/SPIN
```

---

## TME1 — Producteurs/Consommateurs

Implémentation du problème classique des producteurs/consommateurs avec buffer borné, en trois langages.

### C (`TME1/C`)
Implémentation avec threads POSIX, mutex et variables de condition.

```bash
cd TME1/C
make
./prodcons
# ou
./prodcons nb_prod nb_cons cible capacite
make clean
```

### Java (`TME1/JAVA`)
Implémentation équivalente avec les threads Java (`Producteur`, `Consommateur`, `Tapis`, `Compteur`, `Paquet`).

Projet Eclipse (`.project` / `.classpath` fournis).

### Rust (`TME1/TME1_RUST`)
Implémentation Cargo.

```bash
cd TME1/TME1_RUST
cargo run
```

---

## TME2 — FairThreads (ft_v1.1)

Manipulation des threads coopératifs via la bibliothèque **FairThreads**. Système concurrent composé de :
- n threads producteurs, m threads consommateurs, p threads messagers
- deux ordonnanceurs (production / consommation)
- deux tapis (files bornées)
- trois journaux (production, consommation, voyage)

### Compilation de la bibliothèque

```bash
cd TME2/ft_v1.1
# éditer le fichier config si besoin (chemins gcc/make/ar/ranlib)
cd src
make
```

### Compilation et exécution du TME

```bash
cd TME2
make
./tme2
```

Sorties générées : `journal_production.txt`, `journal_consommation.txt`, `journal_voyage.txt`.

---

## TME3 — Introduction à Go / MapReduce

Premiers pas avec Go et implémentation d'un MapReduce simple.

```bash
cd TME3
go run main.go
go run mapreduce.go stop_times.txt
```

---

## TME4 — Système Client/Serveur en Go

Système illustrant :
- communication par canaux synchrones en Go
- implémentation d'interfaces distantes (patron Moignon/Proxy)
- coordination de goroutines sans partage mémoire direct
- **mécanisme anti-famine** : quand la file d'un gestionnaire est pleine, il cesse d'accepter de nouveaux paquets des producteurs et ne traite plus que les retours des ouvriers, empêchant ainsi le blocage du système.

### Lancement (deux terminaux)

**Terminal 1 — Serveur**
```bash
cd TME4/tme4-squelette/tme4-squelette/serveur
go build -o serveur_bin .
./serveur_bin 8080
# Affiche : "Ecoute sur localhost:8080"
```

**Terminal 2 — Client**
```bash
cd TME4/tme4-squelette/tme4-squelette/client
go build -o client_bin .
./client_bin 8080 3000
```

---

## TME5 — Vérification de modèles (Promela / SPIN)

Modélisation et vérification de systèmes concurrents avec SPIN.

### Exercice 1 — Feu tricolore
```bash
spin -search feu_naif.pml        # trouver 2 violations
spin -c -t feu_naif.pml           # afficher trace 1
spin -search feu_correct.pml      # doit passer sans erreur
```

### Exercice 2 — Labyrinthe
Le tableau 2D `bit mur[N][N]` n'étant pas supporté par Promela, il est simulé par un tableau 1D + macro :
```c
bit mur[625];           // 25×25 = 625
#define MUR(i,j) mur[i*N+j]
```
L'astuce de l'assertion inversée `assert(pos != SORTIE)` force SPIN à imprimer le chemin solution lorsqu'il atteint la sortie (assertion fausse → contre-exemple = trace voulue).

```bash
spin -search labyrinthe.pml      # trouver la violation d'assert
spin -c -t labyrinthe.pml        # afficher le chemin solution
```

### Exercice 3 — Famine (TME4)
```bash
spin -search tme4.pml            # trouver la famine
spin -c -t tme4.pml              # afficher la trace de famine
```