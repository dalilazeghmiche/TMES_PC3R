package main

import (
	"bufio"
"net"
"strings"
	"fmt"
	"math/rand"
	"os"
	"regexp"
	"strconv"
	"time"

	st "./structures" // contient la structure Personne
	tr "./travaux"    // contient les fonctions de travail sur les Personnes
)

var ADRESSE string = "localhost"                           // adresse de base pour la Partie 2
var FICHIER_SOURCE string = "./conseillers-municipaux.txt" // fichier dans lequel piocher des personnes
var TAILLE_SOURCE int = 450000                             // inferieure au nombre de lignes du fichier, pour prendre une ligne au hasard
var TAILLE_G int = 5                                       // taille du tampon des gestionnaires
var NB_G int = 2                                           // nombre de gestionnaires
var NB_P int = 2                                           // nombre de producteurs
var NB_O int = 4                                           // nombre d'ouvriers
var NB_PD int = 2                                          // nombre de producteurs distants pour la Partie 2

var pers_vide = st.Personne{Nom: "", Prenom: "", Age: 0, Sexe: "M"} // une personne vide
//canaux globaux
var canal_lec chan message_lec //canal vers le lecteur unique(partie1)
var canal_proxy chan message_proxy //canal vers le proxy (partie2)
var canal_id chan int //canal vers le generateur d'identifiants(partie2)

//message envoye au lecteur unique pour demander une ligne du fichier
type message_lec struct{
	contenu int //num de ligne a lire
	retour chan string // canal pour recevoir la ligne 

}

//message envoyé au proxy pour declencher un appel distant
type message_proxy struct{
	methode string //le nom de la methode a appeler sur le serveur
	id int //identifiant de la personne_serv cible
	retour chan string //canal pour recevoir le result 

}




// paquet de personne, sur lequel on peut travailler, implemente l'interface personne_int
type personne_emp struct {
	ligne       int                             //numero de ligne a lire dans le fichier
	statut      string                          //"V" , "R" ou "C"
	lecteur     chan message_lec                //canal vers le lecteur unique
	afaire      []func(st.Personne) st.Personne //liste de taches restantes
	st.Personne                                 //la personne embarquee(champ anonyme)
}

// paquet de personne distante, pour la Partie 2, implemente l'interface personne_int
type personne_dist struct {
	id int //identifiant unique de la personne_serv correspondante sur le serveur
}

// interface des personnes manipulees par les ouvriers, les
type personne_int interface {
	initialise()          // appelle sur une personne vide de statut V, remplit les champs de la personne et passe son statut à R
	travaille()           // appelle sur une personne de statut R, travaille une fois sur la personne et passe son statut à C s'il n'y a plus de travail a faire
	vers_string() string  // convertit la personne en string
	donne_statut() string // renvoie V, R ou C
}

// fabrique une personne à partir d'une ligne du fichier des conseillers municipaux
// à changer si un autre fichier est utilisé
func personne_de_ligne(l string) st.Personne {

	separateur := regexp.MustCompile("\u0009")
	separation := separateur.Split(l, -1)

	// vérifier que la ligne contient assez de colonnes
	if len(separation) < 8 {
		return pers_vide
	}

	naiss, _ := time.Parse("2/1/2006", separation[7])

	a1, _, _ := time.Now().Date()
	a2, _, _ := naiss.Date()

	agec := a1 - a2

	return st.Personne{
		Nom:    separation[4],
		Prenom: separation[5],
		Sexe:   separation[6],
		Age:    agec,
	}
}

// *** METHODES DE L'INTERFACE personne_int POUR LES PAQUETS DE PERSONNES ***

func (p *personne_emp) initialise() {
	ret := make(chan string)
	p.lecteur <- message_lec{contenu: p.ligne, retour: ret} //demande au lecteur
	ligne := <-ret
	p.Personne = personne_de_ligne(ligne)
	for i := 0; i < rand.Intn(6)+1; i++ { //1 a 6 taches aleatoire
		p.afaire = append(p.afaire, tr.UnTravail())
	}
	p.statut = "P"
}

