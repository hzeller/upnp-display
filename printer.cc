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

#include "printer.h"

#include <stdio.h>
#include "utf8.h"

#define SCREEN_CURSOR_UP_FORMAT    "\033[%dA"  // Move cursor up given lines.

std::string Utf8CutPrefix(const std::string &str, int len) {
  std::string::const_iterator pos = str.begin();
  while (len > 0 && pos != str.end()) {
    utf8_next_codepoint(pos);
    --len;
  }
  return str.substr(0, pos - str.begin());
}

void ConsolePrinter::Print(int line, const std::string &raw_text) {
  if (line > (int)lines_.size()) return;
  const std::string display_text = Utf8CutPrefix(raw_text, width_);
  if (lines_[line] == display_text) return;  // no change.
  lines_[line] = display_text;

  if (!in_place_) {
    printf("[%d]: %s\n", line, display_text.c_str());
    return;
  }
  if (needs_jump_) {
    printf(SCREEN_CURSOR_UP_FORMAT, (int)lines_.size());
  }
  for (size_t i = 0; i < lines_.size(); ++i) {
    printf("%*s\r", width_, "");  // Clear possible last text
    printf("%s\n", lines_[i].c_str());
  }
  needs_jump_ = true;
}
