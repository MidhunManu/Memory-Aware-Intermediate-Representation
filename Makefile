CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

SRCS = lexer.c parser.c ir.c codegen.c main.c
OBJS = $(SRCS:.c=.o)
TARGET = compiler

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET) out.asm out.o out
