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

//costanti (le uso ovunque, così non devo riscrivere i numeri ogni volta)
#define DIM_NOME 50  //mi serve per sapere la lunghezza massima del nome utente
#define TEMPO_MASSIMO 90 // questo è il tempo massimo del timer

//variabili globali (le metto qui per comodità, così le uso in più funzioni)
int max_righe;  //numero di righe del terminale
int max_colonne;  // numero di colonne del terminale

//campo di gioco orizzontale
int gioco_sinistra = 30; //da dove parte il campo di gioco sulla sinistra
int gioco_destra; //dove finisce il campo di gioco a destra
int larghezza_gioco;   //quanti caratteri "larghi" è il campo orizzontalmente

//prato inferiore (alto 6)
int prato_altezza = 6; //il  prato in fondo allo schermo è alto 6 righe
int riga_inizio_prato;
int riga_fine_prato;

//secondo prato dopo le tane (anche lui alto 6)
int prato2_altezza = 6;
int prato2_inizio_riga;
int prato2_fine_riga;

//parametri delle tane
int tana_altezza = 5; //le tane sono alte 5 righe
int tana_inizio_riga;
int tana_fine_riga;
int num_tane = 5; //quante tane voglio
int spazio = 3;  //distanza orizzontale tra ogni tana
int spazio_totale;  //ci calcolo lo spazio totale occupato
int totale_per_le_tane; //quanta larghezza ho a disposizione per le tane
int larghezza_tana; //larghezza della singola tana
int offset_tane;    //offset iniziale orizzontale, se non si dividono perfettamente
//mi serve per allineare bene le tane evitando che l’ultima risulti tagliata o vada a finire fuori dallo spazio di gioco

//variabili di gioco
int vite;  //numero di vite del giocatore
int tempo_rimasto;  //quanto tempo mi rimane (conta alla rovescia)
int punteggio;  //punteggio (se lo voglio usare in futuro)
bool pausa;    //flag per vedere se sono in pausa
bool tane_aperte[5]; //array bool per sapere se la tana i è aperta o no
const char *nickname; //memorizzo il nome utente

//lo sprite è 2x3 in ascii
int rana_altezza = 2; //altezza rana
int rana_larghezza = 3; //larghezza rana
int rana_x; //posizione x della rana
int rana_y; //osizione y della rana
//sprite ascii della rana
const char *prima_linea_sprite = " O ";
const char *seconda_linea_sprite = "/|\\";

//pipe tra padre e figlio
int canale_a_figlio[2];
int canale_a_padre[2];

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

//disegno tutt,le tane gialle, i buchi neri, e i due prati
void disegna_scenario() {
    //coloro in giallo le righe da tana_inizio_riga a tana_fine_riga
    attron(COLOR_PAIR(8));
    for(int r = tana_inizio_riga; r <= tana_fine_riga; r++) {
        mvhline(r, gioco_sinistra, ' ', larghezza_gioco);
    }
    attroff(COLOR_PAIR(8));

    //se la tana i è aperta, ci disegno un buco nero
    int riga_inizio_buco = tana_inizio_riga + 1; //parto un po' più sotto per non coprire
    int riga_fine_buco   = tana_inizio_riga + 4; //altezza del buco
    for(int i = 0; i < num_tane; i++) {
        if(tane_aperte[i]) {
            //calcolo l'inizio della tana in x
            int inizio_tana_x = gioco_sinistra + offset_tane + i * (larghezza_tana + spazio);
            //fine a sinistra + offset + i*(larghezza + spazio)

            int inizio_buco_x = inizio_tana_x + 1;
            int larghezza_buco = larghezza_tana - 1;
            int fine_buco_x = inizio_buco_x + larghezza_buco - 1;

            //e lo coloro di nero
            attron(COLOR_PAIR(9));
            for(int r = riga_inizio_buco; r <= riga_fine_buco; r++) {
                for(int c = 0; c < larghezza_buco; c++) {
                    mvaddch(r, inizio_buco_x + c, ' ');
                }
            }
            attroff(COLOR_PAIR(9));
        }
    }

    //disegno il prato2
    attron(COLOR_PAIR(1));
    for(int r = prato2_inizio_riga; r <= prato2_fine_riga; r++) {
        mvhline(r, gioco_sinistra, ' ', larghezza_gioco);
    }
    attroff(COLOR_PAIR(1));

    //disegno il prato inferiore
    attron(COLOR_PAIR(1));
    for(int r = riga_inizio_prato; r <= riga_fine_prato; r++) {
        mvhline(r, gioco_sinistra, ' ', larghezza_gioco);
    }
    attroff(COLOR_PAIR(1));
}

