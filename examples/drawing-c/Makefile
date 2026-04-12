CFLAGS ?= -Wall -Wextra -std=c99

ifdef EMSDK
	CC = emcc
	LDFLAGS += -s ASYNCIFY -s ASYNCIFY_IGNORE_INDIRECT
	WEB_DIR = web
	WEB = $(WEB_DIR)/index.html
	OUTPUT = $(WEB)
	PORT ?= 9000
else
	ifeq ($(OS),Windows_NT)
		LDFLAGS = -lgdi32
	else
		UNAME_S := $(shell uname -s)
		ifeq ($(UNAME_S),Darwin)
			LDFLAGS = -framework Cocoa
		else
			LDFLAGS = -lX11
		endif
	endif
endif

ifdef EMSDK
$(OUTPUT): main.c ../../fenster.h | $(WEB_DIR)
	$(CC) main.c -I../.. -o $@ $(CFLAGS) $(LDFLAGS)

$(WEB_DIR):
	mkdir -p $(WEB_DIR)
else
main: main.c ../../fenster.h
	$(CC) main.c -I../.. -o $@ $(CFLAGS) $(LDFLAGS)
endif

serve: $(WEB)
	@echo Running server on port $(PORT)
	@-cd $(WEB_DIR) && python3 -m http.server $(PORT)