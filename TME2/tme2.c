
//   ZEGHMICHE DALILA 21509990  &&   BENATSI FERIEL 21509302

#include "fthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define CIBLE_PRODUCTION 8
#define NB_PRODUCTEURS 3
#define NB_CONSOMMATEURS 2
#define NB_MESSAGERS 2
#define CAPACITE_TAPIS_PROD 4
#define CAPACITE_TAPIS_CONSO 3
#define TEMPO_PRODUCTEUR 2
#define TEMPO_CONSOMMATEUR 3
#define TEMPO_MESSAGER 1

typedef struct{
    char contenu[100];
}paquet;


typedef struct{
    paquet *paquets;
    int capacite;
    int taille;
    int debut;
    int fin;
    ft_event_t evt_non_plein; //new_input
    ft_event_t evt_non_vide;//new_output
}tapis;

typedef struct {
    char nom[50];//nom du produit (Pomme , Orange ...)
    int id; //ident de producteur 
}args_producteurs;

typedef struct{
    int id;//ident du consommatuer 
}args_consommateur;


typedef struct{
    int id;//ident du messager
}args_messager;

//variables globales
tapis tapis_prod;
tapis tapis_cons;
//les deux ordonnanceurs
ft_scheduler_t sched_prod;
ft_scheduler_t sched_cons;
//init: cible* nbr_prods
//decrementer par les cons a chaque paquet consommé
//compteur =0:signal d'arret pour cons et messagers
//pas de mutex car fair thread = cooperatif  
int compteur_consommation;

//les 3 journaux
FILE *journal_production;
FILE *journal_consommation;
FILE *journal_voyage;


pthread_mutex_t mutex_compteur;//compteur partager entre cons et messagers(lu)
pthread_mutex_t mutex_voyage; //Tous les messagers
pthread_mutex_t mutex_journal_prod;//LES PRODS
pthread_mutex_t mutex_journal_conso;// les cons




//initialisation d'un tapis 
void tapis_init(tapis *tapis, int capacite, ft_scheduler_t sched){
    tapis->paquets=(paquet*)malloc(capacite * sizeof(paquet));
    tapis->capacite=capacite;
    tapis->taille=0;
    tapis->debut=0;
    tapis->fin=0;
    tapis->evt_non_plein=ft_event_create(sched);
    tapis->evt_non_vide=ft_event_create(sched);


}


//enfiler un paquet dans le tapis
//si plein attend (cooperativement) que le tapis se vide
//puis enfile et reveille les threads en attendent un tapis non vie 
void tapis_enfiler(tapis *tapis, paquet paquet){
    while(tapis->taille >= tapis->capacite){
        ft_thread_await(tapis->evt_non_plein);
    }

    tapis->paquets[tapis->fin] =paquet;
    tapis->fin=(tapis->fin+1) % tapis->capacite;//file circulaire
    tapis->taille++;
    //reveiller les threads en attendent un tapis non vide 
    ft_scheduler_broadcast(tapis->evt_non_vide);
}



//defiler un paquet du tapis
//si vide attend (cooperativement ) 
//puis defile et reveille les threads qui attendent un tapis non plein
paquet tapis_defiler(tapis *tapis){
    while(tapis->taille ==0){
        ft_thread_await(tapis->evt_non_vide);
    }

    paquet paquet = tapis->paquets[tapis->debut];
    tapis->debut=(tapis->debut+1) % tapis->capacite;
    tapis->taille--;
    ft_scheduler_broadcast(tapis->evt_non_plein);
    return paquet;
}



// thread producteur
//attaché a sched_prods
//produit cible de paquets
//a chaque itération:
//cree un paquet "nomProduit_N"
//enfile sur tapis_production(attend si plein)
//ecrit dans journal de prod
//coopere
//arret: quand boucle for termine (cible atteinte)


void producteur(void *arg){
    args_producteurs *a=(args_producteurs *)arg;
    printf("[Producteur %s] Démarrage\n", a -> nom);
    //boucle de production s'arrete a cible_production
   for(int i=0;i<CIBLE_PRODUCTION;i++){
    //cree le paquet 
    paquet p;
    //max 100 caractere a ecrire dans contenu
    snprintf(p.contenu, sizeof(p.contenu),"%s_%d",a->nom,i);
    //enfiler (peut attendre si tapis plein)
    printf("[Producteur %s] Enfile %s \n", a->nom,p.contenu);
    tapis_enfiler(&tapis_prod,p);
    //ecrire dans le journal (pas de mutex: cooperatif)
    
    fprintf(journal_production,"[Producteur %s] Paquet %s enfilé\n",a->nom,p.contenu);
    fflush(journal_production);
    //cooperation
    ft_thread_cooperate_n(TEMPO_PRODUCTEUR);
   }
   
   printf("[Producteur %s] Terminé (cible atteinte)\n",a->nom);
   fprintf(journal_production, "[Producteur %s] terminé \n", a->nom);
   //plus immédiat
   fflush(journal_production);

  
}


