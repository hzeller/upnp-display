PREFIX=/usr/local
LIBS=$(shell pkg-config --cflags --libs libupnp)
INCLUDES=$(shell pkg-config --cflags libupnp)

OBJECTS=main.o upnp-display.o renderer-state.o printer.o controller-state.o \
	lcd-display.o gpio.o scroller.o font-data.o

CFLAGS=-g -O3 -Wall -W -Wextra $(INCLUDES) -D_FILE_OFFSET_BITS=64
CXXFLAGS=$(CFLAGS) -std=c++03

upnp-display: $(OBJECTS)
	g++ -Wall $^ $(LIBS) -o $@

install: upnp-display
	install $^ $(PREFIX)/bin
	setcap cap_sys_nice=eip $(PREFIX)/bin/upnp-display

font-data.c : font/5x8.bdf font/font2c.awk
	awk -f font/font2c.awk < $< > $@

clean :
	rm -f $(OBJECTS) upnp-display
