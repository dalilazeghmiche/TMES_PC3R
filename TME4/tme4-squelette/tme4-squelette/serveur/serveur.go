package main

import (
	"bufio"
	"fmt"
	"math/rand"
	"net"
	"os"
	"strconv"
	"strings"

	st "../client/structures"
	tr "./travaux"
)

var ADRESSE = "localhost"

var pers_vide = st.Personne{Nom: "", Prenom: "", Age: 0, Sexe: "M"}


// message envoyé au mainteneur
type message_maint struct {
	methode string
	id int
	retour chan string
}

// canal global vers le mainteneur
var canal_maint chan message_maint
// type d'un paquet de personne stocke sur le serveur, n'implemente pas forcement personne_int (qui n'existe pas ici)
type personne_serv struct {
	statut string // V vide,  P en cours, C terminie
	afaire []func(st.Personne) st.Personne
	st.Personne //la vraie structure Personne (Nom, Prenom, Age, Sexe)
}

//ensemble de personnes predefinies pour l'initialisation coté serveur 
//pas de lecture de fichier coté serveur
var personnes_init = []st.Personne{
	{Nom: "MARTIN", Prenom: "Jean", Age: 45, Sexe: "M"},
	{Nom: "DUPONT", Prenom: "Marie", Age: 32, Sexe: "F"},
	{Nom: "BERNARD", Prenom: "Pierre", Age: 58, Sexe: "M"},
	{Nom: "PETIT", Prenom: "Sophie", Age: 27, Sexe: "F"},
	{Nom: "ROUSSEAU", Prenom: "Luc", Age: 51, Sexe: "M"},
}





// cree une nouvelle personne_serv, est appelé depuis le client, par le proxy, au moment ou un producteur distant
// produit une personne_dist
func creer(id int) *personne_serv{
	return &personne_serv{
		statut: "V",
		Personne: pers_vide,
		afaire: nil,
	}
}

// Méthodes sur les personne_serv, on peut recopier des méthodes des personne_emp du client
// l'initialisation peut être fait de maniere plus simple que sur le client
// (par exemple en initialisant toujours à la meme personne plutôt qu'en lisant un fichier)
func (p *personne_serv) initialise() {
	p.Personne = personnes_init[rand.Intn(len(personnes_init))]
	for i :=0; i< rand.Intn(5)+1; i++{//1 a 5 taches
       p.afaire=append(p.afaire, tr.UnTravail())
	}
	p.statut = "P"
}

func (p *personne_serv) travaille() {
	p.Personne = p.afaire[0](p.Personne)
	p.afaire=p.afaire[1:]
	if len(p.afaire) == 0 {
		p.statut="C"
	}
}

// vers_string préfixe le résultat par "[DISTANT]" pour qu'on puisse distinguer
// dans le journal du collecteur les personnes locales des personnes distantes
func (p *personne_serv) vers_string() string {
	return fmt.Sprintf("[DISTANT] %s %s, age %d, sexe %s", p.Nom, p.Prenom, p.Age, p.Sexe)
}

func (p *personne_serv) donne_statut() string {
	return p.statut
}

// Goroutine qui maintient une table d'association entre identifiant et personne_serv
// il est contacté par les goroutine de gestion avec un nom de methode et un identifiant
// et il appelle la méthode correspondante de la personne_serv correspondante
func mainteneur() {
	//ID -> personne_serv
	//exemple: 42 -> personne_serv
	table := make(map[int]*personne_serv)
	for{
			//quand une requete arrive
			//regarde la methode
		msg := <-canal_maint
		switch msg.methode{
		case "creer"://cree la personne
			table[msg.id] = creer(msg.id)
			msg.retour <- "OK"
		case "initialise":
			if p, ok := table[msg.id]; ok{
				p.initialise()
			}
			msg.retour <-"OK"
			case "travaille":
				//p = valeur trouvée donc la personne trouver
				//ok = true ou false indique si elle existe si id existe pas ok = false
				//le client envoie 12:travaille le serveur truve table[12] appelle p.travaille()
				if p, ok :=table[msg.id];ok{
					p.travaille()
				}
					msg.retour <-"OK"
			case "vers_string":
				if p, ok := table[msg.id]; ok{
					msg.retour <- p.vers_string()
				}else{
					msg.retour <-""
				}	
			case "donne_statut":
				if p, ok := table[msg.id]; ok{
					msg.retour <- p.donne_statut()

				}else{
					msg.retour <-"V"
				}		

			default:
				msg.retour <- "UNKNOWN"		
		}
	}
	




}

// Goroutine de gestion des connections
// elle attend sur la socketi un message content un nom de methode et un identifiant et appelle le mainteneur avec ces arguments
// elle recupere le resultat du mainteneur et l'envoie sur la socket, puis ferme la socket
func gere_connection(conn net.Conn) {
	reader := bufio.NewReader(conn)
	writer := bufio.NewWriter(conn)

	// Lecture du message entrant
	msg, err := reader.ReadString('\n')
	if err != nil {
		conn.Close()
		return
	}
	msg = strings.TrimSpace(msg)

	// Décodage : "<id>:<methode>"
	parts := strings.SplitN(msg, ":", 2)
	if len(parts) != 2 {
		fmt.Fprintf(writer, "ERROR\n")
		writer.Flush()
		conn.Close()
		return
	}
	id, _ := strconv.Atoi(parts[0])
	methode := parts[1]

	// Délégation au mainteneur (goroutine unique qui accède à la table)
	ret := make(chan string)
	canal_maint <- message_maint{methode: methode, id: id, retour: ret}
	reponse := <-ret

	// Envoi de la réponse
	fmt.Fprintf(writer, "%s\n", reponse)
	writer.Flush()
	conn.Close()
}
func main() {

	if len(os.Args) < 2 {
		fmt.Println("Format: serveur <port>")
		return
	}

	port, _ := strconv.Atoi(os.Args[1])
	addr := ADRESSE + ":" + fmt.Sprint(port)

	// canal du mainteneur
	canal_maint = make(chan message_maint)

	// lancer mainteneur
	go mainteneur()

	// ouvrir le serveur TCP
	ln, err := net.Listen("tcp", addr)
	if err != nil {
		fmt.Println("Erreur écoute :", err)
		return
	}

	fmt.Println("Ecoute sur", addr)

	for {
		conn, err := ln.Accept()
		if err != nil {
			continue
		}

		fmt.Println("Accepte une connexion")

		go gere_connection(conn)
	}
}

