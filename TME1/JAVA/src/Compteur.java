
public class Compteur {

	//classe pour gerer un compteur partagé de maniere thread-safe
	private int valeur;
	
	//constructeur
	public Compteur(int valeur) {
		this.valeur=valeur;
	}
	
	
	
	//decrementer le compteur de maniere atomique
	public synchronized boolean decrementer() {
		if(valeur>0) {
			valeur --;
			return true;
		}
		return false;
	}
	
	
	//obtenir la val actuelle de compteur
	public synchronized int getValeur() {
		return valeur;
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}
