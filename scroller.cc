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

#include "utf8.h"

static const int kBorderWait = 4;  // ticks to wait at end-of-scroll

Scroller::Scroller(const std::string &interlude)
  : interlude_(interlude), characters_on_screen_(0), scroll_timeout_(0) {}

void Scroller::InitIterators() {
  print_start_ = scroll_content_.begin();
  print_end_ = print_start_;
  int i;
  for (i = 0;
       i < width_ && print_end_ != scroll_content_.end();
       ++i, utf8_next_codepoint(print_end_)) {
  }
  characters_on_screen_ = i;
}

void Scroller::SetValue(std::string &content, int width) {
  if (content != orig_content_ || width != width_) {
    orig_content_ = content;
    scroll_content_ = orig_content_;
    width_ = width;
    InitIterators();
    scrolling_needed_ = (print_end_ != scroll_content_.end());
    if (scrolling_needed_) {
      scroll_content_ = orig_content_ + interlude_;
      InitIterators();
    }
    end_of_content_ = scroll_content_.begin() + orig_content_.length();
    scroll_timeout_ = kBorderWait;
  }
}

std::string Scroller::GetScrolledContent() {
  std::string result;
  result.append(print_start_, print_end_);
  if (!scrolling_needed_)
    return result;

  int char_printed = characters_on_screen_;
  // Reached end of scroll content. If there is still space, print the beginng
  // again. We know already that this will never reach end(), so no need to check
  std::string::iterator print_prefix = scroll_content_.begin();
  while (char_printed < width_) {
    ++char_printed;
    utf8_next_codepoint(print_prefix);
  }
  result.append(scroll_content_.begin(), print_prefix);

  return result;
}

void Scroller::NextTick() {
  if (!scrolling_needed_)
    return;

  if (scroll_timeout_ > 0) {
    scroll_timeout_--;
  } else {
    utf8_next_codepoint(print_start_);
    --characters_on_screen_;   // one reduced in front
    if (print_start_ == scroll_content_.end()) {
      InitIterators();
      scroll_timeout_ = kBorderWait;
    } else if (print_end_ != scroll_content_.end()) {
      utf8_next_codepoint(print_end_);
      ++characters_on_screen_; // one added at end.
      if (print_end_ == end_of_content_) {
        scroll_timeout_ = kBorderWait;
      }
    }
  }
}
