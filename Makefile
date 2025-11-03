CC = gcc
CFLAGS = -Wall -std=c99 -Iinclude
LDFLAGS = -lraylib -lm -lpthread -ldl -lGL -lX11

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = tower_defense

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f src/*.o $(TARGET)
