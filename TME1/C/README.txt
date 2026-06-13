


Description:
Implémentation du problème classique des producteurs/consommateurs avec un
buffer borné (tapis) en C, utilisant les threads POSIX et les mécanismes de
synchronisation (mutex et variables de condition).




1.Structures :
   - Paquet : contient une chaîne de caractères
   - Tapis : buffer circulaire avec mutex et conditions

2. Fonctions principales :
   - creer_paquet() / detruire_paquet()
   - creer_tapis() / detruire_tapis()
   - enfiler() : ajoute un paquet (bloque si plein)
   - defiler() : retire un paquet (bloque si vide)

3. Threads :
   - producteur() : produit et enfile
   - consommateur() : défile et consomme
   - main() : initialise et coordonne


compiler: make
execution: ./prodcons
ou ./prodcons nb_prod nb_cons cible capacite
nettoyage: make clean