//disegno sprite
void disegna_sprite() {
    mvprintw(rana_y,   rana_x, "%s", prima_linea_sprite);
    mvprintw(rana_y+1, rana_x, "%s", seconda_linea_sprite);
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

//qui controllo le collisioni,, se la rana entra nell'area della tana e questa è aperta, +vita, se chiusa o colonna gialla, -vita
bool check_tane() {
    int riga_inizio_buco = tana_inizio_riga + 1; //il buco comincia uno sotto
    int riga_fine_buco   = tana_inizio_riga + 4;//finisce un po' prima della fine gialla

    // se la rana non tocca neanche la fascia verticale della tana, esco
    if(rana_y + rana_altezza - 1 < tana_inizio_riga || rana_y > tana_fine_riga) {
        return false;
    }

    for(int i = 0; i < num_tane; i++) {
        //calcolo l'inizio x della tana i
        int inizio_tana_x = gioco_sinistra + offset_tane + i * (larghezza_tana + spazio);
        int fine_tana_x = inizio_tana_x + larghezza_tana - 1;
        int inizio_buco_x = inizio_tana_x + 1;
        int larghezza_buco = larghezza_tana - 1;
        int fine_buco_x = inizio_buco_x + larghezza_buco - 1;

        //controllo orizzontalmente
        bool sovrapposizione_tana =
                (rana_x + rana_larghezza - 1 >= inizio_tana_x) &&
                (rana_x <= fine_tana_x);

        //controllo verticalmente
        bool sovrapposizione_vert_tana =
                (rana_y + rana_altezza - 1 >= tana_inizio_riga) &&
                (rana_y <= tana_fine_riga);

        if(sovrapposizione_tana && sovrapposizione_vert_tana) {
            //controlliamo se stiamo nel buco (in mezzo) oppure sui pilastri gialli
            bool sovrapposizione_buco =
                    (rana_x + rana_larghezza - 1 >= inizio_buco_x) &&
                    (rana_x <= fine_buco_x) &&
                    (rana_y + rana_altezza - 1 >= riga_inizio_buco) &&
                    (rana_y <= riga_fine_buco);

            if(sovrapposizione_buco) {
                //tana aperta => +1 vita, chiusa => -1 vita
                if(tane_aperte[i]) {
                    if(vite < 8) vite++;
                    tempo_rimasto = TEMPO_MASSIMO;
                    tane_aperte[i] = false;
                } else {
                    vite--;
                    tempo_rimasto = TEMPO_MASSIMO;
                }
            } else {
                //se tocc i pilastri gialli => -1 vita
                vite--;
                tempo_rimasto = TEMPO_MASSIMO;
            }

            //mando un comando 'O' per dire "reset" al figlio
            char comando = 'O';
            write(canale_a_figlio[1], &comando, 1);

            //leggo i nuovi x e y del figlio (che avrà resettato la rana)
            int r_x, r_y;
            read(canale_a_padre[0], &r_x, sizeof(int));
            read(canale_a_padre[0], &r_y, sizeof(int));
            rana_x = r_x;
            rana_y = r_y;

            return true;
        }
    }
    return false;
}

//se il tempo arriva a 0, perdo vita e resetto
void timer_scaduto() {
    vite--;
    tempo_rimasto = TEMPO_MASSIMO;

    char comando = 'O';
    write(canale_a_figlio[1], &comando, 1);

    int r_x, r_y;
    read(canale_a_padre[0], &r_x, sizeof(int));
    read(canale_a_padre[0], &r_y, sizeof(int));
    rana_x = r_x;
    rana_y = r_y;
}

//se tutte le tane sono chiuse => vittoria
bool tutte_tane_chiuse() {
    for(int i = 0; i < num_tane; i++) {
        if(tane_aperte[i]) return false;
    }
    return true;
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
