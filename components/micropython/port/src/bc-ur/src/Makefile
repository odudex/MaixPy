# Simple Makefile for testing the C UR implementation

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2
INCLUDES = -I.
SRCDIR = .
OBJDIR = obj

# Source files
SOURCES = utils.c bytewords.c fountain_decoder.c ur_decoder.c ur.c

# Object files
OBJECTS = $(SOURCES:%.c=$(OBJDIR)/%.o)

# Target library
TARGET = libur.a

# Test program
TEST_TARGET = test_ur
TEST_SOURCES = test_ur.c

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	ar rcs $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

test: $(TARGET) $(TEST_TARGET)

$(TEST_TARGET): $(TEST_SOURCES) $(TARGET)
	$(CC) $(CFLAGS) $(INCLUDES) $< -L. -lur -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET) $(TEST_TARGET)

# Dependencies (simplified)
$(OBJDIR)/utils.o: utils.c utils.h
$(OBJDIR)/bytewords.o: bytewords.c bytewords.h utils.h
$(OBJDIR)/fountain_decoder.o: fountain_decoder.c fountain_decoder.h utils.h
$(OBJDIR)/ur_decoder.o: ur_decoder.c ur_decoder.h fountain_decoder.h bytewords.h utils.h
$(OBJDIR)/ur.o: ur.c ur.h ur_decoder.h utils.h