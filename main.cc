//  This file is part of UPnP LCD Display
//
//  Copyright (C) 2013 Henner Zeller <h.zeller@acm.org>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <unistd.h>
#include <stdio.h>

#include "controller-state.h"
#include "upnp-display.h"
#include "lcd-display.h"
#include "printer.h"

// Comment this out, if you don't run this on a Raspberry Pi; then it will
// do some basic printing on the console.
#define USE_RASPBERRY_LCD

// Width of your display. Usually this is just 16 wide, but you can get 24 or
// even 40 wide displays. You can also set this via the -w option.
#define LCD_DISPLAY_WIDTH 16

int main(int argc, char *argv[]) {
  std::string match_name;
  int display_width = LCD_DISPLAY_WIDTH;
  int opt;
  while ((opt = getopt(argc, argv, "hn:w:")) != -1) {
    switch (opt) {
    case 'n':
      if (optarg != NULL) match_name = optarg;
      break;

    case 'w': {
      int w = atoi(optarg);
      if (w < 8) {
        fprintf(stderr, "Invalid width %d\n", w);
        return 1;
      }
      display_width = w;
      break;
    }

    case 'h':
    default:
      fprintf(stderr, "Usage: %s [-n name]\n", argv[0]);
      fprintf(stderr, "\t-n <name or \"uuid:\"<uuid>"
              ": Connect to this renderer.\n"
              "\t-w <display-width>         : Display Width.");
      return 1;
    }
  }

#ifdef USE_RASPBERRY_LCD
  LCDDisplay printer(display_width);
  if (!printer.Init()) {
    fprintf(stderr, "You need to run this as root to have access to GPIO pins. "
            "Run with sudo.\n");
    return 1;
  }
#else
  ConsolePrinter printer;
#endif

  // TODO: drop priviliges (GPIO is set up at this point).

  UPnPDisplay ui(match_name, &printer);
  ControllerState controller(&ui);
  ui.Loop();

  return 0;
}
