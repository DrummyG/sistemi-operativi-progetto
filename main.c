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

#define NOME 50
#define TEMPO_MAX 90

int max_y, max_x; //dimensione schermo
int prato_altezza = 5; //altezza prato inf
int riga_inizio_prato, riga_fine_prato; //coordinate prato inf
int tana_altezza = 5; //altezza spazio tane
int tana_inizio_riga, tana_fine_riga; //coordinate spazio tane
int num_tane = 5;
int spazio = 3; //spazio tra le tane
int spazio_totale;
int totale_per_le_tane; //spazio complessivo in larghezza che serve per le 5 tane
int larghezza_tana; //larfghezza ogni singola tana

int vite;
int tempo_rimasto; //temp o rimanente round
int punteggio; //punt giocatore
bool pausa; //pausa gioco
bool tane_aperte[5]; //per avere un resoconto su quale tana aperta o chiusa

const char *nickname;

// Sprite 2x3
int sprite_altezza = 2;
int sprite_larghezza = 3;
int sprite_x, sprite_y; //posizione attuale rana
const char *prima_linea_sprite = " O ";
const char *seconda_linea_sprite = "/|\\";

//file descriptor, quindi pipe per comunicazione tra padre figlio
int fd_padre_a_figlio[2];
int fd_figlio_a_padre[2];

//disegna scenario serve per disegnare le tane, e il prato inf verde
void disegna_scenario() {
    //tane gialle
    attron(COLOR_PAIR(8));
    for (int r = tana_inizio_riga; r <= tana_fine_riga; r++) {
        mvhline(r, 0, ' ', max_x);
    }
    attroff(COLOR_PAIR(8));

    //per aprire l'infresso delle tane
    int riga_inizio_buco = tana_inizio_riga + 1;
    int riga_fine_buco = tana_inizio_riga + 4;
    for (int i = 0; i < num_tane; i++) {
        if (tane_aperte[i]) {
            int inizio_tana_x = i * (larghezza_tana + spazio);
            attron(COLOR_PAIR(9));
            for (int r = riga_inizio_buco; r <= riga_fine_buco; r++) {
                for (int c = 0; c < larghezza_tana; c++) {
                    mvaddch(r, inizio_tana_x + c, ' '); //per appunto fare il l'ingresso 'nero'
                }
            }
            attroff(COLOR_PAIR(9));
        }
        // se la tana viene attraversata, resta gialla
    }

    //coloro prato inf verde
    attron(COLOR_PAIR(1));
    for (int r = riga_inizio_prato; r <= riga_fine_prato; r++) {
        mvhline(r, 0, ' ', max_x);
    }
    attroff(COLOR_PAIR(1));
}

//stampa a schemro di vite, punteggio timer e nickname
void disegna_info() {
    attron(COLOR_PAIR(2));
    mvprintw(0, max_x - 15, "Vite: %d", vite);
    attroff(COLOR_PAIR(2));

    attron(COLOR_PAIR(2));
    mvprintw(max_y - 1, 0, "Username: %s", nickname);
    attroff(COLOR_PAIR(2));

    attron(COLOR_PAIR(2));
    mvprintw(max_y - 1, 30, "Punteggio: %d", punteggio);
    attroff(COLOR_PAIR(2));

    int barra_lunghezza = 50 * tempo_rimasto / TEMPO_MAX;
    attron(COLOR_PAIR(4));
    mvprintw(max_y - 1, max_x - 60, "Timer: ");
    for (int i = 0; i < barra_lunghezza; i++) {
        mvprintw(max_y - 1, max_x - 52 + i, "|");
    }
    attroff(COLOR_PAIR(4));
}

//disegna il personaggio cone stringa 1 e stringa 2 definite prima
void disegna_sprite() {
    mvprintw(sprite_y, sprite_x, "%s", prima_linea_sprite);
    mvprintw(sprite_y+1, sprite_x, "%s", seconda_linea_sprite);
}

void chiudi_tana(int i) {
    tane_aperte[i] = false;
}