func (p *personne_emp) travaille() {
	p.Personne = p.afaire[0](p.Personne) //applique la premiere tache
	p.afaire = p.afaire[1:]              //retire la tache effectuee
	if len(p.afaire) == 0 {
		p.statut = "C"
	}
}

func (p *personne_emp) vers_string() string {
	return fmt.Sprintf("[LOCAL] %s %s, age %d, sexe %s", p.Nom, p.Prenom, p.Age, p.Sexe)
}

func (p *personne_emp) donne_statut() string {
	return p.statut
}

// *** METHODES DE L'INTERFACE personne_int POUR LES PAQUETS DE PERSONNES DISTANTES (PARTIE 2) ***
// ces méthodes doivent appeler le proxy (aucun calcul direct)

func (p personne_dist) initialise() {
	ret := make(chan string)
	canal_proxy <- message_proxy{methode: "initialise", id:p.id, retour: ret}
	<- ret //attend la confirmation du serveur avant de rendre la main
}

func (p personne_dist) travaille() {
	ret := make(chan string)
	canal_proxy <- message_proxy{methode: "travaille", id: p.id, retour: ret}
	<- ret
}

func (p personne_dist) vers_string() string {
	ret := make(chan string)
	canal_proxy <- message_proxy{methode: "vers_string", id:p.id, retour:ret}
	return <-ret
}

func (p personne_dist) donne_statut() string {
	ret := make(chan string)
	canal_proxy <-message_proxy{methode: "donne_statut", id:p.id, retour: ret}
	return <-ret
}

// *** CODE DES GOROUTINES DU SYSTEME ***

// Partie 2: contacté par les méthodes de personne_dist, le proxy appelle la méthode à travers le réseau et récupère le résultat
// il doit utiliser une connection TCP sur le port donné en ligne de commande
func proxy(port int) {
	for {
		msg := <-canal_proxy
		addr := ADRESSE + ":" + fmt.Sprint(port)
		conn, err := net.Dial("tcp", addr)
		if err != nil {
			fmt.Println("Proxy : erreur de connexion :", err)
			msg.retour <- "ERROR"
			continue
		}
		writer := bufio.NewWriter(conn)
		reader := bufio.NewReader(conn)
		// Protocole : "<id>:<methode>\n"
		fmt.Fprintf(writer, "%d:%s\n", msg.id, msg.methode)
		writer.Flush()
		reponse, _ := reader.ReadString('\n')
		conn.Close()
		msg.retour <- strings.TrimSpace(reponse)
	}
}

// Partie 1 : contacté par la méthode initialise() de personne_emp, récupère une ligne donnée dans le fichier source
//charge toutes les lignes en memoire au demarrage , puis repond aux demandes
//des personne_emp via canal_lec. Garantit l'acces sequentiel sans race condition.
func lecteur() {
	// Ouvre le fichier source contenant les conseillers municipaux
	file, err :=os.Open(FICHIER_SOURCE)
	if err != nil{
		fmt.Println("Lectur: erreur ouverture fichier:", err)
		//absorbe les demandes sans repondre(ne bloque pas le systeme)
		for{
			msg := <- canal_lec // attendre une demande de lecture
			msg.retour <- ""// renvoyer une réponse vide
		}
	}

     // Ferme le fichier automatiquement lorsque la fonction se termine
	defer file.Close()
	// Scanner pour lire le fichier ligne par ligne
	scanner := bufio.NewScanner(file)
	//buffer plus grand pour les longues lignes du CSV
	scanner.Buffer(make([]byte, 1024*1024), 1024*1024)
	// Tableau qui va contenir toutes les lignes du fichier
	var lignes []string
	// Lecture de toutes les lignes du fichier
	for scanner.Scan(){
		lignes=append(lignes, scanner.Text()) // ajoute la ligne au tableau
	}
	fmt.Println("Lecteur: fichier chargé", len(lignes), "lignes.")
	// Boucle infinie : le lecteur attend des demandes de lecture
	for{
		// Réception d'un message venant d'une personne_emp
		msg := <-canal_lec
		// On renvoie la ligne demandée
		// % permet d'éviter un dépassement d'indice si le numéro est trop grand
		msg.retour <- lignes[msg.contenu%len(lignes)] //envoyer la ligne demandée au paquet qui l’a demandée
	}
	
}

