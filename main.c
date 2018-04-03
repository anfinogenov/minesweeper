#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <ncurses.h>

#define FIELD_FLAG 'F'
#define FIELD_MINE 'x'
#define FIELD_NOT_OPENED '.'
#define FIELD_SPACE ' '

enum Color { ONE='1', TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, FLAG=FIELD_FLAG, CLOSED=FIELD_NOT_OPENED, EMPTY=FIELD_SPACE};

const uint8_t width = 60; // TODO: parse settings from commandline
const uint8_t height = 15;
const uint8_t mines = 10; //1 mine in 6 cells
bool exitFlag = false;
chtype **secret_field = NULL;
chtype **open_field = NULL;

int generate_secret(void);
void open_cell (const int x, const int y);
void log_open (void);
void screen_init_s (void);
void exit_s (const char* msg, int exit_code);

typedef struct {
    int16_t w;
    int16_t h;
} Cursor;

int main(/*int argc, char *argv[]*/) {
    srand((time(NULL) - clock())*21920); // some kind of less-predicted random lol
    screen_init_s();
    int total_mines = generate_secret();

    Cursor pointer = {0,0};

    int key;
    while (!exitFlag) {
        int detected_mines = 0;
        for (uint8_t i = 0; i < width; i++)
            for (uint8_t j = 0; j < height; j++) {
                attron(COLOR_PAIR(open_field[i][j]));
                mvaddch(j, i, open_field[i][j]);
                attron(COLOR_PAIR(0));
                if (open_field[i][j] == FIELD_FLAG) detected_mines++;
            }
        if (detected_mines == total_mines) {
            exitFlag = true;
            for (uint8_t i = 0; i < width; i++)
                for (uint8_t j = 0; j < height; j++)
                    if (open_field[i][j] == FIELD_FLAG)
                        exitFlag &= secret_field[i][j] == FIELD_MINE; // if all m's contains mine
            if (exitFlag) {
                mvprintw(height/2, width/2-5, " you win! ");
                refresh();
                napms(5000);
                exit_s("Win!", 0);
            }
        }
        mvprintw(height, 0, "total mines: %d   detected mines: %d  ", total_mines, detected_mines);
        move(pointer.h, pointer.w);

        key = tolower(getch());
        switch (key) {
            case KEY_RIGHT:
            case 'd':
                pointer.w = (pointer.w+1)%width;
                break;
            case KEY_LEFT:
            case 'a':
                pointer.w = (pointer.w-1)%width;
                if (pointer.w < 0) pointer.w+=width;
                break;
            case KEY_UP:
            case 'w':
                pointer.h = (pointer.h-1)%height;
                if (pointer.h < 0) pointer.h+=height;
                break;
            case KEY_DOWN:
            case 's':
                pointer.h = (pointer.h+1)%height;
                break;
            case ' ':
                open_cell(pointer.w, pointer.h);
                break;
            case 'm':
            case 'f':
                open_field[pointer.w][pointer.h] = FIELD_FLAG;
                break;
            case 'q':
                exitFlag = true;
                break;
            default: break;
        }
        move(pointer.h, pointer.w);
        refresh();
        flushinp();
    }
    refresh();
    endwin();
    exit_s("Pressed Q", 0);
    return 0;
}

int amount_of_mines_around (int coordinate_x, int coordinate_y) {
    int amount = 0;

    /* ↓ y; → x
     * [1][2][3]
     * [4][ ][6]
     * [7][8][9]
     */

    if (coordinate_x != 0 &&
        coordinate_y != 0 &&
        secret_field[coordinate_x-1][coordinate_y-1] == 'x') amount++; // 1

    if (coordinate_y != 0 &&
        secret_field[coordinate_x][coordinate_y-1] == 'x') amount++; // 2

    if (coordinate_x != width-1 &&
        coordinate_y != 0 &&
        secret_field[coordinate_x+1][coordinate_y-1] == 'x') amount++; // 3

    if (coordinate_x != 0 &&
        secret_field[coordinate_x-1][coordinate_y] == 'x') amount++; // 4

    if (coordinate_x != width-1 &&
        secret_field[coordinate_x+1][coordinate_y] == 'x') amount++; // 6

    if (coordinate_x != 0 &&
        coordinate_y != height-1 &&
        secret_field[coordinate_x-1][coordinate_y+1] == 'x') amount++; // 7

    if (coordinate_y != height-1 &&
        secret_field[coordinate_x][coordinate_y+1] == 'x') amount++; // 8

    if (coordinate_x != width-1 &&
        coordinate_y != height-1 &&
        secret_field[coordinate_x+1][coordinate_y+1] == 'x') amount++; // 9

    return amount;
}

