##################################################
##      Computer Science 434 - Networking       ##
##          University of Saskatchewan          ##
##                     2020                     ##
##----------------------------------------------##
##                  Kale Yuzik                  ##
##               kay851@usask.ca                ##
##----------------------------------------------##
##                Jason Goertzen                ##
##               jag190@usask.ca                ##
##----------------------------------------------##
##                 Ben Haubrich                 ##
##               bjh885@usask.ca                ##
##################################################


CC = g++

# Disable warnings about future GCC abi changes
CFLAGS = -Wno-psabi
CPPFLAGS = -std=gnu++17 -Wall -Wextra -pedantic -g3 -D_GNU_SOURCE -D_DEFAULT_SOURCE
LDFLAGS = -lconfig++ \
					-lpthread \
					-lmenuw \
					-lncursesw \
					-ltinfo \
					-lboost_system

JSONINC = ./json/include
LIBCONFIGINC = ./libconfig/lib

INCLUDE = -I . \
			-I ./include \
			-I $(JSONINC) \
			-I $(LIBCONFIGINC)
ARCH = $(shell uname -s)$(shell uname -m)

BUILD = ./build
BIN = $(BUILD)/bin/$(ARCH)
OBJ = $(BUILD)/obj/$(ARCH)


.PHONY: all mkdirs debug clean

all: mkdirs $(BIN)/client

mkdirs:
	mkdir -p $(BIN) $(OBJ)

debug: debug_flag all

debug_flag:
	$(eval DEBUG = -D_DEBUG)

clean:
	rm -rf ./build ./client

OBJECTS := $(OBJ)/eth_interface.o \
			$(OBJ)/game_interface.o \
			$(OBJ)/event_wait_mgr.o \
			$(OBJ)/ethabi.o \
			$(OBJ)/misc.o \
			$(OBJ)/client.o

$(OBJ)/%.o: %.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(DEBUG) -o $@ $(INCLUDE) $<

$(BIN)/client: $(OBJECTS)
	$(CC) $(CPPFLAGS) -o $@ \
		${LDFLAGS} $^ 
	ln -fs $@ ./client
