# ROOT_DIR := $(notdir $(patsubst %/,%,$(CURDIR)))
MAKEFILE := $(firstword $(MAKEFILE_LIST))
SHELL := /bin/bash
TARGET := ttytris
LIBS   := ncurses
CC     := gcc

CFLAGS := 

ifdef PKG_LIBS
CFLAGS    += $(shell pkg-config --cflags $(PKG_LIBS)) -std=c11
LIB_FLAGS += $(shell pkg-config --libs $(PKG_LIBS))
endif

LIB_FLAGS += $(addprefix -l, $(LIBS))

PREFIX    ?= /usr/local
BINPREFIX := $(PREFIX)/bin

SOURCES := $(wildcard src/*.c)
HEADERS := $(wildcard src/*.h)
OBJECTS := $(SOURCES:.c=.o)

TEST_SOURCES := $(wildcard test/*.c)
TEST_HEADERS := $(wildcard test/*.h)
TEST_OBJECTS := $(TEST_SOURCES:.c=.o)
TEST_TARGET  := test/runtests


.PHONY: debug default uninstall clean test


all:	# Multi-threaded make by default
	$(MAKE) -j $(shell nproc) $(TARGET)

debug: CFLAGS += -D DEBUG
debug: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIB_FLAGS) $^ -o $@ -O3

$(OBJECTS): $(SOURCES) $(HEADERS)

%.o: %.c
	$(CC) $(FEATURES) $(CFLAGS) $(LDFLAGS) $(LIB_FLAGS) -c $< -o $@

$(SOURCES): $(MAKEFILE) # If Makefile changes, recompile
	@touch $(SOURCES)


install: $(TARGET)
	install -m 755 -D --target-directory "$(BINPREFIX)" "$(TARGET)"

uninstall:
	rm -f "$(BINPREFIX)/$(TARGET)"

clean:
	rm -f $(TARGET) $(TEST_TARGET) $(OBJECTS) $(TEST_OBJECTS)


test: $(TEST_TARGET)
	$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS) $(filter-out src/main.o, $(OBJECTS))
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIB_FLAGS) $^ -o $@ -O3

$(TEST_OBJECTS): $(TEST_SOURCES) $(TEST_HEADERS)