// ZEGHMICHE DALILA BENATSI FERIEL
/*
 COMPILATION:   cargo build --release

 

 EXECUTION:

  cargo run -- [nb_producteurs] [nb_consommateurs] [cible] [capacite]

  Exemple: cargo run -- 3 2 5 3
*/

use std::sync::{Arc, Mutex, Condvar};
use std::collections::VecDeque;
use std::thread;

#[derive(Clone, Debug)]
struct Paquet {
    contenu: String,
}

struct Tapis {
    file: Mutex<VecDeque<Paquet>>,
    capacite: usize,
    non_vide: Condvar,
    non_plein: Condvar,
}

fn main() {
    let nb_producteurs = 3;
    let nb_consommateurs = 2;
    let production_par_personne = 5;
    let capacite_tapis = 3;
    let total_a_manger = nb_producteurs * production_par_personne;

    let tapis = Arc::new(Tapis {
        file: Mutex::new(VecDeque::with_capacity(capacite_tapis)),
        capacite: capacite_tapis,
        non_vide: Condvar::new(),
        non_plein: Condvar::new(),
    });

    let restant = Arc::new(Mutex::new(total_a_manger));
    let mut threads = vec![];

    // --- SECTION PRODUCTEURS ---
    for id_prod in 0..nb_producteurs {
        let tapis_partage = Arc::clone(&tapis);
        let t = thread::spawn(move || {
            for i in 0..production_par_personne {
                let p = Paquet { contenu: format!("Objet {} du Prod {}", i, id_prod) };
                let mut queue = tapis_partage.file.lock().unwrap();

                while queue.len() >= tapis_partage.capacite {
                    queue = tapis_partage.non_plein.wait(queue).unwrap();
                }

                println!("[PROD {}] Ajout : {}", id_prod, p.contenu);
                queue.push_back(p);
                tapis_partage.non_vide.notify_all(); // Changé notify_one en notify_all
            }
        });
        threads.push(t);
    }

    // --- SECTION CONSOMMATEURS ---
    for id_cons in 0..nb_consommateurs {
        let tapis_partage = Arc::clone(&tapis);
        let restant_partage = Arc::clone(&restant);

        let t = thread::spawn(move || {
            loop {
                // Vérification si c'est fini
                {
                    let n = restant_partage.lock().unwrap();
                    if *n == 0 { break; }
                }

                let mut queue = tapis_partage.file.lock().unwrap();

                while queue.is_empty() {
                    let n = restant_partage.lock().unwrap();
                    if *n == 0 { return; } 
                    queue = tapis_partage.non_vide.wait(queue).unwrap();
                }

                let p = queue.pop_front().unwrap();
                
                // Décrémentation AVANT le notify pour que les autres voient que c'est fini
                let mut n = restant_partage.lock().unwrap();
                *n -= 1;
                println!("[CONS {}] Miam : {} (Reste : {})", id_cons, p.contenu, *n);

                tapis_partage.non_plein.notify_all(); // Changé notify_one en notify_all
                tapis_partage.non_vide.notify_all();  // IMPORTANT : Réveiller les collègues pour qu'ils voient que n == 0
            }
        });
        threads.push(t);
    }

    for t in threads {
        t.join().unwrap();
    }

    println!("Fini ! Tout a été produit et consommé.");
}