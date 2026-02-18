package main

import (
	"bufio"   // pour lire le fichier ligne par ligne
	"fmt"     // pour afficher les résultats
	"os"      // pour ouvrir le fichier + recuperer les arguments de la ligne de commande
	"strconv" // pour la conversion de string 	a int
	"strings" // pour manipuler les strings
	"sync"    // pour la synchronisation des goroutines
	"time"    // pour simuler les temps de travail et mesurer les performances
)

// ====== CONSTANTES =========

const NB_T = 4 // nombre de travailleurs
// si on met 1 : on aura un programme sequentiel

// Temps additionnels

const WORKER_EXTRA_WORK = 10 * time.Millisecond // simule un traitement coté travailleur
const SERVER_EXTRA_WORK = 50 * time.Millisecond // simule un calcul lourd coté serveur

// ============ STRUTURE ===========
type Paquet struct {
	arrivee int         // heure d'arrivee en secondes
	depart  int         // heure de depart en secondes
	arret   int         //duréee de l'arret en secondes(calculer par le serveur)
	retour  chan Paquet // canal pour envoyer le résultat au travailleur
}

// parseTime convertit "HH:MM:SS" en secondes
func parseTime(s string) int {
	parts := strings.Split(strings.TrimSpace(s), ":")
	if len(parts) != 3 {
		return 0
	}
	h, _ := strconv.Atoi(parts[0])
	m, _ := strconv.Atoi(parts[1])
	sec, _ := strconv.Atoi(parts[2])
	return h*3600 + m*60 + sec
}

// lecteur lit le fichier et envoie les lignes aux travailleurs
func lecteur(fichier string, workerchans []chan string) {
	f, err := os.Open(fichier)
	if err != nil {
		fmt.Printf("Erreur ouverture fichier: %v\n", err)

		for _, ch := range workerchans {
			close(ch) // fermer les canaux pour signaler la fin de la lecture
		}
		return
	}
	defer f.Close()

	scanner := bufio.NewScanner(f)
	premiereLigne := true
	i := 0

	for scanner.Scan() {
		line := scanner.Text()
		if premiereLigne {
			premiereLigne = false // ignorer l'en-tete
			continue
		}
		workerchans[i%NB_T] <- line // envoyer la ligne au travailleur i%NB_T
		i++
	}

	fmt.Printf("Lecteur a lu %d lignes\n", i)

}

// ======= TRAVAILLEUR ===========

func travailleur(id int, lignes chan string, serverchan chan Paquet, reducerChan chan int, wg *sync.WaitGroup) {

	//id pour identifier le travailleur
	//lignes : canal pour recevoir les lignes du lecteur
	//serverchan : canal pour envoyer les paquets au serveur
	//reducerChan : canal pour envoyer les résultats au réducteur
	// wg : WaitGroup pour synchroniser la fin des travailleurs

	defer wg.Done() // signaler la fin du travailleur

	reschan := make(chan Paquet) // canal pour recevoir les résultats du serveur

	traites := 0

	for ligne := range lignes { // lire les lignes du canal
		parts := strings.Split(ligne, ",")
		if len(parts) < 3 {
			continue // ignorer les lignes mal formées
		}

		arrivee := parseTime(parts[0])
		depart := parseTime(parts[1])
		if arrivee < 0 || depart < 0 {
			continue // ignorer les lignes avec des temps invalides
		}

		// créer le parqueet
		pkt := Paquet{
			arrivee: arrivee,
			depart:  depart,
			arret:   0,       // à calculer par le serveur
			retour:  reschan, // canal pour recevoir le résultat du serveur
		}

		time.Sleep(WORKER_EXTRA_WORK)

		// envoyer le paquet au serveur
		serverchan <- pkt

		// attendre le résultat du serveur
		resultat := <-reschan

		// envoyer la durée de l'arret au réducteur
		reducerChan <- resultat.arret
		traites++
	}
	fmt.Printf("Travailleur %d a traité %d lignes\n", id, traites)
}

// ============= SERVEUR DE CALCUL ============

func serveur(serverchan chan Paquet) {
	for pkt := range serverchan { // lire les paquets du canal
		go func(p Paquet) { // traiter chaque paquet dans une goroutine séparée
			time.Sleep(SERVER_EXTRA_WORK)

			// calculer la durée de l'arret
			p.arret = p.depart - p.arrivee
			if p.arret < 0 {
				p.arret = 0
			}

			// envoyer le résultat au travailleur via le canal de retour
			p.retour <- p
		}(pkt)
	}
}

// ============= REDUCTEUR ============

func reducteur(reducerChan chan int, done chan struct{}, resultChan chan float64) {

	total := 0
	count := 0

	for {
		select {
		case arret := <-reducerChan: // recevoir les durées d'arret des travailleurs
			total += arret
			count++
		case <-done: // signal de fin
			avg := 0.0

			if count > 0 {
				avg = float64(total) / float64(count) // calculer la moyenne
			}
			fmt.Printf("Réducteur a reçu %d durées d'arrêt, total=%ds\n", count, total)
			resultChan <- avg // envoyer le résultat final
			return
		}
	}

}

// ============= MAIN ============

func main() {
	if len(os.Args) < 3 {
		fmt.Println("Usage: ./tme3 <fichier_stop_times.txt> <durée_secondes>")
		os.Exit(1)
	}

	filename := os.Args[1]
	duree, err := strconv.Atoi(os.Args[2])

	if err != nil || duree <= 0 {
		fmt.Println("Durée doit être un entier positif")
		os.Exit(1)
	}

	fmt.Printf("=== MapReduce SNCF | %d travailleurs | durée=%ds ===\n", NB_T, duree)

	// CREATION DES CANAUX

	workerchans := make([]chan string, NB_T) // canaux pour envoyer les lignes aux travailleurs

	for i := range workerchans {
		workerchans[i] = make(chan string)
	}

	serverchan := make(chan Paquet)  // canal pour envoyer (travailleur)les paquets au serveur
	reducerChan := make(chan int)    // canal pour envoyer (travailleur) les durées d'arret au réducteur
	doneChan := make(chan struct{})  // canal pour signaler (main) la fin du réducteur
	resultChan := make(chan float64) // canal pour recevoir (par le main) le résultat final du réducteur

	// LANCEMENT DES GOROUTINES

	// lancer le serveur de calcul
	go serveur(serverchan)
	// lancer le reducteur
	go reducteur(reducerChan, doneChan, resultChan)

	// lancer les travailleurs
	var wg sync.WaitGroup
	for i := 0; i < NB_T; i++ {
		wg.Add(1)
		go travailleur(i, workerchans[i], serverchan, reducerChan, &wg)
	}

	//goroutine pour fermer les canaux du serveur
	go func() {
		wg.Wait()         // attendre la fin de tous les travailleurs
		close(serverchan) // fermer le canal du serveur pour signaler la fin des paquets
		fmt.Printf("Tous les travailleurs ont terminé, canal serveur fermé")
	}()

	// lancer le lecteur (dans le main)
	lecteur(filename, workerchans)
	//attendre la durée configuré
	fmt.Printf("[Main] Attente de %d secondes...\n", duree)
	time.Sleep(time.Duration(duree) * time.Second)

	// signaler la fin au réducteur
	fmt.Println("[Main] Signal de fin envoyé au réducteur")
	doneChan <- struct{}{}

	//recupérer le résultat final du réducteur
	moyenne := <-resultChan

	fmt.Printf("\n=== RÉSULTAT ===\n")
	fmt.Printf("Durée moyenne d'arrêt : %.2f secondes (%.2f minutes)\n", moyenne, moyenne/60.0)
}
