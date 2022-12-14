/**
 *
 *	@file		ncursor.h
 *
 *	@author		Mohamad Farman
 *
 *	@date		23 September 2022
 *
 *	@brief		Handles all ncursor related functionality
 *
 */

#ifndef NCURSES_H
#define NCURSES_H

#include "database.h"
#include "sms.h"
#include <ncurses.h>
#include <menu.h>
#include <form.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define HEIGHT 40
#define WIDTH 80
#define STARTY_CENTER (LINES  / 2)
#define STARTX_CENTER (COLS  / 2)

#define MSG_ERROR 1
#define MSG_SUCCESS 2

struct sms_window {
	WINDOW *win;
	unsigned int width, height, starty, startx;
};

struct ncursor_menu {
	MENU *menu;
	ITEM **items;
};

//****************** PROTOTYPES ******************//

// Functionality
static void die(WINDOW *local_win);
static void init_ncurses();
static MENU *populate_menu(const char *local_menu_choices[], int num_choices);
static WINDOW *create_newwin(int height, int width, int start_y, int start_x);
static void redraw_window(WINDOW *local_win, int width, const char *string);
static void clear_window(WINDOW *local_win);
static void clear_menu(MENU *local_win, int num_options);

// Print
static void print_in_middle(WINDOW *local_win, int starty, int startx, const char *string, chtype color);

// Menus
static void main_menu();
static void user_menu();

// Windows
static void popup_msg_win(int status, const char *msg);
static void sign_in_win();
static void about_win();
static void admin_add_user_win(int height, int width, int y_offset, int x_offset);
static void admin_remove_user_win(int height, int width, int y_offset, int x_offset);
static void admin_edit_user_info_win(int height, int width, int y_offset, int x_offset);
static void admin_manage_users_win();
static void get_users(WINDOW *local_win, int pos_y, int pos_x, int width);

//****************** DEFINITIONS ******************//
static void init_ncurses() {
	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
}

static WINDOW *create_newwin(int height, int width, int start_y, int start_x) {
	WINDOW *local_win = NULL;
	keypad(local_win, TRUE);
	local_win = newwin(height, width, start_y, start_x);

	box(local_win, 0, 0);

	wrefresh(local_win);
	return local_win;
}

static MENU *populate_menu(const char *local_menu_choices[], int num_choices) {
	ITEM **local_menu_items;
	int i;

	local_menu_items = (ITEM **)calloc(num_choices + 1, sizeof(ITEM *));
	for (i = 0; i < num_choices; ++i) {
		local_menu_items[i] = new_item(local_menu_choices[i], "");
	}

	return new_menu(local_menu_items);
}

static void print_header(WINDOW *local_win, int width, const char *string) {
	int max_y, max_x, start_y = 2;
	init_pair(1, COLOR_CYAN, COLOR_BLACK);

	getmaxyx(local_win, max_y, max_x);

	wattron(local_win, A_REVERSE | COLOR_PAIR(1));
	mvwprintw(local_win, 1, ( max_x - strlen(string) )/2, "%s", string);
	wattroff(local_win, A_REVERSE | COLOR_PAIR(1));

	mvwaddch(local_win, 2, 0, ACS_LTEE);
	mvwhline(local_win, 2, 1, ACS_HLINE, width - 2);
	mvwaddch(local_win, 2, width - 1, ACS_RTEE);

	wrefresh(local_win);
	refresh();
}


static void print_in_middle(WINDOW *local_win, int starty, int startx, const char *string, chtype color) {
	int max_y, max_x;

	getmaxyx(local_win, max_y, max_x);

	wattron(local_win, A_REVERSE | COLOR_PAIR(1));
	mvwprintw(local_win, starty, ( max_x - strlen(string) )/2, "%s", string);
	wattroff(local_win, A_REVERSE | COLOR_PAIR(1));

	wrefresh(local_win);
	refresh();
}

static void die(WINDOW *local_win) {
	delwin(local_win);
	endwin();
	exit(EXIT_SUCCESS);
}

static void clear_window(WINDOW *local_win) {
	wclear(local_win);
	wrefresh(local_win);
	delwin(local_win);
}

static void clear_menu(MENU *local_win, int num_options) {
}

static void clear_redraw_window(WINDOW *local_win, int width, const char *string) {
	wclear(local_win);
	box(local_win, 0, 0);
	print_header(local_win, width, string);
	wrefresh(local_win);
}

static void redraw_window(WINDOW *local_win, int width, const char *string) {
	box(local_win, 0, 0);
	print_header(local_win, width, string);
	wrefresh(local_win);
}

