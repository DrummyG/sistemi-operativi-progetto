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
}

void disegna_timer(){
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

void pulisci_schermo() {
    for (int y = prato2_fine_riga; y < LINES; ++y) {
        mvhline(y, gioco_sinistra, ' ', larghezza_gioco);
    }
}

void funzione_padre(int spawn_colonna, int spawn_riga, struct personaggio *coccodrilli) {
    struct personaggio recupero; //variabile in cui salvo i valori letti dal figlio
    struct personaggio rana; rana.tipo = RANA; rana.lunghezza = 3; rana.posizione.x = spawn_colonna; rana.posizione.y = spawn_riga;

    //mando un reset iniziale per la rana //queste righe dovrebbero essere obsolete
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

    int vite_scorse = vite;
    disegna_info();


    bool fine_gioco = false;

    while(!fine_gioco) {
        //leggo una sola volta il valore per evitare confusione
        read(canale_a_padre[0], &recupero, sizeof(struct personaggio));
        if(recupero.tipo == RANA) rana = recupero;
        if(recupero.tipo == COCCODRILLO) coccodrilli[recupero.id] = recupero;

        int tasto = getch();
        if(tasto == 'p' || tasto == 'P') {
            pausa = !pausa;
        } else if(!pausa && tasto != ERR) {
            char comando = 0;
            //trasformare in uno switch case //funzione per la rana non lo deve fare il padre
            if(tasto == KEY_UP) comando = 'U';
            else if(tasto == KEY_DOWN) comando = 'D';
            else if(tasto == KEY_LEFT) comando = 'L';
            else if(tasto == KEY_RIGHT) comando = 'R';

            if(comando != 0) {
                write(canale_a_figlio[1], &comando, 1);
                controllo_bordi(rana);
            }
        }

        controllo_coccodrilli(rana, coccodrilli);


        //controllo se sto sulle tane
        if(check_tane(rana)){
            //mando un comando 'O' per dire "reset" al figlio
            char comando = 'O';
            write(canale_a_figlio[1], &comando, 1);
        }

        //se tutte le tane sono chiuse => vittoria
        if(tutte_tane_chiuse()) { //spostare in un'altra funzione
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

        pulisci_schermo();
        disegna_scenario(); //prima lo scenario
        for(int i = 0; i < NCOCCODRILLI; i++) { //poi i coccodrilli
            disegna_coccodrillo(coccodrilli[i]);
        }
        disegna_sprite(rana); //poi la rana così sta sopra i coccodrilli
        if(vite != vite_scorse) {
            disegna_info();
        }
        vite_scorse = vite;
        disegna_timer();

        //se sono in pausa, mostro un messaggio
        if(pausa) {
            attron(A_BOLD | COLOR_PAIR(3));
            mvprintw(max_righe / 2, (max_colonne - 15) / 2, "[ GIOCO IN PAUSA ]");
            attroff(A_BOLD | COLOR_PAIR(3));
        }

        //gestisco il timer
        gettimeofday(&tempo_attuale, NULL);
        long secondi_passati = tempo_attuale.tv_sec - tempo_iniziale.tv_sec;
        if(!pausa && secondi_passati > ultimo_secondo) {
            ultimo_secondo = secondi_passati;
            tempo_rimasto--;
            if(tempo_rimasto < 0) {
                timer_scaduto(&rana);
            }
        }

        //se vite = 0 => game over
        if(vite == 0) { //spostare in un'altra funzione
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