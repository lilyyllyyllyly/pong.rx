CC=clang
CFLAGS ?= -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-but-set-variable -O2
LIBS ?= -lraylib

compile:
	${CC} -shared ${CFLAGS} raylib-rx.c ${LIBS} -o libexternal.so

