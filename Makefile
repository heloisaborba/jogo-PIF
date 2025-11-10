CC = gcc
CFLAGS = -Wall -std=c99 -Iinclude
LDFLAGS = -lraylib -lm -lpthread -ldl -lGL -lX11

SRC = src/main.c src/game.c src/enemy.c src/hero.c src/menu.c src/tower.c src/utils.c src/recursos.c
OBJ = ${SRC:.c=.o}
EXEC = tower_defense

all: ${EXEC}

${EXEC}: ${OBJ}
	${CC} ${OBJ} -o ${EXEC} ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -f ${OBJ} ${EXEC}