static void main_menu() {
	WINDOW *main_win;
	const char *header = " Main Menu ";
	int height = 15;
	int width = 40;
	MENU *main_menu;
	ITEM *selected_item;
	const char *selected_item_str;
	int c;
	const char *mainMenuChoices[] = {
		"Sign in",
		"About",
		"Exit",
	};
	int num_choices = ARRAY_SIZE(mainMenuChoices);

	main_win = create_newwin(height, width, STARTY_CENTER - height, STARTX_CENTER - ( width / 2 ));
	keypad(main_win, TRUE);

	main_menu = populate_menu(mainMenuChoices, num_choices);

	set_menu_win(main_menu, main_win);
	set_menu_sub(main_menu, derwin(main_win, height - 4, width - 4, 4, 1));
	set_menu_mark(main_menu, " >> ");
	post_menu(main_menu);

	print_header(main_win, width, header);

	wrefresh(main_win);

	while ( (c = wgetch(main_win)) != KEY_F(1) ) {
		switch(c) {
			case KEY_UP:
				menu_driver(main_menu, REQ_UP_ITEM);
				break;
			case KEY_DOWN:
				menu_driver(main_menu, REQ_DOWN_ITEM);
				break;
				// VIM Keybindings
			case 107:	
				menu_driver(main_menu, REQ_UP_ITEM);
				break;
			case 106:
				menu_driver(main_menu, REQ_DOWN_ITEM);
				break;
			case 10:	// RETURN, Select an item
				selected_item_str = item_name(current_item(main_menu));
				mvwprintw(stdscr, 2, 0, "Selected: %s", selected_item_str);
				wrefresh(stdscr);

				// Turn item into a string then compare with an option
				if (strcmp(item_name(current_item(main_menu)), mainMenuChoices[0]) == 0)	// Sign in
				{
					sign_in_win();
					// Reset selected item
					memset(&selected_item_str, 0, sizeof(selected_item_str));
					redraw_window(main_win, width, header);
					break;
				}
				else if (strcmp(item_name(current_item(main_menu)), mainMenuChoices[1]) == 0)	// About
				{
					about_win();
					memset(&selected_item_str, 0, sizeof(selected_item_str));
					redraw_window(main_win, width, header);
					break;
				}
				else {
					die(main_win);
				}
		}
	}

	die(main_win);
}

/**
 *	@brief Handles sign in menu
 */
static void sign_in_win() {
	WINDOW *sign_in_win;
	const char *header = " Sign in Menu ";
	int height = 15;
	int width = 40;
	int c;
	char username[32];
	char password[32];
	char query[255], error_msg[255];
	int result;

	sign_in_win = create_newwin(height, width, STARTY_CENTER - height, STARTX_CENTER - ( width / 2 ));
	keypad(sign_in_win, TRUE);
	box(sign_in_win, 0, 0);

	print_header(sign_in_win, width, header);

	// Print out info to window
	wattron(sign_in_win, A_BOLD);
	mvwprintw(sign_in_win, 4, 2, "Username: ");
	mvwprintw(sign_in_win, 6, 2, "Password: ");
	wattroff(sign_in_win, A_BOLD);

	wattron(sign_in_win, A_REVERSE);
	mvwprintw(sign_in_win, 10, width/2 - strlen("Enter"), " Enter ");
	wattroff(sign_in_win, A_REVERSE);
	wrefresh(sign_in_win);

	// Handle input from user
	echo();
	curs_set(1);

	mvwscanw(sign_in_win, 4, 2 + strlen("Username: "), "%s", username);
	mvwscanw(sign_in_win, 6, 2 + strlen("Password: "), "%s", password);

	// Check for credentials
	// Database
	sprintf(query, "SELECT * FROM CREDENTIALS WHERE username='%s' AND password='%s'", username, password);
	result = sqlite3_exec(db, query, callback_sign_in, 0, &error_msg);

	// Error
	if (result != SQLITE_OK) {
	}

	// If correct credentials
	if (credentials) {
		mvwprintw(stdscr, 6, 0, "Success!");
		refresh();

		noecho();
		curs_set(0);
		user_menu();

		//TODO:
		// 1) User menu
		// 2) Teacher menu
		// 3) Admin Menu
		// Based on old src. One function to rule them all?
	}

	noecho();
	curs_set(0);
	clear_window(sign_in_win);
}

