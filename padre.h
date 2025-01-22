//
// Created by Davide Balestrino on 15/01/25.
//

#ifndef PROGETTO_SO_PADRE_H
#define PROGETTO_SO_PADRE_H

#include <sys/time.h>
#include <ncurses.h>
#include <string.h>

#include "regole_gioco.h"

extern bool pausa;    //flag per vedere se sono in pausa
extern const char *nickname; //memorizzo il nome utente
//variabili globali (le metto qui per comodità, così le uso in più funzioni)
extern int max_righe;  //numero di righe del terminale
extern int max_colonne;  // numero di colonne del terminale

void disegna_quadrato_vita(int riga, int colonna);
void disegna_info();
void disegna_timer();
void pulisci_schermo();

void funzione_padre(int spawn_colonna, int spawn_riga, struct personaggio *coccodrilli);

#endif //PROGETTO_SO_PADRE_H