int generate_secret (void) {
    int total_mines = 0;
    for (uint8_t i = 0; i < width; i++)
        for (uint8_t j = 0; j < height; j++)
            open_field[i][j] = FIELD_NOT_OPENED;

    for (uint8_t i = 0; i < width; i++)
        for (uint8_t j = 0; j < height; j++) {
            if(!(rand()%mines)) {
                secret_field[i][j] = FIELD_MINE;
                total_mines++;
            } else secret_field[i][j] = FIELD_SPACE;
        }

    for (uint8_t i = 0; i < width; i++) {
        for (uint8_t j = 0; j < height; j++) {
            if (secret_field[i][j] == FIELD_MINE) continue;
            int amount = amount_of_mines_around(i, j);
            secret_field[i][j] = amount + '0';
        }
    }
    return total_mines;
}

void open_cell (const int x, const int y) {
    if (open_field[x][y] == FIELD_FLAG) {
        open_field[x][y] = FIELD_SPACE;
        return;
    }
    if (secret_field[x][y] == '0') {
        open_field[x][y] = FIELD_SPACE; // recursively open connected empty space
        for (int i = -1; i <= 1; i++)
            for (int j = -1; j <= 1; j++)
                if (x+i >= 0 && // if we are not behind the left border
                    x+i < width && // and not behind the right border
                    y+j >= 0 && // and not behind the upper edge
                    y+j < height && // and didn't broke our floor
                    (x+i != x || y+j != y)) // and if we aren't rotating on place, then:
                {
                    secret_field[x][y] = FIELD_SPACE;
                    open_cell(x+i, y+j);
                }
    } else if (secret_field[x][y] == FIELD_MINE) {
        mvprintw(height/2, width/2-11,   " you've opened a mine! ");
        mvprintw(height/2+2, width/2-11, "    Press Q to exit    ");
        refresh();
        while (tolower(getch()) != 'q');
        exit_s("Mine opened", 0);
    } else {
        open_field[x][y] = secret_field[x][y];
    }
    return;
}

void screen_init_s (void) {
    if(!initscr()) exit_s("initscr() failed", 1);

    if (!has_colors()) {
        exit_s("Your terminal does not support color", 2);
    }

    start_color();
    int bg = COLOR_WHITE;
    init_pair(ONE, COLOR_CYAN, bg);
    init_pair(TWO, COLOR_GREEN, bg);
    init_pair(THREE, COLOR_RED, bg);
    init_pair(FOUR, COLOR_BLUE, bg);
    init_pair(FIVE, COLOR_YELLOW, bg);
    init_pair(SIX, COLOR_RED, bg);
    init_pair(SEVEN, COLOR_MAGENTA, bg);
    init_pair(EIGHT, COLOR_BLACK, bg);

    init_pair(CLOSED, COLOR_WHITE, COLOR_BLACK);
    init_pair(EMPTY, COLOR_WHITE, COLOR_WHITE);
    init_pair(FLAG, COLOR_BLACK, COLOR_RED);

    open_field = (chtype**)calloc(sizeof(chtype*), width);
    secret_field = (chtype**)calloc(sizeof(chtype*), width);
    for (int i = 0; i < width; i++) {
        open_field[i] = (chtype*)calloc(sizeof(chtype), height);
        secret_field[i] = (chtype*)calloc(sizeof(chtype), height);
    }

    noecho();
    curs_set(1);
    keypad(stdscr, 1);
}

void exit_s (const char* msg, int exit_code) {
    for (int i = 0; i < width; i++) {
        if (open_field != NULL) free(open_field[i]);
        if (secret_field != NULL) free(secret_field[i]);
    }
    if (open_field != NULL) free(open_field);
    if (secret_field != NULL) free(secret_field);

    endwin();
    if (msg[0]) printf("%s\n", msg);

    //printf("\x1b[0m"); disables color
    exit(exit_code);
}
