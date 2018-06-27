CC=gcc
DIR=.
DIROBJ= $(DIR)/obj/
BIN=$(DIR)/bin/
SRC=$(DIR)/src/
EXAMPLE=$(DIR)/examples/
LDFLAGS = -pthread -O2 -Wall
OBJ=$(DIROBJ)cconcurrent.o

all: $(BIN)example_future $(BIN)example_pool

$(BIN)example_future: $(OBJ) $(SRC)cconcurrent.h
	@if [ -d $(BIN) ]; then : ; else mkdir $(BIN); fi
	$(CC) $(LDFLAGS) $(EXAMPLE)example_future.c -o $@ $(OBJ)

$(BIN)example_pool: $(OBJ) $(SRC)cconcurrent.h
	@if [ -d $(BIN) ]; then : ; else mkdir $(BIN); fi
	$(CC) $(LDFLAGS) $(EXAMPLE)example_pool.c -o $@ $(OBJ)


$(OBJ): $(SRC)cconcurrent.c
	@if [ -d $(DIROBJ) ]; then : ; else mkdir $(DIROBJ); fi
	$(CC) -c $(LDFLAGS) $< -o $(OBJ)

clean: 
	rm -fr $(BIN) $(OBJ)  *~