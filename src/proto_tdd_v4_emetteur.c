#include <stdio.h>
#include <stdlib.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

#define SEQ_NUM_SIZE 16

int verifier_controle(paquet_t *paquet);
int generer_controle(paquet_t *paquet);

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[]) {
    unsigned char message[MAX_INFO];
    int taille_msg; 
    paquet_t buffer[SEQ_NUM_SIZE], pack;
    int ack_recus[SEQ_NUM_SIZE] = {0}; 
    
    int evt, borne_inf = 0, curseur = 0;
    int fin_fichier = 0; 
    
    int window = 4; 
    if (argc > 1) {
        window = atoi(argv[1]);
        if (window > SEQ_NUM_SIZE / 2 || window <= 0) {
            printf("[TRP] Erreur : la taille doit etre entre 1 et %d.\n", SEQ_NUM_SIZE / 2);
            return 1;
        }
    }
    printf("[TRP] Taille de la fenetre d'emission : %d\n", window);

    init_reseau(EMISSION);
    printf("[TRP] Initialisation reseau : OK.\n");

    while(!fin_fichier || (borne_inf != curseur)) {

        // 1. Envoi de nouvelles données
        if(!fin_fichier && dans_fenetre(borne_inf, curseur, window)) {
            
            de_application(message, &taille_msg);
            
            if (taille_msg == 0) {
                fin_fichier = 1; 
            }

            for (int i = 0; i < taille_msg; i++) {
                buffer[curseur].info[i] = message[i];
            }
            buffer[curseur].type = DATA;
            buffer[curseur].num_seq = curseur;
            buffer[curseur].lg_info = taille_msg;
            buffer[curseur].somme_ctrl = generer_controle(&buffer[curseur]);

            ack_recus[curseur] = 0; 
            
            vers_reseau(&buffer[curseur]);
            
            // ---> CORRECTION ICI : On utilise le numéro de séquence comme numéro de timer
            depart_temporisateur_num(curseur, 100); 

            curseur = (curseur + 1) % SEQ_NUM_SIZE; 
        }
        else {
            // 2. Attente d'un événement
            evt = attendre();
            
            if(evt == PAQUET_RECU){
                de_reseau(&pack);
                
                if(verifier_controle(&pack) && pack.type == ACK){
                    if(dans_fenetre(borne_inf, pack.num_seq, window)){
                        ack_recus[pack.num_seq] = 1;
                        
                        // ---> CORRECTION ICI : On arrête spécifiquement le timer de l'ACK reçu
                        arret_temporisateur_num(pack.num_seq); 
                        
                        // Décalage de la fenêtre si la base est acquittée
                        while(ack_recus[borne_inf]) {
                            ack_recus[borne_inf] = 0;
                            borne_inf = (borne_inf + 1) % SEQ_NUM_SIZE;
                            
                            if (fin_fichier && borne_inf == curseur) {
                                break;
                            }
                        }
                    }
                }
            }
            else { 
                // TIMEOUT : evt contient le numéro du timer qui a expiré (donc le num_seq)
                printf("[TRP] TIMEOUT sur le paquet %d. Retransmission unique.\n", evt);
                vers_reseau(&buffer[evt]);
                
                // ---> CORRECTION ICI : On relance spécifiquement ce timer
                depart_temporisateur_num(evt, 100);
            }
        }
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}