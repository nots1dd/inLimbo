CC = g++
CFLAGS = -Wall -Wextra -Wpedantic

SRCS = test.cpp \

TAG_LIB = $(shell pkg-config --libs --cflags taglib)

TARGET = dirsort-test

all: $(TARGET)

$(TARGET):
	 $(CC) -o $@ $(SRCS) $(TAG_LIB)

clean:
	rm -f $(TARGET)

.PHONY: all clean
