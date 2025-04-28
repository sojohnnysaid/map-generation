# Compiler settings
CC = clang
CFLAGS = -Wall -Wextra -g -Iinclude -O2
LDFLAGS = -lm

# Project structure
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin
TARGET = $(BINDIR)/mapgen

# Sources and Objects
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, obj/%.o, $(SOURCES))

# Default target
all: $(TARGET)

# Link executable
$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

# Compile .c to .o
obj/%.o: $(SRCDIR)/%.c $(wildcard $(INCDIR)/*.h) | obj
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

# Create obj directory
obj:
	@mkdir -p obj

# Clean build files
clean:
	rm -rf obj $(TARGET)
	@echo "Cleaned build files."

# Run program
run: all
	./$(TARGET)

# Rule to time the program execution
time: all
	time ./$(TARGET)

.PHONY: all clean run time obj
