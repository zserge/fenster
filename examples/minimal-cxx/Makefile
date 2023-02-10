CXXFLAGS ?= -Wall -Wextra -std=c++11

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

main: main.cc ../../fenster.h
	$(CXX) main.cc -I../.. -o $@ $(CXXFLAGS) $(LDFLAGS)