static void about_win() {
	WINDOW *about_win;
	const char *header = " About ";
	int height = 15;
	int width = 40;
	int c;

	about_win = create_newwin(height, width, STARTY_CENTER - height, STARTX_CENTER - ( width / 2 ));
	keypad(about_win, TRUE);
	box(about_win, 0, 0);

	print_header(about_win, width, header);

	// Print out info to window
	//wattron(sign_in_win, A_BOLD);
	mvwaddstr(about_win, 4, 2,  "This software is maintained and");
	mvwaddstr(about_win, 5, 2,  "developed by Mohamad Farman");
	mvwaddstr(about_win, 7, 2,  "For any inquires send to");
	mvwaddstr(about_win, 8, 2,  "mohamadfarman977@gmail.com");
	//wattroff(sign_in_win, A_BOLD);

	// Check for user exiting software.
	c = wgetch(about_win);
	if (c == KEY_F(1)) {
		die(about_win);
	}

	clear_window(about_win);
}

static void user_menu() {
	WINDOW *user_win;
	int height = 15;
	int width = 40;
	const char *header;
	MENU *user_menu;
	ITEM *selected_item;
	const char *selected_item_str;
	int c;
	// Menu options for each type of user
	const char *adminMenuChoices[] = {
		//"Add new user",
		//"Remove user",
		//"Edit user info",
		//"Courses menu",
		//"Change admin password",
		//"Sign out",
		"Manage Users menu",
		"Courses menu",
		"Change admin password",
		"Sign out",
	};
	const char *teacherMenuChoices[] = {
		"View courses",
		"View personal info",
		"Edit personal info or credentials",
		"Sign out",
	};
	const char *studentMenuChoices[] = {
		"View courses",
		"View personal info",
		"Edit personal info or credentials",
		"Sign out",
	};

	// Initialize header and menu options based on user privilege
	if (strcmp(privilege, "admin") == 0) {
		header = " Admin Menu ";
		int num_choices = ARRAY_SIZE(adminMenuChoices);
		user_menu = populate_menu(adminMenuChoices, num_choices);
	}

	else if (strcmp(privilege, "teacher") == 0) {
		header = " Teacher Menu ";
		int num_choices = ARRAY_SIZE(teacherMenuChoices);
		user_menu = populate_menu(teacherMenuChoices, num_choices);
	}

	else {
		header = " Student Menu ";
		int num_choices = ARRAY_SIZE(studentMenuChoices);
		user_menu = populate_menu(studentMenuChoices, num_choices);
	}

	// Create window
	user_win = create_newwin(height, width, STARTY_CENTER - height, STARTX_CENTER - ( width / 2 ));
	keypad(user_win, TRUE);

	// Create and initialize menu
	set_menu_win(user_menu, user_win);
	set_menu_sub(user_menu, derwin(user_win, height - 4, width - 4, 4, 1));
	set_menu_mark(user_menu, " >> ");
	post_menu(user_menu);

	print_header(user_win, width, header);

	wrefresh(user_win);

	while ( (c = wgetch(user_win)) != KEY_F(1) ) {
		switch(c) {
			case KEY_UP:
				menu_driver(user_menu, REQ_UP_ITEM);
				break;
			case KEY_DOWN:
				menu_driver(user_menu, REQ_DOWN_ITEM);
				break;
				// VIM Keybindings
			case 107:	
				menu_driver(user_menu, REQ_UP_ITEM);
				break;
			case 106:
				menu_driver(user_menu, REQ_DOWN_ITEM);
				break;
			case 10:	// RETURN, Select an item
				selected_item_str = item_name(current_item(user_menu));
				mvwprintw(stdscr, 2, 0, "Selected: %s", selected_item_str);
				wrefresh(stdscr);

				// Turn item into a string then compare with an option
				if (strcmp(item_name(current_item(user_menu)), adminMenuChoices[0]) == 0)	// Manage users menu
				{
					admin_manage_users_win();
					redraw_window(user_win, width, header);
					break;
				}
				else if (strcmp(item_name(current_item(user_menu)), adminMenuChoices[1]) == 0)	// Courses menu
				{
					break;
				}
				else if (strcmp(item_name(current_item(user_menu)), adminMenuChoices[2]) == 0)	// Change admin password
				{
					break;
				}
				else if (strcmp(item_name(current_item(user_menu)), adminMenuChoices[3]) == 0)	// Sign out
				{
					// Reset selected item
					memset(&selected_item_str, 0, sizeof(selected_item_str));
					clear_window(user_win);
					return;
					break;
				}
				//else if (strcmp(item_name(current_item(user_menu)), adminMenuChoices[4]) == 0)	// 
				//{
				//	break;
				//}
				//else if (strcmp(item_name(current_item(user_menu)), adminMenuChoices[5]) == 0)	// 
				//{
				//	// Reset selected item
				//	memset(&selected_item_str, 0, sizeof(selected_item_str));
				//	clear_window(user_win);
				//	return;
				//	break;
				//}
				else if (strcmp(item_name(current_item(user_menu)), studentMenuChoices[0]) == 0)	// View courses info
				{
					// Reset selected item
					//memset(&selected_item_str, 0, sizeof(selected_item_str));
					//redraw_window(user_win, width, header);
					break;
				}
				else if (strcmp(item_name(current_item(user_menu)), studentMenuChoices[1]) == 0)	// View personal info
				{
					// Reset selected item
					//memset(&selected_item_str, 0, sizeof(selected_item_str));
					//redraw_window(user_win, width, header);
					break;
				}
				else if (strcmp(item_name(current_item(user_menu)), studentMenuChoices[2]) == 0)	// Edit personal info or credentials
				{
					// Reset selected item
					//memset(&selected_item_str, 0, sizeof(selected_item_str));
					//redraw_window(user_win, width, header);
					break;
				}
				else if (strcmp(item_name(current_item(user_menu)), studentMenuChoices[3]) == 0)	// Sign out
				{
					// Reset selected item
					memset(&selected_item_str, 0, sizeof(selected_item_str));
					clear_window(user_win);
					return;
					break;
				}
				else {
					die(user_win);
				}
		}
	}
	die(user_win);
}

