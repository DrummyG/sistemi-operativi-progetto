//
// Created by Davide Balestrino on 06/01/25.
//

#ifndef PROGETTO_SO_NPC_RANA_H
#define PROGETTO_SO_NPC_RANA_H

#include <ncurses.h>
#include <unistd.h>
#include <sys/time.h>

#include "prato_tane.h"

#define UDELAY 2000000
#define UDELAY2 1000000

typedef enum {RANA, COCCODRILLO, PROIETTILE} tipo;

struct posizione {
    int x;
    int y;
};

struct personaggio {
    struct posizione posizione;
    tipo  tipo;
    int lunghezza;
    int id;
};

extern int canale_a_figlio[2];
extern int canale_a_padre[2];

//lo sprite è 2x3 in ascii
extern int rana_altezza; //altezza rana
//sprite ascii della rana
extern const char *prima_linea_sprite;
extern const char *seconda_linea_sprite;

void disegna_sprite(struct personaggio p);
void processo_rana(int spawn_riga, int spawn_colonna, int alt,
                   int max_colonne_schermo, int max_righe_schermo,
                   int sinistra_campo, int larghezza_campo);

void disegna_coccodrillo(struct personaggio p);
void processo_coccodrilli(struct personaggio coccodrillo);

#endif //PROGETTO_SO_NPC_RANA_H
