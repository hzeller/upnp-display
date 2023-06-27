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
#ifndef UPNP_DISPLAY_UTF8_H
#define UPNP_DISPLAY_UTF8_H

#include <stdint.h>
#include <string>

// Utility function that reads UTF-8 encoded codepoints from byte iterator.
// No error checking, we assume string is UTF-8 clean.
template <typename byte_iterator>
uint32_t utf8_next_codepoint(byte_iterator &it) {
  uint32_t cp = *it++;
  if (cp < 0x80) {
    return cp;   // iterator already incremented.
  }
  else if ((cp & 0xE0) == 0xC0) {
    cp = ((cp & 0x1F) << 6) + (*it & 0x3F);
  }
  else if ((cp & 0xF0) == 0xE0) {
    cp = ((cp & 0x0F) << 12) + ((*it & 0x3F) << 6);
    cp += (*++it & 0x3F);
  }
  else if ((cp & 0xF8) == 0xF0) {
    cp = ((cp & 0x07) << 18) + ((*it & 0x3F) << 12);
    cp += (*++it & 0x3F) << 6;
    cp += (*++it & 0x3F);
  }
  else if ((cp & 0xFC) == 0xF8) {
    cp = ((cp & 0x03) << 24) + ((*it & 0x3F) << 18);
    cp += (*++it & 0x3F) << 12;
    cp += (*++it & 0x3F) << 6;
    cp += (*++it & 0x3F);
  }
  else if ((cp & 0xFE) == 0xFC) {
    cp = ((cp & 0x01) << 30) + ((*it & 0x3F) << 24);
    cp += (*++it & 0x3F) << 18;
    cp += (*++it & 0x3F) << 12;
    cp += (*++it & 0x3F) << 6;
    cp += (*++it & 0x3F);
  }
  ++it;
  return cp;
}

template <typename byte_iterator>
int utf8_character_count(const byte_iterator &begin,
                         const byte_iterator &end) {
  int result = 0;
  for (byte_iterator it = begin; it != end; utf8_next_codepoint(it)) {
    ++result;
  }
  return result;
}

inline int utf8_len(const std::string& str) {
    return utf8_character_count(str.begin(), str.end());
}
#endif  // UPNP_DISPLAY_UTF8_H
