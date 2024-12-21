CC = g++
CFLAGS = -Wall -Wextra -Wpedantic

SRCS = main.cpp \
			 rbtree.cpp \
			 taglib_parser.cpp \
			 songmap.cpp \

TAG_LIB = $(shell pkg-config --libs --cflags taglib)

TARGET = dirsort

all: $(TARGET)

$(TARGET):
	 $(CC) -o $@ $(SRCS) $(TAG_LIB)

clean:
	rm -f $(TARGET)

.PHONY: all clean
