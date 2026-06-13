
public class Main {

	//classe principale initialise et coordonne le sys
	
	public static void main(String[] args) {
		
		//params par defaut
		int nbProds =3;
		int nbCons=2;
		int cibleProduction=10;
		int capaciteTapis=5;
		
		// Lecture des arguments (optionnel)
        if (args.length >= 4) {
            try {
                nbProds = Integer.parseInt(args[0]);
                nbCons = Integer.parseInt(args[1]);
                cibleProduction = Integer.parseInt(args[2]);
                capaciteTapis = Integer.parseInt(args[3]);
            } catch (NumberFormatException e) {
                System.err.println("Erreur: Les arguments doivent être des entiers");
                System.err.println("Usage: java Main [nb_prod] [nb_cons] [cible] [capacite]");
                return;
            }
        }
		
		
        System.out.println("=== Démarrage du système ===");
        System.out.println("Producteurs: " + nbProds);
        System.out.println("Consommateurs: " + nbCons);
        System.out.println("Cible par producteur: " + cibleProduction);
        System.out.println("Capacité du tapis: " + capaciteTapis);
        System.out.println();
		
		Tapis tapis=new Tapis(capaciteTapis);
		
		Compteur compteur = new Compteur(nbProds * cibleProduction);
		
		
		
		String[] nomsProduits= {"Pomme","Banane", "Orange", "Poire", "Fraise", "Cerise"};
		
		Thread[] producteurs=new Thread[nbProds];
		
		for(int i=0; i< nbProds;i++) {
			
			String nom=nomsProduits[i % nomsProduits.length];
			
			producteurs[i]=new Producteur(tapis,nom, cibleProduction);
			producteurs[i].start();
			
		}
		
		
		//creer et lancer les cons
		
		Thread[] consommateurs = new Thread[nbCons];
		
		for(int i=0; i< nbCons;i++) {
			consommateurs[i]= new Consommateur(i+1,tapis,compteur);
			consommateurs[i].start();
			
		}
		
		// Attendre la fin des producteurs
        for (Thread producteur : producteurs) {
            try {
                producteur.join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
        
        // Attendre la fin des consommateurs
        for (Thread consommateur : consommateurs) {
            try {
                consommateur.join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
        
        System.out.println();
        System.out.println("=== Tous les threads ont terminé ===");
        System.out.println("Compteur final: " + compteur.getValeur() + " (devrait être 0)");
        System.out.println("Taille du tapis: " + tapis.getTaille() + " (devrait être 0)");
    }
}