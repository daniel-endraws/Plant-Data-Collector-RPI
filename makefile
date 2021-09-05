# CPP Compiler
CC = g++
# Add debug info and turn on warnings
CFLAGS  = -g -Wall
# Use rf24 and sqlite3 libraries
LIBS = -lrf24 -lsqlite3

default: collect_data

collect_data:
	$(CC) $(CFLAGS) $(LIBS) collect_data.cpp -o collect_data
