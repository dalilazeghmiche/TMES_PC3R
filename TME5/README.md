
ZEGHMICHE DALILA && BENATSI FERIEL 



Exercice 2 — Labyrinthe
Problème tableau 2D : Promela n'accepte pas bit mur[N][N]
 SOLUTION : simuler avec tableau 1D + macro 
bit mur[625];                25×25 = 625 
#define MUR(i,j) mur[i*N+j]


L'astuce assert pour forcer la trace
 Si pos == SORTIE  → assertion FAUSSE → SPIN imprime le chemin 
 Si impasse        → assertion VRAIE  → SPIN ignore ce chemin   
assert(pos != SORTIE)



# Exercice 1 — Feu Tricolore
spin -search feu_naif.pml        # trouver 2 violations
spin -c -t feu_naif.pml          # afficher trace 1
spin -search feu_correct.pml     # doit passer sans erreur

# Exercice 2 — Labyrinthe
spin -search labyrinthe.pml      # trouver la violation d'assert
spin -c -t labyrinthe.pml        # afficher le chemin solution

# Exercice 3 — Famine TME4
spin -search tme4.pml            # trouver la famine
spin -c -t tme4.pml              # afficher la trace de famine