static void admin_add_user_win(int height, int width, int y_offset, int x_offset) {
	WINDOW *add_user_win;;
	//int height = 22;
	//int width = 40;
	const char *header = " Add New User ";
	char query[255], error_msg[255];
	int result;
	// Input
	char firstname[32];
	char lastname[32];
	int age;
	char address[50];
	char mail[32];
	int phone_number;
	char new_username[32];
	char new_password[32];
	char new_privilege[10];
	int c;

	// Create window
	add_user_win = create_newwin(height, width, y_offset, x_offset);
	keypad(add_user_win, TRUE);

	print_header(add_user_win, width, header);
	wrefresh(add_user_win);

	// Print out info to window for personal info
	wattron(add_user_win, A_BOLD);
	mvwprintw(add_user_win, 4, 2, "First name: ");
	mvwprintw(add_user_win, 6, 2, "Last name: ");
	mvwprintw(add_user_win, 8, 2, "Age: ");
	mvwprintw(add_user_win, 10, 2, "Mail: ");
	mvwprintw(add_user_win, 12, 2, "Address (Optional): ");
	mvwprintw(add_user_win, 14, 2, "Phone number: ");
	wattroff(add_user_win, A_BOLD);

	wattron(add_user_win, A_REVERSE);
	mvwprintw(add_user_win, height - 1, width/2 - strlen("Enter"), " Enter ");
	wattroff(add_user_win, A_REVERSE);
	wrefresh(add_user_win);

	// Handle input from user
	echo();
	curs_set(1);

	// Input for personal info
	mvwscanw(add_user_win, 4, 2 + strlen("First name: "), "%s", firstname);
	mvwscanw(add_user_win, 6, 2 + strlen("Last name: "), "%s", lastname);
	mvwscanw(add_user_win, 8, 2 + strlen("Age: "), "%d", &age);
	mvwscanw(add_user_win, 10, 2 + strlen("Mail: "), "%s", mail);
	mvwscanw(add_user_win, 12, 2 + strlen("Address (Optional): "), "%s", address);
	mvwscanw(add_user_win, 14, 2 + strlen("Phone number: "), "%d", &phone_number);

	// Redraw window
	add_user_win = create_newwin(height, width, y_offset, x_offset);
	keypad(add_user_win, TRUE);

	print_header(add_user_win, width, header);
	wrefresh(add_user_win);

	// Print out info to window for credentials
	wattron(add_user_win, A_BOLD);
	mvwprintw(add_user_win, 4, 2, "Username: ");
	mvwprintw(add_user_win, 6, 2, "Password: ");
	mvwprintw(add_user_win, 8, 2, "Privilege (teacher, student): ");
	wattroff(add_user_win, A_BOLD);

	wattron(add_user_win, A_REVERSE);
	mvwprintw(add_user_win, height - 1, width/2 - strlen("Enter"), " Enter ");
	wattroff(add_user_win, A_REVERSE);
	wrefresh(add_user_win);

	// Input for credentials
	mvwscanw(add_user_win, 4, 2 + strlen("Username: "), "%s", new_username);
	mvwscanw(add_user_win, 6, 2 + strlen("Password: "), "%s", new_password);
	mvwscanw(add_user_win, 8, 2 + strlen("Privilege (teacher, student): "), "%s", new_privilege);

	// Query to Person table w personal info
	sprintf(query, "INSERT INTO Person (first_name, last_name, age, mail, address, phone_number) \
			VALUES ('%s', '%s', %d, '%s', '%s', %d);", firstname, lastname, age, mail, address, phone_number);

	result = sqlite3_exec(db, query, callback_generic, 0, &error_msg);

	mvwprintw(stdscr, 8, 0, "result = %d", result);
	if (result != SQLITE_OK) {
		popup_msg_win(MSG_ERROR, "User personal info could not be added to database.");
	}

	// Query to Credentials table w credentials
	sprintf(query, "INSERT INTO Credentials (credentials_id, username, password, privilege) \
			VALUES ((SELECT person_id FROM Person WHERE first_name = '%s' AND last_name = '%s'), '%s', '%s', '%s');", firstname, lastname, new_username, new_password, new_privilege);

	result = sqlite3_exec(db, query, callback_generic, 0, &error_msg);

	mvwprintw(stdscr, 8, 0, "result = %d", result);
	if (result != SQLITE_OK) {
		popup_msg_win(MSG_ERROR, "User credentials could not be added to database.");
	}
	else { 
		popup_msg_win(MSG_SUCCESS, "User added successfully!");
	}

	noecho();
	curs_set(0);
	clear_window(add_user_win);
}

