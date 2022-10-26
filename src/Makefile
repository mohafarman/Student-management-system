PROGRAM=sms
COMPILER=c99
OPTIONS= -g -Wall -Wextra -pedantic -std=c99
LDLIBS= -lmenu -lncurses -lform -lsqlite3
COMPILE=$(COMPILER) $(OPTIONS) $(LDLIBS)

PROGRAM: sms.c
	$(COMPILE) sms.c -o a.out
