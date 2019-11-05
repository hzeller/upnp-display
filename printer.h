// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
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
#ifndef UPNP_DISPLAY_PRINTER_
#define UPNP_DISPLAY_PRINTER_

#include <string>

// Interface for a simple display.
class Printer {
public:
  virtual ~Printer() {}

  virtual int width() const { return 16; }

  // Print line. The text is given in UTF-8, the printer has to attempt
  // to try its best to display it.
  virtual void Print(int line, const std::string &text) = 0;

  // Put screen in sleep mode if possible.
  virtual void SaveScreen() = 0;
};

// Very simple implementation of the above, mostly for debugging. Just prints
// stuff continuously.
class ConsolePrinter : public Printer {
public:
  explicit ConsolePrinter(int width) : width_(width) {}
  virtual int width() const { return width_; }
  virtual void Print(int line, const std::string &text);
  virtual void SaveScreen() {}

private:
  const int width_;
};

#endif  // UPNP_DISPLAY_PRINTER_
