//
// Created by Davide Balestrino on 06/01/25.
//

#ifndef PROGETTO_SO_REGOLE_GIOCO_H
#define PROGETTO_SO_REGOLE_GIOCO_H

#include "npc_rana.h"

#define TEMPO_MASSIMO 90 // questo è il tempo massimo del timer

//variabili di gioco
extern int vite;  //numero di vite del giocatore
extern int tempo_rimasto;  //quanto tempo mi rimane (conta alla rovescia)
extern int punteggio;  //punteggio (se lo voglio usare in futuro)

void timer_scaduto(struct personaggio *rana);

void controllo_bordi(struct personaggio rana);

bool tutte_tane_chiuse();
bool check_tane(struct personaggio rana);



#endif //PROGETTO_SO_REGOLE_GIOCO_H
