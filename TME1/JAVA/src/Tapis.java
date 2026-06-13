import java.util.LinkedList;
import java.util.Queue;

public class Tapis {

	private Queue<Paquet> file;
	private int capacite;
	
	public Tapis(int capacite) {
		this.file=new LinkedList<>();
		this.capacite=capacite;
	}
	
	
	//enfiler un paquet sur le tapis
	//bloque si le tapis plein
	
	public synchronized void enfiler (Paquet paquet) {
		
	
	while(file.size()>= capacite) {
	
	try {
		wait();
	}catch(InterruptedException e) {
		Thread.currentThread().interrupt();
		return;
	}
	
	}
	
	file.add(paquet);
	//Notifier les threads en attente(Consommateurs
	notifyAll();
	
	
	}
	
	//defiler un paquet du tapis
	//bloque si le tapis vide
	public synchronized Paquet defiler() {
		
		while(file.isEmpty()) {
			try {
				wait();
			}catch(InterruptedException e) {
				Thread.currentThread().interrupt();
				return null;
			}
		}
		
		//retirer le paquet
		Paquet paquet=file.poll();
		//notifier les threads en attente (producteurs
		notifyAll();
		return paquet;
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	//nbr de paquets sur le tapis
	public synchronized int getTaille() {
		return file.size();
	}
	
	//la capacite maximale
	public int getCapacite() {
		return capacite;
	}
}
