#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

int inc(int const n, int const mod);
int generer_controle(const paquet_t *paquet);
/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[]) {
    unsigned char message[MAX_INFO];
    int taille_msg; 
    paquet_t p_data;
    paquet_t pack;
    paquet_t buffer[SEQ_NUM_SIZE];
    int evt;

    int borne_inf = 0;
    int curseur = 0;
    int fin_fichier = 0; // Astuce : permet d'intégrer le paquet de fin dans la boucle
    
    // Gestion de la taille de fenêtre (par défaut 4, ou argument ligne de commande)
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

    // On boucle tant qu'on n'a pas fini de lire OU qu'il reste des paquets en vol
    while(!fin_fichier || (borne_inf != curseur)) {

        // Si on a de la place et qu'on n'a pas encore lu la fin du fichier
        if(dans_fenetre(borne_inf, curseur, window) && !fin_fichier) {
            
            de_application(message, &taille_msg);
            
            // Si c'est la fin, on lève le drapeau, mais on envoie QUAND MÊME ce paquet !
            if (taille_msg == 0) {
                fin_fichier = 1; 
            }
            
            for (int i = 0; i < taille_msg; i++) {
                p_data.info[i] = message[i];
            }

            p_data.type = DATA;
            p_data.num_seq = curseur;
            p_data.lg_info = taille_msg;
            p_data.somme_ctrl = generer_controle(&p_data);

            buffer[curseur] = p_data; // Sauvegarde pour retransmission
            vers_reseau(&p_data);

            if(curseur == borne_inf){
                depart_temporisateur(100);
            }

            curseur = (curseur, SEQ_NUM_SIZE);
        }
        else {
            evt = attendre();
            if(evt == PAQUET_RECU){
                de_reseau(&pack);
                
                // ACK Cumulatif
                if(dans_fenetre(borne_inf, pack.num_seq, window) && verifier_controle(&pack) && pack.type == ACK){
                    borne_inf = inc(pack.num_seq, SEQ_NUM_SIZE);
                    arret_temporisateur();
                    
                    // S'il reste des paquets non acquittés, on relance
                    if(borne_inf != curseur){
                        depart_temporisateur(100);
                    }
                }
            }
            else{ // TIMEOUT
                printf("[TRP] TIMEOUT ! Go-Back-N depuis le paquet %d\n", borne_inf);
                int i = borne_inf;
                // Retransmission de toute la fenêtre
                while(i != curseur){
                    vers_reseau(&buffer[i]);
                    i = inc(i, SEQ_NUM_SIZE);
                }
                depart_temporisateur(100);
            }
        }
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}