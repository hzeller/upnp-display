LIBS=$(shell pkg-config --cflags --libs libupnp)
INCLUDES=$(shell pkg-config --cflags --libs libupnp)

OBJECTS=upnp-display.o renderer-state.o printer.o controller-state.o \
	lcd-display.o gpio.o font-data.o

CFLAGS=-g -Wall $(INCLUDES)
CXXFLAGS=-g -Wall $(INCLUDES)

upnp-display: $(OBJECTS)
	g++ -Wall $^ $(LIBS) -o $@

font-data.c : font/5x7.bdf
	awk < $< \
        'BEGIN { i=0; printf("// Generated from $<. Do not edit.\n#include \"font-data.h\"\n\nstruct Font5x7 kFontData[] = {\n"); } \
        /^STARTCHAR/ { val=$$2 }\
        /^ENCODING/  { printf("\t{ %5d, { ", $$2); } \
        /^BITMAP/    { in_bitmap=1; } \
        /^[0-9a-fA-F][0-9a-fA-F]/ { if (in_bitmap) printf("0x%s, ", $$1); } \
        /^ENDCHAR/ { printf("}},  // %s\n", val); in_bitmap=0; i++; }\
        END { printf("};\nuint32_t kFontDataSize = %i;\n", i) }' > $@

clean :
	rm -f $(OBJECTS) upnp-display
