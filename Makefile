LIBS=$(shell pkg-config --cflags --libs libupnp)
INCLUDES=$(shell pkg-config --cflags --libs libupnp)

OBJECTS=upnp-display.o renderer-state.o printer.o controller-state.o \
	lcd-display.o gpio.o

CFLAGS=-g -Wall $(INCLUDES)
CXXFLAGS=-g -Wall $(INCLUDES)

upnp-display: $(OBJECTS)
	g++ -Wall $^ $(LIBS) -o $@

clean :
	rm -f $(OBJECTS) upnp-display
