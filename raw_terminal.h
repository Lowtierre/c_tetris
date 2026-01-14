/**
 * terminal_compat.h
 * 
 * Funzioni di compatibilità cross-platform per la gestione del terminale.
 * Supporta Windows, Linux e macOS.
 * 
 * Uso: #include "terminal_compat.h" (o con path relativo "../terminal_compat.h")
 */

#ifndef TERMINAL_COMPAT_H
#define TERMINAL_COMPAT_H

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    /* ========== Windows ========== */
    #include <conio.h>
    #include <windows.h>
    
    #define sleep_ms(ms) Sleep(ms)
    #define clear_screen() system("cls")
    
    /* _kbhit() e _getch() sono già disponibili in <conio.h> */

#else
    /* ========== Linux / macOS (POSIX) ========== */
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #include <sys/ioctl.h>
    
    #define sleep_ms(ms) usleep((ms) * 1000)
    #define clear_screen() system("clear")

    /**
     * Controlla se è stato premuto un tasto (non bloccante).
     * Equivalente a kbhit() di Windows.
     * @return 1 se c'è un tasto disponibile, 0 altrimenti
     */
    static inline int _kbhit(void) {
        struct termios oldt, newt;
        int ch;
        int oldf;
        
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        
        ch = getchar();
        
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        
        if (ch != EOF) {
            ungetc(ch, stdin);
            return 1;
        }
        return 0;
    }

    /**
     * Legge un carattere senza mostrarlo a schermo (non bloccante).
     * Equivalente a getch() di Windows.
     * @return Il carattere letto
     */
    static inline int _getch(void) {
        struct termios oldt, newt;
        int ch;
        
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        
        ch = getchar();
        
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }

#endif /* _WIN32 */

    /**
     * Restituisce il carattere in codifica univoca
     */
    static inline int manage_input(void) {
        int last_key = 0;
    
        while (_kbhit()) {
            int key = 0;
            int ch = _getch();
    
    #ifdef _WIN32
            if (ch == 0 || ch == 224) {
                // tasto esteso (frecce, ecc.) -> seconda lettura
                key = _getch(); // es: 75, 77, 80...
            } else {
                // caratteri normali: accetta solo quelli che ti interessano
                if (ch == 'a' || ch == 'd' || ch == 'o' || ch == 'p' || ch == 's')
                    key = ch;
                else
                    key = 0; // ignora altri caratteri
            }
    #else
            /**
             * Linux/macOS: frecce inviano ESC [ A/B/C/D
             * TODO: implementare
             * Per ora accettiamo solo wasd/op (singolo char).
             */
            if (ch == 'a' || ch == 'd' || ch == 'o' || ch == 'p' || ch == 's')
                key = ch;
            else
                key = 0;
    #endif
    
            if (key != 0) last_key = key; // tieni SOLO l'ultimo valido
        }
    
        return last_key;
    }

#endif /* TERMINAL_COMPAT_H */
