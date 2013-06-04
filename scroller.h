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

#ifndef UPNP_DISPLAY_SCROLLER_
#define UPNP_DISPLAY_SCROLLER_

#include <string>

// Utility class that implements the scrolling logic.
class Scroller {
public:
  Scroller();

  // Set text value to be scrolled and the display width available.
  // If the value or width is different from a previously set value, the scroll
  // position is set to the beginning of the string.
  void SetValue(std::string &content, int width);

  // Returns the scrolled content.
  std::string GetScrolledContent();

  // Next time tick to advance position according to internal state.
  void NextTick();

private:
  void InitIterators();

  int width_;
  std::string orig_content_;
  bool scrolling_needed_;         // If text is short, this won't need scrolling.

  std::string scroll_content_;    // scrollable content, including interlude.

  std::string::const_iterator print_start_;
  std::string::const_iterator print_end_;
  std::string::const_iterator end_of_content_;  // before interlude. Wait there.

  int characters_on_screen_; // portion shown; characters, not bytes.
  int scroll_timeout_;
};

#endif  // UPNP_DISPLAY_SCROLLER_
