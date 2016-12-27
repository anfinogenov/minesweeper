#include <iostream>
#include <fstream>
#include <ctime>
#include <cstring>
#include <thread>
#include <ncurses.h>

const long startTime = time(NULL);

void log_open (void);
void log_out (const std::string & msg, char msg_type);
void screen_init_s (void);
void exit_s (char* msg, char msg_type);

std::ofstream file_log;

int main(/*int argc, char *argv[]*/)
{
    srand(time(nullptr));
    log_open();
    screen_init_s();
    printw("this is ncurses screen");
    refresh();
    napms(2000);
    endwin();
    return 0;
}

void log_open (void) {
    char* log_name = new char[30];
    sprintf(log_name, "log-%lu.txt", time(NULL));
    file_log.open(log_name);
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
    file_log << msg_type << " [" << time(NULL) - startTime << "]: " << msg << std::endl;
    //out message with msgType and time from start
}

void screen_init_s (void) {
    if(!initscr()) exit_s((char*)"initscr() failed", 'e');
    log_out("initscr executed normally", 'n');
}

void exit_s (char* msg, char msg_type) {
    endwin();
    log_out(msg, msg_type);
    log_out("", 'q');
    file_log.close();
    //printf("\x1b[0m"); disables color
    exit(1);
}
