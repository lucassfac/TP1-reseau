#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */

int main(int argc, char* argv[]){
    unsigned char message[MAX_INFO];
    int taille_msg;
    paquet_t p_data;
    int evt;

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    de_application(message,&taille_msg);

    // ==========================================
    // BOUCLE DU PROF (inchangée)
    // ==========================================
    while(taille_msg != 0){

        for (int i = 0; i < taille_msg; i++)
        {
            p_data.info[i] = message[i];
        }

        p_data.lg_info = taille_msg;
        p_data.type = DATA;
        p_data.somme_ctrl = generer_controle(&p_data);

        vers_reseau(&p_data);
        depart_temporisateur(100);
        evt = attendre();
        while(evt != -1){
            printf("[TRP] RéEmission paquet %d\n", p_data.num_seq);
            vers_reseau(&p_data);
            depart_temporisateur(100);
            evt = attendre();
        }

        arret_temporisateur();
        de_application(message, &taille_msg);
    }

    // ==========================================
    // AJOUT : ENVOI DU PAQUET DE FIN (Taille 0)
    // ==========================================
    // On utilise exactement la même logique que ton prof
    
    p_data.lg_info = 0; 
    p_data.type = DATA;
    p_data.somme_ctrl = generer_controle(&p_data);

    vers_reseau(&p_data);
    depart_temporisateur(100);
    evt = attendre();
    
    while(evt != -1){
        printf("[TRP] RéEmission paquet de FIN\n");
        vers_reseau(&p_data);
        depart_temporisateur(100);
        evt = attendre();
    }
    
    arret_temporisateur();
    // ==========================================

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}