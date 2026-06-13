//objectif: exhiber la famine des ouvriers 
//blocage des files des gestionnaires par les producteurs

//2 prods : creent des paquets vides -> file gestionnaires
//2 gestionnaires: file bornee(CAP_FILE), propose tete de file aux ouvriers
//3 ouvriers: trait les paquets et les retournent au gestionnaire 
//(ou les envoient au collecteur si finis)
//1 collecteur: recoit les paquets finis

//famine: les producteurs remplissent les files des gestionnaires, 
//les ouvriers ne peuvent plus retourner leurs paquets



mtype={V, R, C} //statuts: Vide , en couRs , fini(Complete)

#define NB_prod 2
#define NB_GEST 2
#define NB_OUV 3
#define CAP_FILE 2 //capacite bornee petite pour provoquer la famine
#define NB_TACHES 2 //nombre de travaux nécessaires pour terminer un paquet

typedef Paquet{
    mtype statut;
    byte taches;
    byte gest_id; //gestionnaire auqeul retourner le paquet
}

//file de chaque gestionnaire: recoit des producteurs et des ouvriers
chan file_gest[NB_GEST] = [CAP_FILE] of {Paquet}

//canal partagé gestionnaire -> ouvriers(synchrone : pas de buffer)
chan vers_ouvriers = [0] of {Paquet}

//canal vers collecteur
chan vers_collecteur = [5] of {Paquet}

//flag de famine: ouvrier[i] est bloque en essayant de rendre 
//un paquet a un gestionnaire dont la file est pleine 

bool bloque[NB_OUV]

//producteur: cree des paquets vides en boucle et les enfile
//chez son gestionnaire attitré
proctype producteur(byte id){
    Paquet p;
    p.statut = V;
    p.taches =0;
    p.gest_id=id % NB_GEST;
    do
    :: //envoi bloquant si la file est pleine c'est lui qui bloque les ouvries 
    file_gest[id % NB_GEST]!p
    od
}

//gestionnaire : lit sa file et propose le paquet aux ouvriers
//ne fait pas de distiction producteur/ouvrier:
//tout arrive dans la meme file bornee

proctype gestionnaire(byte id){
    Paquet p;
    do
    :: file_gest[id]?p -> //prendre la tete de file
    p.gest_id = id; //marquer l'origine pour le retour
    vers_ouvriers!p//proposer aux ouvriers (synchrone)
    od
}

proctype ouvrier(byte id){
    Paquet p;
    do
    :: vers_ouvriers?p ->
    if
    //paquet vide: initialiser(passer a R, donner des taches)
    :: p.statut == V ->p.statut=R;
       p.taches = NB_TACHES;
       //retourner au gestionnaire peut bloquer si la file pleine
       bloque[id] = true;
       file_gest[p.gest_id]!p;
       bloque[id]=false;
       //paquet en cours effectuer une taches

    :: p.statut==R-> 
       p.taches--;
       if
       :: p.taches == 0 -> p.statut = C
       :: else          -> skip;
       fi   
       if
       //paquet maintenant fini: envoyer au collecteur
       :: p.statut == C -> vers_collecteur!p
       //encore des taches : retourner au gestionnaire
       :: else-> bloque[id]=true;
          file_gest[p.gest_id]!p;
          bloque[id]=false;
          fi
        //paquet fini (cas direct)  
       :: p.statut == C -> vers_collecteur!p
       fi
    od      

       
       
       }

proctype collecteur(){
    Paquet p;
    int nb_collectes;
    do 
    :: vers_collecteur?p -> nb_collectes = nb_collectes +1;
    od


}       

//observateur de famine
//detecte quand un ouvrier est bloque (veut rendre un paquet)
//alors que la file du gestionnaire cible est pleine.
proctype obs_famine(){
    do
    // verifier chaque ouvrier
    :: atomic{
        (bloque[0] && len(file_gest[0]) == CAP_FILE) || 
        (bloque[0] && len(file_gest[1]) == CAP_FILE) ||
        (bloque[1] && len(file_gest[0]) == CAP_FILE) ||
        (bloque[1] && len(file_gest[1]) == CAP_FILE) ||
        (bloque[2] && len(file_gest[0]) == CAP_FILE) ||
        (bloque[2] && len(file_gest[1]) == CAP_FILE) ->
        //famine detectee: assertion violee = SPIN produit la trace
        assert(false)

    }
    od 

}



init{
    run collecteur();
    run obs_famine();
    atomic{
        run producteur(0);
        run producteur(1);
        run gestionnaire(0);
        run gestionnaire(1);
        run ouvrier(0);
        run ouvrier(1);
        run ouvrier(2);
    }
}
