// controllo se la rana entra in una tana aperta o sbatte in una tana chiusa
bool check_tane() {
    int riga_inizio_buco = tana_inizio_riga + 1; // 2
    int riga_fine_buco = tana_inizio_riga + 4;   // 5

    if (sprite_y + sprite_altezza - 1 < riga_inizio_buco || sprite_y > riga_fine_buco) {
        return false;
    }

    for (int i = 0; i < num_tane; i++) {
        int inizio_tana_x = i * (larghezza_tana + spazio);
        int fine_tana_x = inizio_tana_x + larghezza_tana - 1;

        if (sprite_x + sprite_larghezza - 1 >= inizio_tana_x && sprite_x <= fine_tana_x) {
            //nel caso in cui la rana si trova nello spazio delle tane
            if (tane_aperte[i]) {
                //se la tana è aperta si può attraversare
                if (vite < 8) vite++;
                tempo_rimasto = TEMPO_MAX;
                chiudi_tana(i);

                //reset dello sprite
                char cmd = 'O';
                write(fd_padre_a_figlio[1], &cmd, 1);
                int rx, ry;
                read(fd_figlio_a_padre[0], &rx, sizeof(int));
                read(fd_figlio_a_padre[0], &ry, sizeof(int));
                sprite_x = rx;
                sprite_y = ry;

            } else {
                // tana chiusa, quindi collisione e quindi torna indietro allo spawn
                vite--;
                tempo_rimasto = TEMPO_MAX;
                char cmd = 'O';
                write(fd_padre_a_figlio[1], &cmd, 1);
                int rx, ry;
                read(fd_figlio_a_padre[0], &rx, sizeof(int));
                read(fd_figlio_a_padre[0], &ry, sizeof(int));
                sprite_x = rx;
                sprite_y = ry;
            }

            return true;
        }
    }
    return false;
}

void timer_scaduto() {
    vite--;
    tempo_rimasto = TEMPO_MAX;

    char cmd = 'O';
    write(fd_padre_a_figlio[1], &cmd, 1);
    int rx, ry;
    read(fd_figlio_a_padre[0], &rx, sizeof(int));
    read(fd_figlio_a_padre[0], &ry, sizeof(int));
    sprite_x = rx;
    sprite_y = ry;
}

//controllo per vedere se le tane son tutte chiuse per far terminare la partita
bool tutte_tane_chiuse() {
    for (int i = 0; i < num_tane; i++) {
        if (tane_aperte[i]) return false;
    }
    return true;
}

//riceve comando dal padre, sposta la rana e restituisce la posizione aggiorntaa
void processo_sprite(int bottom_y, int bottom_x, int w, int h, int maxx, int maxy) {
    int s_x = bottom_x;
    int s_y = bottom_y;

    for (;;) {
        char cmd;
        int n = read(fd_padre_a_figlio[0], &cmd, 1);
        if (n > 0) {
            if (cmd == 'U' && s_y > 0) s_y--;
            else if (cmd == 'D' && s_y < maxy - h) s_y++;
            else if (cmd == 'L' && s_x > 0) s_x--;
            else if (cmd == 'R' && s_x < maxx - w) s_x++;
            else if (cmd == 'O') {
                s_y = bottom_y;
                s_x = bottom_x;
            }

            write(fd_figlio_a_padre[1], &s_x, sizeof(int));
            write(fd_figlio_a_padre[1], &s_y, sizeof(int));
        }
    }
}

void menu();

