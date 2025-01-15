//
// Created by Davide Balestrino on 06/01/25.
//

#include "regole_gioco.h"

//variabili di gioco
int vite;  //numero di vite del giocatore
int tempo_rimasto;  //quanto tempo mi rimane (conta alla rovescia)
int punteggio;  //punteggio (se lo voglio usare in futuro)

//se il tempo arriva a 0, perdo vita e resetto
void timer_scaduto(struct personaggio *rana) {
    vite--;
    tempo_rimasto = TEMPO_MASSIMO;

    char comando = 'O';
    write(canale_a_figlio[1], &comando, 1);


    read(canale_a_padre[0], rana, sizeof(int));
}

void controllo_bordi(struct personaggio rana){
    //se vado oltre i limiti a sinistra o destra
    if(rana.posizione.x < gioco_sinistra || (rana.posizione.x + rana.lunghezza - 1) > gioco_destra) {
        vite--;
        tempo_rimasto = TEMPO_MASSIMO;

        char comando2 = 'O';
        write(canale_a_figlio[1], &comando2, 1);
    }

    //se vado sotto il prato
    if((rana.posizione.y + rana_altezza - 1) > riga_fine_prato) {
        vite--;
        tempo_rimasto = TEMPO_MASSIMO;

        char comando2 = 'O';
        write(canale_a_figlio[1], &comando2, 1);
    }
}

//se tutte le tane sono chiuse => vittoria
bool tutte_tane_chiuse() {
    for(int i = 0; i < num_tane; i++) {
        if(tane_aperte[i]) return false;
    }
    return true;
}

//qui controllo le collisioni,, se la rana entra nell'area della tana e questa è aperta, +vita, se chiusa o colonna gialla, -vita
bool check_tane(struct personaggio rana) {
    int riga_inizio_buco = tana_inizio_riga + 1; //il buco comincia uno sotto
    int riga_fine_buco   = tana_inizio_riga + 4;//finisce un po' prima della fine gialla

    // se la rana non tocca neanche la fascia verticale della tana, esco
    if(rana.posizione.x + rana_altezza - 1 < tana_inizio_riga || rana.posizione.y > tana_fine_riga) {
        return false;
    }

    for(int i = 0; i < num_tane; i++) {
        //calcolo l'inizio x della tana i
        int inizio_tana_x = gioco_sinistra + offset_tane + i * (larghezza_tana + spazio);
        int fine_tana_x = inizio_tana_x + larghezza_tana - 1;
        int inizio_buco_x = inizio_tana_x + 1;
        int larghezza_buco = larghezza_tana - 1;
        int fine_buco_x = inizio_buco_x + larghezza_buco - 1;

        //controllo orizzontalmente
        bool sovrapposizione_tana =
                (rana.posizione.x + rana.lunghezza - 1 >= inizio_tana_x) &&
                (rana.posizione.x <= fine_tana_x);

        //controllo verticalmente
        bool sovrapposizione_vert_tana =
                (rana.posizione.y + rana_altezza - 1 >= tana_inizio_riga) &&
                (rana.posizione.y <= tana_fine_riga);

        if(sovrapposizione_tana && sovrapposizione_vert_tana) {
            //controlliamo se stiamo nel buco (in mezzo) oppure sui pilastri gialli
            bool sovrapposizione_buco =
                    (rana.posizione.x + rana.lunghezza - 1 >= inizio_buco_x) &&
                    (rana.posizione.x <= fine_buco_x) &&
                    (rana.posizione.y + rana_altezza - 1 >= riga_inizio_buco) &&
                    (rana.posizione.y <= riga_fine_buco);

            if(sovrapposizione_buco) {
                //tana aperta => +1 vita, chiusa => -1 vita
                if(tane_aperte[i]) {
                    if(vite < 8) vite++;
                    tempo_rimasto = TEMPO_MASSIMO;
                    tane_aperte[i] = false;
                } else {
                    vite--;
                    tempo_rimasto = TEMPO_MASSIMO;
                }
            } else {
                //se tocc i pilastri gialli => -1 vita
                vite--;
                tempo_rimasto = TEMPO_MASSIMO;
            }

            return true;
        }
    }
    return false;
}