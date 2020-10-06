CC=gcc
CFLAGS=-g -Wall -Wextra -Werror -std=c11 -D_SVID_SOURCE
LDLIBS=-lpthread

SRC= $(shell find . -name "*.c")

TARGET=$(patsubst %.c, %, $(SRC))

all: $(TARGET)

%:%.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -f $(TARGET)

run:
	$(TARGET)
run_vg:
	G_SLICE=always-malloc G_DEBUG=gc-friendly  valgrind -v --tool=memcheck --leak-check=full --num-callers=40 --track-origins=yes --log-file=valgrind.log $(TARGET)