void inizio(const char *username) {
    clear();
    curs_set(0);
    getmaxyx(stdscr, max_y, max_x); //prendo dimensioni max terminale

    punteggio = 0; //punteggio parte da zero
    vite = 5; //vite 5
    tempo_rimasto = TEMPO_MAX; //timer
    pausa = false; //false perchè ovviamente non iniizia in pausa il gioco
    nickname = username;

    //coordinate prato inferior
    riga_inizio_prato = (max_y - 2) - (prato_altezza - 1);
    riga_fine_prato = max_y - 2;

    //coordinate spazio tane
    tana_inizio_riga = 1;
    tana_fine_riga = tana_inizio_riga + tana_altezza - 1;

    //calcolo per avere larghezza tane e spazio tra tane
    spazio_totale = (num_tane - 1) * spazio; //spazio per i buchi
    totale_per_le_tane = max_x - spazio_totale; //spoazio totale per tutte le tane
    larghezza_tana = totale_per_le_tane / num_tane; //larghezza effettiva di ogni tana

    //cosi permetto l'apertura di tutte le tane all'inizio del gioc
    for (int i = 0; i < num_tane; i++) {
        tane_aperte[i] = true;
    }

    //pipe tra padre figlio
    if (pipe(fd_padre_a_figlio) == -1 || pipe(fd_figlio_a_padre) == -1) {
        endwin(); //chiudo ncurses se esplode qualcosa
        perror("pipe");
        exit(1);
    }

    //per processo figlio
    pid_t pid = fork();
    if (pid == -1) { //-1 nel senso se la fork fallisce
        endwin();
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // processo figlio, movimento rana
        //chiudo inoltre le due estremità della pipe, sia scrittura che lettura
        close(fd_padre_a_figlio[1]);
        close(fd_figlio_a_padre[0]);
        //posiziono rana al centro del prato inferiore
        int bottom_y = riga_fine_prato - sprite_altezza + 1;
        int bottom_x = (max_x - sprite_larghezza) / 2;
        //gestione rana
        processo_sprite(bottom_y, bottom_x, sprite_larghezza, sprite_altezza, max_x, max_y);
        exit(0); //finisce qui il processo se la funzione fallisce
    }

    // padre, quindi gioco principale
    //anche qui chiudo le estremità della pipe, sia scrittura che lettura
    close(fd_padre_a_figlio[0]);
    close(fd_figlio_a_padre[1]);

    //resetto la posizione della rana
    {
        char cmd = 'O';
        write(fd_padre_a_figlio[1], &cmd, 1); //comunico il cmd al figlio
        int rx, ry;
        //prendo nuova x e y
        read(fd_figlio_a_padre[0], &rx, sizeof(int));
        read(fd_figlio_a_padre[0], &ry, sizeof(int));
        sprite_x = rx;
        sprite_y = ry;
    }

    nodelay(stdscr, TRUE); //input tastiera non bloccante

    //gestione timer
    struct timeval start, attuale;
    gettimeofday(&start, NULL);
    long ultimo_secondo = 0;

    bool fine_gioco = false; // per capire quando uscire dal loop principale

    while (!fine_gioco) {
        int ch = getch();
        if (ch == 'p' || ch == 'P') {
            pausa = !pausa; //metto o tolgo la pausa
        } else if (!pausa && ch != ERR) { //se non è in pausa e ricevo input
            char cmd = 0;
            if (ch == KEY_UP) cmd='U'; //su
            else if (ch == KEY_DOWN) cmd='D'; //giù
            else if (ch == KEY_LEFT) cmd='L'; //sx
            else if (ch == KEY_RIGHT) cmd='R'; //dx

            if (cmd != 0) {
                write(fd_padre_a_figlio[1], &cmd, 1); //comunico al figlio
                int rx, ry;
                //prendo nuovamente x e y coordinate
                read(fd_figlio_a_padre[0], &rx, sizeof(int));
                read(fd_figlio_a_padre[0], &ry, sizeof(int));
                sprite_x = rx;
                sprite_y = ry;
            }
        }

        check_tane(); //check se la rana ha attraversato una tana

        // se tutte le tane sono chiuse, mostro il messaggio di vittoria
        if (tutte_tane_chiuse()) {
            clear();
            attron(COLOR_PAIR(1) | A_BOLD);
            mvprintw(max_y / 2 - 1, (max_x - (int)(strlen(nickname) + 25)) / 2,
                     "COMPLIMENTI %s HAI VINTO", nickname);
            mvprintw(max_y / 2 + 1, (max_x - 45) / 2,
                     "(Premi invio per tornare al menu principale)");
            attroff(COLOR_PAIR(1) | A_BOLD);
            refresh();

            //l'ho messo cosi aspetto input dopo messaggio vittoria per tornare al menu
            nodelay(stdscr, FALSE);
            int c;
            do {
                c = getch();
            } while (c != 10 && c != KEY_ENTER);

            //  premuto invio  si torna al menu principale
            fine_gioco = true;
            break;
        }

        clear();
        disegna_scenario(); //prato, tane e contorno
        disegna_sprite(); //rana
        disegna_info(); //vite, punti, timer e nickname

        if (pausa) { //se si mette pausa, mostro in rosso messaggio di pausa
            attron(A_BOLD | COLOR_PAIR(3));
            mvprintw(max_y / 2, (max_x - 15) / 2, "[ GIOCO IN PAUSA ]");
            attroff(A_BOLD | COLOR_PAIR(3));
        }

        refresh();

        gettimeofday(&attuale, NULL);
        long secondi_passati = attuale.tv_sec - start.tv_sec; //calcolo secondoi trascorsi
        if (!pausa && secondi_passati > ultimo_secondo) {
            ultimo_secondo = secondi_passati;
            tempo_rimasto--; //decremento timer
            if (tempo_rimasto < 0) { //se timer scaduto
                timer_scaduto();
                if (vite == 0) { //se si finiscono le vite, game over
                    clear();
                    attron(COLOR_PAIR(3));
                    mvprintw(max_y / 2, (max_x - 10) / 2, "GAME OVER");
                    attroff(COLOR_PAIR(3));
                    refresh();
                    // Attendi invio per tornare a menu?
                    nodelay(stdscr, FALSE);
                    int c;
                    do {
                        c = getch();
                    } while (c != 10 && c != KEY_ENTER);
                    fine_gioco = true;
                    break;
                }
            }
        }

        if (vite == 0) {
            clear();
            attron(COLOR_PAIR(3));
            mvprintw(max_y / 2, (max_x - 10) / 2, "GAME OVER");
            attroff(COLOR_PAIR(3));
            refresh();
            //invio per tornare al menu
            nodelay(stdscr, FALSE);
            int c;
            do {
                c = getch();
            } while (c != 10 && c != KEY_ENTER);
            fine_gioco = true;
            break;
        }
    }

    //termino figlio e pulisco
    kill(pid, SIGKILL);
    waitpid(pid, NULL, 0);
}