static void popup_msg_win(int status, const char *msg) {
	WINDOW *popup_msg_win;
	int height = 10;
	int width = 50;
	const char *header;
	int c;

	if (status == MSG_ERROR) {
		header = " ERROR! ";
	}

	else if (status == MSG_SUCCESS) {
		header = " SUCCESS! ";
	}

	popup_msg_win = create_newwin(height, width, STARTY_CENTER - height, STARTX_CENTER - ( width / 2 ));
	keypad(popup_msg_win, TRUE);
	box(popup_msg_win, 0, 0);

	print_header(popup_msg_win, width, header);

	// Print out info to window
	//wattron(popup_msg_win, A_BOLD);
	mvwaddstr(popup_msg_win, 4, 2, msg);
	wattron(popup_msg_win, A_REVERSE | A_COLOR);
	mvwaddstr(popup_msg_win, height - 2, width / 2, "[ ENTER ]");
	wattroff(popup_msg_win, A_REVERSE | A_COLOR);
	//wattroff(popup_msg_win, A_BOLD);

	// Check for user exiting software.
	c = wgetch(popup_msg_win);
	if (c == KEY_F(1)) {
		die(popup_msg_win);
	}

	clear_window(popup_msg_win);
}

static void admin_remove_user_win(int height, int width, int y_offset, int x_offset) {
	WINDOW *remove_user_win;
	const char *header = " Remove User ";
	char query[255], error_msg[255];
	int result;
	int user_id;
	int c;
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);

	remove_user_win = create_newwin(height, width, y_offset, x_offset);
	keypad(remove_user_win, TRUE);
	box(remove_user_win, 0, 0);

	print_header(remove_user_win, width, header);

	// Print out info to window for personal info
	wattron(remove_user_win, A_BOLD);
	mvwprintw(remove_user_win, 4, 2, "User ID: ");
	wattroff(remove_user_win, A_BOLD);

	// Handle input from user
	echo();
	curs_set(1);

	// Input for personal info
	mvwscanw(remove_user_win, 4, 2 + strlen("User ID: "), "%d", &user_id);

	noecho();
	curs_set(0);

	sprintf(query, "DELETE FROM Person WHERE person_id = %d", user_id);
	result = sqlite3_exec(db, query, callback_sign_in, 0, &error_msg);

	// Error
	if (result != SQLITE_OK) {
		wattron(remove_user_win, A_BOLD | A_REVERSE | COLOR_PAIR(2));
		mvwprintw(remove_user_win, 10, 10, "User ID %d was not deleted.", user_id);
		wattroff(remove_user_win, A_BOLD | A_REVERSE  | COLOR_PAIR(2));
	}
	else {
		wattron(remove_user_win, A_BOLD | A_REVERSE  | COLOR_PAIR(1));
		mvwprintw(remove_user_win, 10, 10, "User ID %d deleted successfully.", user_id);
		wattroff(remove_user_win, A_BOLD | A_REVERSE  | COLOR_PAIR(1));
	}

	wrefresh(remove_user_win);

	// Check for user exiting software.
	c = wgetch(remove_user_win);
	if (c == KEY_F(1)) {
		die(remove_user_win);
	}

	clear_window(remove_user_win);
}

