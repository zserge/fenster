CFLAGS ?= -I../.. -Wall -Wextra -std=c99

# debug
# CFLAGS += -g

ifeq ($(OS),Windows_NT)
	LDFLAGS = -lgdi32
	TARGET_EXT = .exe
else
	UNAME_S := $(shell uname -s)
	ifneq (,$(findstring emcc,$(CC)))
		LDFLAGS = -lm -sASYNCIFY=1 -sSTACK_SIZE=524288
		TARGET_EXT = .html
	else ifeq ($(UNAME_S),Darwin)
		LDFLAGS = -framework Cocoa
	else
		LDFLAGS = -lX11 -lasound -lm
		TARGET_EXT =
	endif
endif

TARGET = main$(TARGET_EXT)

$(TARGET): main.c
	$(CC) $? -o $(TARGET) $(CFLAGS) $(LDFLAGS)
