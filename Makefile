CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -pthread

SRCS = src/main.c src/logic/snake.c src/platform/input.c src/platform/terminal.c src/view/render.c src/logic/bot.c src/utils/d_array.c
OBJS = $(SRCS:.c=.o)

DEPS = src/config.h

TARGET = snake

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
