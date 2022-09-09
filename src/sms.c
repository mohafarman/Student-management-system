#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <malloc.h>
#include <menu.h>
#include "sqlite3.h"
#include "sms.h"

void HeaderText(char* str, bool about);
void PrintInMiddle(WINDOW *localWin, int starty, int startx, char *string, chtype color);
void UserMenu();
void CourseMenu();
void CheckCredentials();
void PrintAbout();

void MainMenu();
//void AboutMenu(WINDOW *localWin, MENU *localMenu);
void AboutMenu();
void DrawWindowBorder(WINDOW *localWin, char *string, chtype color);

bool DatabaseInit();
void DatabaseAddUserWindow();
void DatabaseRemoveUser();
void DatabaseUpdateUser();
void DatabaseViewUserInfo();
void DatabaseViewCoursesInfo();
void DatabaseAdminUpdatePassword();

void DatabaseAddCourse();
void DatabaseViewCoursesInfo();

int flush();
void die(WINDOW *localWin);

sqlite3 *db;
static int DatabaseCallbackSelectSignIn(void *data, int argc, char **argv, char **azColName);
static int DatabaseCallback(void *NotUsed, int argc, char **argv, char **azColName);
static int DatabaseCallbackViewUserInfo(void *NotUsed, int argc, char **argv, char **azColName);
static int DatabaseCallbackTableCreate(void *NotUsed, int argc, char **argv, char **azColName);
static int DatabaseCallbackViewCoursesInfo(void *NotUsed, int argc, char **argv, char **azColName);
static int DatabaseCallbackViewCoursesInfoWindow(void *NotUsed, int argc, char **argv, char **azColName);

WINDOW *create_newwin();
int SetUserMenu(WINDOW *localWin, MENU *localMenu, ITEM **localMenuItmes);
void DestroyWinMenu(WINDOW *localWin, MENU *localMenu, ITEM **localMenuItems, int numChoices);

char *mainMenuChoices[] = {
	"Sign in",
	"About",
	"Exit",
};

char *adminMenuChoices[] = {
	"Add new user",
	"Remove user",
	"Edit user info",
	"Menu courses",
	"Change admin password",
	"Sign out",
};

char *teacherMenuChoices[] = {
	"View courses",
	"View personal info",
	"Edit personal info or credentials",
	"Sign out",
};

char *studentMenuChoices[] = {
	"View courses",
	"View personal info",
	"Edit personal info or credentials",
	"Sign out",
};

int starty = 0, startx = 0;

char username[32];
char password[32];
bool credentials = false;
char privilege[10];
int userID;

int main() {
	int resultDb;

	// Initialize the DB
	resultDb = sqlite3_open(DATABASE, &db);

	if (resultDb) {
		fprintf(stderr, "Failed to connect to database: %s" , sqlite3_errmsg(db));
		perror("Exiting program");
		exit(1);
	}

	// Initialize ncurses
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	start_color();

	// Store terminal information in variables
	getmaxyx(stdscr, starty, startx);

	mvprintw(0, 0, "Press F1 to exit");
	refresh();

	MainMenu();

	sqlite3_close(db);
	endwin();
	return 0;
}

void HeaderText(char* str, bool about) {
	//CLEAR_SCREEN;
	//RESET_CURSOR;

	if (about) {
		printf("\t\t* * * * * * * * * * * * * * * * * * * * *\t\t\n");
		printf("\t\t* * * * * * * * * * * * * * * * * * * * *\t\t\n");
		printf("\t\t*\tStudent Management System\t*\t\t\n");
		printf("\t\t*\t\t%s\t\t*\t\t\n", str);
		printf("\t\t* * * * * * * * * * * * * * * * * * * * *\t\t\n");
		printf("\t\t* * * * * * * * * * * * * * * * * * * * *\t\t\n");
		printf("\t\tThis software is produced and maintained by Mohamad Farman.\n \
				\r\t\tAny inquires can be sent to mohamadfarman@fakemail.com\t\t\n\n");
	}
	else {
		printf("\t\t* * * * * * * * * * * * * * * * * * * * *\t\t\n");
		printf("\t\t* * * * * * * * * * * * * * * * * * * * *\t\t\n");
		printf("\t\t*\tStudent Management System\t*\t\t\n");
		printf("\t\t*\t\t%s\t\t*\t\t\n", str);
		printf("\t\t* * * * * * * * * * * * * * * * * * * * *\t\t\n");
		printf("\t\t* * * * * * * * * * * * * * * * * * * * *\t\t\n");
	}
}

void CheckCredentialsWindow() {
	char query[255];
	int resultQuery;
	char *errorMsg;
	char username[32];
	char password[32];
	int x_offset = 4 + strlen("username");
	WINDOW *checkCredentialsWin;
	curs_set(1);
	echo();
	//nocbreak();

	checkCredentialsWin = create_newwin();
	keypad(checkCredentialsWin , TRUE);

	DrawWindowBorder(checkCredentialsWin , " SIGN IN ", COLOR_PAIR(1));

	// Adding offset for x to type in credentials
	mvwprintw(checkCredentialsWin, 4, 2, "Username: ");
	mvwscanw(checkCredentialsWin, 4, x_offset, "%s", username);
	mvwprintw(checkCredentialsWin, 6, 2, "Password: ");
	mvwscanw(checkCredentialsWin, 6, x_offset, "%s", password);

	sprintf(query, "SELECT * FROM CREDENTIALS WHERE USERNAME='%s' AND PASSWORD='%s'", username, password);
	resultQuery = sqlite3_exec(db, query, DatabaseCallbackSelectSignIn, 0, &errorMsg);

	if (resultQuery != SQLITE_OK) {
		fprintf(stderr, "Query error: %s", errorMsg); return;
		return;
	}

	noecho();
	curs_set(0);
	wrefresh(checkCredentialsWin);
	refresh();

	wclear(checkCredentialsWin);
	wrefresh(checkCredentialsWin);
	delwin(checkCredentialsWin);

	if (credentials) {
		UserMenu();
	}

}

void CheckCredentials() {
	char query[255];
	int result;
	char *errorMsg;
	// Keeps track of number of tries to sign in.
	int counter = 5;

	do {
		if (counter <= 3 && counter > 0) { 
			printf("\t\t%d number of tries left.\n", counter); 
		}
		else if (counter <= 0){
			break;
		}
		printf("\t\tUsername: ");
		scanf("%s", username);
		flush();
		printf("\t\tPassword: ");
		scanf("%s", password);
		flush();

		// Check the CREDENTIALS table
		sprintf(query, "SELECT * FROM CREDENTIALS WHERE USERNAME='%s' AND PASSWORD='%s'", username, password);
		result = sqlite3_exec(db, query, DatabaseCallbackSelectSignIn, 0, &errorMsg);

		if (result != SQLITE_OK) {
			fprintf(stderr, "Query error: %s", errorMsg); return;
			return;
		}

		if (credentials) {
			sqlite3_close(db);
			return;
		}

		counter--;
	} while (counter > 0);

	sqlite3_close(db);
}

