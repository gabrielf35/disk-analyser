CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LIBS = -lncurses

SRCS = src/main.c src/dir_operations.c src/ui.c
OBJS = $(patsubst src/%.c, bin/obj/%.o, $(SRCS))
TARGET = bin/disk_analyser
INSTALL_DIR = /usr/bin

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

bin/obj/%.o: src/%.c
	@mkdir -p bin/obj
	$(CC) $(CFLAGS) -c $< -o $@

install: all
	@echo "Installing $(TARGET) to $(INSTALL_DIR)..."
	sudo cp $(TARGET) $(INSTALL_DIR)
	@echo "Installation complete."

clean:
	rm -f bin/obj/*.o $(TARGET)
	rmdir bin/obj bin 2>/dev/null || true
