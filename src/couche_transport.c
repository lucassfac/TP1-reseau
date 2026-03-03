#include <stdio.h>
#include "couche_transport.h"
#include "services_reseau.h"
#include "application.h"

/* ************************************************************************** */
/* *************** Fonctions utilitaires couche transport ******************* */
/* ************************************************************************** */

// RAJOUTER VOS FONCTIONS DANS CE FICHIER...

int generer_controle(const paquet_t *paquet) {
    int somme = paquet->type ^ paquet->num_seq ^ paquet->lg_info;
    
    for (int i = 0; i < paquet->lg_info; i++) {
        somme ^= paquet->info[i];
    }
    
    return somme;
}

int verifier_controle(const paquet_t *paquet) {
    return generer_controle(paquet) == paquet->somme_ctrl;
}

int inc(int const n, int const mod){
    return (n+1) % mod;
}


/*--------------------------------------*/
/* Fonction d'inclusion dans la fenetre */
/*--------------------------------------*/
int dans_fenetre(unsigned int inf, unsigned int pointeur, int taille) {

    unsigned int sup = (inf+taille-1) % SEQ_NUM_SIZE;

    return
        /* inf <= pointeur <= sup */
        ( inf <= sup && pointeur >= inf && pointeur <= sup ) ||
        /* sup < inf <= pointeur */
        ( sup < inf && pointeur >= inf) ||
        /* pointeur <= sup < inf */
        ( sup < inf && pointeur <= sup);
}
