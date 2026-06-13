#define N      25
#define ENTREE 24
#define SORTIE 0

/*  0  1  2  3  4   row 5 (haut) - SORTIE = 0
   
   20 21 22 23 24   row 1 (bas)  - ENTREE = 24  */

bit mur[625];
bit visite[25];
int chemin[25];
int longueur;

#define MUR(i,j) mur[i*N+j]
proctype explorateur() {
    int pos;
    pos = ENTREE;
    chemin[0] = ENTREE;
    longueur = 1;
    visite[ENTREE] = 1;

    do
    :: pos == SORTIE -> break
    :: pos != SORTIE ->
        if 
           //aller vers le HAUT
        :: (pos-5 >= 0 && !MUR(pos,pos-5) && !visite[pos-5]) ->
            atomic { pos = pos-5; visite[pos]=1; chemin[longueur]=pos; longueur++; }
          //aller vers la GAUCHE
        :: (pos%5 != 0 && !MUR(pos,pos-1) && !visite[pos-1]) ->
            atomic { pos = pos-1; visite[pos]=1; chemin[longueur]=pos; longueur++; }
          //aller vers la DROITE
        :: (pos%5 != 4 && !MUR(pos,pos+1) && !visite[pos+1]) ->
            atomic { pos = pos+1; visite[pos]=1; chemin[longueur]=pos; longueur++; }
          //aller vers le BAS :
        :: (pos+5 < N && !MUR(pos,pos+5) && !visite[pos+5]) ->
            atomic { pos = pos+5; visite[pos]=1; chemin[longueur]=pos; longueur++; }
        
        /* IMPASSE : Si on est coincé, on quitte la boucle proprement.
           SPIN va automatiquement annuler ce chemin et essayer les autres. */
        :: else -> break 
        fi
    od;

    /* MAGIE : Si on a quitté la boucle à cause d'une impasse, pos n'est pas 0.
       L'assertion est VRAIE, il ne se passe rien (pas d'erreur).
       Mais si on a trouvé la sortie, pos VAUT 0 ! 
       L'assertion devient FAUSSE, et SPIN imprime la trace de la solution ! */
    assert(pos != SORTIE)
}

init {
    MUR(20,21)=1; MUR(21,20)=1;
    MUR(15,16)=1; MUR(16,15)=1;
    MUR(15,10)=1; MUR(10,15)=1;

    MUR(6,5)=1;   MUR(5,6)=1;
    MUR(6,1)=1;   MUR(1,6)=1;
    MUR(5,10)=1;  MUR(10,5)=1;
    MUR(7,2)=1;   MUR(2,7)=1;

    MUR(3,4)=1;   MUR(4,3)=1;
    MUR(8,9)=1;   MUR(9,8)=1;
    MUR(8,13)=1;  MUR(13,8)=1;
    MUR(7,12)=1;  MUR(12,7)=1;
    MUR(11,12)=1; MUR(12,11)=1;
    MUR(12,17)=1; MUR(17,12)=1;
    MUR(13,18)=1; MUR(18,13)=1;
    MUR(13,14)=1; MUR(14,13)=1;
    MUR(17,22)=1; MUR(22,17)=1;
    MUR(18,23)=1; MUR(23,18)=1;
    MUR(23,24)=1; MUR(24,23)=1;

    run explorateur()
}