//
// Created by Davide Balestrino on 15/01/25.
//

#include "padre.h"

bool pausa;
const char *nickname; //memorizzo il nome utente

int max_righe;
int max_colonne;

//disegno una specie di blocchetto 2x2 per le vite
void disegna_quadrato_vita(int riga, int colonna) {
    //qui faccio " __ "
    move(riga, colonna);
    addch(' ');
    attron(COLOR_PAIR(7));
    addstr("__");
    attroff(COLOR_PAIR(7));
    addch(' ');

    //qui faccio "|__|"
    move(riga + 1, colonna);
    addch('|');
    attron(COLOR_PAIR(7));
    addstr("__");
    attroff(COLOR_PAIR(7));
    addch('|');
}

//disegno  nickname, punteggio, vite e timer
void disegna_info() {
    int riga_centrale = max_righe / 2; //calcolo una specie di "centro" verticale

    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(riga_centrale - 4, 0, "Username: ");
    attroff(A_BOLD);
    printw("%s", nickname);

    attron(A_BOLD);
    mvprintw(riga_centrale - 2, 0, "Punteggio: ");
    attroff(A_BOLD);
    printw("%d", punteggio);
    attroff(COLOR_PAIR(6));

    int vite_colonna = max_colonne - 7;
    if(vite_colonna < 0) vite_colonna = 0;

    int vite_riga_inizio = 23;
    mvprintw(vite_riga_inizio, vite_colonna, "VITE:");

    //disegno i quadratini vita in verticale
    for(int v = 0; v < vite; v++) {
        int riga_blocco = vite_riga_inizio + 2 + v * 4;
        disegna_quadrato_vita(riga_blocco, vite_colonna);
    }

    //barretta timer
    int dimensione_barra = 50 * tempo_rimasto / TEMPO_MASSIMO;
    const char *etichetta = "Timer: ";
    int lung_etichetta = (int)strlen(etichetta);
    int lunghezza_totale = lung_etichetta + dimensione_barra;
    int colonna_centrale = (max_colonne - lunghezza_totale) / 2;
    int riga_in_basso = max_righe - 1;

    attron(COLOR_PAIR(4));
    mvprintw(riga_in_basso, colonna_centrale, "%s", etichetta);
    for(int i = 0; i < dimensione_barra; i++) {
        mvprintw(riga_in_basso, colonna_centrale + lung_etichetta + i, "|");
    }
    attroff(COLOR_PAIR(4));
}

void funzione_padre(int spawn_colonna, int spawn_riga) {
    struct personaggio recupero;
    struct personaggio rana; rana.tipo = RANA; rana.lunghezza = 3; rana.posizione.x = spawn_colonna; rana.posizione.y = spawn_riga;
    struct personaggio coccodrillo; coccodrillo.tipo = COCCODRILLO; coccodrillo.lunghezza = 4; coccodrillo.posizione.x = 30 - coccodrillo.lunghezza; coccodrillo.posizione.y = riga_fine_prato + 1;

    //mando un reset iniziale per la rana
    {
        int prato_y_centrale = (riga_inizio_prato + riga_fine_prato - rana_altezza) / 2;
        int prato_x_centrale = gioco_sinistra + (larghezza_gioco - rana.lunghezza) / 2;
        if(prato_y_centrale % 2 != 0) prato_y_centrale--;
    }

    nodelay(stdscr, true);

    struct timeval tempo_iniziale;
    struct timeval tempo_attuale;
    gettimeofday(&tempo_iniziale, NULL);
    long ultimo_secondo = 0;

    bool fine_gioco = false;

    while(!fine_gioco) {

        read(canale_a_padre[0], &recupero, sizeof(struct personaggio));
        if(recupero.tipo == RANA) rana = recupero;
        if(recupero.tipo == COCCODRILLO) coccodrillo = recupero;

        int tasto = getch();
        if(tasto == 'p' || tasto == 'P') {
            pausa = !pausa;
        } else if(!pausa && tasto != ERR) {
            char comando = 0;
            if(tasto == KEY_UP) comando = 'U';
            else if(tasto == KEY_DOWN) comando = 'D';
            else if(tasto == KEY_LEFT) comando = 'L';
            else if(tasto == KEY_RIGHT) comando = 'R';

            if(comando != 0) {
                write(canale_a_figlio[1], &comando, 1);
                controllo_bordi(rana);
            }
        }

        //controllo se sto sulle tane
        if(check_tane(rana)){
            //mando un comando 'O' per dire "reset" al figlio
            char comando = 'O';
            write(canale_a_figlio[1], &comando, 1);
        }

        //se tutte le tane sono chiuse => vittoria
        if(tutte_tane_chiuse()) {
            clear();
            attron(COLOR_PAIR(1) | A_BOLD);
            mvprintw(max_righe / 2 - 1,
                     (max_colonne - (int)(strlen(nickname) + 25)) / 2,
                     "COMPLIMENTI %s HAI VINTO", nickname);
            mvprintw(max_righe / 2 + 1,
                     (max_colonne - 45) / 2,
                     "(Premi invio per tornare al menu principale)");
            attroff(COLOR_PAIR(1) | A_BOLD);
            refresh();

            nodelay(stdscr, false);
            int c;
            do {
                c = getch();
            } while(c != 10 && c != KEY_ENTER);

            fine_gioco = true;
            break;
        }

        clear();
        disegna_scenario();
        disegna_sprite(rana);
        disegna_coccodrillo(coccodrillo);
        disegna_info();

        //se sono in pausa, mostro un messaggio
        if(pausa) {
            attron(A_BOLD | COLOR_PAIR(3));
            mvprintw(max_righe / 2, (max_colonne - 15) / 2, "[ GIOCO IN PAUSA ]");
            attroff(A_BOLD | COLOR_PAIR(3));
        }

        refresh();

        //gestisco il timer
        gettimeofday(&tempo_attuale, NULL);
        long secondi_passati = tempo_attuale.tv_sec - tempo_iniziale.tv_sec;
        if(!pausa && secondi_passati > ultimo_secondo) {
            ultimo_secondo = secondi_passati;
            tempo_rimasto--;
            if(tempo_rimasto < 0) {
                //tempo scaduto
                timer_scaduto(&rana);
                //tolto l'if delle vite rimaste c'è già sotto qua è ripetuto
            }
        }

        //se vite = 0 => game over
        if(vite == 0) {
            clear();
            attron(COLOR_PAIR(3));
            mvprintw(max_righe / 2, (max_colonne - 10) / 2, "GAME OVER");
            attroff(COLOR_PAIR(3));
            refresh();
            nodelay(stdscr, false);
            int c;
            do {
                c = getch();
            } while(c != 10 && c != KEY_ENTER);
            fine_gioco = true;
            break;
        }
    }
}