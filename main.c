#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>

#include "regole_gioco.h"
#include "padre.h"

//gcc main.c prato_tane.c schermo.c npc_rana.c regole_gioco.c padre.c -l ncurses -o esercizio

//costanti (le uso ovunque, così non devo riscrivere i numeri ogni volta)
#define DIM_NOME 50  //mi serve per sapere la lunghezza massima del nome utente

//inizio del gioco,preparo tutto, calcolo spawn, fork e avvio
void inizio(const char *nome_utente) {
    clear();
    curs_set(0);
    getmaxyx(stdscr, max_righe, max_colonne);

    gioco_sinistra = 30;
    gioco_destra = max_colonne - 31;
    larghezza_gioco = (gioco_destra - gioco_sinistra + 1);

    punteggio = 0;
    vite = 5;
    tempo_rimasto = TEMPO_MASSIMO;
    pausa = false;
    nickname = nome_utente;

    //definisco il prato in basso
    riga_inizio_prato = (max_righe - 2) - (prato_altezza - 1);
    riga_fine_prato   = riga_inizio_prato + (prato_altezza - 1);

    //allineo le righe a multipli di 2, così la rana non sta a metà
    if(riga_inizio_prato % 2 != 0) riga_inizio_prato--;
    riga_fine_prato = riga_inizio_prato + (prato_altezza - 1);
    if(riga_fine_prato % 2 != 0) riga_fine_prato++;

    //le tane partono dalla riga 1, alte 5
    tana_inizio_riga = 1;
    tana_fine_riga   = tana_inizio_riga + tana_altezza - 1;

    //il secondo prato inizia dopo la fine delle tane, alto 6
    prato2_inizio_riga = tana_fine_riga + 1;
    prato2_fine_riga   = prato2_inizio_riga + (prato2_altezza - 1);

    //faccio due conti su come distribuire le tane orizzontalmente
    spazio_totale = (num_tane - 1) * spazio;
    totale_per_le_tane = larghezza_gioco - spazio_totale;
    larghezza_tana = totale_per_le_tane / num_tane;
    offset_tane = (totale_per_le_tane % num_tane) / 2;

    //calcolo la posizione di spawn al centro del prato inferiore
    int prato_y_centrale = (riga_inizio_prato + riga_fine_prato - rana_altezza) / 2;
    int prato_x_centrale = gioco_sinistra + (larghezza_gioco - 3) / 2;
    if(prato_y_centrale % 2 != 0) prato_y_centrale--;

    //all'inizio tutte le tane sono aperte
    for(int i = 0; i < num_tane; i++) {
        tane_aperte[i] = true;
    }

    //creo le pipe
    if(pipe(canale_a_figlio) == -1 || pipe(canale_a_padre) == -1) {
        endwin();
        perror("pipe");
        exit(1);
    }

    //fork per generare il figlio
    pid_t pid = fork();
    if(pid == -1) {
        endwin();
        perror("fork");
        exit(1);
    }else if(pid == 0) {
        //figlio
        close(canale_a_figlio[1]);
        close(canale_a_padre[0]);
         //da eliminare cose inutili nel richiamo della funzione
        processo_rana(prato_y_centrale, prato_x_centrale, rana_altezza,
                      max_colonne, max_righe,
                      gioco_sinistra, larghezza_gioco);
    }

    //ricordati di aggiungere la deallocazione della memoria
    struct personaggio *coccodrilli = (struct personaggio*) calloc(NCOCCODRILLI, sizeof(struct personaggio));
    int y_iniziale = riga_inizio_prato - 2;
    int x_iniziale = gioco_sinistra;
    int distanza_coccodrilli = 20;
    int distanza_coccodrilli_2 = 30;
    int offset = 10;

    for(int i = 0; i < NCOCCODRILLI; i++) {
        coccodrilli[i].id = i;
        coccodrilli[i].tipo = COCCODRILLO;
        if(i % 2 == 0) { //decido la lunghezza
            coccodrilli[i].lunghezza = 8;
        }else if(i % 3 == 0){
            coccodrilli[i].lunghezza = 16;
        }else{
            coccodrilli[i].lunghezza = 4;
        }

        if(i % 5 == 0 && i != 0){ //vado avanti di riga se ci sono cinque coccodrilli nella riga
            y_iniziale -= 2;
            if(y_iniziale > LINES/2 && y_iniziale < riga_fine_prato - 20){ //se ho raggiunto il prato in mezzo
                y_iniziale = LINES/2 + 1; //salto fino alla fine del prato
            }
            x_iniziale = gioco_sinistra;
            if(i % 10 == 0){
                x_iniziale += offset; //lo aggiungo per evitare che i coccodrilli siano alla stessa altezza
            }
        }else if(i != 0 && i % 3 == 0){ //distanza tra i coccdrilli laterale
            x_iniziale = coccodrilli[i - 1].posizione.x + distanza_coccodrilli;
        }else if(i != 0){
            x_iniziale = coccodrilli[i - 1].posizione.x + distanza_coccodrilli_2;
        }
        //assegno posizione
        coccodrilli[i].posizione.x = x_iniziale;
        coccodrilli[i].posizione.y = y_iniziale;
        //avvio processo figlio
        pid_t pid_coccodrilli = fork();
        if(pid_coccodrilli == -1) {
            endwin();
            perror("fork");
            exit(1);
        }else if(pid_coccodrilli == 0) {
            //figlio
            close(canale_a_figlio[1]);
            close(canale_a_padre[0]);
            processo_coccodrilli(coccodrilli[i]);
        }
    }

    //padre
    close(canale_a_figlio[0]);
    close(canale_a_padre[1]);
    funzione_padre(prato_x_centrale, prato_y_centrale, coccodrilli);


    //killo il figlio
    kill(pid, SIGKILL);
    waitpid(pid, NULL, 0);
}

