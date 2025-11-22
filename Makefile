CC = gcc
CFLAGS = -Wall -std=c99 -Iinclude -Iraylib/include
LDFLAGS = -Lraylib/lib -lraylib -lm -lpthread -ldl -lGL -lX11 -lXrandr -lXinerama -lXi -lXcursor

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
