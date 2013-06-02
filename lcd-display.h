//  -*- c++ -*-
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
#ifndef UPNP_DISPLAY_LCD_
#define UPNP_DISPLAY_LCD_

#include <string>

#include "printer.h"

// An implementation of an interface to a standard 16x2 LCD display
// connected to RPi GPIO pins.
class LCDDisplay : public Printer {
public:
  LCDDisplay(int width);

  // Call this first.
  bool Init();

  virtual int width() const { return width_; }

  // Print text in given line.
  virtual void Print(int line, const std::string &text);

private:
  const int width_;
  bool initialized_;
  std::string last_line_[2];
};

#endif // UPNP_DISPLAY_LCD_
