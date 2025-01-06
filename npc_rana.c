//
// Created by Davide Balestrino on 06/01/25.
//

#include <ncurses.h>
#include "npc_rana.h"

//pipe tra padre e figlio
int canale_a_figlio[2];
int canale_a_padre[2];

//lo sprite è 2x3 in ascii
int rana_altezza = 2; //altezza rana
int rana_larghezza = 3; //larghezza rana
int rana_x; //posizione x della rana
int rana_y; //osizione y della rana
//sprite ascii della rana
const char *prima_linea_sprite = " O ";
const char *seconda_linea_sprite = "/|\\";

//disegno sprite
void disegna_sprite() {
    mvprintw(rana_y,   rana_x, "%s", prima_linea_sprite);
    mvprintw(rana_y+1, rana_x, "%s", seconda_linea_sprite);
}

//processo figlio, gestisce i movimenti della rana
void processo_rana(int spawn_riga, int spawn_colonna, int largh, int alt,
                   int max_colonne_schermo, int max_righe_schermo,
                   int sinistra_campo, int larghezza_campo) {

    //parto dalla posizione iniziale
    int pos_x = spawn_colonna;
    int pos_y = spawn_riga;

    while(true) {
        char comando;
        int n = read(canale_a_figlio[0], &comando, 1);
        if(n > 0) {
            //se ricevo su e c'è spazio sopra, vado su di 2
            if(comando == 'U' && pos_y > 0) {
                pos_y -= 2;
            }
                //se giu e ho ancora spazio sotto, vado giù di 2
            else if(comando == 'D' && pos_y < max_righe_schermo - alt) {
                pos_y += 2;
            }
                //se sinistra, vado a sinistra di 2
            else if(comando == 'L') {
                pos_x -= 2;
            }
                //se destra, vado a destra di 2
            else if(comando == 'R') {
                pos_x += 2;
            }
                //se 'O', resetto la rana allo spawn
            else if(comando == 'O') {
                pos_x = spawn_colonna;
                pos_y = spawn_riga;
            }

            //mando la posizione aggiornata al padre
            write(canale_a_padre[1], &pos_x, sizeof(int));
            write(canale_a_padre[1], &pos_y, sizeof(int));
        }
    }
}