CFLAGS = `pkg-config --cflags x11 xft libinput libudev`
LDFLAGS = `pkg-config --libs x11 xft libinput libudev`
CC = gcc

main.out: main.c
	$(CC) $(CFLAGS) main.c -o main.out $(LDFLAGS)