bool DatabaseInit() {
	sqlite3 *db;
	char *query;
	char *errorMsg;
	bool result = false;

	result = sqlite3_open(DATABASE, &db);

	if (result) {
		fprintf(stderr, "Failed to connect to database: %s" , sqlite3_errmsg(db));
		return false;
	}
	else {
		fprintf(stderr, "Connected to database.\n");
		// Return true only if tables exist.

		// Check the PERSON table
		query = "SELECT * FROM PERSON";
		result = sqlite3_exec(db, query, DatabaseCallbackSelectSignIn, 0, &errorMsg);

		if (result != SQLITE_OK) {
			fprintf(stderr, "Query error. PERSON table not found: %s", errorMsg);
			return false;
		}

		// Check the CREDENTIALS table
		query = "SELECT * FROM CREDENTIALS";
		result = sqlite3_exec(db, query, DatabaseCallbackSelectSignIn, 0, &errorMsg);

		if (result != SQLITE_OK) {
			fprintf(stderr, "Query error. CREDENTIALS table not found: %s", errorMsg); return false;
		}
	}

	sqlite3_close(db);
	return true;
}

void UserMenu() {
	WINDOW *userMenuWin;
	MENU *userMenu;
	ITEM **userMenuItems;
	ITEM *userMenuSelectedItem;
	const char *userMenuSelectedItemStr;
	int c, i, numChoices;

	init_pair(1, COLOR_CYAN, COLOR_BLACK);

	// Initialize the correct menu based on privilege
	if (strcmp(privilege, "admin") == 0) {
		numChoices = ARRAY_SIZE(adminMenuChoices);
		userMenuItems = (ITEM **)calloc(numChoices + 1, sizeof(ITEM *));

		for (i = 0; i < numChoices; ++i) {
			userMenuItems[i] = new_item(adminMenuChoices[i], "");
		}
	}
	else if (strcmp(privilege, "teacher") == 0) {
		numChoices = ARRAY_SIZE(teacherMenuChoices);
		userMenuItems = (ITEM **)calloc(numChoices + 1, sizeof(ITEM *));

		for (i = 0; i < numChoices; ++i) {
			userMenuItems[i] = new_item(teacherMenuChoices[i], "");
		}
	}
	else {
		numChoices = ARRAY_SIZE(studentMenuChoices);
		userMenuItems = (ITEM **)calloc(numChoices + 1, sizeof(ITEM *));

		for (i = 0; i < numChoices; ++i) {
			userMenuItems[i] = new_item(studentMenuChoices[i], "");
		}
	}

	userMenu = new_menu(userMenuItems);
	userMenuWin = create_newwin();
	keypad(userMenuWin, TRUE);

	set_menu_win(userMenu, userMenuWin);
	set_menu_sub(userMenu, derwin(userMenuWin, HEIGHT - 4, WIDTH - 4, 4, 1));

	set_menu_mark(userMenu, " >> ");

	post_menu(userMenu);

	DrawWindowBorder(userMenuWin, privilege, COLOR_PAIR(1));

	while ( (c = wgetch(userMenuWin)) != KEY_F(1) ) {
		switch(c) {
			case KEY_UP:
				menu_driver(userMenu, REQ_UP_ITEM);
				break;
			case KEY_DOWN:
				menu_driver(userMenu, REQ_DOWN_ITEM);
				break;
			// VIM Keybindings
			case 107:	
				menu_driver(userMenu, REQ_UP_ITEM);
				break;
			case 106:
				menu_driver(userMenu, REQ_DOWN_ITEM);
				break;
			case 10:	// RETURN, Select an item
				userMenuSelectedItem = current_item(userMenu);
				userMenuSelectedItemStr = item_name(userMenuSelectedItem);
				mvwprintw(stdscr, 2, 0, "Selected: %s", userMenuSelectedItemStr);
				wrefresh(stdscr);

				if (userMenuSelectedItemStr == "View courses") {
					//wclear(userMenuWin);
					//wrefresh(userMenuWin);
					DatabaseViewCoursesInfo();
					DrawWindowBorder(userMenuWin, privilege, COLOR_PAIR(1));
					break;
				}
				else if (userMenuSelectedItemStr == "View personal info") {
					break;
				}
				else if (userMenuSelectedItemStr == "Edit personal info or credentials") {
					break;
				}
				else if (userMenuSelectedItemStr == "Sign out") {
					// When a user signs out reset necessary global variables
					username[0] = 0;
					password[0] = 0;
					privilege[0] = 0;
					credentials = false;
					DestroyWinMenu(userMenuWin, userMenu, userMenuItems, numChoices);
					return;
				}
				break;
		}
	}

	DestroyWinMenu(userMenuWin, userMenu, userMenuItems, numChoices);

	// Admin
	//if (strcmp(privilege, "admin") == 0) {

	//	unsigned int options = USER_ADD | USER_REMOVE | USER_EDIT | CHANGE_ADMIN_PASS | SIGN_OUT_ADMIN;

	//	do {
	//		HeaderText("Admin Menu", false);
	//		printf("\t\tWith great power, comes great responsibility.\n");
	//		printf("\t\t(1) : Add new user\n");
	//		printf("\t\t(2) : Remove user\n");
	//		printf("\t\t(3) : Edit user info\n");
	//		printf("\t\t(4) : Menu for courses\n");
	//		printf("\t\t(5) : Change admin password\n");
	//		printf("\t\t(6) : Sign out\n");

	//		printf("\t\t>>> ");
	//		scanf("%u", &options);
	//		flush();
	//		switch(options) {
	//			case USER_ADD: 
	//				printf("Adding user...\n");
	//				DatabaseAddUser();
	//				break;
	//			case USER_REMOVE: 
	//				printf("Removing user...\n");
	//				DatabaseRemoveUser();
	//				break;
	//			case USER_EDIT: 
	//				printf("Editing user...\n");
	//				DatabaseUpdateUser();
	//				break;
	//			case COURSES_MENU: 
	//				printf("Adding new course...\n");
	//				CourseMenu();
	//				break;
	//			case CHANGE_ADMIN_PASS: 
	//				printf("Changing admin password...\n");
	//				DatabaseAdminUpdatePassword();
	//				break;
	//		}
	//	} while (options != SIGN_OUT_ADMIN);

	//}
	//// Teacher
	//else if (strcmp(privilege, "teacher") == 0) {

	//	unsigned int options = VIEW_COURSES | VIEW_USER_INFO | EDIT_PERSONAL_CREDENTIALS_INFO | SIGN_OUT_TEACHER;

	//	do {
	//		HeaderText("Teacher Menu", false);
	//		printf("\t\t(1) : View courses\n");
	//		printf("\t\t(2) : View personal info\n");
	//		printf("\t\t(3) : Edit personal info or credentials\n");
	//		printf("\t\t(4) : Sign out\n");

	//		printf("\t\t>>> ");
	//		scanf("%u", &options);
	//		flush();
	//		switch(options) {
	//			case VIEW_COURSES: 
	//				printf("Viewing courses...\n");
	//				break;
	//			case VIEW_USER_INFO: 
	//				DatabaseViewUserInfo();
	//				break;
	//			case EDIT_PERSONAL_CREDENTIALS_INFO: 
	//				printf("Updating user info...\n");
	//				DatabaseUpdateUser();
	//				break;
	//		}
	//	} while (options != SIGN_OUT_TEACHER);
	//}
	//// Student
	//else {

	//	unsigned int options = VIEW_COURSES | VIEW_USER_INFO | EDIT_PERSONAL_CREDENTIALS_INFO | SIGN_OUT_STUDENT;

	//	do {
	//		HeaderText("Student Menu", false);
	//		printf("\t\tYour average grade is: [PLACEHOLDER]\n");
	//		printf("\t\t(1) : View courses\n");
	//		printf("\t\t(2) : View personal info.\n");
	//		printf("\t\t(3) : Change personal info or credentials.\n");
	//		printf("\t\t(4) : Sign Out\n");

	//		printf("\t\t>>> ");
	//		scanf("%u", &options);
	//		flush();
	//		switch(options) {
	//			case VIEW_COURSES: 
	//				printf("Viewing courses...\n");
	//				break;
	//			case VIEW_USER_INFO: 
	//				DatabaseViewUserInfo();
	//				break;
	//			case EDIT_PERSONAL_CREDENTIALS_INFO: 
	//				printf("Editing info...\n");
	//				DatabaseUpdateUser();
	//				break;
	//		}
	//	} while (options != SIGN_OUT_STUDENT);

	//}

	// When a user signs out reset necessary global variables
	username[0] = 0;
	password[0] = 0;
	privilege[0] = 0;
	credentials = false;
	die(userMenuWin);
}

