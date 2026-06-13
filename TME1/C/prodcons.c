// ZEGHMICHE DALILA BENATSI FERIEL
#include<stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>// LINUX POSIX

//structure representant un paquet
typedef struct{
    char* contenu;
}Paquet;


typedef struct{

    Paquet** file;//file de paquets 
    int capacite;//max
    int taille;//nombre actuel de paquets
    int debut;
    int fin;//fin file
    pthread_mutex_t mutex;//proteger l'acces
    pthread_cond_t non_plein;//cond:tapis non plein -- attendre
    pthread_cond_t non_vide;//cond:tapis non vide


}Tapis;

//structure pour les args des producteurs
typedef struct{
    Tapis* tapis;
    char* nom_produit;
    int cible;
   
}ArgProducteur;

//structure pour les args des consommateurs
typedef struct{
    Tapis* tapis;
    int id;
    int* compteur;
    pthread_mutex_t* mutex_compteur;
}ArgConsommateur;

//fcts pour le paquet
Paquet* creer_paquet(const char* contenu){
    Paquet* p=(Paquet*)malloc(sizeof(Paquet));
    p->contenu=(char*)malloc(strlen(contenu)+1);//cree espace
    strcpy(p->contenu, contenu);
    return p;
}

void detruire_paquet(Paquet* p) {
    if (p) {
        free(p->contenu);
        free(p);
    }
}



//fcts pour le tapis

Tapis* creer_tapis(int capacite){
    Tapis* t=(Tapis*)malloc(sizeof(Tapis));
    t->file=(Paquet**)malloc(capacite*sizeof(Paquet*));
    t->capacite=capacite;
    t->debut=0;
    t->fin=0;
    t->taille=0;
    pthread_mutex_init(&t->mutex, NULL);
    pthread_cond_init(&t->non_plein,NULL);
    pthread_cond_init(&t->non_vide,NULL);
    return t;
}



void detruire_tapis(Tapis* t) {
    if (t) {
        // Libérer les paquets restants
        while (t->taille > 0) {
            detruire_paquet(t->file[t->debut]);
            t->debut = (t->debut + 1) % t->capacite;
            t->taille--;
        }
        free(t->file);
        pthread_mutex_destroy(&t->mutex);
        pthread_cond_destroy(&t->non_plein);
        pthread_cond_destroy(&t->non_vide);
        free(t);
    }
}


//enfiler un paquet sur le tapis(bloque si plein)

void enfiler(Tapis* t, Paquet* p){
    pthread_mutex_lock(&t->mutex);

    //attendre que le tapis ne soit pas plein
    while(t->taille >= t->capacite){
        pthread_cond_wait(&t->non_plein, &t->mutex);//libere le mutex
    }

    //ajout de paquet
    t->file[t->fin]=p;
    t->fin=(t->fin +1)% t->capacite;//circulaire -- case debut consommer--revenir
    t->taille++;
    

    //signaler que il ya un nv paquet
    pthread_cond_signal(&t->non_vide);

    pthread_mutex_unlock(&t->mutex);

}

//defiler un paquet du tapis (bloque si vide)
Paquet* defiler(Tapis* t){
    pthread_mutex_lock(&t->mutex);
    while(t->taille==0){
        pthread_cond_wait(&t->non_vide,&t->mutex);
    }

    Paquet* p=t->file[t->debut];
    t->debut=(t->debut +1) %t->capacite;
    t->taille--;

    pthread_cond_signal(&t->non_plein);
    pthread_mutex_unlock(&t->mutex);
    return p;

}

