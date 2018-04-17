###########################################################################
#
# Author:
# 					Ashkan Moatamed
#
#
#
# Makefile for alarm_app.c
#
###########################################################################

# C Compiler
CC = \gcc

# Compiler Flags
CFLAGS = -ansi -pedantic -pedantic-errors \
	-Wall -Wextra -Werror -Wconversion \
	-Wno-format-nonliteral -Wpointer-arith -Wcast-qual \
	-Wstrict-prototypes -Wmissing-prototypes -Waggregate-return \
	-Wmissing-declarations -Wold-style-definition \
	-Wno-missing-braces -Wunreachable-code -Wredundant-decls \
	-Wswitch-default -Wswitch-enum -fshort-enums \
	-Wnested-externs -Wshadow -Wwrite-strings \
	-W -g -O3

# Linker Flags
LFLAGS = -D_POSIX_PTHREAD_SEMANTICS -pthread



# C Header Files
LIB = $(wildcard *.h)
# C Source Files
SRC = $(wildcard *.c)
# C Object Files
OBJ = $(SRC:.c=.o)

# Application Executable
EXE = alarm_app
# Application Log File
LOG = App_Log.txt



# Link all Object Files with External Libraries into Binaries
$(EXE): $(OBJ) $(LIB)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ) $(LFLAGS)

# Compile and Assemble C Source Files into Object Files
%.o: %.c $(LIB)
	$(CC) $(CFLAGS) -c $*.c



# Clean Utility
.PHONY: clean
clean:
	\yes | \rm -f $(EXE) $(OBJ) $(LOG) > /dev/null
