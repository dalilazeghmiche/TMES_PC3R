package travaux

import (
	"math/rand"

	st "../../client/structures"
)

// *** LISTES DE FONCTION DE TRAVAIL DE Personne DANS Personne DU SERVEUR ***
// Essayer de trouver des fonctions *différentes* de celles du client


// Préfixe "SRV_" sur le nom
func f1(p st.Personne) st.Personne {
	np := p
	np.Nom = "SRV_" + p.Nom
	return np
}

// Multiplie l'âge par 2 (clairement identifiable dans le journal)
func f2(p st.Personne) st.Personne {
	np := p
	np.Age = p.Age * 2
	return np
}

// Préfixe "SRV_" sur le prénom
func f3(p st.Personne) st.Personne {
	np := p
	np.Prenom = "SRV_" + p.Prenom
	return np
}

// Remplace le code sexe par "X" ou "Y" (inexistant côté client)
func f4(p st.Personne) st.Personne {
	np := p
	if p.Sexe == "M" {
		np.Sexe = "X"
	} else {
		np.Sexe = "Y"
	}
	return np
}
func UnTravail() func(st.Personne) st.Personne {
	tableau := make([]func(st.Personne) st.Personne, 0)
	tableau = append(tableau, func(p st.Personne) st.Personne { return f1(p) })
	tableau = append(tableau, func(p st.Personne) st.Personne { return f2(p) })
	tableau = append(tableau, func(p st.Personne) st.Personne { return f3(p) })
	tableau = append(tableau, func(p st.Personne) st.Personne { return f4(p) })
	i := rand.Intn(len(tableau))
	return tableau[i]
}
