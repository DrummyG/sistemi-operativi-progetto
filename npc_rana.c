//
// Created by Davide Balestrino on 06/01/25.
//

#include <ncurses.h>
#include <time.h>
#include "npc_rana.h"

//pipe tra padre e figlio
int canale_a_figlio[2];
int canale_a_padre[2];

//lo sprite della rana è 2x3 in ascii
int rana_altezza = 2; //altezza rana
//sprite ascii della rana
const char *prima_linea_sprite = " O "; //da rielaborare e fare a matrice
const char *seconda_linea_sprite = "/|\\";

//lo sprite del coccodrillo è 2xn in ascii
int coccodrillo_altezza = 2; //altezza coccodrillo
const char *prima_linea_sprite_c = "/////////"; //da rielaborare e fare a matrice
const char *seconda_linea_sprite_c = "|||||||||||";

//disegno sprite
void disegna_sprite(struct personaggio p) {
    for(int i = 0; i < p.lunghezza; i++){
        if(p.posizione.x + i> gioco_sinistra && p.posizione.x + i < gioco_destra){
            mvaddch(p.posizione.y,   p.posizione.x + i, prima_linea_sprite[i]);
            mvaddch(p.posizione.y + 1, p.posizione.x + i, seconda_linea_sprite[i]);
        }
    }
}

void disegna_coccodrillo(struct personaggio p){
    for(int i = 0; i < p.lunghezza; i++){
        if(p.posizione.x + i> gioco_sinistra && p.posizione.x + i < gioco_destra){
            mvaddch(p.posizione.y,   p.posizione.x + i, 'A');
            mvaddch(p.posizione.y + 1, p.posizione.x + i, 'V');
        }
    }
}

//processo figlio, gestisce i movimenti della rana
void processo_rana(int spawn_riga, int spawn_colonna, int alt,
                   int max_colonne_schermo, int max_righe_schermo,
                   int sinistra_campo, int larghezza_campo) {

    struct personaggio rana;
    rana.posizione.x = spawn_colonna;
    rana.posizione.y = spawn_riga;
    rana.tipo = RANA;
    rana.lunghezza = 3;

    struct timeval last_update, current_time;
    gettimeofday(&last_update, NULL); // Tempo iniziale

    //parto dalla posizione iniziale

    while(true) {
        char comando;
        int n = read(canale_a_figlio[0], &comando, 1);

        gettimeofday(&current_time, NULL);
        double elapsed = (current_time.tv_sec - last_update.tv_sec) +
                         (current_time.tv_usec - last_update.tv_usec) / 1000000.0;

        if (elapsed >= comando) {
            last_update = current_time; // Aggiorna il riferimento temporale

            // Aggiornamenti temporizzati in base al valore di `n`
            if (comando == 1) {
                rana.posizione.x -= 2;
            } else if (comando == 2) {
                rana.posizione.x += 2;
            }
        }



        int tasto = getch();
         if(tasto != ERR) {
             //deve diventare uno switch case
            comando = 0;
            if(tasto == KEY_UP) comando = 'U';
            else if(tasto == KEY_DOWN) comando = 'D';
            else if(tasto == KEY_LEFT) comando = 'L';
            else if(tasto == KEY_RIGHT) comando = 'R';
        }

        mvprintw(0, 0, "Valore di n: %d", n);
        refresh();

        if(n > 0) {
            //se ricevo su e c'è spazio sopra, vado su di 2
            if(comando == 'U' && rana.posizione.y > 0) {
                rana.posizione.y -= 2;
            }
                //se giu e ho ancora spazio sotto, vado giù di 2
            else if(comando == 'D' && rana.posizione.y < max_righe_schermo - alt) {
                rana.posizione.y += 2;
            }
                //se sinistra, vado a sinistra di 2
            else if(comando == 'L') {
                rana.posizione.x -= 2;
            }
                //se destra, vado a destra di 2
            else if(comando == 'R') {
                rana.posizione.x += 2;
            }
                //se 'O', resetto la rana allo spawn
            else if(comando == 'O') {
                rana.posizione.x = spawn_colonna;
                rana.posizione.y = spawn_riga;
            }

            //mando la posizione aggiornata al padre
            write(canale_a_padre[1], &rana, sizeof(struct personaggio));
        }
    }
}

void processo_coccodrilli(struct personaggio coccodrillo){
    while(true) {

        //mando la posizione aggiornata al padre
        write(canale_a_padre[1], &coccodrillo, sizeof(struct personaggio));

        //movimento coccodrillo
        if(coccodrillo.id < 20 || 40 <= coccodrillo.id && coccodrillo.id < 50 || 60 <= coccodrillo.id && coccodrillo.id < 70) {
            coccodrillo.posizione.x -= 2;
            if(coccodrillo.posizione.x < gioco_sinistra){
                coccodrillo.posizione.x = gioco_destra + coccodrillo.lunghezza;
            }
        }else{
            coccodrillo.posizione.x += 2;
            if(coccodrillo.posizione.x > gioco_destra){
                coccodrillo.posizione.x = gioco_sinistra - coccodrillo.lunghezza;
            }
        }
        //delay del movimento, potrei migliorarlo
        if(coccodrillo.id < 20 || 40 <= coccodrillo.id && coccodrillo.id < 50 || 60 <= coccodrillo.id && coccodrillo.id < 70){
            usleep(UDELAY2);
        }else{
            usleep(UDELAY);
        }


        /*if(coccodrillo.id < 10 || 30 <= coccodrillo.id && coccodrillo.id < 40 || 50 <= coccodrillo.id && coccodrillo.id < 60) {
            usleep(UDELAY2);
        }else{
            usleep(UDELAY);
        }*/
    }
}