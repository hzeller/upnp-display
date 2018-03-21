LIBS=$(shell pkg-config --cflags --libs libupnp)

EXTRA_CFLAGS=    
EXTRA_OBJECTS=
ifdef USE_INBUS
    LIBS+=$(shell pkg-config --cflags --libs libinbusclient)
    EXTRA_CFLAGS+=-DUSE_INBUS
    EXTRA_OBJECTS+=inbus-publisher.o
endif

INCLUDES=$(shell pkg-config --cflags --libs libupnp)

OBJECTS=main.o upnp-display.o display-writer.o renderer-state.o \
  controller-state.o console-printer.o lcd-display.o gpio.o scroller.o font-data.o \
  $(EXTRA_OBJECTS)

CFLAGS=-g -Wall $(INCLUDES) $(EXTRA_CFLAGS)
CXXFLAGS=-g -Wall $(INCLUDES) $(EXTRA_CFLAGS)

upnp-display: $(OBJECTS)
	g++ -Wall $^ $(LIBS) -o $@

font-data.c : font/5x8.bdf font/font2c.awk
	awk -f font/font2c.awk < $< > $@

clean :
	rm -f $(OBJECTS) upnp-display
