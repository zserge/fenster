CFLAGS ?= -Wall -Wextra -std=c99

ifeq ($(OS),Windows_NT)
	LDFLAGS = -lgdi32 -lwinmm
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LDFLAGS = -framework Cocoa -framework AudioToolbox
	else
		LDFLAGS = -lX11 -lasound
	endif
endif

main: main.c ../../fenster.h ../../fenster_audio.h
	$(CC) main.c -I../.. -o $@ $(CFLAGS) $(LDFLAGS)
