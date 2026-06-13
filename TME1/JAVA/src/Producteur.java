
public class Producteur extends Thread{//prod est un thread 

	private Tapis tapis;
	private int cible;//nbr paquets a produire
	private String nom_produit;//nom produit a creer 
	
	
	//constructeur
	public Producteur(Tapis tapis, String nom_produit, int cible) {
		this.tapis=tapis;
		this.nom_produit=nom_produit;
		this.cible=cible;
	}
	
	
	@Override
	public void run() {
		
		for(int i=0; i< cible; i++) {
			//crrer le contenu de paquet
			String contenu= nom_produit+ " " +i;
			Paquet paquet=new Paquet(contenu);
			tapis.enfiler(paquet);
			System.out.println("Producteur["+ nom_produit +"] a produit:"+contenu);
			
			//simuler le temps de production
			try {
				Thread.sleep((long) (Math.random()*10));
			}catch(InterruptedException e) {
				Thread.currentThread().interrupt();//si on fait pas ca les autre threads et le code exterieur  ne voient pas que le thread est interrompu deja parqlq
				
				break;
			}
		}
		
		System.out.println("Producteur [" + nom_produit + "] a terminé sa production");
		
		
		
		
		
		
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}
