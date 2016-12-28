#include <iostream>
#include <fstream>
#include <ctime>
#include <cstring>
#include <thread>
#include <cstdint>
#include <ncurses.h>

const uint64_t startTime = time(nullptr);
const uint8_t width = 20;
const uint8_t height = 10;
const uint8_t mines = 10; //1 mine in 6 cells
bool exitFlag = false;
chtype secret_field[width][height] = {0};
chtype open_field[width][height] = {0};

int generate_secret(void);
void open_cell (const int x, const int y);
void log_open (void);
void screen_init_s (void);
void log_out (const std::string & msg, char msg_type);
void exit_s (const std::string & msg, char msg_type);

struct cursor {
    int16_t w;
    int16_t h;
};

std::ofstream file_log;

int main(/*int argc, char *argv[]*/) {
    srand((time(nullptr) - clock())*21920);
    log_open();
    screen_init_s();
    int total_mines = generate_secret();

    cursor pointer{0,0};

    int key;
    while (!exitFlag) {
        int detected_mines = 0;
        for (uint8_t i = 0; i < width; i++)
            for (uint8_t j = 0; j < height; j++) {
                mvaddch(j, i, open_field[i][j]);
                if (open_field[i][j] == 'm') detected_mines++;
            }
        if (detected_mines == total_mines) {
            exitFlag = true;
            for (uint8_t i = 0; i < width; i++)
                for (uint8_t j = 0; j < height; j++)
                    if (open_field[i][j] == 'm')
                        exitFlag &= secret_field[i][j] == 'x';
            if (exitFlag) {
                mvprintw(height/2, width/2-5, " you win! ");
                refresh();
                napms(5000);
                exit_s("win", 'n');
            }
        }
        mvprintw(height, 0, "total mines: %d   detected mines: %d  ", total_mines, detected_mines);
        move(pointer.h, pointer.w);

        key = tolower(getch());
        if (tolower(key) == 'q') exitFlag = true;
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
                open_field[pointer.w][pointer.h] = 'm';
                break;
            default: break;
        }
        move(pointer.h, pointer.w);
        refresh();
        flushinp();
    }
    refresh();
    endwin();
    exit_s("returning 0", 'n');
    return 0;
}

int generate_secret (void) {
    int total_mines = 0;
    for (uint8_t i = 0; i < width; i++)
        for (uint8_t j = 0; j < height; j++)
            open_field[i][j] = '_';

    for (uint8_t i = 0; i < width; i++)
        for (uint8_t j = 0; j < height; j++) {
            if(!(rand()%mines)) {
                secret_field[i][j] = 'x';
                total_mines++;
            } else secret_field[i][j] = ' ';
        }

    for (uint8_t i = 0; i < width; i++) {
        for (uint8_t j = 0; j < height; j++) {
            if (secret_field[i][j] == 'x') continue;
            int amount = 0;

            if (i != 0 && j != 0 && secret_field[i-1][j-1] == 'x') amount++;
            if (j != 0 && secret_field[i][j-1] == 'x') amount++;
            if (i != width-1 && j != 0 && secret_field[i+1][j-1] == 'x') amount++;

            if (i != 0 && secret_field[i-1][j] == 'x') amount++;
            if (i != width-1 && secret_field[i+1][j] == 'x') amount++;

            if (i != 0 && j != height-1 && secret_field[i-1][j+1] == 'x') amount++;
            if (j != height-1 && secret_field[i][j+1] == 'x') amount++;
            if (i != width-1 && j != height-1 && secret_field[i+1][j+1] == 'x') amount++;


            secret_field[i][j] = amount + '0';
        }
    }
    return total_mines;
}

void open_cell (const int x, const int y) {
    if (open_field[x][y] == 'm') {
        open_field[x][y] = ' ';
        return;
    }
    if (secret_field[x][y] == '0') {
        open_field[x][y] = ' ';
        for (int i = -1; i < 2; i++)
            for (int j = -1; j < 2; j++)
                if (x+i >= 0 && x+i < width)
                    if (y+j >= 0 && y+j < height)
                        if (x+i != x || y+j != y) {
                            secret_field[x][y] = ' ';
                            open_cell(x+i, y+j);
                        }
    } else if (secret_field[x][y] == 'x') {
        mvprintw(height/2, width/2-11, " you\'ve opened a mine! ");
        refresh();
        napms(5000);
        exit_s("mine", 'n');
    } else {
        open_field[x][y] = secret_field[x][y];
    }
    return;
}

void log_open (void) {
    char* log_name = new char[30];
    sprintf(log_name, "log-%lu.txt", time(nullptr));
    file_log.open(log_name);
    log_out("", 's');
    delete[] log_name;
}

void log_out (const std::string & msg, char msgType) {
    static char msg_type[5];
    switch (msgType) { //converting char msgType to string
        case 'n':
            strcpy(msg_type, "okay");
            break;
        case 'w':
            strcpy(msg_type, "warn");
            break;
        case 'e':
            strcpy(msg_type, "err ");
            break;
        case 'q':
            file_log << "-- END OF LOG --" << std::endl;
            return;
        case 's':
            file_log << "-- START OF LOG --" << std::endl;
            return;
        default:
            strcpy(msg_type, "unkn");
            break;
    }
    file_log.width(0);
    file_log << msg_type << " [";
    file_log.width(4);
    file_log.fill('0');
    file_log << (time(nullptr) - startTime);
    file_log.width(0);
    file_log << "]: " << msg << std::endl;
    //out message with msgType and time from start
}

void screen_init_s (void) {
    if(!initscr()) exit_s((char*)"initscr() failed", 'e');
    log_out("initscr executed normally", 'n');
    noecho();
    curs_set(1);
    keypad(stdscr, 1);
}

void exit_s (const std::string & msg, char msg_type) {
    endwin();
    log_out(msg, msg_type);
    log_out("", 'q');
    file_log.close();
    //printf("\x1b[0m"); disables color
    exit(1);
}
