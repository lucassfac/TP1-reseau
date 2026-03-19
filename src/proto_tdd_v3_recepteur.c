#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - recepteur  */
/* =============================== */
int verifier_controle(paquet_t *paquet);

int main(int argc, char* argv[]) {
    unsigned char message[MAX_INFO];
    paquet_t p_data, pack;
    pack.lg_info = 0; // Un ACK ne contient pas de données
    
    int fin = 0;
    int num_seq_attendu = 0; 

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport GBN (v3).\n");

    while (!fin) {
        de_reseau(&p_data);
        
        // 1. Le paquet est INTACT et c'est le BON numéro (En séquence)
        if (verifier_controle(&p_data) && p_data.num_seq == num_seq_attendu) {
            
            // On acquitte ce paquet
            pack.type = ACK;
            pack.num_seq = num_seq_attendu;
            pack.somme_ctrl = generer_controle(&pack);
            vers_reseau(&pack); 
            
            // On écrit les données
            for (int i = 0; i < p_data.lg_info; i++) {
                message[i] = p_data.info[i];
            }
            fin = vers_application(message, p_data.lg_info);
            
            // On attend le suivant
            num_seq_attendu = (num_seq_attendu + 1) % SEQ_NUM_SIZE;

        } 
        // 2. Le paquet est CORROMPU ou HORS SÉQUENCE (Déséquencement)
        else {
            pack.type = ACK;
            // On acquitte le DERNIER paquet reçu correctement
            // L'astuce (+ SEQ_NUM_SIZE - 1) permet de faire -1 sans tomber dans les nombres négatifs
            pack.num_seq = (num_seq_attendu + SEQ_NUM_SIZE - 1) % SEQ_NUM_SIZE;
            pack.somme_ctrl = generer_controle(&pack);
            vers_reseau(&pack);
        }
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}