static void admin_manage_users_win() {
	WINDOW *manage_users_win, *options_users_win;;
	int height = 30;
	int width = 60;
	int y_offset = 14, x_offset_users = 60, x_offset_options = 120;
	MENU *manage_users_menu;
	ITEM *selected_item;
	const char *selected_item_str;
	const char *header_users = " Users ", *header_options = " Options ";
	char query[255], error_msg[255];
	sqlite3_stmt *stmt;
	int pos_y = 4, pos_x = 4;	// For printing out list of users
	int column_length = width / 3;	// Divided by number of columns retrieved from db
	int result;
	int c, i;

	const char *manage_users_menu_options[] = {
		"Add new user",
		"Remove user",
		"Edit user info",
		"Go back"
	};
	int num_choices = ARRAY_SIZE(manage_users_menu_options);

	// Create window
	manage_users_win = create_newwin(height, width, y_offset, x_offset_users);
	options_users_win = create_newwin(height, width, y_offset, x_offset_options);
	keypad(manage_users_win, TRUE);
	keypad(options_users_win, TRUE);

	print_header(manage_users_win, width, header_users);
	print_header(options_users_win, width, header_options);


	// Create and initialize menu
	manage_users_menu  = populate_menu(manage_users_menu_options, num_choices);
	set_menu_win(manage_users_menu, options_users_win);
	set_menu_sub(manage_users_menu, derwin(options_users_win, height - 4, width - 2, 4, 2));
	//set_menu_format(manage_users_menu, 1, 3);
	set_menu_mark(manage_users_menu, " >> ");
	post_menu(manage_users_menu);

	// Print out users to the window
	sprintf(query, "SELECT person_id, first_name, last_name FROM Person");
	sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);

	pos_y = 4;
	while(sqlite3_step(stmt) == SQLITE_ROW) {
		for (i = 0; i < sqlite3_column_count(stmt); ++i) {
			mvwprintw(manage_users_win, pos_y, pos_x, "%s", sqlite3_column_text(stmt, i));
			pos_x += column_length;
		}
		// Reset the starting position for the next row
		pos_y += 1;
		pos_x = 4;
	}

	sqlite3_finalize(stmt);

	// Print out the heading for each column
	pos_y = 4;
	wattr_on(manage_users_win, A_UNDERLINE | A_BOLD, NULL);
	mvwprintw(manage_users_win, pos_y, pos_x, "User ID");
	pos_x += column_length;
	mvwprintw(manage_users_win, pos_y, pos_x, "First Name");
	pos_x += column_length;
	mvwprintw(manage_users_win, pos_y, pos_x, "Last Name");
	pos_x += column_length;
	wattr_off(manage_users_win,A_UNDERLINE | A_BOLD, NULL);

	wrefresh(manage_users_win);
	wrefresh(options_users_win);


	while ( (c = wgetch(options_users_win)) != KEY_F(1) ) {
		switch(c) {
			case KEY_RIGHT:
				menu_driver(manage_users_menu, REQ_RIGHT_ITEM);
				break;
			case KEY_LEFT:
				menu_driver(manage_users_menu, REQ_LEFT_ITEM);
				break;
				// VIM Keybindings
			case 107:	
				menu_driver(manage_users_menu, REQ_UP_ITEM);
				break;
			case 106:
				menu_driver(manage_users_menu, REQ_DOWN_ITEM);
				break;
			case 10:	// RETURN, Select an item
				selected_item_str = item_name(current_item(manage_users_menu));
				mvwprintw(stdscr, 2, 0, "Selected: %s", selected_item_str);
				wrefresh(stdscr);

				// Turn item into a string then compare with an option
				if (strcmp(item_name(current_item(manage_users_menu)), manage_users_menu_options[0]) == 0)	// Add new user
				{
					admin_add_user_win(height, width, y_offset, x_offset_options);
					// Update user list
					get_users(manage_users_win, pos_y, pos_x, width);
					
					// Redraw windows 
					redraw_window(manage_users_win, width, header_users);
					redraw_window(options_users_win, width, header_options);
					break;
				}
				else if (strcmp(item_name(current_item(manage_users_menu)), manage_users_menu_options[1]) == 0)	// Remove user
				{
					admin_remove_user_win(height, width, y_offset, x_offset_options);
					// Update user list
					get_users(manage_users_win, pos_y, pos_x, width);

					// Redraw windows 
					redraw_window(manage_users_win, width, header_users);
					redraw_window(options_users_win, width, header_options);
					break;
				}
				else if (strcmp(item_name(current_item(manage_users_menu)), manage_users_menu_options[2]) == 0)	// Edit user info
				{
					admin_edit_user_info_win(height, width, y_offset, x_offset_options);
					// Redraw windows 
					clear_redraw_window(manage_users_win, width, header_users);
					clear_redraw_window(options_users_win, width, header_options);
					
					// Update user list
					get_users(manage_users_win, pos_y, pos_x, width);
					// Upon return the menu is not visible with a user input
					// With this menu_driver the menu items will be visible again
					// and puts the user on the first item
					menu_driver(manage_users_menu, REQ_FIRST_ITEM);		
					break;
				}
				else {	// Go back
					clear_window(manage_users_win);
					clear_window(options_users_win);
					return;
				}
		}
	}

	// Check for user exiting software.
	c = wgetch(manage_users_win);
	if (c == KEY_F(1)) {
		die(manage_users_win);
		die(options_users_win);
	}

	clear_window(manage_users_win);
	clear_window(options_users_win);
}