//thread cons
//attaché a sched consommation
//tourne tq compteur >0
//a chaque itération:
//vérif si cpt =0 sort
//défile un paquet du tapis de cons (attend si vide)
//ecrit dans journal de cons 
//decrémente le compteur (signal d'arret)
////coopere
//arret: quand compteur =0


void consommateur (void *arg){
    args_consommateur *a=(args_consommateur*) arg;
    printf("[Consommateur %d] Démarrage \n", a-> id);

//boucle cons s'arrete quand compteur =0
    while(compteur_consommation >0){
        //défile paquet peut attendre si tapis vide
        paquet p = tapis_defiler(&tapis_cons);
        printf("[Consommateur %d] Défile %s\n",a->id,p.contenu);
        //ecrire dans le journal (cooperatif)
        fprintf(journal_consommation,"[Consommateur %d] Paquet %s défilé \n",a->id,p.contenu);
        fflush(journal_consommation);
        //decrementer le compteur(signal d'arret)
        compteur_consommation--;
        printf("[Consommateur %d] Compteur = %d\n",a->id,compteur_consommation);
        //cooperer
        ft_thread_cooperate_n(TEMPO_CONSOMMATEUR);
    }

    //terminaison cpt=0
    printf("[Consommateur %d] terminé(compteur =0)\n,", a->id);
    fprintf(journal_consommation,"[Consommateur %d] Terminé \n",a->id);
    fflush(journal_consommation);

   





}




//thread messager
//initialement: non attaché(unlink)
//tourne tq compteur >0
//a chaque itération:
//se lie a sched_prod
//defile du tapis_prod(attend si vide)
//se détache de sched_prod
//ecrit dasn journal de voyage
//se lie a sched _cons
//enfile sur tapis de cons (attend si plein)
//se détache de sched de cons 
//coopere 
//arret quand compteur de cons =0


void messager(void *arg){
    args_messager *a = (args_messager*)arg;
    printf("[Messager %d] Démarrage (non attaché)\n",a->id);
    //se détacher(initialement non lié)
    ft_thread_unlink();
    //boucle de transport: s'arrete quand compteur =0
    while(compteur_consommation>0){
        //phase production: se lier et défiler
        ft_thread_link(sched_prod);
        paquet p=tapis_defiler(&tapis_prod);
        ft_thread_unlink();
        //ecrire dans le journal voyage (messager detaché et pas de race )
        fprintf(journal_voyage,"[Messager %d] transport de %s\n",a->id,p.contenu);
        fflush(journal_voyage);
        printf("[Messager %d] Transporte %s",a->id,p.contenu);

        //phase de consommation se lier et enfiler
        ft_thread_link(sched_cons);
        tapis_enfiler(&tapis_cons,p);
        ft_thread_unlink();
        //cooperer 
        ft_thread_cooperate_n(TEMPO_MESSAGER);
    }
    

    printf("[Messager %d] términé (compteur =0)\n", a->id);
    fprintf(journal_voyage,"[Messager %d] Terminé \n", a->id);
    fflush(journal_voyage);

   


}


