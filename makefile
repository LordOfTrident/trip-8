OUT     = trip8
INSTALL = /usr/bin/$(OUT)
SRC     = $(wildcard src/*.c)
DEPS    = $(wildcard src/*.h)
OBJ     = $(addsuffix .o,$(subst src/,bin/,$(basename $(SRC))))

CFLAGS = -pedantic -Wpedantic -Wshadow -Wvla -Wuninitialized -Wundef -Wno-deprecated-declarations \
         -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE -D_XOPEN_SOURCE
LDFLAGS = -lm $(shell sdl2-config --cflags --libs)

#-target aarch64-linux-gnu -I./lib

#-target x86_64-linux-gnu.2.5
#-target aarch64-linux-gnu

.PHONY: debug release clean install uninstall all

debug: CFLAGS += -Werror -DDEBUG -g -Og -fsanitize=address
debug: $(OUT)

release: CFLAGS += -DRELEASE -g0 -O2
release: $(OUT)

win-x86_64: CC       = "x86_64-w64-mingw32-gcc"
win-x86_64: CFLAGS  += "-I./lib"
win-x86_64: LDFLAGS += "-L./lib/x86_64-w64"
win-x86_64: release

win-i686: CC       = "i686-w64-mingw32-gcc"
win-i686: CFLAGS  += "-I./lib"
win-i686: LDFLAGS += "-L./lib/i686-w64"
win-i686: release

$(OUT): bin $(OBJ) $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(OUT) $(OBJ)

bin/%.o: src/%.c $(DEPS)
	$(CC) -c $< $(CFLAGS) -o $@

bin:
	mkdir -p bin

install: $(OUT)
	cp $(OUT) $(INSTALL)

uninstall: $(INSTALL)
	rm $(INSTALL)

clean: bin
	rm -r bin/*
	rm $(OUT)

all:
	@echo build, install, uninstall, clean
