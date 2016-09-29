PREFIX=/usr/local
TARGET = screen-poller

CC = gcc
PWD = $(shell pwd)
CFLAGS = -Wall -g `pkg-config --cflags x11 xrandr libdrm`
LDFLAGS = -Wall -g `pkg-config --libs x11 xrandr libdrm`

OBJS := screen_poller.o
vpath $.c ${PWD}

all: ${TARGET}

${TARGET}: ${OBJS}
	${CC} ${LDFLAGS} $^ -o $@

%.o: %.c
	${CC} ${CFLAGS} -c $<

install: all
	mkdir -p ${PREFIX}/bin
	cp -f ${TARGET} ${PREFIX}/bin
	cp screen-poller.desktop /etc/xdg/autostart

clean:
	rm -rf ${OBJS} ${TARGET}

rebuild: clean all
