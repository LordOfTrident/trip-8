OUT     = trip8
INSTALL = /usr/bin/$(OUT)
SRC     = $(wildcard src/*.c)
DEPS    = $(wildcard src/*.h)
OBJ     = $(addsuffix .o,$(subst src/,bin/,$(basename $(SRC))))

CFLAGS = -pedantic -Wpedantic -Wshadow -Wvla -Wuninitialized -Wundef -Wno-deprecated-declarations \
         -Wall -Wextra -std=c99 $(shell sdl2-config --cflags)
LDFLAGS = -lm $(shell sdl2-config --libs)

.PHONY: debug release install uninstall clean all release-win-x86_64 release-win-i686

debug: CFLAGS += -Werror -DDEBUG -g -Og -fsanitize=address
debug: $(OUT)

release: CFLAGS += -DRELEASE -g0 -O2
release: $(OUT)

$(OUT): bin $(OBJ) $(SRC)
	$(CC) -o $(OUT) $(OBJ) $(LDFLAGS)

bin/%.o: src/%.c $(DEPS)
	$(CC) -c $< $(CFLAGS) -o $@

bin:
	mkdir -p bin

install: $(OUT)
	cp $(OUT) $(INSTALL)

uninstall: $(INSTALL)
	rm $(INSTALL)

clean: bin
	rm -f -r bin/*
	rm -f $(OUT) $(OUT).exe

all:
	@echo debug, release, install, uninstall, clean, release-win-x86_64, release-win-i686

# Windows Mingw release builds
release-win-x86_64: CC       = x86_64-w64-mingw32-gcc
release-win-x86_64: CFLAGS  += -I./lib/include
release-win-x86_64: LDFLAGS += -L./lib/bin/x86_64-w64
release-win-x86_64: release

release-win-i686: CC       = i686-w64-mingw32-gcc
release-win-i686: CFLAGS  += -I./lib/include
release-win-i686: LDFLAGS += -L./lib/bin/i686-w64
release-win-i686: release
