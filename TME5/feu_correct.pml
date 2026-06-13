
//ZEGHMICHE DALILA && BENATSI FERIEL

//le feu est deterministe et suit le cycle imposé:
//rouge -> vert -> orange -> rouge ->....

mtype={rouge, vert, orange}
proctype feu(chan obs){
    do
    :: obs!rouge; obs!vert; obs!orange //a chaque tour rouge → vert → orange
    od
}


//observateur correct: verifier le cycle complet 
//chaque assertion garantit la bonne couleur a chaque etape 

proctype observateur(chan obs){
    mtype c;
    loop:
    obs?c;
    printf("Recu: %e (attendu:rouge)\n", c);
    assert(c== rouge);

    obs?c;
    printf("Recu: %e (attendu: vert)\n",c);
    assert(c==vert);


    obs?c;
    printf("Recu: %e (attendu: orange)\n",c);
    assert(c == orange);

    goto loop
}


init{
    chan obs =  [0] of {mtype};
    run feu(obs);
    run observateur(obs)
}