PREFIX=/usr/local
TARGET = deepin-screen-poller

CC = gcc
PWD = $(shell pwd)
CFLAGS = -Wall -g `pkg-config --cflags x11 xrandr libdrm`
LDFLAGS = -Wall -g `pkg-config --libs x11 xrandr libdrm`

OBJS := deepin_screen_poller.o
vpath $.c ${PWD}

all: ${TARGET}

${TARGET}: ${OBJS}
	${CC} ${LDFLAGS} $^ -o $@

%.o: %.c
	${CC} ${CFLAGS} -c $<

install: all
	mkdir -p ${PREFIX}/bin
	cp -f deepin-screen-poller ${PREFIX}/bin
	cp deepin-screen-poller.desktop /etc/xdg/autostart

clean:
	rm -rf ${OBJS} ${TARGET}

rebuild: clean all
