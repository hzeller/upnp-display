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

#include "console-printer.h"

#include <stdio.h>

ConsolePrinter::ConsolePrinter(int width) : width_(width) {}

ConsolePrinter::~ConsolePrinter() {}

int ConsolePrinter::width() const { 
  return width_;
}

void ConsolePrinter::Print(int line, const std::string &text) {
  printf("[%d]%s\n", line, text.c_str());
}

