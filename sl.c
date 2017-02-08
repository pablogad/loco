/*========================================
 *    sl.c:
 *        Copyright 1993,1998 Toyoda Masashi
 *                (toyoda@is.titech.ac.jp)
 *        Last Modified: 1998/ 7/22
 *========================================
 */
/* sl version 3.03 : add usleep(20000)                                       */
/*                                              by Toyoda Masashi 1998/ 7/22 */
/* sl version 3.02 : D51 flies! Change options.                              */
/*                                              by Toyoda Masashi 1993/ 1/19 */
/* sl version 3.01 : Wheel turns smoother                                    */
/*                                              by Toyoda Masashi 1992/12/25 */
/* sl version 3.00 : Add d(D51) option                                       */
/*                                              by Toyoda Masashi 1992/12/24 */
/* sl version 2.02 : Bug fixed.(dust remains in screen)                      */
/*                                              by Toyoda Masashi 1992/12/17 */
/* sl version 2.01 : Smoke run and disappear.                                */
/*                   Change '-a' to accident option.                         */
/*                                              by Toyoda Masashi 1992/12/16 */
/* sl version 2.00 : Add a(all),l(long),F(Fly!) options.                     */
/*                                              by Toyoda Masashi 1992/12/15 */
/* sl version 1.02 : Add turning wheel.                                      */
/*                                              by Toyoda Masashi 1992/12/14 */
/* sl version 1.01 : Add more complex smoke.                                 */
/*                                              by Toyoda Masashi 1992/12/14 */
/* sl version 1.00 : SL runs vomitting out smoke.                            */
/*                                              by Toyoda Masashi 1992/12/11 */

#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sl.h"

int ACCIDENT  = 0;
int LOGO      = 0;
int TRACK     = 0;
int FLY       = 0;
int IGN_CTRLC = 0;
int VAGONES   = 0;  /* Number of cars to add */
char WORD[ 16+1 ];  /* Form word with cars */

int my_mvaddstr(int y, int x, char *str)
{
    for ( ; x < 0; ++x, ++str)
        if (*str == '\0')  return ERR;
    for ( ; *str != '\0'; ++str, ++x)
        if (mvaddch(y, x, *str) == ERR)  return ERR;
    return OK;
}

void option(char *str)
{
    while (*str != '\0') {
        switch (*str++) {
            case 'a': ACCIDENT = 1; break;
            case 'F': FLY      = 1; break;
            case 'l': LOGO     = 1; break;
            case 'T': TRACK    = 1; break;
            case 'e': IGN_CTRLC = 1; break;
            case 'v': VAGONES  = atoi(str); str += strlen(str); break;
            case 'w': strncpy(WORD,str,16); if(!VAGONES) VAGONES=strlen(WORD); str+=strlen(str); break;
            case 'h': printf ( "Opciones:\r\n" \
                               "    F: fly\r\n" \
                               "    l: toy\r\n" \
                               "    T: draw track\r\n" \
                               "    e: allow CTRL+C\r\n" \
                               "    vN: number of cars\r\n" \
                               "    wTXT: write text with cars\r\n" ); exit(0);

            default:                break;
        }
    }
}

int main(int argc, char *argv[])
{
    int x, i;
    int longitud_tren = D51LENGTH;
    *WORD = 0x00;

    for (i = 1; i < argc; ++i) {
        if (*argv[i] == '-') {
                option(argv[i] + 1);
        }
    }

    longitud_tren += VAGON1_LEN*VAGONES;

    initscr();
    if( !IGN_CTRLC )
       signal(SIGINT, SIG_IGN);
    noecho();
    leaveok(stdscr, TRUE);
    scrollok(stdscr, FALSE);

    /* TRACK no compatible con FLY: */
    if( TRACK && !FLY ) if( LOGO ) add_track( LINES/2+LOGOHIGHT/2 ); else add_track( LINES/2+D51HIGHT/2 );

    for (x = COLS - 1; ; --x) {
        if (LOGO == 0) {
            int cnt_vag = 0, err=OK;
            if (add_D51(x, longitud_tren) == ERR) break;
            while (cnt_vag != VAGONES && (err=add_vagon(x, cnt_vag++, longitud_tren)) != ERR );
            if( err ) break;
        } else {
            if (add_sl(x) == ERR) break;
        }
        refresh();
        usleep(20000);
    }
    mvcur(0, COLS - 1, LINES - 1, 0);
    endwin();
    return 0;
}


