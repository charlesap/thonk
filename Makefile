TARGET = thonk
LIBS = -lm
CC = gcc
CFLAGS = -g -Wall 

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -pthread $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall -pthread $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
