SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c, %.o, $(SRCS))
BIN:=client
CC:=gcc
LIBS:=-lmysqlclient
FLAGS:=-g -w -lcrypt -lcrypto

$(BIN):$(OBJS)
	$(CC) -o $@ $^ $(FLAGS)
%.o:%.c
	$(CC) -c $^ -o $@ $(FLAGS) $(LIBS)

clean:
	rm -rf $(OBJS) $(BIN)
