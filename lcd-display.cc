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

#include "lcd-display.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gpio.h"
#include "font-data.h"
#include "utfcpp/utf8.h"

GPIO gpio;

// According to datasheet, basic ops are typically ~37usec
#define LCD_DISPLAY_OPERATION_WAIT_USEC 50

#define LCD_E (1<<18)
#define LCD_RS (1<<14)

// We have the bits assigned in an unusal pattern to accomodate simple wiring.
#define LCD_D0_BIT (1<<23)
#define LCD_D1_BIT (1<<24)
#define LCD_D2_BIT (1<<25)
#define LCD_D3_BIT (1<<8)

static void WriteNibble(bool is_command, uint8_t b) {
  uint32_t out = is_command ? 0 : LCD_RS;
  out |= (b & 0x1) ? LCD_D0_BIT : 0;
  out |= (b & 0x2) ? LCD_D1_BIT : 0;
  out |= (b & 0x4) ? LCD_D2_BIT : 0;
  out |= (b & 0x8) ? LCD_D3_BIT : 0;
  out |= LCD_E;
  gpio.Write(out);
  usleep(1);      // > 230ns
  out &= ~LCD_E;
  gpio.Write(out);
}

// Write data to display. Differentiates if this is a command byte or data
// byte.
static void WriteByte(bool is_command, uint8_t b) {
  WriteNibble(is_command, (b >> 4) & 0xf);
  WriteNibble(is_command, b & 0xf);
  usleep(LCD_DISPLAY_OPERATION_WAIT_USEC);
}

// Find font for codepoint from our sorted compiled-in table.
const static Font5x8 *findFont(const uint32_t codepoint) {
  int lo = 0, hi = kFontDataSize;
  int pos;
  for (pos = (hi + lo) / 2; hi > lo; pos = (hi + lo) / 2) {
    if (codepoint > kFontData[pos].codepoint)
      lo = pos + 1;
    else
      hi = pos;
  }
  return (kFontData[pos].codepoint == codepoint) ? &kFontData[pos] : NULL;
}

static void RegisterFont(int num, const Font5x8 *font) {
  assert(font);
  assert(num < 8);
  for (int i = 0; i < 8; ++i) {
    WriteByte(true, 0x40 + (num << 3) + i);
    WriteByte(false, font->bitmap[i] >> 3);
  }
}

uint8_t LCDDisplay::FindCharacterFor(Codepoint cp, bool *register_new) {
  *register_new = false;
  if (cp < 0x7F) return cp;

  // Conceptually, we need a map codepoint -> char; but this is a really small
  // list, so this is faster to iterate than having a bulky map.
  for (int i = 0; i < 8; ++i) {
    if (special_characters_[i] == cp) return i;
  }

  // Ok, not there, just use the next free. We're doing a very simple
  // round-robin approach here, potentially re-using characters that are
  // still in use. For now, we just assume that the active number of different
  // characters is small enough to fit.
  const Font5x8 *font = findFont(cp);
  if (font == NULL) return '?';  // unicode without font.

  const uint32_t new_char = (next_free_special_++ % 8);
  RegisterFont(new_char, font);
  *register_new = true;
  special_characters_[new_char] = cp;
  return new_char;
}

LCDDisplay::LCDDisplay(int width) : width_(width), initialized_(false),
                                    next_free_special_(0) {
  memset(special_characters_, 0, sizeof(special_characters_));
}

bool LCDDisplay::Init() {
  if (!gpio.Init())
    return false;

  gpio.InitOutputs(LCD_E | LCD_RS |
                   LCD_D0_BIT | LCD_D1_BIT | LCD_D2_BIT | LCD_D3_BIT);

  // -- This seems to be a reliable initialization sequence:

  // Start with 8 bit mode, then instruct to switch to 4 bit mode.
  WriteNibble(true, 0x03);
  usleep(5000);            // If we were in 4 bit mode, timeout makes this 0x30
  WriteNibble(true, 0x03);
  usleep(5000);

  // Transition to 4 bit mode.
  WriteNibble(true, 0x02); // Interpreted as 0x20: 8-bit cmd to switch to 4-bit.
  usleep(LCD_DISPLAY_OPERATION_WAIT_USEC);

  // From now on, we can write full bytes that we transfer in nibbles.
  WriteByte(true, 0x28);  // Function set: 4-bit mode, two lines, 5x8 font
  WriteByte(true, 0x06);  // Entry mode: increment, no shift
  WriteByte(true, 0x0c);  // Display control: on, no cursor

  WriteByte(true, 0x01);  // Clear display
  usleep(2000);           // ... which takes up to 1.6ms

  initialized_ = true;

  return true;
}

void LCDDisplay::Print(int row, const std::string &text) {
  assert(initialized_);  // call Init() first.
  assert(row < 2);       // uh, out of range.

  if (last_line_[row] == text)
    return;  // nothing to update.

  // Set address to write to; line 2 starts at 0x40
  WriteByte(true, 0x80 + ((row > 0) ? 0x40 : 0));

  std::string::const_iterator it = text.begin();
  int screen_pos = 0;
  for (screen_pos = 0; screen_pos < width_ && it != text.end(); ++screen_pos) {
    const uint32_t codepoint = utf8::unchecked::next(it);
    bool ddram_dirty = false;
    uint8_t char_to_print = FindCharacterFor(codepoint, &ddram_dirty);
    if (ddram_dirty) {
      WriteByte(true, 0x80 + ((row > 0) ? 0x40 : 0) + screen_pos);
    }
    WriteByte(false, char_to_print);
  }
  // Fill rest with spaces.
  for (int i = screen_pos; i < width_; ++i) {
    WriteByte(false, ' ');
  }
  last_line_[row] = text;
};
