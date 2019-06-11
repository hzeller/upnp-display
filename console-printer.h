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

#ifndef UPNP_DISPLAY_CONSOLE_PRINTER_
#define UPNP_DISPLAY_CONSOLE_PRINTER_

#include <string>

#include "printer.h"

// Console printer for debugging.
class ConsolePrinter : public Printer {
public:
  explicit ConsolePrinter(int width);
  virtual ~ConsolePrinter();
  virtual int width() const;
  virtual void Print(int line, const std::string &text);

private:
  const int width_;
};

#endif  // UPNP_DISPLAY_CONSOLE_PRINTER_
