#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "sqlite3.h"
#include "sms.h"

void HeaderText(char* str, bool about);
void CheckCredentials();
void PrintAbout();
bool DatabaseInit();
void DatabaseAddUser();
void UserMenu(char *privilege);
int flush();

static int DatabaseCallbackSelectSignIn(void *data, int argc, char **argv, char **azColName);
static int DatabaseCallbackInsertUser(void *NotUsed, int argc, char **argv, char **azColName);

static int DatabaseCallbackTableCreate(void *NotUsed, int argc, char **argv, char **azColName);

char username[32];
char password[32];
int credentials = 0;
char privilege[10];

int main() {
	bool dbConnect = false;
	bool about = false;
	unsigned int options = SIGN_IN | ABOUT | EXIT_PROGRAM;

	do {
		HeaderText("Main Menu", about);
		printf("\t\t(1) : Sign In\n");
		printf("\t\t(2) : About\n");
		printf("\t\t(3) : EXIT\n");

		printf("\t\t>>> ");
		scanf("%u", &options);
		switch(options) {
			case SIGN_IN: 
				CheckCredentials();
				if (credentials) { 
					printf("User signed in!\n"); 
					//DatabaseInit();
					UserMenu(privilege);
				}
				else { printf("User failed to sign in!\n"); }
				break;
			case ABOUT: 
				about = true;
				break;
		}
	} while (options != EXIT_PROGRAM);

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

void CheckCredentials() {
	sqlite3 *db;
	char *query;
	int resultDb;
	int result;
	char *errorMsg;
	// Keeps track of number of tries to sign in.
	int counter = 5;

	resultDb = sqlite3_open(DATABASE, &db);

	if (resultDb) {
		fprintf(stderr, "Failed to connect to database: %s" , sqlite3_errmsg(db));
	}

	HeaderText("Sign in page", false);
	do {
		if (counter <= 3 && counter > 0) { 
			printf("\t\t%d number of tries left.\n", counter); 
		}
		else if (counter <= 0){
			break;
		}
		printf("\t\tUsername: ");
		scanf("%s", username);
		printf("\t\tPassword: ");
		scanf("%s", password);

		// Check the CREDENTIALS table
		query = "SELECT USERNAME, PASSWORD, PRIVILEGE FROM CREDENTIALS";
		result = sqlite3_exec(db, query, DatabaseCallbackSelectSignIn, 0, &errorMsg);

		if (result != SQLITE_OK) {
			fprintf(stderr, "Query error: %s", errorMsg); return;
			return;
		}

		if (credentials == 2) {
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


// Create PERSON table
// Create SQL statement
//query = "CREATE TABLE PERSON("  \
//				 "ID						INT PRIMARY KEY			NOT NULL," \
//				 "NAME						TEXT    NOT NULL," \
//				 "LASTNAME				TEXT    NOT NULL," \
//				 "AGE							INT     NOT NULL," \
//				 "ADDRESS					CHAR(50)," \
//				 "MAIL						TEXT		NOT NULL );"
//				 "PHONE_NUMBER		INT);";
//				 // Execute SQL statement
//				 result = sqlite3_exec(db, query, DatabaseCallbackTableCreate, 0, &errorMsg);
//
//				 if (result != SQLITE_OK) {
//					 fprintf(stderr, "Query error: %s", errorMsg);
//					 return false;
//				 }
//else {
//	fprintf(stdout, "Table PERSON created successfully.\n");
//}
//}
//
//query = "SELECT * FROM CREDENTIALS";
//result = sqlite3_exec(db, query, DatabaseCallbackSelect, 0, &errorMsg);
//
//if (result != SQLITE_OK) {
//	fprintf(stderr, "Query error. CREDENTIALS table not found: %s", errorMsg);
//
//	// Create PERSON table
//	// Create SQL statement
//	query = "CREATE TABLE CREDENTIALS("  \
//					 "ID						INT PRIMARY KEY			NOT NULL," \
//					 "NAME						TEXT    NOT NULL," \
//					 "LASTNAME				TEXT    NOT NULL," \
//					 "AGE							INT     NOT NULL," \
//					 "ADDRESS					CHAR(50)," \
//					 "MAIL						TEXT		NOT NULL );"
//					 "PHONE_NUMBER		INT);";
//	// Execute SQL statement
//	result = sqlite3_exec(db, query, DatabaseCallbackTableCreate, 0, &errorMsg);
//
//	if (result != SQLITE_OK) {
//		fprintf(stderr, "Query error: %s", errorMsg);
//		return false;
//	}
//	else {
//		fprintf(stdout, "Table CREDENTIALS created successfully.\n");
//	}
//}
//
//return true;
//}
//}

void UserMenu(char *privilege) {
	// Admin
	if (strcmp(privilege, "admin") == 0) {

		unsigned int options = USER_ADD | USER_REMOVE | USER_EDIT | CHANGE_ADMIN_PASS | SIGN_OUT_ADMIN;

		do {
			HeaderText("Admin Menu", false);
			printf("\t\tWith great power, comes great responsibility.\n");
			printf("\t\t(1) : Add new user\n");
			printf("\t\t(2) : Remove user\n");
			printf("\t\t(3) : Edit user info\n");
			printf("\t\t(4) : Change admin password\n");
			printf("\t\t(5) : Sign out\n");

			printf("\t\t>>> ");
			scanf("%u", &options);
			switch(options) {
				case USER_ADD: 
					printf("Adding user...\n");
					DatabaseAddUser();
					break;
				case USER_REMOVE: 
					printf("Removing user...\n");
					break;
				case USER_EDIT: 
					printf("Editing user...\n");
					break;
				case CHANGE_ADMIN_PASS: 
					printf("Changing admin password...\n");
					break;
			}
		} while (options != SIGN_OUT_ADMIN);

	}
	// Teacher
	else if (strcmp(privilege, "teacher") == 0) {

	}
	// Student
	else {

		unsigned int options = VIEW_COURSES | PERSONAL_INFO | SIGN_OUT;

		do {
			HeaderText("Student Menu", false);
			printf("\t\tYour average grade is: [PLACEHOLDER]\n");
			printf("\t\t(1) : View courses\n");
			printf("\t\t(2) : Personal information\n");
			printf("\t\t(3) : Sign Out\n");

			printf("\t\t>>> ");
			scanf("%u", &options);
			switch(options) {
				case VIEW_COURSES: 
					printf("Viewing courses...\n");
					break;
				case PERSONAL_INFO: 
					printf("Editing personal info...\n");
					break;
			}
		} while (options != SIGN_OUT);

	}

	// When a user signs out reset necessary global variables
	strcpy(username, "");
	strcpy(password, "");
	strcpy(privilege, "");
	credentials = 0;
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
	fprintf(stderr, "%s: ", (const char*)data);

	for(i = 0; i<argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");

	for(i = 0; i<argc; i++) {
		printf("Inside for: %s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		if (strcmp(azColName[i], "USERNAME") == 0) 
		{
			printf("Inside if: %s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
			if (strcmp(argv[i], username) == 0) {
				//printf("Password found (password = %s)!: %s\n", password, argv[i]);
				credentials++;
			}
			else { credentials = 0; }
		}
		if (strcmp(azColName[i], "PASSWORD") == 0) 
		{
			printf("Inside if: %s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
			if (strcmp(argv[i], password) == 0) {
				printf("Password found (password = %s)!: %s\n", password, argv[i]);
				credentials++;
			}
			else { credentials = 0; }
		}
		if (strcmp(azColName[i], "PRIVILEGE") == 0) 
		{
			printf("Inside if: %s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
			if (strcmp(argv[i], "admin") == 0) {
				strcpy(privilege, "admin");
			}
			else if (strcmp(argv[i], "teacher") == 0) { 
				strcpy(privilege, "teacher");
			}
			else { strcpy(privilege, "teacher"); }
		}
	}

	printf("\n");
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

void DatabaseAddUser() {
	sqlite3 *db;
	char query[255];
	char *errorMsg;
	bool result = false;
	//int i, stime;
	//long ltime;

	//ltime = time(NULL);
	//stime = (unsigned) ltime/2;
	//srand(stime);

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

	result = sqlite3_exec(db, query, DatabaseCallbackInsertUser, 0, &errorMsg);

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

	result = sqlite3_exec(db, query, DatabaseCallbackInsertUser, 0, &errorMsg);

	if (result != SQLITE_OK) {
		fprintf(stderr, "Query error: %s" , sqlite3_errmsg(db));
		sqlite3_free(db);
		return;
	}
	else {
		fprintf(stderr, "User credentials added successfully\n.");
	}

	sqlite3_close(db);

	//query = "CREATE TABLE PERSON("  \
	//				 "ID						INT PRIMARY KEY			NOT NULL," \
	//				 "NAME						TEXT    NOT NULL," \
	//				 "LASTNAME				TEXT    NOT NULL," \
	//				 "AGE							INT     NOT NULL," \
	//				 "ADDRESS					CHAR(50)," \
	//				 "MAIL						TEXT		NOT NULL );"
	//				 "PHONE_NUMBER		INT);";
	//				 // Execute SQL statement
	//				 result = sqlite3_exec(db, query, DatabaseCallbackTableCreate, 0, &errorMsg);
}

int flush() {
	int ch;
	while ((ch = getchar()) != EOF && ch != '\n') ;
	return 0;
}