void get_users(WINDOW *local_win, int pos_y, int pos_x, int width) {
	char query[255], error_msg[255];
	sqlite3_stmt *stmt;
	int column_length = width / 3;	// Divided by number of columns retrieved from db
	int result;
	int i;

	// Print out users to the window
	sprintf(query, "SELECT person_id, first_name, last_name FROM Person");
	sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);

	pos_y = 4;
	while(sqlite3_step(stmt) == SQLITE_ROW) {
		for (i = 0; i < sqlite3_column_count(stmt); ++i) {
			mvwprintw(local_win, pos_y, pos_x, "%s", sqlite3_column_text(stmt, i));
			pos_x += column_length;
		}
		// Reset the starting position for the next row
		pos_y += 1;
		pos_x = 4;
	}

	sqlite3_finalize(stmt);

	// Print out the heading for each column
	pos_y = 4;
	wattr_on(local_win, A_UNDERLINE | A_BOLD, NULL);
	mvwprintw(local_win, pos_y, pos_x, "User ID");
	pos_x += column_length;
	mvwprintw(local_win, pos_y, pos_x, "First Name");
	pos_x += column_length;
	mvwprintw(local_win, pos_y, pos_x, "Last Name");
	pos_x += column_length;
	wattr_off(local_win,A_UNDERLINE | A_BOLD, NULL);

	wrefresh(local_win);
}

