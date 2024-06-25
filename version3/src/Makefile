Srcs := $(wildcard *.c)
Outs := $(patsubst %.c, %.o, $(Srcs))
BIN:=server
CC := gcc
LIBS:=-lmysqlclient -lcrypt
CFLAGS:= -Wall -g -lpthread

ALL: $(Outs) 
	$(CC) -o $(BIN) $^ $(CFLAGS) $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(LIBS)

.PHONY: clean rebuild ALL

clean:
	$(RM) $(Outs) $(BIN)
rebuild: clean ALL