//threads producteur et consommateur
void* producteur(void* arg){

    ArgProducteur* args=(ArgProducteur*) arg;

    Tapis* tapis= args->tapis;
    char* nom=args->nom_produit;
    int cible= args->cible;


    for(int i=0; i< cible;i++){
        //cree le contenu de paquet
        char contenu[256];
        //contenu="nom i"
        snprintf(contenu, sizeof(contenu), "%s %d", nom, i);

        //creer et enfiler le paquet
        Paquet* p=creer_paquet(contenu);
        enfiler(tapis,p);

        printf("Producteur [%s] a produit: %s\n", nom, contenu);
        
        // Simuler le temps de production
        usleep(rand() % 10000);
    }
    
    printf("Producteur [%s] a terminé sa production\n", nom);
    free(args->nom_produit);
    free(args);
    return NULL;

}


    void* consommateur(void* arg){
        ArgConsommateur* args= (ArgConsommateur*)arg;
        int id=args->id;
         int* compteur=args->compteur;
         Tapis* tapis=args->tapis;
         pthread_mutex_t* mutex_compteur =args->mutex_compteur;
    
        while(1){
            pthread_mutex_lock(mutex_compteur);
            if(*compteur <=0){
                pthread_mutex_unlock(mutex_compteur);
                break;
            }

            (*compteur)--;
            pthread_mutex_unlock(mutex_compteur);
            Paquet* p= defiler(tapis);
        printf("C%d mange %s\n", id, p->contenu);
        
        // Détruire le paquet
        detruire_paquet(p);
        
        // Simuler le temps de consommation
        usleep(rand() % 10000);
    }
    
    printf("Consommateur C%d a terminé\n", id);
    free(args);
    return NULL;
}






//main
int main(int argc, char* argv[]){

//param par defaut
int nb_producteurs=3;
int nb_consommateurs=2;
int cible_production=10;
int capacite_tapis=5;


//lecture des args(optionnel)
if(argc >=5){
    nb_producteurs=atoi(argv[1]);
    nb_consommateurs=atoi(argv[2]);
    cible_production=atoi(argv[3]);
    capacite_tapis=atoi(argv[4]);
}


    printf("=== Démarrage du système ===\n");
    printf("Producteurs: %d\n", nb_producteurs);
    printf("Consommateurs: %d\n", nb_consommateurs);
    printf("Cible par producteur: %d\n", cible_production);
    printf("Capacité du tapis: %d\n\n", capacite_tapis);

// Initialiser le générateur aléatoire
    srand(time(NULL));

    Tapis* tapis=creer_tapis(capacite_tapis);


    //creer le compteur partage
    int compteur=nb_producteurs * cible_production;
    pthread_mutex_t mutex_compteur;
    pthread_mutex_init(&mutex_compteur, NULL);


    //creer les threads 
    pthread_t* threads_prod=(pthread_t*)malloc(nb_producteurs * sizeof(pthread_t));
    pthread_t* threads_cons=(pthread_t*)malloc(nb_consommateurs * sizeof(pthread_t));

    //Lancer les producteurs
    char* noms_produits[]={"Pomme", "Banane","Orange","Poire", "Fraise","Cerise"};
    
    for(int i=0;i<nb_producteurs;i++){
        ArgProducteur* args=(ArgProducteur*)malloc(sizeof(ArgProducteur));
        args->tapis= tapis;
        args->nom_produit=(char*)malloc(strlen(noms_produits[i % 6])+1);
        strcpy(args->nom_produit,noms_produits[i%6]);
        args->cible=cible_production;
        pthread_create(&threads_prod[i],NULL,producteur,args);

    }


    //lancer consommateurs
    for(int i=0; i<nb_consommateurs;i++){

        ArgConsommateur* args=(ArgConsommateur*)malloc(sizeof(ArgConsommateur));
        args->id=i+1;
        args->tapis=tapis;
        args->compteur=&compteur;
        args->mutex_compteur=&mutex_compteur;

        pthread_create(&threads_cons[i],NULL,consommateur,args);

    }


    //attendre la fin des producteurs
    for(int i=0;i<nb_producteurs;i++){
        pthread_join(threads_prod[i],NULL);
    }


 //attendre la fin des cons
    for(int i=0;i<nb_consommateurs;i++){
        pthread_join(threads_cons[i],NULL);
    }


    printf("\n=== Tous les threads ont terminé ===\n");
    printf("Compteur final: %d (devrait être 0)\n", compteur);
    // Nettoyage
    detruire_tapis(tapis);
    pthread_mutex_destroy(&mutex_compteur);
    free(threads_prod);
    free(threads_cons);





return 0;







}






