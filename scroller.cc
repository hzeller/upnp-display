// -*- c++ -*-
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

#include "scroller.h"

#include "utfcpp/utf8.h"

static const int kBorderWait = 4;  // ticks to wait at end-of-scroll

static const char kInterlude[] = "   *   ";  // between scrolls.

Scroller::Scroller() : characters_on_screen_(0), scroll_timeout_(0) {}

void Scroller::InitIterators() {
  print_start_ = content_.begin();
  print_end_ = print_start_;
  int i;
  for (i = 0;
       i < width_ && print_end_ != content_.end();
       ++i, utf8::unchecked::next(print_end_)) {
  }
  characters_on_screen_ = i;
}

void Scroller::SetValue(std::string &content, int width) {
  if (content != content_ || width != width_) {
    content_ = content;
    width_ = width;
    InitIterators();
    scroll_timeout_ = kBorderWait;
  }
}

std::string Scroller::GetScrolledContent() {
  std::string result;
  result.append(print_start_, print_end_);
  if (print_start_ == content_.begin() && print_end_ == content_.end())
    return result;  // fits entirely in screen. No scrolling/interlude needed.

  // If we have reached end of string, but there is still width left, interlude
  // to the next beginning with some spacing string.
  int char_printed = characters_on_screen_;
  const char *inter = kInterlude;
  for (/**/; char_printed < width_ && *inter; ++char_printed, ++inter) {
    result.append(1, *inter);
  }

  // Reached end of interlude. Now if there is still space, print the beginng
  // of the string. Note, we already know that this will never go to
  // content_.end(), so no need to check that.
  std::string::iterator print_prefix = content_.begin();
  while (char_printed < width_) {
    ++char_printed;
    utf8::unchecked::next(print_prefix);
  }
  result.append(content_.begin(), print_prefix);

  return result;
}

void Scroller::NextTick() {
  if (print_start_ == content_.begin() && print_end_ == content_.end())
    return;  // fits entirely in screen. No scrolling needed.

  if (scroll_timeout_ > 0) {
    scroll_timeout_--;
  } else {
    utf8::unchecked::next(print_start_);
    --characters_on_screen_;   // one reduced in front
    if (print_start_ == content_.end()) {
      InitIterators();
      scroll_timeout_ = kBorderWait;
    } else if (print_end_ != content_.end()) {
      utf8::unchecked::next(print_end_);
      ++characters_on_screen_; // one added at end.
      if (print_end_ == content_.end()) {
        scroll_timeout_ = kBorderWait;
      }
    }
  }
}
