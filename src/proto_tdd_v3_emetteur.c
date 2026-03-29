#include <stdio.h>
#include <stdlib.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

#define SEQ_NUM_SIZE 16

// Prototypes
int verifier_controle(paquet_t *paquet);
int generer_controle(paquet_t *paquet);

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[]) {
    unsigned char message[MAX_INFO];
    int taille_msg; 
    paquet_t p_data, pack, buffer[SEQ_NUM_SIZE];
    int evt, borne_inf = 0, curseur = 0;
    
    // Gestion de la taille de fenêtre (4 par défaut)
    int window = 4; 
    if (argc > 1) {
        window = atoi(argv[1]);
        if (window >= SEQ_NUM_SIZE || window <= 0) {
            printf("[TRP] Erreur : la taille doit etre entre 1 et %d.\n", SEQ_NUM_SIZE - 1);
            return 1;
        }
    }
    printf("[TRP] Taille de la fenetre d'emission : %d\n", window);

    init_reseau(EMISSION);
    printf("[TRP] Initialisation reseau : OK.\n");

    de_application(message, &taille_msg);

    // ==========================================
    // PHASE 1 : TRANSFERT DES DONNÉES (GBN)
    // ==========================================
    while(taille_msg != 0 || (borne_inf != curseur)) {

        // On envoie tant qu'il y a de la place ET des données à lire
        if(dans_fenetre(borne_inf, curseur, window) && taille_msg != 0) {

            for (int i = 0; i < taille_msg; i++) {
                p_data.info[i] = message[i];
            }
            p_data.type = DATA;
            p_data.num_seq = curseur;
            p_data.lg_info = taille_msg;
            p_data.somme_ctrl = generer_controle(&p_data);

            buffer[curseur] = p_data; // Sauvegarde dans la fenêtre
            vers_reseau(&p_data);

            if(curseur == borne_inf){
                depart_temporisateur(100);
            }

            curseur = (curseur + 1) % SEQ_NUM_SIZE; 
            de_application(message, &taille_msg); // On lit le suivant
        }
        else {
            evt = attendre();
            if(evt == PAQUET_RECU){
                de_reseau(&pack);
                
                // Si l'ACK est sans erreur, on décale la fenêtre de manière cumulative
                if(verifier_controle(&pack) && pack.type == ACK){
                    if(dans_fenetre(borne_inf, pack.num_seq, window)){
                        borne_inf = (pack.num_seq + 1) % SEQ_NUM_SIZE;
                        arret_temporisateur();
                        
                        if(borne_inf != curseur){
                            depart_temporisateur(100);
                        }
                    }
                }
            }
            else{ // TIMEOUT
                printf("[TRP] TIMEOUT ! Go-Back-N depuis le paquet %d\n", borne_inf);
                int i = borne_inf;
                // Retransmission de TOUTE la fenêtre jusqu'au pointeur courant
                while(i != curseur){
                    vers_reseau(&buffer[i]);
                    i = (i + 1) % SEQ_NUM_SIZE;
                }
                depart_temporisateur(100);
            }
        }
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}