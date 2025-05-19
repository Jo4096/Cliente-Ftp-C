CC = gcc
CFLAGS = -Wall -Iinclude
SRC_DIR = src
TARGET = clientFtp  # Nome do executável na raiz

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,%.o,$(SRCS)) # Arquivos objeto na raiz

all: $(TARGET)

$(TARGET): $(OBJS) main.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET)

run: all
	./$(TARGET)