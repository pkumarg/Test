CC=g++

SRC= $(shell find . -name "*.cpp")

CFLAGS=--std=c++17 -g -Wall

TARGET= $(patsubst %.cpp, %, $(SRC))

all: $(TARGET)

%: %.cpp
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGET)