// Partie 1: récupèrent des personne_int depuis les gestionnaires, font une opération dépendant de donne_statut()
// Si le statut est V, ils initialise le paquet de personne puis le repasse aux gestionnaires
// Si le statut est R, ils travaille une fois sur le paquet puis le repasse aux gestionnaires
// Si le statut est C, ils passent le paquet au collecteur
func ouvrier(canaux_retours []chan personne_int, canal_ouv chan personne_int, canal_coll chan personne_int) {
	for{
		p := <- canal_ouv
		switch p.donne_statut(){
		case "V":
			p.initialise()
			canaux_retours[rand.Intn(NB_G)] <- p //retourne au gestionnaire
		case "C":
			canal_coll <-p 
		default: //"P": en cours de modification
		    p.travaille()
			if p.donne_statut() == "C"{
				canal_coll <- p
			
			}else{
				canaux_retours[rand.Intn(NB_G)] <- p
			}
		   		
		}
	}
}

// Partie 1: les producteurs cree des personne_int implementees par des personne_emp initialement vides,
// de statut V mais contenant un numéro de ligne (pour etre initialisee depuis le fichier texte)
// la personne est passée aux gestionnaires
func producteur(canaux_prods []chan personne_int) {
	// Boucle infinie : le producteur produit des paquets en continu
	for{
		// Choisit un numéro de ligne aléatoire dans le fichier source
		// TAILLE_SOURCE représente le nombre maximum de lignes utilisables
		ligne :=rand.Intn(TAILLE_SOURCE)
		// Création d'un nouveau paquet personne_emp
		// & signifie que l'on crée un pointeur vers la structure
		p :=&personne_emp{
			ligne: ligne,
			statut: "V",
			// Canal vers le lecteur qui permet de lire la ligne dans le fichier
			lecteur: canal_lec,

		}
		// Envoie la personne créée vers un gestionnaire choisi aléatoirement
		// NB_G = nombre de gestionnaires
		canaux_prods[rand.Intn(NB_G)] <- p
	}

	
}

// Partie 2: les producteurs distants cree des personne_int implementees par des personne_dist qui contiennent un identifiant unique
// utilisé pour retrouver l'object sur le serveur
// la creation sur le client d'une personne_dist doit declencher la creation sur le serveur d'une "vraie" personne, initialement vide, de statut V
//la vraie personne, seulement un identifiant
//la vraie personne(personne_serv) sera stockee sur le serveur
func producteur_distant(canaux_prods []chan personne_int) {
	for{
		//recupere un identifiant unique depuis generateur_id
		id := <-canal_id
		//creer la personne_serv correspondante sur le serveur
		//canal qui servira a recevoir la reponse du serveur
		ret := make(chan string)
		//envoi d'une requete au proxy
		//le proxy contactera le serveur via tcp 
		canal_proxy <- message_proxy{methode: "creer", id:id, retour:ret}
		<- ret //attend la confirmaton avant de mettre en circulation le moignon
		//creer le moignon local et l'injecter dans le systeme
		p := personne_dist{id:id}
		canaux_prods[rand.Intn(NB_G)] <- p //envoie ce paquet a un gestionnaire et le traiteront comme n'importe quelle personne_int
	}
	
}

//partie 2: prod des ident entiers uniques et croissants
//distribues aux producteurs distants via canal_id
func generateur_id(){
	id := 0
	for{
		canal_id <- id
		id++
	}
}