static void admin_edit_user_info_win(int height, int width, int y_offset, int x_offset) {
	struct ncursor_menu;
	WINDOW *edit_user_info_win;
	MENU *edit_user_info_menu;
	const char *selected_item_str;
	const char *header = " Edit User Info ";
	char query[255], error_msg[255];
	sqlite3_stmt *stmt;
	int pos_y = 4, pos_x = 25;	// For printing out list of users
	int result;
	int user_id;
	int c, i, ch, rows, cols;
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);

	const char *edit_user_info_menu_options[] = {
		//"User ID:",
		"First Name:",
		"Last Name:",
		"Age:",
		"Mail:",
		"Address:",
		"Phone Number:",
		"Exit"
	};
	int num_choices = ARRAY_SIZE(edit_user_info_menu_options);

	edit_user_info_win = create_newwin(height, width, y_offset, x_offset);
	keypad(edit_user_info_win, TRUE);
	box(edit_user_info_win, 0, 0);

	print_header(edit_user_info_win, width, header);

	// Print out info to window for personal info
	wattron(edit_user_info_win, A_BOLD);
	mvwprintw(edit_user_info_win, 4, 4, "User ID: ");
	wattroff(edit_user_info_win, A_BOLD);

	// Handle input from user
	echo();
	curs_set(1);

	// Input for personal info
	//mvwscanw(edit_user_info_win, 4, 4 + strlen("User ID: "), "%d", &user_id);
	mvwscanw(edit_user_info_win, 4, pos_x, "%d", &user_id);

	noecho();
	curs_set(0);

	//clear_window(edit_user_info_win);
	//wclear(edit_user_info_win);
	redraw_window(edit_user_info_win, width, header);

	sprintf(query, "SELECT first_name, last_name, age, mail, address, phone_number FROM Person WHERE person_id = %d", user_id);

	// Print out users to the window
	sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);

	while(sqlite3_step(stmt) == SQLITE_ROW) {
		for (i = 0; i < sqlite3_column_count(stmt); ++i) {
			mvwprintw(edit_user_info_win, pos_y, pos_x, "%s", sqlite3_column_text(stmt, i));
			pos_y++;
		}
	}

	wattron(edit_user_info_win, A_UNDERLINE);
	mvwprintw(edit_user_info_win, 12, 6, "Select an option to edit or exit");
	wattroff(edit_user_info_win, A_UNDERLINE);

	sqlite3_finalize(stmt);

	// Create and initialize menu
	edit_user_info_menu  = populate_menu(edit_user_info_menu_options, num_choices);
	set_menu_win(edit_user_info_menu, edit_user_info_win);
	set_menu_sub(edit_user_info_menu, derwin(edit_user_info_win, height - 4, width - 2, 4, 2));
	//set_menu_format(edit_user_info_menu, num_choices, 1);
	set_menu_mark(edit_user_info_menu, " >> ");
	//set_menu_fore(edit_user_info_menu, A_BOLD, A_UNDERLINE, A_BLINK);
	post_menu(edit_user_info_menu);

	redraw_window(edit_user_info_win, width, header);

	while ( (c = wgetch(edit_user_info_win)) != KEY_F(1) ) {
		switch(c) {
			case KEY_UP:
				menu_driver(edit_user_info_menu, REQ_UP_ITEM);
				break;
			case KEY_DOWN:
				menu_driver(edit_user_info_menu, REQ_DOWN_ITEM);
				break;
				// VIM Keybindings
			case 107:	
				menu_driver(edit_user_info_menu, REQ_UP_ITEM);
				break;
			case 106:
				menu_driver(edit_user_info_menu, REQ_DOWN_ITEM);
				break;
			case 10:	// RETURN, Select an item
				selected_item_str = item_name(current_item(edit_user_info_menu));
				mvwprintw(stdscr, 2, 0, "Selected: %s", selected_item_str);
				wrefresh(stdscr);

				if (strcmp(item_name(current_item(edit_user_info_menu)), edit_user_info_menu_options[0]) == 0)	// First name
				{
					char new_username[32];

					wattron(edit_user_info_win, A_UNDERLINE | COLOR_PAIR(1));
					mvwprintw(edit_user_info_win, 12, 6, "Write a new first name and press [ENTER] to save");
					wattroff(edit_user_info_win, A_UNDERLINE | COLOR_PAIR(1));

					echo();
					curs_set(1);
					mvwscanw(edit_user_info_win, 4, 25, "%s", new_username);

					noecho();
					curs_set(0);

					sprintf(query, "UPDATE Person SET first_name = '%s' WHERE person_id = %d", new_username, user_id);
					result = sqlite3_exec(db, query, callback_sign_in, 0, &error_msg);

					clear_window(edit_user_info_win);
					free_menu(edit_user_info_menu);
					return;
					/*
					 *	1) mvwscan to position of first name
					 *	2) take user input
					 *	3) perform query
					 *	4) upon success write everything to screen again and render
					 *	5) upon fail write everything to screen again and render - provide w an error message
					 */
					break;
				}
				else if (strcmp(item_name(current_item(edit_user_info_menu)), edit_user_info_menu_options[1]) == 0)	// Last name
				{
					break;
				}
				if (strcmp(item_name(current_item(edit_user_info_menu)), edit_user_info_menu_options[2]) == 0)	// Age
				{
					break;
				}
				if (strcmp(item_name(current_item(edit_user_info_menu)), edit_user_info_menu_options[3]) == 0)	// Mail
				{
					break;
				}
				if (strcmp(item_name(current_item(edit_user_info_menu)), edit_user_info_menu_options[4]) == 0)	// Address
				{
					break;
				}
				if (strcmp(item_name(current_item(edit_user_info_menu)), edit_user_info_menu_options[5]) == 0)	// Phone number
				{
					break;
				}
				else if (strcmp(item_name(current_item(edit_user_info_menu)), edit_user_info_menu_options[6]) == 0)	// Exit
				{
					clear_window(edit_user_info_win);
					free_menu(edit_user_info_menu);
					return;
					break;
				}
		}
	}

	wrefresh(edit_user_info_win);

	// Check for user exiting software.
	c = wgetch(edit_user_info_win);
	if (c == KEY_F(1)) {
		die(edit_user_info_win);
	}

	clear_window(edit_user_info_win);
	free_menu(edit_user_info_menu);
}

#endif