//menu principale, scegliere se iniziare a giocare o uscire
void menu() {
    const char *opzioni[] = {"Inizia a giocare", "Esci"};
    int scelta = 0;
    int tasto;
    char nome_utente[DIM_NOME];
    int num_opzioni;

    getmaxyx(stdscr, max_righe, max_colonne);
    num_opzioni = sizeof(opzioni) / sizeof(opzioni[0]);

    nodelay(stdscr, false);

    while(true) {
        clear();
        attron(COLOR_PAIR(2));
        mvprintw(max_righe / 2 - 3, (max_colonne - 33) / 2,
                 "#################################");
        mvprintw(max_righe / 2 - 2, (max_colonne - 33) / 2,
                 "#       Frogger Resurrection    #");
        mvprintw(max_righe / 2 - 1, (max_colonne - 33) / 2,
                 "#################################");
        attroff(COLOR_PAIR(2));

        //stampo le opzioni di menu
        for(int i = 0; i < num_opzioni; i++) {
            if(i == scelta) {
                attron(A_REVERSE | COLOR_PAIR(2));
            }
            mvprintw(max_righe / 2 + i + 1,
                     (max_colonne - (int)strlen(opzioni[i])) / 2,
                     "%s", opzioni[i]);
            if(i == scelta) {
                attroff(A_REVERSE | COLOR_PAIR(2));
            }
        }

        tasto = getch();
        switch(tasto) {
            case KEY_UP:
                scelta = (scelta - 1 + num_opzioni) % num_opzioni;
                break;
            case KEY_DOWN:
                scelta = (scelta + 1) % num_opzioni;
                break;
            case 10: //invio
                if(scelta == 0) {
                    clear();
                    attron(A_BOLD);
                    mvprintw(max_righe / 2 - 1, (max_colonne - 25) / 2,
                             "INSERISCI IL TUO USERNAME:");
                    attroff(A_BOLD);

                    echo();
                    curs_set(1);
                    int colonna_inizio = (max_colonne - DIM_NOME) / 2;
                    mvprintw(max_righe / 2 + 1, colonna_inizio, "");
                    mvgetnstr(max_righe / 2 + 1, colonna_inizio, nome_utente, DIM_NOME - 1);
                    noecho();
                    curs_set(0);
                    nome_utente[DIM_NOME - 1] = '\0';

                    clear();
                    attron(A_BOLD);
                    mvprintw(max_righe / 2 - 2,
                             (max_colonne - (int)strlen(nome_utente) - 20) / 2,
                             "BENVENUTO, %s!", nome_utente);
                    mvprintw(max_righe / 2,
                             (max_colonne - 28) / 2,
                             "IL GIOCO STA PER INIZIARE...");
                    attroff(A_BOLD);

                    refresh();
                    nodelay(stdscr, true);

                    inizio(nome_utente);
                }
                else if(scelta == 1) {
                    clear();
                    mvprintw(3, 5, "Grazie per aver giocato!");
                    getch();
                    return;
                }
                break;
            default:
                break;
        }
    }
}

//main, inizializzo ncurses e i colori, poi chiamo menu()
int main() {
    initscr();
    start_color();
    keypad(stdscr, true);

    init_pair(1, COLOR_BLACK, COLOR_GREEN);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);
    init_pair(5, COLOR_WHITE, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_RED);
    init_pair(8, COLOR_BLACK, COLOR_YELLOW);
    init_pair(9, COLOR_BLACK, COLOR_BLACK);

    noecho();
    cbreak();
    curs_set(0);

    menu();
    endwin();
    return 0;
}
