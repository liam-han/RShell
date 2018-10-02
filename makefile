CC = g++
CC_FLAGS = -Wall -Werror -ansi -pedantic
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

all: rshell

rshell: $(OBJECTS)
	@mkdir -p bin
	$(CC) $(CC_FLAGS) $^ -o bin/rshell

clean:
	rm -f $(OBJECTS)
	rm -rf bin
