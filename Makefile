LIBS=$(shell pkg-config --cflags --libs libupnp)
INCLUDES=$(shell pkg-config --cflags --libs libupnp)

OBJECTS=main.o upnp-display.o display-writer.o renderer-state.o printer.o \
  controller-state.o lcd-display.o gpio.o scroller.o font-data.o

CFLAGS=-g -Wall $(INCLUDES)
CXXFLAGS=-g -Wall $(INCLUDES)

upnp-display: $(OBJECTS)
	g++ -Wall $^ $(LIBS) -o $@

font-data.c : font/5x8.bdf font/font2c.awk
	awk -f font/font2c.awk < $< > $@

clean :
	rm -f $(OBJECTS) upnp-display
