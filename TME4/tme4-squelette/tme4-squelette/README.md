
ZEGHMICHE DALILA && BENATSI FERIEL




Le système illustre plusieurs concepts fondamentaux du cours :

communication par canaux synchrones en Go
implémentation d'interfaces distantes (patron Moignon/Proxy)
coordination de goroutines sans partage mémoire direct
mécanismes d'évitement de la famine dans un système producteur-consommateur



Mécanisme anti-famine des ouvriers (Partie 1)
Quand la file d'un gestionnaire est pleine, il cesse d'accepter les nouveaux paquets des producteurs et ne traite plus que les retours des ouvriers. Cela empêche les producteurs d'inonder le système et de bloquer les ouvriers qui veulent rendre leurs paquets.



Partie 2 (client + serveur) — deux terminaux
Terminal 1 :
bashcd serveur/
go build -o serveur_bin .
./serveur_bin 8080
# Affiche : "Ecoute sur localhost:8080"


Terminal 2 :
bashcd client/
go build -o client_bin .
./client_bin 8080 3000

