CC = gcc
CFLAGS = -std=c23 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -Wall -Wextra -Werror -Wno-unused-parameter -fno-asm
INCLUDES = -Iinclude

# As you add files (lexer.c, parser.c), just add them to this list
SRCS = src/main.c

TARGET = shell.out

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)