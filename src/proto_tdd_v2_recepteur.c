#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - recepteur  */
/* =============================== */
int verifier_controle(paquet_t *paquet);
int main(int argc, char* argv[]){
    unsigned char message[MAX_INFO];
    paquet_t p_data,pack;
    pack.lg_info = 0;
    int fin = 0;

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    while(!fin){

        de_reseau(&p_data);
        if(verifier_controle(&p_data)){
            pack.type = ACK;
            vers_reseau(&pack);
            for (int i = 0; i < p_data.lg_info; i++)
            {
                message[i] = p_data.info[i];
            }
            fin = vers_application(message,p_data.lg_info); 

        }
        else{
            printf("[TRP] Erreur de somme de controle.\n");
        }
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}