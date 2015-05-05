#-----------------------------------------------------
#File: Makefile
#(c) 2015 - Emanuel Valente - emanuelvalente@gmail.com
#
#--------------------------------------------------------

CC=gcc
CFLAGS=-Wall

all:sproxy cproxy

sproxy: sproxy.c protocol.h
	$(CC) $(CFLAGS) $< -o $@

cproxy: cproxy.c protocol.h
	$(CC) $(CFLAGS) $< -o $@

debug:CFLAGS += -g
debug: all

clean:
	rm -rf *~ sproxy cproxy

	
	