int add_sl(int x)
{
    static char *sl[LOGOPATTERNS][LOGOHIGHT + 1]
        = {{LOGO1, LOGO2, LOGO3, LOGO4, LWHL11, LWHL12, DELLN},
           {LOGO1, LOGO2, LOGO3, LOGO4, LWHL21, LWHL22, DELLN},
           {LOGO1, LOGO2, LOGO3, LOGO4, LWHL31, LWHL32, DELLN},
           {LOGO1, LOGO2, LOGO3, LOGO4, LWHL41, LWHL42, DELLN},
           {LOGO1, LOGO2, LOGO3, LOGO4, LWHL51, LWHL52, DELLN},
           {LOGO1, LOGO2, LOGO3, LOGO4, LWHL61, LWHL62, DELLN}};

    static char *coal[LOGOHIGHT + 1]
        = {LCOAL1, LCOAL2, LCOAL3, LCOAL4, LCOAL5, LCOAL6, DELLN};

    static char *car[LOGOHIGHT + 1]
        = {LCAR1, LCAR2, LCAR3, LCAR4, LCAR5, LCAR6, DELLN};

    int i, y, py1 = 0, py2 = 0, py3 = 0;
    int longi = LOGOLENGTH;
    if( VAGONES ) longi -= (2 - VAGONES)*21;

    if (x < - longi)  return ERR;
    y = LINES / 2 - 3;

    if (FLY == 1) {
            y = (x / 6) + LINES - (COLS / 6) - LOGOHIGHT;
            py1 = 2;  py2 = 4;  py3 = 6;
    }
    for (i = 0; i <= LOGOHIGHT; ++i) {
           if( !FLY && TRACK && i==LOGOHIGHT ) break;
           my_mvaddstr(y + i, x, sl[(LOGOLENGTH + x) / 3 % LOGOPATTERNS][i]);
           my_mvaddstr(y + i + py1, x + 21, coal[i]);
           if( VAGONES )
           {
               int cntvag=0, py1=2;
               while( cntvag != VAGONES )
               {   if( FLY ) py1 +=2;
                   my_mvaddstr(y + i + py1, x + 42 + 21*cntvag, car[i]), cntvag++;
               }
           }
           else
           {
               my_mvaddstr(y + i + py2, x + 42, car[i]);
               my_mvaddstr(y + i + py3, x + 63, car[i]);
           }
    }
    if (ACCIDENT == 1) {
        add_man(y + 1, x + 14);
            add_man(y + 1 + py2, x + 45);  add_man(y + 1 + py2, x + 53);
            add_man(y + 1 + py3, x + 66);  add_man(y + 1 + py3, x + 74);
    }
    add_smoke(y - 1, x + LOGOFUNNEL);

    return OK;
}


int add_D51(int x, int longitud_tren)
{
    static char *d51[D51PATTERNS][D51HIGHT + 1]
        = {{D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL11, D51WHL12, D51WHL13, D51DEL},
           {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL21, D51WHL22, D51WHL23, D51DEL},
           {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL31, D51WHL32, D51WHL33, D51DEL},
           {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL41, D51WHL42, D51WHL43, D51DEL},
           {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL51, D51WHL52, D51WHL53, D51DEL},
           {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL61, D51WHL62, D51WHL63, D51DEL}};
    static char *coal[D51HIGHT + 1]
        = {COAL01, COAL02, COAL03, COAL04, COAL05,
           COAL06, COAL07, COAL08, COAL09, COAL10, COALDEL};

    int y, i, dy = 0;

    if (x < -longitud_tren)  return ERR;
    y = LINES / 2 - 5;

    if (FLY == 1) {
        y = (x / 7) + LINES - (COLS / 7) - D51HIGHT;
        dy = 1;
    }
    if(x+D51LENGTH >= 0)
    {
        for (i = 0; i <= D51HIGHT; ++i) {
                my_mvaddstr(y + i, x, d51[(D51LENGTH + x) % D51PATTERNS][i]);
                my_mvaddstr(y + i + dy, x + 53, coal[i]);
            if( i+1==D51HIGHT && TRACK && !FLY ) break;   /* No machacar via */
        }
        if (ACCIDENT == 1) {
                add_man(y + 2, x + 43);
                add_man(y + 2, x + 47);
        }
    }
    add_smoke(y - 1, x + D51FUNNEL);
    return OK;
}

