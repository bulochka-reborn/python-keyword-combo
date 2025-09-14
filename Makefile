CFLAGS = `pkg-config --cflags x11 xft libinput libudev freetype2`
LDFLAGS = `pkg-config --libs x11 xft libinput libudev freetype2`
CC = gcc

main.out: main.c
	$(CC) $(CFLAGS) main.c -o main.out $(LDFLAGS)
	rm keywords.py