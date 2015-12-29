CC     ?= cc
CFLAGS  = -std=c99 -Wall -Wextra -Wno-unused-function -Isrc -Ideps
LDFLAGS = -lpthread
SRCS    = $(wildcard src/*.c)
SRCS   += $(wildcard test/*.c)
DEPS    = $(wildcard deps/*/*.c)
OBJS    = $(SRCS:.c=.o)
OBJS   += $(DEPS:.c=.o)
BIN     = winp


all: $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<


.PHONY: release
release: CFLAGS += -O2
release: all

.PHONY: debug
debug: CFLAGS += -O0 -g -D_DEBUG
debug: all

.PHONY: clean
clean:
	rm -f $(BIN) $(OBJS)
