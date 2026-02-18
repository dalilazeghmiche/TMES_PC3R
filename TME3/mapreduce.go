package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

// paquet contient les donnees d'un arret
type Paquet struct {
	arrivee string
	depart  string
	arret   int
}

// requete envoyée au serveur avec canal de retour privé
type Requete struct {
	paquet Paquet
	retour chan Paquet
}

const NB_T int = 10

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
func lecteur(fichier string, lignes chan<- string) {
	f, err := os.Open(fichier)
	if err != nil {
		fmt.Println("Erreur:", err)
		close(lignes)
		return
	}
	defer f.Close()

	scanner := bufio.NewScanner(f)
	scanner.Scan() //sauter l'en-tete
}
