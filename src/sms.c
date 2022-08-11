#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sms.h"

void HeaderText(char* str);

int main() {
	char* headerSignIn = "Sign in page";
	HeaderText(headerSignIn);

	unsigned int options = STUDENT_PORTAL | TEACHER_PORTAL | ADMIN_PORTAL | ABOUT | EXIT_PROGRAM;

	do {
		printf("\t\t(1) : Student Portal\n");
		printf("\t\t(2) : Teacher Portal\n");
		printf("\t\t(3) : Admin Portal\n");
		printf("\t\t(4) : About\n");
		printf("\t\t(5) : EXIT\n");

		printf("\t\t>>> ");
		scanf("%u", &options);
		switch(options) {
			case STUDENT_PORTAL: printf("STUDENT_PORTAL\n");
													 break;
			case TEACHER_PORTAL: printf("TEACHER_PORTAL\n");
													 break;
			case ADMIN_PORTAL: printf("ADMIN_PORTAL\n");
													 break;
			case ABOUT: printf("ABOUT\n");
													 break;
		}
	} while (options != EXIT_PROGRAM);

	return 0;
}

void HeaderText(char* str) {
	CLEAR_SCREEN;
	RESET_CURSOR;
	
	// print Header
	printf("\t\t* * * * * * * * * * * * * * * * * * * * *\t\t\n");
	printf("\t\t* * * * * * * * * * * * * * * * * * * * *\t\t\n");
	printf("\t\t*\tStudent Management System\t*\t\t\n");
	printf("\t\t*\t\t%s\t\t*\t\t\n", str);
	printf("\t\t* * * * * * * * * * * * * * * * * * * * *\t\t\n");
	printf("\t\t* * * * * * * * * * * * * * * * * * * * *\t\t\n");
}
