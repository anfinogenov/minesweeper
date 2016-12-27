#include <iostream>
#include <fstream>
#include <ctime>
#include <ncurses.h>

void log_open (void);

std::ofstream file_log;

int main(int argc, char *argv[])
{
    //initscr();
    //endwin();
    log_open();
    file_log << "Hello World!" << std::endl;
    return 0;
}

void log_open (void) {
    char* log_name = new char[30];
    sprintf(log_name, "log-%lu.txt", time(NULL));
    std::ofstream file_log(log_name);
}
