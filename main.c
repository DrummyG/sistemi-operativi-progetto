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

//gcc main.c prato_tane.c schermo.c npc_rana.c regole_gioco.c -l ncurses -o esercizio

//costanti (le uso ovunque, così non devo riscrivere i numeri ogni volta)
#define DIM_NOME 50  //mi serve per sapere la lunghezza massima del nome utente

//variabili globali (le metto qui per comodità, così le uso in più funzioni)
int max_righe;  //numero di righe del terminale
int max_colonne;  // numero di colonne del terminale

//variabili di gioco
bool pausa;    //flag per vedere se sono in pausa
const char *nickname; //memorizzo il nome utente

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
    }

    if(pid == 0) {
        //figlio
        close(canale_a_figlio[1]);
        close(canale_a_padre[0]);

        //calcolo la posizione di spawn al centro del prato inferiore
        int prato_y_centrale = (riga_inizio_prato + riga_fine_prato - rana_altezza) / 2;
        int prato_x_centrale = gioco_sinistra + (larghezza_gioco - rana_larghezza) / 2;
        if(prato_y_centrale % 2 != 0) prato_y_centrale--;

        processo_rana(prato_y_centrale, prato_x_centrale,
                      rana_larghezza, rana_altezza,
                      max_colonne, max_righe,
                      gioco_sinistra, larghezza_gioco);
        exit(0);
    }

    //padre
    close(canale_a_figlio[0]);
    close(canale_a_padre[1]);

    //mando un reset iniziale per la rana
    {
        int prato_y_centrale = (riga_inizio_prato + riga_fine_prato - rana_altezza) / 2;
        int prato_x_centrale = gioco_sinistra + (larghezza_gioco - rana_larghezza) / 2;
        if(prato_y_centrale % 2 != 0) prato_y_centrale--;

        char comando = 'O';
        write(canale_a_figlio[1], &comando, 1);

        int r_x, r_y;
        read(canale_a_padre[0], &r_x, sizeof(int));
        read(canale_a_padre[0], &r_y, sizeof(int));
        rana_x = r_x;
        rana_y = r_y;
    }

    nodelay(stdscr, true);

    struct timeval tempo_iniziale;
    struct timeval tempo_attuale;
    gettimeofday(&tempo_iniziale, NULL);
    long ultimo_secondo = 0;

    bool fine_gioco = false;

    while(!fine_gioco) {
        int tasto = getch();
        if(tasto == 'p' || tasto == 'P') {
            pausa = !pausa;
        }
        else if(!pausa && tasto != ERR) {
            char comando = 0;
            if(tasto == KEY_UP) comando = 'U';
            else if(tasto == KEY_DOWN) comando = 'D';
            else if(tasto == KEY_LEFT) comando = 'L';
            else if(tasto == KEY_RIGHT) comando = 'R';

            if(comando != 0) {
                write(canale_a_figlio[1], &comando, 1);
                int r_x, r_y;
                read(canale_a_padre[0], &r_x, sizeof(int));
                read(canale_a_padre[0], &r_y, sizeof(int));
                rana_x = r_x;
                rana_y = r_y;

                //se vado oltre i limiti a sinistra o destra
                if(rana_x < gioco_sinistra || (rana_x + rana_larghezza - 1) > gioco_destra) {
                    vite--;
                    tempo_rimasto = TEMPO_MASSIMO;

                    char comando2 = 'O';
                    write(canale_a_figlio[1], &comando2, 1);
                    int rrxx, rryy;
                    read(canale_a_padre[0], &rrxx, sizeof(int));
                    read(canale_a_padre[0], &rryy, sizeof(int));
                    rana_x = rrxx;
                    rana_y = rryy;
                }

                //se vado sotto il prato
                if((rana_y + rana_altezza - 1) > riga_fine_prato) {
                    vite--;
                    tempo_rimasto = TEMPO_MASSIMO;

                    char comando2 = 'O';
                    write(canale_a_figlio[1], &comando2, 1);
                    int rrxx, rryy;
                    read(canale_a_padre[0], &rrxx, sizeof(int));
                    read(canale_a_padre[0], &rryy, sizeof(int));
                    rana_x = rrxx;
                    rana_y = rryy;
                }
            }
        }

        //controllo se sto sulle tane
        check_tane();

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
        disegna_sprite();
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
                timer_scaduto();
                if(vite == 0) {
                    //se ho finito le vite => game over
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
