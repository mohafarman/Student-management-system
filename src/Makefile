PROGRAM=sms
COMPILER=c99
OPTIONS= -g -Wall -Wextra -pedantic -std=c99
LDLIBS= -lmenu -lncurses -lsqlite3
COMPILE=$(COMPILER) $(OPTIONS) $(LDLIBS)

PROGRAM: sms.c
	$(COMPILE) sms.c -o a.out
#PROGRAM: test.c
#	$(COMPILE) test.c -o test.out
