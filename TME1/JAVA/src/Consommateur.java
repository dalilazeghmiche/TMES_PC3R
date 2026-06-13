
public class Consommateur extends Thread{
	//thread cons qui defile les paquets du tapis et les consome
    private int id;
    private Tapis tapis;
    private Compteur compteur;
    
    public Consommateur(int id, Tapis tapis, Compteur compteur) {
    	this.tapis=tapis;
    	this.compteur=compteur;
    	this.id=id;
    }
    
    
    @Override
    public void run() {
    	
    	while(true) {
    		
    		if(!compteur.decrementer()) {
    			break;
    		}
    		
    		Paquet paquet= tapis.defiler();
    		
    		if(paquet == null) {
    			break; //thread interrompu
    		}
    		
    		// Consommer le paquet
            System.out.println("C" + id + " mange " + paquet.getContenu());
            
            // Simuler le temps de consommation
            try {
                Thread.sleep((long) (Math.random() * 10));
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                break;
            }
        }
        
        System.out.println("Consommateur C" + id + " a terminé");
    }
}