void menu() {
    const char *opzioni[] = {"Inizia a giocare", "Esci"}; //array con due opzioni all'inizio
    int scelta = 0; //per tner traccia della scelta fatta nel menu
    int key; //uso key per memorizzare il tasto premuto
    char username[NOME];
    int num_opzioni; //numero totale delle opzioni che ho messo nel menu

    getmaxyx(stdscr, max_y, max_x); //dimensioni schermo
    num_opzioni = sizeof(opzioni) / sizeof(opzioni[0]);

    nodelay(stdscr, FALSE); //aspetto un tasto

    while (1) {
        clear();
        //titolo al centro
        attron(COLOR_PAIR(2));
        mvprintw(max_y / 2 - 3, (max_x - 33) / 2, "#################################");
        mvprintw(max_y / 2 - 2, (max_x - 33) / 2, "#       Frogger Resurrection    #");
        mvprintw(max_y / 2 - 1, (max_x - 33) / 2, "#################################");
        attroff(COLOR_PAIR(2));

        //stampo le opzioni del menu e si evidenziano quando ci si va sopra
        for (int i = 0; i < num_opzioni; i++) {
            if (i == scelta) { //se questa è la scelta selezionata
                attron(A_REVERSE | COLOR_PAIR(2)); //la evidenzio e inverso
            }
            mvprintw(max_y / 2 + i + 1, (max_x - (int)strlen(opzioni[i])) / 2, "%s", opzioni[i]);
            if (i == scelta) { //disattivo l'evidenziazione
                attroff(A_REVERSE | COLOR_PAIR(2));
            }
        }

        key = getch(); // aspett oinput, o freccia per spostarsi nelle opzioni, o invio per confermare

        //c'è solo su e giù perche le opzioni nel menu le ho messe in verticale, sx e dx non serve
        switch (key) {
            case KEY_UP:
                scelta = (scelta - 1 + num_opzioni) % num_opzioni;
                break;
            case KEY_DOWN:
                scelta = (scelta + 1) % num_opzioni;
                break;
            case 10: // Invio
                if (scelta == 0) {
                    clear();
                    attron(A_BOLD); //grassetto
                    mvprintw(max_y / 2 - 1, (max_x - 25) / 2, "INSERISCI IL TUO USERNAME:");
                    attroff(A_BOLD);

                    echo(); //input visibile
                    curs_set(1);
                    int start_col = (max_x - NOME) / 2;
                    mvprintw(max_y / 2 + 1, start_col, "");
                    mvgetnstr(max_y / 2 + 1, start_col, username, NOME - 1);
                    noecho();
                    curs_set(0);
                    username[NOME - 1] = '\0'; //per ssicuramri il nickname finisca col terminatore

                    //benvenuto messaggio
                    clear();
                    attron(A_BOLD);
                    mvprintw(max_y / 2 - 2, (max_x - (int)strlen(username) - 20) / 2, "BENVENUTO, %s!", username);
                    mvprintw(max_y / 2, (max_x - 28) / 2, "IL GIOCO STA PER INIZIARE...");
                    attroff(A_BOLD);

                    refresh(); //aggiorno lo schermo
                    nodelay(stdscr, TRUE);
                    nickname = username;
                    inizio(username); //avvio gioco
                } else if (scelta == 1) { //1 è l'opzione esci
                    clear();
                    mvprintw(3, 5, "Grazie per aver giocato!");
                    getch(); //aspetto un tasto per chiudere
                    return;
                }
                break;
            default:
                break;
        }
    }
}

int main() {
    initscr();
    start_color();
    keypad(stdscr, TRUE);

    init_pair(1, COLOR_BLACK, COLOR_GREEN);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);
    init_pair(5, COLOR_WHITE, COLOR_BLACK);
    init_pair(8, COLOR_BLACK, COLOR_YELLOW);
    init_pair(9, COLOR_BLACK, COLOR_BLACK);

    noecho();
    cbreak();
    curs_set(0);

    menu();
    endwin();
    return 0;
}