// Partie 1: les gestionnaires recoivent des personne_int des producteurs et des ouvriers et maintiennent chacun une file de personne_int
// ils les passent aux ouvriers quand ils sont disponibles
// ATTENTION: la famine des ouvriers doit être évitée: si les producteurs inondent les gestionnaires de paquets, les ouvrier ne pourront
// plus rendre les paquets surlesquels ils travaillent pour en prendre des autres
func gestionnaire(prod_chan chan personne_int, ouv_retour chan personne_int, vers_ouv chan personne_int ) {
	file :=make([]personne_int, 0)
	for{
		if len(file) == 0{
			//file vide: attendre n'importe quelle source
			select{
			case p := <- prod_chan:
				file = append(file, p)
			case p:= <-ouv_retour:
				file = append(file,p)

			}
		}else if len(file) < TAILLE_G {
			//cas normal: envoyer aux ouvriers ou recevoir
			select{
			case vers_ouv <- file[0]:
				file = file[1:]
			case p := <- prod_chan:
				file= append(file,p)
			case p := <-ouv_retour:
				file= append(file,p)		
			}
		}else{
			//file pleine (ANTI famine): pas de prod_chan, seulement ouvriers et envoi
			select{
			case vers_ouv <- file[0]:
				file=file[1:]
			case p := <-ouv_retour:
				file = append(file,p)	
			}
		}
	}
}

// Partie 1: le collecteur recoit des personne_int dont le statut est c, il les collecte dans un journal
// quand il recoit un signal de fin du temps, il imprime son journal.
func collecteur(canal_coll chan personne_int, fintemps chan int) {
	journal := ""
	for{
		select{
		case p := <- canal_coll:
			journal +=p.vers_string() + "\n"
		case <-fintemps:
			fmt.Print(journal)
			fintemps <- 0	//confirmation a la goroutine principale
			return 
		}
	}
}

func main() {
	rand.Seed(time.Now().UTC().UnixNano()) // graine pour l'aleatoire
	if len(os.Args) < 3 {
		fmt.Println("Format: client <port> <millisecondes d'attente>")
		return
	}
	port, _ := strconv.Atoi(os.Args[1])   // utile pour la partie 2
	millis, _ := strconv.Atoi(os.Args[2]) // duree du timeout
	fintemps := make(chan int)
	
	// creer les canaux
  
	canal_lec = make(chan message_lec)
	canal_proxy=make(chan message_proxy)
	canal_id=make(chan int)
    //NB_G nbr de canaux a cree 
	canaux_prods := make([]chan personne_int, NB_G) //prods/ouvries -> gestionnaires
	canaux_retours :=make([] chan personne_int, NB_G)//ouvries -> gestionnaires(retour)
	canal_ouv := make(chan personne_int) //gestionnaires envoient -> ouvries recoivent (un seul canal partagé) et go va auto donner le message a un seul ouvrier dispo
	canal_coll := make(chan personne_int) //ouvries ->collector
	// création réelle des canaux dans les slices
	for i := 0; i < NB_G; i++ {
		canaux_prods[i] = make(chan personne_int)
		canaux_retours[i] = make(chan personne_int)
	}
	// lancer les goroutines (parties 1 et 2): 1 lecteur, 1 collecteur, des producteurs, des gestionnaires, des ouvriers
     go lecteur()
	 go collecteur(canal_coll, fintemps)
	 for i := 0; i< NB_G; i++{
		go gestionnaire(canaux_prods[i], canaux_retours[i], canal_ouv)
	 }
	 for i :=0; i<NB_O; i++{
		go ouvrier(canaux_retours, canal_ouv, canal_coll)
	 } 
	 for i:=0; i<NB_P; i++{
		go producteur(canaux_prods)
	 }
	// lancer les goroutines (partie 2): des producteurs distants, un proxy
	go generateur_id()
	go proxy(port)
	for i:=0; i< NB_PD; i++{
		go producteur_distant(canaux_prods)
	}

//attendre la durée demandée puis signaler la fin au collecteur
	time.Sleep(time.Duration(millis) * time.Millisecond)
	fintemps <- 0 //signal au collecteur
	<-fintemps //attendre de la confirmation du collector 
}