static int DatabaseCallbackTableCreate(void *NotUsed, int argc, char **argv, char **azColName) {
	int i;
	for(i = 0; i<argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

static int DatabaseCallbackSelectSignIn(void *data, int argc, char **argv, char **azColName) {
	int i;

	//for(i = 0; i<argc; i++) {
	//	printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	//}
	//printf("\n");

	if (argc) {
		credentials = true;
	}

	for(i = 0; i<argc; i++)
	{

		if (strcmp(azColName[i], "ID") == 0)  {
			// Convert the const char* argv[i] to an int
			// so it can be stored in userID and used by the SQL query
			sscanf(argv[i], "%d", &userID);
		}

		if (strcmp(azColName[i], "privilege") == 0) 
		{
			if (strcmp(argv[i], "admin") == 0) {
				strcpy(privilege, "admin");
			}
			else if (strcmp(argv[i], "teacher") == 0) { 
				strcpy(privilege, "teacher");
			}
			else { strcpy(privilege, "student"); }
		}
	}

	fprintf(stderr, "%s: ", (const char*)data);
	//printf("\n");

	//printf("\n");
	return 0;
}

static int DatabaseCallbackInsertUser(void *NotUsed, int argc, char **argv, char **azColName) {
	int i;
	for(i = 0; i<argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

static int DatabaseCallback(void *NotUsed, int argc, char **argv, char **azColName) {
	int i;
	for(i = 0; i<argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

static int DatabaseCallbackViewUserInfo(void *NotUsed, int argc, char **argv, char **azColName) {
	HeaderText("User info", false);
	int i;
	for(i = 0; i<argc; i++) {
		printf("\t\t%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n\t\tPress [Enter] to go back.");
	getchar();
	return 0;
}

static int DatabaseCallbackViewCoursesInfoWindow(void *NotUsed, int argc, char **argv, char **azColName) {
	WINDOW *viewCoursesInfoWindow;
	int height = 20, width = 120;
	int pstarty = 4, pstartx = 2;
	int c, i, column;
	int columnLength = width / 6;	// 6 columns

	curs_set(1);
	echo();
	init_pair(2, COLOR_CYAN, COLOR_BLACK);

	viewCoursesInfoWindow = newwin(height, width, (starty / 2) - (height + height) / 2, (startx / 2) - (width / 2));
	keypad(viewCoursesInfoWindow, TRUE);
	
	mvwaddch(viewCoursesInfoWindow, 2, 0, ACS_LTEE);
	mvwhline(viewCoursesInfoWindow, 2, 1, ACS_HLINE, width);
	mvwaddch(viewCoursesInfoWindow, 2, width - 1, ACS_RTEE);

	wattr_on(viewCoursesInfoWindow, A_UNDERLINE | A_BOLD, NULL);
	mvwprintw(viewCoursesInfoWindow, pstarty, pstartx, "Name");
	pstartx += columnLength;
	mvwprintw(viewCoursesInfoWindow, pstarty, pstartx, "Teacher");
	pstartx += columnLength;
	mvwprintw(viewCoursesInfoWindow, pstarty, pstartx, "Spots Available");
	pstartx += columnLength;
	mvwprintw(viewCoursesInfoWindow, pstarty, pstartx, "Finished");
	pstartx += columnLength;
	mvwprintw(viewCoursesInfoWindow, pstarty, pstartx, "Start date");
	pstartx += columnLength;
	mvwprintw(viewCoursesInfoWindow, pstarty, pstartx, "Finish date");
	wattr_off(viewCoursesInfoWindow,A_UNDERLINE | A_BOLD, NULL);

	pstarty += 2;
	pstartx = 2;
	
	for(i = 0; i<argc; i++) {
		mvwprintw(viewCoursesInfoWindow, pstarty, pstartx, "%s", argv[i]);
		pstartx += columnLength;
		// If 6 columns has been printed, then it's next course
		// so continue printing on a new line.
		if (column == 6) {
			column = 0;
			pstartx = 2;
			pstarty += 2;
		}
		else {
			++column;
		}
	}

	mvwprintw(viewCoursesInfoWindow, height - 2, width / 2, "Press any key to go back");

	DrawWindowBorder(viewCoursesInfoWindow, " VIEW COURSES ", COLOR_PAIR(2));

	wrefresh(viewCoursesInfoWindow);
	wrefresh(stdscr);
	c = wgetch(viewCoursesInfoWindow);

	if (c == KEY_F(1)) {
		die(viewCoursesInfoWindow);
	}

	wclear(viewCoursesInfoWindow);
	wrefresh(viewCoursesInfoWindow);
	delwin(viewCoursesInfoWindow);

	curs_set(0);
	noecho();
	return 0;
}

void DatabaseAddUserWindow() {
	sqlite3 *db;
	char query[255];
	char *errorMsg;
	bool result = false;
	srand(time(NULL));
	int id = rand();
	char firstname[32];
	char lastname[32];
	int age;
	char address[50];
	char mail[32];
	int phone_number;
	char newUsername[32];
	char newPassword[32];
	char newPrivilege[10];
	int x_offset = 4 + strlen("username");

	WINDOW *databaseAddUserWindow;
	curs_set(1);
	echo();
	//nocbreak();

	databaseAddUserWindow = create_newwin();
	keypad(databaseAddUserWindow , TRUE);

	DrawWindowBorder(databaseAddUserWindow , " ADD USER ", COLOR_PAIR(1));

	// Adding offset for x to type in credentials
	mvwprintw(databaseAddUserWindow, 4, 2, "Username: ");
	mvwscanw(databaseAddUserWindow, 4, x_offset, "%s", username);
	mvwprintw(databaseAddUserWindow, 6, 2, "Password: ");
	mvwscanw(databaseAddUserWindow, 6, x_offset, "%s", password);

	result = sqlite3_open(DATABASE, &db);

	if (result != SQLITE_OK) {
		fprintf(stderr, "Failed to connect to database: %s" , sqlite3_errmsg(db));
		sqlite3_free(db);
		return;
	}

	printf("\t\tFirst name: ");
	scanf("%s", firstname);
	flush();
	printf("\t\tLast name: ");
	scanf("%s", lastname);
	flush();
	printf("\t\tAge: ");
	scanf("%d", &age);
	flush();
	printf("\t\tAddress: ");
	scanf("%s", address);
	flush();
	printf("\t\tMail: ");
	scanf("%s", mail);
	flush();
	printf("\t\tPhone number: ");
	scanf("%d", &phone_number);
	flush();

	sprintf(query, "INSERT INTO PERSON \
			VALUES (%i, '%s', '%s', %d, '%s', '%s', %d);", id, firstname, lastname, age, address, mail, phone_number);

	result = sqlite3_exec(db, query, DatabaseCallback, 0, &errorMsg);

	if (result != SQLITE_OK) {
		fprintf(stderr, "Query error: %s" , sqlite3_errmsg(db));
		sqlite3_free(db);
		return;
	}
	else {
		fprintf(stderr, "User details added successfully.\n");
	}

	printf("\t\tUsername: ");
	scanf("%s", newUsername);
	flush();
	printf("\t\tPassword: ");
	scanf("%s", newPassword);
	flush();
	printf("\t\tPrivilege(student, teacher or admin): ");
	scanf("%s", newPrivilege);
	flush();

	sprintf(query, "INSERT INTO CREDENTIALS \
			VALUES (%i, '%s', '%s', '%s');", id, newUsername, newPassword, newPrivilege);

	result = sqlite3_exec(db, query, DatabaseCallback, 0, &errorMsg);

	if (result != SQLITE_OK) {
		fprintf(stderr, "Query error: %s" , sqlite3_errmsg(db));
		sqlite3_free(db);
		return;
	}
	else {
		fprintf(stderr, "User credentials added successfully\n.");
	}

	sqlite3_close(db);
}

void DatabaseRemoveUser() {
	sqlite3 *db;
	char query[255];
	char *errorMsg;
	bool result = false;
	int id;

	result = sqlite3_open(DATABASE, &db);

	if (result != SQLITE_OK) {
		fprintf(stderr, "Failed to connect to database: %s" , sqlite3_errmsg(db));
		sqlite3_free(db);
		return;
	}

	printf("\t\tID of user to be removed: ");
	scanf("%d", &id);
	flush();

	sprintf(query, "DELETE FROM PERSON WHERE ID = %d", id);

	result = sqlite3_exec(db, query, DatabaseCallback, 0, &errorMsg);

	if (result != SQLITE_OK) {
		fprintf(stderr, "Query error: %s" , sqlite3_errmsg(db));
		sqlite3_free(db);
		return;
	}
	else {
		fprintf(stderr, "User personal info removed successfully.\n");
	}

	sprintf(query, "DELETE FROM CREDENTIALS WHERE ID = %d", id);

	result = sqlite3_exec(db, query, DatabaseCallback, 0, &errorMsg);

	if (result != SQLITE_OK) {
		fprintf(stderr, "Query error: %s" , sqlite3_errmsg(db));
		sqlite3_free(db);
		return;
	}
	else {
		fprintf(stderr, "User credentials removed successfully.\n");
	}

	sqlite3_close(db);
}

void DatabaseUpdateUser() {
	sqlite3 *db;
	char query[255];
	char valueToUpdateStr[32];
	int valueToUpdateInt;
	char *errorMsg;
	bool result = false;
	unsigned int options = PERSONAL | CREDENTIALS | EXIT_USER_UPDATE;
	unsigned int optionsPersonal = UPDATE_FIRSTNAME | UPDATE_LASTNAME | UPDATE_AGE | UPDATE_ADDRESS | UPDATE_MAIL | UPDATE_PHONENUMBER | EXIT_USER_UPDATE;
	unsigned int optionsCredentials = UPDATE_USERNAME | UPDATE_PASSWORD | EXIT_USER_UPDATE;
	int id;

	result = sqlite3_open(DATABASE, &db);

	if (result != SQLITE_OK) {
		fprintf(stderr, "Failed to connect to database: %s" , sqlite3_errmsg(db));
		sqlite3_free(db);
		return;
	}

	if (userID == 1) {
		printf("\t\tID of user to be updated: ");
		scanf("%d", &id);
		flush();
	}
	else {
		id = userID;
	}

	HeaderText("Updating user", false);
	do {
		printf("\t\t(1) : Update personal information\n");
		printf("\t\t(2) : Update login credentials\n");
		printf("\t\t(9) : Exit user update menu\n");

		printf("\t\t>>> ");
		scanf("%u", &options);
		switch(options) {
			case PERSONAL: 
				do {
					printf("\t\t(1) : Update first name\n");
					printf("\t\t(2) : Update last name\n");
					printf("\t\t(3) : Update age\n");
					printf("\t\t(4) : Update address\n");
					printf("\t\t(5) : Update mail\n");
					printf("\t\t(6) : Update phone number\n");
					printf("\t\t(9) : Exit user update menu\n");

					printf("\t\t>>> ");
					scanf("%u", &optionsPersonal);

					switch(optionsPersonal) {
						case UPDATE_FIRSTNAME:
							printf("\t\tNew first name: ");
							scanf("%s", valueToUpdateStr);
							flush();

							sprintf(query, "UPDATE PERSON SET NAME = '%s' WHERE ID = %d", valueToUpdateStr, id);
							result = sqlite3_exec(db, query, DatabaseCallback, 0, &errorMsg);
							if (result != SQLITE_OK) {
								fprintf(stderr, "Query error: %s" , sqlite3_errmsg(db));
								sqlite3_free(db);
								return;
							}
							else {
								fprintf(stderr, "User first name updated successfully.\n");
							}
							break;

						case UPDATE_LASTNAME:
							printf("\t\tNew last name: ");
							scanf("%s", valueToUpdateStr);
							flush();

							sprintf(query, "UPDATE PERSON SET LASTNAME = '%s' WHERE ID = %d", valueToUpdateStr, id);
							result = sqlite3_exec(db, query, DatabaseCallback, 0, &errorMsg);
							if (result != SQLITE_OK) {
								fprintf(stderr, "Query error: %s" , sqlite3_errmsg(db));
								sqlite3_free(db);
								return;
							}
							else {
								fprintf(stderr, "User last name updated successfully.\n");
							}
							break;

						case UPDATE_AGE:
							printf("\t\tNew age: ");
							scanf("%d", &valueToUpdateInt);
							flush();

							sprintf(query, "UPDATE PERSON SET AGE = %d WHERE ID = %d", valueToUpdateInt, id);
							result = sqlite3_exec(db, query, DatabaseCallback, 0, &errorMsg);
							if (result != SQLITE_OK) {
								fprintf(stderr, "Query error: %s" , sqlite3_errmsg(db));
								sqlite3_free(db);
								return;
							}
							else {
								fprintf(stderr, "User age updated successfully.\n");
							}
							break;

						case UPDATE_ADDRESS:
							printf("\t\tNew address: ");
							scanf("%s", valueToUpdateStr);
							flush();

							sprintf(query, "UPDATE PERSON SET ADDRESS = '%s' WHERE ID = %d", valueToUpdateStr, id);
							result = sqlite3_exec(db, query, DatabaseCallback, 0, &errorMsg);
							if (result != SQLITE_OK) {
								fprintf(stderr, "Query error: %s" , sqlite3_errmsg(db));
								sqlite3_free(db);
								return;
							}
							else {
								fprintf(stderr, "User address updated successfully.\n");
							}
							break;

						case UPDATE_MAIL:
							printf("\t\tNew mail: ");
							scanf("%s", valueToUpdateStr);
							flush();

							sprintf(query, "UPDATE PERSON SET MAIL = '%s' WHERE ID = %d", valueToUpdateStr, id);
							result = sqlite3_exec(db, query, DatabaseCallback, 0, &errorMsg);
							if (result != SQLITE_OK) {
								fprintf(stderr, "Query error: %s" , sqlite3_errmsg(db));
								sqlite3_free(db);
								return;
							}
							else {
								fprintf(stderr, "User mail updated successfully.\n");
							}
							break;

						case UPDATE_PHONENUMBER:
							printf("\t\tNew phone number: ");
							scanf("%d", &valueToUpdateInt);
							flush();

							sprintf(query, "UPDATE PERSON SET PHONE_NUMBER = %d WHERE ID = %d", valueToUpdateInt, id);
							result = sqlite3_exec(db, query, DatabaseCallback, 0, &errorMsg);
							if (result != SQLITE_OK) {
								fprintf(stderr, "Query error: %s" , sqlite3_errmsg(db));
								sqlite3_free(db);
								return;
							}
							else {
								fprintf(stderr, "User phone number updated successfully.\n");
							}
							break;
						case EXIT_USER_UPDATE:
							break;
					}
				} while (optionsPersonal != EXIT_USER_UPDATE);
				break;

			case CREDENTIALS: 

				do {
					printf("\t\t(1) : Update username\n");
					printf("\t\t(2) : Update user password\n");
					printf("\t\t(9) : Exit user update menu\n");

					printf("\t\t>>> ");
					scanf("%u", &optionsCredentials);
					switch(optionsCredentials) {
						case UPDATE_USERNAME:
							printf("\t\tNew username: ");
							scanf("%s", valueToUpdateStr);
							flush();

							sprintf(query, "UPDATE CREDENTIALS SET USERNAME = '%s' WHERE ID = %d", valueToUpdateStr, id);
							result = sqlite3_exec(db, query, DatabaseCallback, 0, &errorMsg);
							if (result != SQLITE_OK) {
								fprintf(stderr, "Query error: %s" , sqlite3_errmsg(db));
								sqlite3_free(db);
								return;
							}
							else {
								fprintf(stderr, "User username updated successfully.\n");
							}
							break;

						case UPDATE_PASSWORD: 
							printf("\t\tNew password: ");
							scanf("%s", valueToUpdateStr);
							flush();

							sprintf(query, "UPDATE CREDENTIALS SET PASSWORD = '%s' WHERE ID = %d", valueToUpdateStr, id);
							result = sqlite3_exec(db, query, DatabaseCallback, 0, &errorMsg);
							if (result != SQLITE_OK) {
								fprintf(stderr, "Query error: %s" , sqlite3_errmsg(db));
								sqlite3_free(db);
								return;
							}
							else {
								fprintf(stderr, "User password updated successfully.\n");
							}
							break;
					}
				} while (optionsCredentials != EXIT_USER_UPDATE);

				break;
		}
	} while (options != EXIT_USER_UPDATE);

	sqlite3_close(db);
}

void DatabaseAdminUpdatePassword() {
	sqlite3 *db;
	char query[255];
	char valueToUpdateStr[32];
	char *errorMsg;
	bool result = false;

	result = sqlite3_open(DATABASE, &db);

	if (result != SQLITE_OK) {
		fprintf(stderr, "Failed to connect to database: %s" , sqlite3_errmsg(db));
		sqlite3_free(db);
		return;
	}

	printf("\t\t(Enter 0 to cancel)\n");
	printf("\t\tNew password: ");
	scanf("%s", valueToUpdateStr);
	flush();

	if (strcmp(valueToUpdateStr, "0") == 0) {
		fprintf(stderr, "Admin password update cancelled.\n");
		return;
	}
	else {
		sprintf(query, "UPDATE CREDENTIALS SET PASSWORD = '%s' WHERE ID = 1", valueToUpdateStr);

		result = sqlite3_exec(db, query, DatabaseCallback, 0, &errorMsg);
		if (result != SQLITE_OK) {
			fprintf(stderr, "Query error: %s" , sqlite3_errmsg(db));
			sqlite3_free(db);
			return;
		}
		else {
			fprintf(stderr, "Admin password updated successfully.\n");
		}
	}
}

void DatabaseViewUserInfo() {
	sqlite3 *db;
	char query[255];
	char *errorMsg;
	bool result = false;

	result = sqlite3_open(DATABASE, &db);

	if (result != SQLITE_OK) {
		fprintf(stderr, "Failed to connect to database: %s" , sqlite3_errmsg(db));
		sqlite3_free(db);
		return;
	}

	sprintf(query, "SELECT * FROM PERSON WHERE ID = %d", userID);

	result = sqlite3_exec(db, query, DatabaseCallbackViewUserInfo, 0, &errorMsg);
	if (result != SQLITE_OK) {
		fprintf(stderr, "Query error: %s" , sqlite3_errmsg(db));
		sqlite3_free(db);
		return;
	}
	else {
		fprintf(stderr, "Viewing user info successfully.\n");
	}

	sqlite3_close(db);
}

void DatabaseAddCourse() {
	sqlite3 *db;
	char query[255];
	char *errorMsg;
	bool result = false;
	srand(time(NULL));
	int id = rand();
	char courseCode[6];
	char courseName[32];
	char teacher[32];
	char description[255];
	int spotsMax;
	int spotsAvailable;
	int spotsUnavailable = 0;
	char students[1000];
	bool finishedCourse = false;
	char dateInputStart[32];
	char dateInputFinish[32];
	char *parse;
	struct tm startDate, finishDate;
	time_t startDate_t, finishDate_t;

	//startDate.tm_year = 2023-1900;
	//startDate.tm_mon = 6;
	//startDate.tm_mday = 24;

	// Initialize rest of tm struct
	startDate.tm_hour = 0;
	startDate.tm_min = 0;
	startDate.tm_sec = 0;
	startDate.tm_isdst = 0;

	finishDate.tm_hour = 0;
	finishDate.tm_min = 0;
	finishDate.tm_sec = 0;
	finishDate.tm_isdst = 0;

	result = sqlite3_open(DATABASE, &db);

	if (result != SQLITE_OK) {
		fprintf(stderr, "Failed to connect to database: %s" , sqlite3_errmsg(db));
		sqlite3_free(db);
		return;
	}

	printf("\t\tCourse name: ");
	scanf("%s", courseName);
	flush();
	printf("\t\tCourse code: ");
	scanf("%s", courseCode);
	flush();
	printf("\t\tTeacher's name: ");
	scanf("%s", teacher);
	flush();
	printf("\t\tDescription of the course: ");
	scanf("%s", description);
	flush();
	printf("\t\tMax spots available: ");
	scanf("%d", &spotsMax);
	flush();
	spotsAvailable = spotsMax;

	printf("\t\tStart date (dd-mm-yyyy): ");
	scanf("%s", dateInputStart);
	flush();
	// Parse and store the date in the respective
	// struct field.
	parse = strtok(dateInputStart, "-");
	sscanf(parse, "%d", &startDate.tm_mday);
	parse = strtok('\0', "-");
	sscanf(parse, "%d", &startDate.tm_mon);
	parse = strtok('\0', "-");
	sscanf(parse, "%d", &startDate.tm_year);

	printf("\t\tFinish date (dd-mm-yyyy): ");
	scanf("%s", dateInputFinish);
	flush();

	parse = strtok(dateInputFinish, "-");
	sscanf(parse, "%d", &finishDate.tm_mday);
	parse = strtok('\0', "-");
	sscanf(parse, "%d", &finishDate.tm_mon);
	parse = strtok('\0', "-");
	sscanf(parse, "%d", &finishDate.tm_year);

	sprintf(query, "INSERT INTO COURSES \
			VALUES ('%s', '%s', '%s', '%s', %d, %d, %d, '%s', %d, '%s', '%s');", courseCode, courseName, teacher, description, spotsMax, spotsAvailable, spotsUnavailable, students, finishedCourse, dateInputStart, dateInputFinish);

	result = sqlite3_exec(db, query, DatabaseCallback, 0, &errorMsg);

	if (result != SQLITE_OK) {
		fprintf(stderr, "Query error: %s" , sqlite3_errmsg(db));
		sqlite3_free(db);
		return;
	}
	else {
		fprintf(stderr, "New course added successfully.\n");
	}

	sqlite3_close(db);
}

void CourseMenu() {
	CLEAR_SCREEN;
	RESET_CURSOR;

	// Admin
	if (strcmp(privilege, "admin") == 0) {

		unsigned int options = COURSES_VIEW_ALL | COURSES_ADD_NEW | COURSES_REMOVE | COURSES_STUDENT_ADD | COURSES_EDIT_GRADE | COURSES_STUDENT_REMOVE | COURSES_EXIT;

		do {
			HeaderText("Courses Menu", false);
			printf("\t\tWith great power, comes great responsibility.\n");
			printf("\t\t(1) : View all courses\n");
			printf("\t\t(2) : Add new course\n");
			printf("\t\t(3) : Remove course\n");
			printf("\t\t(4) : Add student to course\n");
			printf("\t\t(5) : Edit grade for student in a course\n");
			printf("\t\t(6) : Exit Courses Menu\n");

			printf("\t\t>>> ");
			scanf("%u", &options);
			flush();
			switch(options) {
				case COURSES_VIEW_ALL: 
					printf("Viewing courses...\n");
					DatabaseViewCoursesInfo();
					break;
				case COURSES_ADD_NEW: 
					printf("Adding a new course...\n");
					break;
				case COURSES_REMOVE: 
					printf("Removing a course...\n");
					break;
				case COURSES_STUDENT_ADD: 
					printf("Adding a student to course...\n");
					break;
				case COURSES_EDIT_GRADE: 
					printf("Editing student grade...\n");
					break;
				case COURSES_STUDENT_REMOVE: 
					printf("Removing student from a course...\n");
					break;
			}
		} while (options != COURSES_EXIT);

	}
	// Teacher
	else if (strcmp(privilege, "teacher") == 0) {
		unsigned int options = COURSES_VIEW_ALL | COURSES_ADD_NEW | COURSES_REMOVE | COURSES_STUDENT_ADD | COURSES_EDIT_GRADE | COURSES_STUDENT_REMOVE | COURSES_EXIT;

		do {
			HeaderText("Courses Menu", false);
			printf("\t\tWith great power, comes great responsibility.\n");
			printf("\t\t(1) : View all courses\n");
			printf("\t\t(2) : Add new course\n");
			printf("\t\t(3) : Remove course\n");
			printf("\t\t(4) : Add student to course\n");
			printf("\t\t(5) : Edit grade for student in a course\n");
			printf("\t\t(6) : Exit Courses Menu\n");

			printf("\t\t>>> ");
			scanf("%u", &options);
			flush();
			switch(options) {
				case COURSES_VIEW_ALL: 
					printf("Viewing courses...\n");
					break;
				case COURSES_ADD_NEW: 
					printf("Adding a new course...\n");
					break;
				case COURSES_REMOVE: 
					printf("Removing a course...\n");
					break;
				case COURSES_STUDENT_ADD: 
					printf("Adding a student to course...\n");
					break;
				case COURSES_EDIT_GRADE: 
					printf("Editing student grade...\n");
					break;
				case COURSES_STUDENT_REMOVE: 
					printf("Removing student from a course...\n");
					break;
			}
		} while (options != COURSES_EXIT);
	}
	// Student
	else {

		unsigned int options = VIEW_COURSES | VIEW_USER_INFO | EDIT_PERSONAL_CREDENTIALS_INFO | SIGN_OUT_STUDENT;

		do {
			HeaderText("Courses Menu", false);
			printf("\t\tYour average grade is: [PLACEHOLDER]\n");
			printf("\t\t(1) : View courses\n");
			printf("\t\t(2) : View personal info.\n");
			printf("\t\t(3) : Change personal info or credentials.\n");
			printf("\t\t(4) : Sign Out\n");

			printf("\t\t>>> ");
			scanf("%u", &options);
			flush();
			switch(options) {
				case VIEW_COURSES: 
					printf("Viewing courses...\n");
					break;
				case VIEW_USER_INFO: 
					DatabaseViewUserInfo();
					break;
				case EDIT_PERSONAL_CREDENTIALS_INFO: 
					printf("Editing info...\n");
					DatabaseUpdateUser();
					break;
			}
		} while (options != SIGN_OUT_STUDENT);

	}
}

void DatabaseViewCoursesInfo() {
	sqlite3 *db;
	sqlite3_stmt *stmt;
	char *sql;
	bool result = false;
	WINDOW *viewCoursesInfoWindow;
	int height = 20, width = 120;
	int pstarty = 4, pstartx = 2;
	int c, i;
	int columnLength = width / 6;	// 6 columns

	result = sqlite3_open(DATABASE, &db);

	if (result != SQLITE_OK) {
		fprintf(stderr, "Failed to connect to database: %s" , sqlite3_errmsg(db));
		sqlite3_free(db);
		return;
	}

	curs_set(1);
	echo();
	init_pair(2, COLOR_CYAN, COLOR_BLACK);

	viewCoursesInfoWindow = newwin(height, width, (starty / 2) - (height + height) / 2, (startx / 2) - (width / 2));
	keypad(viewCoursesInfoWindow, TRUE);
	
	mvwaddch(viewCoursesInfoWindow, 2, 0, ACS_LTEE);
	mvwhline(viewCoursesInfoWindow, 2, 1, ACS_HLINE, width);
	mvwaddch(viewCoursesInfoWindow, 2, width - 1, ACS_RTEE);

	wattr_on(viewCoursesInfoWindow, A_UNDERLINE | A_BOLD, NULL);
	mvwprintw(viewCoursesInfoWindow, pstarty, pstartx, "Name");
	pstartx += columnLength;
	mvwprintw(viewCoursesInfoWindow, pstarty, pstartx, "Teacher");
	pstartx += columnLength;
	mvwprintw(viewCoursesInfoWindow, pstarty, pstartx, "Spots Available");
	pstartx += columnLength;
	mvwprintw(viewCoursesInfoWindow, pstarty, pstartx, "Finished");
	pstartx += columnLength;
	mvwprintw(viewCoursesInfoWindow, pstarty, pstartx, "Start date");
	pstartx += columnLength;
	mvwprintw(viewCoursesInfoWindow, pstarty, pstartx, "Finish date");
	wattr_off(viewCoursesInfoWindow,A_UNDERLINE | A_BOLD, NULL);

	pstarty += 2;
	pstartx = 2;
	
	//sprintf(query, "SELECT course_name, course_teacher_id, spots_available, finished, start_date, finish_date FROM Courses");
	sql =  "SELECT course_name, course_teacher_id, spots_available, finished, start_date, finish_date FROM Courses";
	//sql =  "SELECT course_name, course_teacher_id, spots_available, finished, start_date, finish_date FROM Courses";
	sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL);

	while(sqlite3_step(stmt) == SQLITE_ROW) {

		for (i = 0; i < sqlite3_column_count(stmt); i++) {
			mvwprintw(viewCoursesInfoWindow, pstarty, pstartx, "%s", sqlite3_column_text(stmt, i));
			pstartx += columnLength;
		}
		pstarty += 2;
		pstartx = 2;
	}
	sqlite3_finalize(stmt);

	mvwprintw(viewCoursesInfoWindow, height - 2, width / 2, "Press any key to go back");

	DrawWindowBorder(viewCoursesInfoWindow, " VIEW COURSES ", COLOR_PAIR(2));

	wrefresh(viewCoursesInfoWindow);
	wrefresh(stdscr);
	c = wgetch(viewCoursesInfoWindow);

	if (c == KEY_F(1)) {
		die(viewCoursesInfoWindow);
	}

	wclear(viewCoursesInfoWindow);
	wrefresh(viewCoursesInfoWindow);
	delwin(viewCoursesInfoWindow);

	curs_set(0);
	noecho();
	sqlite3_close(db);
}

WINDOW *create_newwin() {
	WINDOW *local_win;
	// Variables defined in sms.h
	// Sets the window to upper center
	local_win = newwin(HEIGHT, WIDTH, (starty / 2) - (HEIGHT + HEIGHT/2), (startx / 2) - (WIDTH / 2));
	box(local_win, 0 , 0);
	/* 0, 0 gives default characters
	* for the vertical and horizontal
	* lines
	*/
	wrefresh(local_win);
	/* Show that box
	*/
	return local_win;
}

void PrintInMiddle(WINDOW *localWin, int starty, int startx, char *string, chtype color) {
	int y, x, strLength;

	getmaxyx(localWin, y, x);
	strLength = strlen(string);

	wattron(localWin, A_REVERSE | COLOR_PAIR(1));
	mvwprintw(localWin, starty, ( x - strLength )/2, "%s", string);
	wattroff(localWin, A_REVERSE | COLOR_PAIR(1));
	wrefresh(localWin);
	refresh();
}

void MainMenu() {
	WINDOW *mainMenuWin;
	MENU *mainMenu;
	ITEM **mainMenuItems;
	ITEM *mainMenuSelectedItem;
	const char *mainMenuSelectedItemStr;
	int c;
	int numChoices, i;
	getmaxyx(stdscr, starty, startx);

	init_pair(1, COLOR_CYAN, COLOR_BLACK);

	numChoices = ARRAY_SIZE(mainMenuChoices);
	mainMenuItems = (ITEM **)calloc(numChoices + 1, sizeof(ITEM *));

	for (i = 0; i < numChoices; ++i) {
		mainMenuItems[i] = new_item(mainMenuChoices[i], "");
	}

	mainMenu = new_menu(mainMenuItems);
	mainMenuWin = create_newwin();
	keypad(mainMenuWin, TRUE);

	set_menu_win(mainMenu, mainMenuWin);
	set_menu_sub(mainMenu, derwin(mainMenuWin, HEIGHT - 4, WIDTH - 4, 4, 1));

	set_menu_mark(mainMenu, " >> ");

	post_menu(mainMenu);

	DrawWindowBorder(mainMenuWin, " MAIN MENU ", COLOR_PAIR(1));

	while ( (c = wgetch(mainMenuWin)) != KEY_F(1) ) {
		switch(c) {
			case KEY_UP:
				menu_driver(mainMenu, REQ_UP_ITEM);
				break;
			case KEY_DOWN:
				menu_driver(mainMenu, REQ_DOWN_ITEM);
				break;
			// VIM Keybindings
			case 107:	
				menu_driver(mainMenu, REQ_UP_ITEM);
				break;
			case 106:
				menu_driver(mainMenu, REQ_DOWN_ITEM);
				break;
			case 10:	// RETURN, Select an item
				mainMenuSelectedItem = current_item(mainMenu);
				mainMenuSelectedItemStr = item_name(mainMenuSelectedItem);
				mvwprintw(stdscr, 2, 0, "Selected: %s", mainMenuSelectedItemStr);
				wrefresh(stdscr);

				if (mainMenuSelectedItemStr == "About") {
					AboutMenu();
					DrawWindowBorder(mainMenuWin, " MAIN MENU ", COLOR_PAIR(1));
					break;
				}
				else if (mainMenuSelectedItemStr == "Sign in") {
					CheckCredentialsWindow();
					DrawWindowBorder(mainMenuWin, " MAIN MENU ", COLOR_PAIR(1));
					break;
				}
				break;
		}
	}

	DestroyWinMenu(mainMenuWin, mainMenu, mainMenuItems, numChoices);
};

void DrawWindowBorder(WINDOW *localWin, char *string, chtype color) {
	box(localWin, 0, 0);
	PrintInMiddle(localWin, 1, WIDTH / 2, string, color);

	mvwaddch(localWin, 2, 0, ACS_LTEE);
  mvwhline(localWin, 2, 1, ACS_HLINE, 38);
  mvwaddch(localWin, 2, 39, ACS_RTEE);

	wrefresh(localWin);
}

void AboutMenu() {
	int c;
	WINDOW *aboutWin;

	init_pair(2, COLOR_CYAN, COLOR_BLACK);

	aboutWin = create_newwin();
	keypad(aboutWin, TRUE);

	DrawWindowBorder(aboutWin, " ABOUT ", COLOR_PAIR(2));

	mvwprintw(aboutWin, 4, 2, "This software is produced and");
	mvwprintw(aboutWin, 5, 2, "maintained by Mohamad Farman.");
	mvwprintw(aboutWin, 6, 2, "Any inquires can be sent to");
	mvwprintw(aboutWin, 7, 2, "mohamadfarman@gmail.com");
	mvwprintw(aboutWin, 12, 8, "Press any key to go back");
	wrefresh(aboutWin);
	wrefresh(stdscr);
	c = wgetch(aboutWin);

	if (c == KEY_F(1)) {
		die(aboutWin);
	}

	wclear(aboutWin);
	wrefresh(aboutWin);
	delwin(aboutWin);
}

int SetUserMenu(WINDOW *localWin, MENU *localMenu, ITEM **localMenuItems) {
	int numChoices, i;

	getmaxyx(stdscr, starty, startx);

	init_pair(1, COLOR_CYAN, COLOR_BLACK);

	// Initialize the correct menu based on privilege
	if (strcmp(privilege, "admin")) {
		numChoices = ARRAY_SIZE(adminMenuChoices);
		localMenuItems = (ITEM **)calloc(numChoices + 1, sizeof(ITEM *));

		for (i = 0; i < numChoices; ++i) {
			localMenuItems[i] = new_item(adminMenuChoices[i], "");
		}
	}
	else if (strcmp(privilege, "teacher")) {
		numChoices = ARRAY_SIZE(teacherMenuChoices);
		localMenuItems = (ITEM **)calloc(numChoices + 1, sizeof(ITEM *));

		for (i = 0; i < numChoices; ++i) {
			localMenuItems[i] = new_item(teacherMenuChoices[i], "");
		}
	}
	else {
		numChoices = ARRAY_SIZE(studentMenuChoices);
		localMenuItems = (ITEM **)calloc(numChoices + 1, sizeof(ITEM *));

		for (i = 0; i < numChoices; ++i) {
			localMenuItems[i] = new_item(studentMenuChoices[i], "");
		}
	}

	localMenu = new_menu(localMenuItems);
	localWin = create_newwin();
	keypad(localWin, TRUE);

	set_menu_win(localMenu, localWin);
	set_menu_sub(localMenu, derwin(localWin, HEIGHT - 4, WIDTH - 4, 4, 1));

	set_menu_mark(localMenu, " ** ");

	post_menu(localMenu);

	DrawWindowBorder(localWin, privilege, COLOR_PAIR(1));
	return numChoices;
}

void DestroyWinMenu(WINDOW *localWin, MENU *localMenu, ITEM **localMenuItems, int numChoices) {
	int i;
	unpost_menu(localMenu);
	free_menu(localMenu);
	for (i = 0; i < numChoices; ++i) {
		free_item(localMenuItems[i]);
	}

	delwin(localWin);

}

int flush() {
	int ch;
	while ((ch = getchar()) != EOF && ch != '\n') ;
	return 0;
}

void die(WINDOW *localWin) {
	delwin(localWin);
	endwin();
	exit(EXIT_SUCCESS);
}
