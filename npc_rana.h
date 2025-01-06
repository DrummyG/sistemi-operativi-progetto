//
// Created by Davide Balestrino on 06/01/25.
//

#ifndef PROGETTO_SO_NPC_RANA_H
#define PROGETTO_SO_NPC_RANA_H

#include <ncurses.h>
#include <unistd.h>

extern int canale_a_figlio[2];
extern int canale_a_padre[2];

//lo sprite è 2x3 in ascii
extern int rana_altezza; //altezza rana
extern int rana_larghezza; //larghezza rana
extern int rana_x; //posizione x della rana
extern int rana_y; //osizione y della rana
//sprite ascii della rana
extern const char *prima_linea_sprite;
extern const char *seconda_linea_sprite;

void disegna_sprite();
void processo_rana(int spawn_riga, int spawn_colonna, int largh, int alt,
                   int max_colonne_schermo, int max_righe_schermo,
                   int sinistra_campo, int larghezza_campo);

#endif //PROGETTO_SO_NPC_RANA_H
