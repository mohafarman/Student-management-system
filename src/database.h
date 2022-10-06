#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistdio.h>

#include "sms.h"

#define DATABASE "sms.db"

//****************** PROTOTYPES ******************//

static void db_connect(sqlite3 *db);
static int callback_sign_in(void *data, int argc, char **argv, char **azColName);

//***************** DEFINITIONS ******************//
static void db_connect(sqlite3 *db) {
	int result;
	result = sqlite3_open(DATABASE, &db);

	if (result) {
		fprintf(stderr, "Failed to connect to database %s\n", DATABASE);
		exit(EXIT_FAILURE);
	}
}


/**
 *	@brief Assigns correct privilege based on database callback information
 */
static int callback_sign_in(void *data, int argc, char **argv, char **azColName) {
	int i = 0;

	if (argc) {
		credentials = true;
	}
	else { return 1; }

	for (i = 0; i < argc; i++) {
		if (strcmp(azColName[i], "credentials_id") == 0) {
			sscanf(argv[i], "%d", &user_id);
		}

		if (strcmp(azColName[i], "privilege") == 0) {

			if (strcmp(argv[i], "admin") == 0) {
				strcpy(privilege, argv[i]);
			}
			else if (strcmp(argv[i], "teacher") == 0) {
				strcpy(privilege, argv[i]);
			}
			else { strcpy(privilege, argv[i]); }
		}
	}

	return 0;
}

#endif
