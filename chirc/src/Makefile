OBJS = main.o server.o bstring/bstrlib.o constants.o util.o irc_strings.o dispatch.o handlers.o reply.o message.o hash.o client.o channel.o
DEPS = $(OBJS:.o=.d)
CC = gcc
CFLAGS = -I../../include -g3 -Wall -fpic -std=gnu99 -MMD -MP -DDEBUG
BIN = ../chirc
LDLIBS = -pthread

all: $(BIN)
	
$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(OBJS) -o $(BIN)
	
%.d: %.c

clean:
	-rm -f $(OBJS) $(BIN) *.d
