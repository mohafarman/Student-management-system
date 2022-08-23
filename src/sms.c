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
void DatabaseRemoveUser();
void DatabaseUpdateUser();
void DatabaseViewUserInfo();
void DatabaseAdminUpdatePassword();
void UserMenu(char *privilege);
int flush();

static int DatabaseCallbackSelectSignIn(void *data, int argc, char **argv, char **azColName);
static int DatabaseCallback(void *NotUsed, int argc, char **argv, char **azColName);
static int DatabaseCallbackViewUserInfo(void *NotUsed, int argc, char **argv, char **azColName);
static int DatabaseCallbackTableCreate(void *NotUsed, int argc, char **argv, char **azColName);

char username[32];
char password[32];
bool credentials = false;
char privilege[10];
int userID;

int main() {
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
	CLEAR_SCREEN;
	RESET_CURSOR;

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
	char query[255];
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

void UserMenu(char *privilege) {
	CLEAR_SCREEN;
	RESET_CURSOR;

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
			flush();
			switch(options) {
				case USER_ADD: 
					printf("Adding user...\n");
					DatabaseAddUser();
					break;
				case USER_REMOVE: 
					printf("Removing user...\n");
					DatabaseRemoveUser();
					break;
				case USER_EDIT: 
					printf("Editing user...\n");
					DatabaseUpdateUser();
					break;
				case CHANGE_ADMIN_PASS: 
					printf("Changing admin password...\n");
					DatabaseAdminUpdatePassword();
					break;
			}
		} while (options != SIGN_OUT_ADMIN);

	}
	// Teacher
	else if (strcmp(privilege, "teacher") == 0) {

		unsigned int options = VIEW_COURSES | VIEW_USER_INFO | EDIT_PERSONAL_CREDENTIALS_INFO | SIGN_OUT_TEACHER;

		do {
			HeaderText("Teacher Menu", false);
			printf("\t\t(1) : View courses\n");
			printf("\t\t(2) : View personal info\n");
			printf("\t\t(3) : Edit personal info or change credentials\n");
			printf("\t\t(4) : Sign out\n");

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
					printf("Updating user info...\n");
					break;
			}
		} while (options != SIGN_OUT_TEACHER);
	}
	// Student
	else {

		unsigned int options = VIEW_COURSES | VIEW_USER_INFO | EDIT_PERSONAL_CREDENTIALS_INFO | SIGN_OUT_STUDENT;

		do {
			HeaderText("Student Menu", false);
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
					break;
			}
		} while (options != SIGN_OUT_STUDENT);

	}

	// When a user signs out reset necessary global variables
	username[0] = 0;
	password[0] = 0;
	privilege[0] = 0;
	credentials = false;
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

	for(i = 0; i<argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");

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

		if (strcmp(azColName[i], "PRIVILEGE") == 0) 
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
	printf("\n");

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

void DatabaseAddUser() {
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

	printf("\t\tID of user to be updated: ");
	scanf("%d", &id);
	flush();

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

int flush() {
	int ch;
	while ((ch = getchar()) != EOF && ch != '\n') ;
	return 0;
}
