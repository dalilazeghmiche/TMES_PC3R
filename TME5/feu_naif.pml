





mtype={rouge, vert, orange}

//canal de communication feu -> observateur
//non-deterministe: choix libre entre les 3 couleurs
proctype feu(chan obs){
    do
    :: obs!rouge
    :: obs!vert
    :: obs!orange
    od
}

//observateur naif:
//verifie seulement qu'on ne voit pas deux fois la meme couleur
//consecutivement. n'assure pas le cycle complet 

proctype observateur_naif(chan obs){
    mtype prev, curr;
    obs?prev;
    printf("Couleur initiale: %e\n",prev);
    do
    :: obs?curr ->
       printf("Couleur recue: %e (precedente:%e)\n", curr, prev);
       assert(curr != prev);
       prev = curr;

    od   
}



init{
    chan obs = [0] of {mtype};
    run feu(obs);
    run observateur_naif(obs);
}