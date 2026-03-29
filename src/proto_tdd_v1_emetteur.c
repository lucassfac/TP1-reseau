#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int generer_controle(const paquet_t *paquet);

int main(int argc, char* argv[]){
    unsigned char message[MAX_INFO];
    int taille_msg;
    paquet_t p_data, pack;

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    de_application(message, &taille_msg);

    while ( taille_msg != 0 ) {

        for (int i=0; i<taille_msg; i++) {
            p_data.info[i] = message[i];
        }
        p_data.lg_info = taille_msg;
        p_data.type = DATA;
        p_data.somme_ctrl = generer_controle(&p_data);

        pack.type = NACK;
        while (pack.type == NACK) {
            vers_reseau(&p_data);
            de_reseau(&pack);
        }
        de_application(message, &taille_msg);
    }

    p_data.lg_info = 0;
    p_data.type = DATA;
    p_data.somme_ctrl = generer_controle(&p_data);

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}