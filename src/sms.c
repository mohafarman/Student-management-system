#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "database.h"
#include "ncursor.h"
#include "sms.h"

int main() {
	// Connect to database
	db_connect();

	// Initialize ncursor
	init_ncurses();

	// User message to exit program
	mvwprintw(stdscr, 0, 0, "Press F1 to exit.");
	refresh();
	
	// Initialize the program with the main menu
	main_menu();

	sqlite3_close(db);
	endwin();
}