int main(void){

    printf("\n");
    printf("TME2:Producteurs/Consommateurs Cooperatifs fair Threads ");
    printf("\n");

    //creer les ordonnanceurs

    sched_prod=ft_scheduler_create();
    sched_cons=ft_scheduler_create();
    printf("ordonnanceurs crees \n");

    //initialiser les tapis
    tapis_init(&tapis_prod,CAPACITE_TAPIS_PROD,sched_prod);
    tapis_init(&tapis_cons,CAPACITE_TAPIS_CONSO,sched_cons);
    printf("Tapis initialisés(prod:%d, conso:%d )\n",CAPACITE_TAPIS_PROD,CAPACITE_TAPIS_CONSO);

    //ouvrir les journaux

    journal_production=fopen("journal_production.txt", "w");
    journal_consommation=fopen("journal_consommation.txt","w");
    journal_voyage=fopen("journal_voyage.txt","w");

    if(!journal_production || !journal_production || !journal_voyage){
        fprintf(stderr,"Erreur: Impossible d'ouvrir les journaux\n");
        return EXIT_FAILURE;
    }

    printf("journaux ouverts \n");


    //initialiser le compteur (cle de l'arret )
    //total de paquets= cible * nbr_prods
    //ce compteur sera decremente par les consommateurs 
    //quand il atteint 0 -> signal d'arret

    compteur_consommation=CIBLE_PRODUCTION * NB_PRODUCTEURS;
    printf("compteur initialisé:%d paquets \n", compteur_consommation);


    printf("configuration:\n");
    printf("producteurs:%d(cible :%d paquets chacun)\n",NB_PRODUCTEURS,CIBLE_PRODUCTION);
    printf("consomateurs:%d\n",NB_CONSOMMATEURS);
    printf("messagers:%d\n",NB_MESSAGERS);
    printf("total paquets a produire:%d\n",compteur_consommation);
    printf("\n");

    printf("Creation des threads\n\n");



    //producteurs
    args_producteurs args_prod[NB_PRODUCTEURS];
    const char *nom_produits[]={"Pomme","Orange","Banane"};
    

    for(int i =0; i< NB_PRODUCTEURS;i++){
        args_prod[i].id=i;
        strncpy(args_prod[i].nom,nom_produits[i] , sizeof(args_prod[i].nom) -1);
        ft_thread_create(sched_prod,producteur,NULL,&args_prod[i]);
        printf("Producteur %s créé (sched_production)\n",args_prod[i].nom);
    }


    //consommateurs
    args_consommateur args_cons[NB_CONSOMMATEURS];
    for(int i=0; i< NB_CONSOMMATEURS;i++){
        args_cons[i].id=i;
        ft_thread_create(sched_cons,consommateur,NULL,&args_cons[i]);
        printf("Consommateur %d créé (sched_consommation)\n",args_cons[i].id);

    }



    //messagers
    args_messager args_mess[NB_MESSAGERS];
    for(int i=0;i< NB_MESSAGERS;i++){
    args_mess[i].id=i;
    //messagers crees sur sched_production mais se detachant immédiatement 
    //la fonction messager:premier appel ft_thread_unlink())
    ft_thread_create(sched_prod,messager,NULL,&args_mess[i]);
    printf("Messager %d créé (initialement non lié)\n",args_mess[i].id);

    }

    printf("\n");
    printf("=============Démarrage=============");
    printf("\n");


    //boucle principale

    //mecanisme d'arret 
    //le thread principal fait tourner les deux ordonnanceurs
    //en alternance tq le compteur >0
    //quand le compteur atteint 0:
    //les cons sortent de leur boucle
    //les messagers sortent de leur boucle
    //les prods ont deja terminé (cible atteinte)
    //le main sort de cette boucle while

  int iteration =0;

  while(compteur_consommation >0){
        iteration++;//faire tourner les deux ordonnanceurs*/
        ft_scheduler_start(sched_prod);

        ft_scheduler_start(sched_cons);




        //afficher progression toutes les 10 itérations

      if(iteration % 10 ==0){
       printf("... Iteration %d, Compteur =%d...\n",iteration,compteur_consommation);

     }

        //coopérer(laisser du temps logique)
       ft_thread_cooperate();
   }

    //terminaison

    printf("\n");
    printf("=======compteur a zero tous les paquets ont été consommés");
    printf("\n");

    printf("Statistiques:\n");
   //printf("  - Itérations totales: %d\n", iteration);
    printf("  - Paquets produits: %d\n", CIBLE_PRODUCTION * NB_PRODUCTEURS);
    printf("  - Paquets consommés: %d\n", CIBLE_PRODUCTION * NB_PRODUCTEURS);
    printf("  - Compteur final: %d\n", compteur_consommation);
    printf("\n");


//nettoyage
    printf("=======nettoyage==========");


fclose(journal_production);
    fclose(journal_consommation);
    fclose(journal_voyage);
    printf("✓ Journaux fermés\n");
    
    //Libérer la mémoire des tapis 
    free(tapis_prod.paquets);
    free(tapis_cons.paquets);
    printf("✓ Mémoire libérée\n");
    
    printf("\n");
    printf("Journaux disponibles:\n");
    printf("  → journal_production.txt\n");
    printf("  → journal_consommation.txt\n");
    printf("  → journal_voyage.txt\n");
    printf("\n");
    
    
    printf("=======FIN DU PROGRAMME====\n");
    
    printf("\n");
    
    //Terminer Fair Threads proprement 
    ft_exit();
    
    return EXIT_SUCCESS;














    
}








