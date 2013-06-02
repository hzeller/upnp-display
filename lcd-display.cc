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
#include "gpio.h"

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

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

LCDDisplay::LCDDisplay(int width) : width_(width), initialized_(false) {
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

  for (int i = 0; i < 16 && i < (int)text.length(); ++i) {
    WriteByte(false, text[i]);
  }
  // Fill rest with spaces.
  for (int i = text.length(); i < 16; ++i) {
    WriteByte(false, ' ');
  }
  last_line_[row] = text;
};
