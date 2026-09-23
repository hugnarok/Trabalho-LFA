CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g -Ilib
SRC = src/main.c src/automato.c src/json_io.c \
      src/funcoes_stubs.c
HDR = lib/automato.h lib/json_io.h lib/funcoes.h
BIN = lfa

.PHONY: all clean run

all: $(BIN)

$(BIN): $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

run: $(BIN)
	./$(BIN)

clean:
	rm -rf $(BIN) $(BIN).dSYM
