// ZEGHMICHE DALILA BENATSI FERIEL
// montrer ce qui ce passe sans protection (mutex)

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

// structure de paquet

typedef struct
{
    char contenu[256];
} Paquet;

// structure de tapis (sans protection)
typedef struct
{
    Paquet *file;
    int capacite;
    int tete;
    int queue;
    int taille;
} TapisNaif;

// probleme 01 : plusieurs threads peuvent le modifier simultanement (pas de mutex)
// probleme 02 : on peut avoir de l'attente active (sans condition variable)

// compteur naif (sans protection)
typedef struct
{
    int valeur;
} CompteurNaif;

// arguements des threads

typedef struct
{
    TapisNaif *tapis;
    char nom_produit[256];
    int cible;
} ArgProducteurNaif;

typedef struct
{
    TapisNaif *tapis;
    int id;
    CompteurNaif *compteur;
} ArgConsommateurNaif;

// initialisation tapis naif
void initialiser_tapis_naif(TapisNaif *t, int capacite)
{
    t->file = (Paquet *)malloc(sizeof(Paquet) * capacite);
    if (t->file == NULL)
    {
        perror("Erreur d'allocation de mémoire pour le tapis naif");
        exit(EXIT_FAILURE);
    }
    t->capacite = capacite;
    t->tete = 0;
    t->queue = 0;
    t->taille = 0;
}

void detruire_tapis_naif(TapisNaif *t)
{
    free(t->file);
}

// enfiler naif
void enfiler_naif(TapisNaif *t, Paquet p)
{
    // attente active si plein
    while (t->taille == t->capacite)
    {
        usleep(100); // petite pause
    }

    int position = t->queue;
    // insertion
    t->file[position] = p;

    t->queue = (t->queue + 1) % t->capacite;
    t->taille++; // cest pas atomique on peut avoir DR
}

// defiler naif
Paquet defiler_naif(TapisNaif *t)
{
    // attente active si vide
    while (t->taille <= 0)
    {
        usleep(100);
    }
    int position = t->tete;
    Paquet p = t->file[position];
    t->tete = (t->tete + 1) % t->capacite;
    t->taille--; // cest pas atomique

    return p;
}

// threads producteur et consommateur naif
void *producteur_naif(void *arg)
{
    ArgProducteurNaif *args = (ArgProducteurNaif *)arg;

    printf("[PROD-NAIF] %s demarre\n", args->nom_produit);

    for (int i = 0; i < args->cible; i++)
    {
        // cree le contenu de paquet
        Paquet p;
        snprintf(p.contenu, "%s %d", args->nom_produit, i);

        // enfiler le paquet
        enfiler_naif(args->tapis, p);

        printf("Producteur [%s] a produit: %s\n", args->nom_produit, p.contenu);

        //  Petit delai aleatoire pour augmenter les chances de race conditions
        usleep(rand() % 10000);
    }

    printf("[PROD-NAIF] %s termine\n", args->nom_produit);
    return NULL;
}

void *consommateur_naif(void *arg)
{
    ArgConsommateurNaif *args = (ArgConsommateurNaif *)arg;
    int id = args->id;

    printf("[CONS-NAIF] C%d demarre\n", id);

    while (1)
    {
        // verifier si y a encore a consommer
        if (args->compteur->valeur <= 0)
        {
            break; // terminer si plus rien a consommer
        }

        // decrementer le compteur (pas atomique)
        args->compteur->valeur--;

        // defiler un paquet
        Paquet p = defiler_naif(args->tapis);
    }
    return NULL;
}

// main naif
int main_naif(int argc, char *argv[])
{
    printf("======================================================================\n");
    printf("  ATTENTION: VERSION NAIVE SANS SYNCHRONISATION\n");
    printf("======================================================================\n\n");

    // param par defaut
    int nb_producteurs = 3;
    int nb_consommateurs = 2;
    int cible_production = 5;
    int capacite_tapis = 3;

    int compteur = nb_producteurs * cible_production;

    // initialiser tapis naif
    TapisNaif tapis;
    initialiser_tapis_naif(&tapis, capacite_tapis);

    CompteurNaif compteur_naif;
    compteur_naif.valeur = compteur;

    // creer les threads naif
    pthread_t *threads_prod = (pthread_t *)malloc(nb_producteurs * sizeof(pthread_t));
    pthread_t *threads_cons = (pthread_t *)malloc(nb_consommateurs * sizeof(pthread_t));

    ArgProducteurNaif *args_prod = malloc(nb_producteurs * sizeof(ArgProducteurNaif));
    ArgConsommateurNaif *args_cons = malloc(nb_consommateurs * sizeof(ArgConsommateurNaif));

    const char *noms_produits[] = {
        "Pomme", "Banane", "Orange", "Poire", "Fraise",
        "Cerise", "Kiwi", "Mangue", "Ananas", "Citron"};

    // lancer les producteurs naif
    for (int i = 0; i < nb_producteurs; i++)
    {
        strcpy(args_prod[i].nom_produit, noms_produits[i % 10]);
        args_prod[i].cible = cible_production;
        args_prod[i].tapis = &tapis;
        pthread_create(&threads_prod[i], NULL, producteur_naif, &args_prod[i]);
    }

    // lancer les consommateurs naif
    for (int i = 0; i < nb_consommateurs; i++)
    {
        args_cons[i].id = i + 1;
        args_cons[i].tapis = &tapis;
        args_cons[i].compteur = &compteur_naif;
        pthread_create(&threads_cons[i], NULL, consommateur_naif, &args_cons[i]);
    }

    // attendre la fin des producteurs
    for (int i = 0; i < nb_producteurs; i++)
    {
        pthread_join(threads_prod[i], NULL);
    }

    // attendre la fin des consommateurs
    for (int i = 0; i < nb_consommateurs; i++)
    {
        pthread_join(threads_cons[i], NULL);
    }

    // nettoyer
    detruire_tapis_naif(&tapis);
    free(threads_prod);
    free(threads_cons);
    free(args_prod);
    free(args_cons);

    printf("\n=== Fin du programme naif ===\n");
    return 0;
}