#include <stdio.h>
#include <stdlib.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - recepteur  */
/* =============================== */
int verifier_controle(paquet_t *paquet);
int generer_controle(const paquet_t *paquet);
int inc(int const n, int const mod);

int main(int argc, char* argv[]) {
    paquet_t buffer[SEQ_NUM_SIZE], pack, ack;
    int recu[SEQ_NUM_SIZE] = {0}; // Tableau pour marquer les paquets bufferisés
    
    int fin = 0;
    int borne_inf = 0; // Le prochain numéro de séquence attendu en ordre
    
    int window = 4;
    if (argc > 1) {
        window = atoi(argv[1]);
        if (window > SEQ_NUM_SIZE / 2 || window <= 0) {
            printf("[TRP] Erreur taille fenetre.\n");
            return 1;
        }
    }

    init_reseau(RECEPTION);
    printf("[TRP] Initialisation reseau : OK. Fenetre : %d\n", window);

    while (!fin) {
        de_reseau(&pack);
        
        if (verifier_controle(&pack) && pack.type == DATA) {
            
            // 1. Règle absolue : on acquitte TOUJOURS un paquet reçu sans erreur
            ack.type = ACK;
            ack.num_seq = pack.num_seq;
            ack.lg_info = 0;
            ack.somme_ctrl = generer_controle(&ack);
            vers_reseau(&ack); 
            
            // 2. Si le paquet est dans notre fenêtre d'intérêt et pas encore bufferisé
            if (dans_fenetre(borne_inf, pack.num_seq, window) && !recu[pack.num_seq]) {
                
                // On le stocke au chaud (bufferisation hors séquence)
                buffer[pack.num_seq] = pack;
                recu[pack.num_seq] = 1;
                
                // 3. Remise en ordre à la couche application
                // Tant que la borne inférieure est disponible dans le buffer
                while (recu[borne_inf]) {
                    
                    if (buffer[borne_inf].lg_info == 0) {
                        fin = 1; // C'est le paquet de fin
                    } else {
                        vers_application(buffer[borne_inf].info, buffer[borne_inf].lg_info);
                    }
                    
                    // On libère la case et on décale la fenêtre
                    recu[borne_inf] = 0;
                    borne_inf = (borne_inf + 1) % SEQ_NUM_SIZE;
                }
            }
            // Si le paquet n'est pas dans la fenêtre (vieux paquet), on l'a quand même acquitté au point 1 !
        }
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}