/* PGAD BEGIN */
void add_track(int y)
{
    int x=0;

    while( x!=COLS ) mvaddch(y, x++, '=') != ERR;
}

int add_vagon(int x, int num_vag, int longitud_tren)
{
    static char *vagon1[VAGON1_HGHT+1]
       = {VAGON11,VAGON12,VAGON13,VAGON14,VAGON15,VAGON16,VAGON17,VAGON18,VAGON19,VAGON110,VAGON1DEL};
    static char *vagon2[VAGON2_HGHT+1]
       = {VAGON21,VAGON22,VAGON23,VAGON24,VAGON25,VAGON26,VAGON27,VAGON28,VAGON29,VAGON210,VAGON2DEL};

    int y, i, dy = 0;
    int x_vagon=x+D51LENGTH+num_vag*VAGON1_LEN;

    if(x < -longitud_tren) return ERR;  /* Parar si ha desaparecido ultimo vagón de la pantalla */
    y = LINES / 2 - 5;

    if (FLY) {
            y = (x / 7) + LINES - (COLS / 7) - VAGON1_HGHT + 2 + num_vag;
            dy = 1;
    }

    if( x_vagon < COLS )
    for (i = 0; i <= VAGON1_HGHT; ++i) {
        if( i==VAGON1_HGHT && TRACK && !FLY ) break;  /* no machacar via */
        if( i==VAGON1_HGHT/2 && *WORD != 0x00 )
        {
            char tmp[VAGON1_LEN + 1];
            strcpy(tmp,vagon1[i]);
            *(tmp+VAGON1_LEN/2)=WORD[num_vag%strlen(WORD)];
            my_mvaddstr(y + i, x_vagon, tmp);
        }
        else
        {
            my_mvaddstr(y + i, x_vagon, vagon1[i]);
        }
    }

    return OK;
}
/* PGAD END */

void add_man(int y, int x)
{
    static char *man[2][2] = {{"", "(O)"}, {"Help!", "\\O/"}};
    int i;

    for (i = 0; i < 2; ++i) {
        my_mvaddstr(y + i, x, man[(LOGOLENGTH + x) / 12 % 2][i]);
    }
}


void add_smoke(int y, int x)
#define SMOKEPTNS        16
{
    static struct smokes {
        int y, x;
        int ptrn, kind;
    } S[1000];
    static int sum = 0;
    static char *Smoke[2][SMOKEPTNS]
        = {{"(   )", "(    )", "(    )", "(   )", "(  )",
            "(  )" , "( )"   , "( )"   , "()"   , "()"  ,
            "O"    , "O"     , "O"     , "O"    , "O"   ,
            " "                                          },
           {"(@@@)", "(@@@@)", "(@@@@)", "(@@@)", "(@@)",
            "(@@)" , "(@)"   , "(@)"   , "@@"   , "@@"  ,
            "@"    , "@"     , "@"     , "@"    , "@"   ,
            " "                                          }};
    static char *Eraser[SMOKEPTNS]
        =  {"     ", "      ", "      ", "     ", "    ",
            "    " , "   "   , "   "   , "  "   , "  "  ,
            " "    , " "     , " "     , " "    , " "   ,
            " "                                          };
    static int dy[SMOKEPTNS] = { 2,  1, 1, 1, 0, 0, 0, 0, 0, 0,
                                 0,  0, 0, 0, 0, 0             };
    static int dx[SMOKEPTNS] = {-2, -1, 0, 1, 1, 1, 1, 1, 2, 2,
                                 2,  2, 2, 3, 3, 3             };
    int i;

    if (x % 4 == 0) {
        for (i = 0; i < sum; ++i) {
            my_mvaddstr(S[i].y, S[i].x, Eraser[S[i].ptrn]);
            S[i].y    -= dy[S[i].ptrn];
            S[i].x    += dx[S[i].ptrn];
            S[i].ptrn += (S[i].ptrn < SMOKEPTNS - 1) ? 1 : 0;
            my_mvaddstr(S[i].y, S[i].x, Smoke[S[i].kind][S[i].ptrn]);
        }
        my_mvaddstr(y, x, Smoke[sum % 2][0]);
        S[sum].y = y;    S[sum].x = x;
        S[sum].ptrn = 0; S[sum].kind = sum % 2;
        sum ++;
    }
}
