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

#ifndef FONT_DATA_H
#define FONT_DATA_H

#include <stdint.h>

// Fixed font. Defined in font-data.c, that is generated from public domain
// font 5x7.bdf
struct Font5x8 {
  uint32_t codepoint;
  uint8_t bitmap[8];
};

extern struct Font5x8 kFontData[];
extern uint32_t kFontDataSize;

#endif  // FONT_DATA_H
