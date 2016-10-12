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
#ifndef UPNP_DISPLAY_DISPLAY_WRITER_H
#define UPNP_DISPLAY_DISPLAY_WRITER_H

#include <string>

#include "render-info-consumer.h"

class Printer;
class Scroller;

class DisplayWriter : public RenderInfoConsumer {
public:
  // Manages scrollable two-line based content and sends it to a Printer
  DisplayWriter(Printer *printer);
  virtual ~DisplayWriter();

  // Implement interfaces
  void OnStart();
  void OnRenderInfo(const RenderInfo &render_info);
  void OnExit();
  
private:

  // Format time for display.
  std::string formatTime(int time);

  // Format play state for display
  std::string formatPlayState(PlayState play_state);

  // Text formatting utility.
  void CenterAlign(std::string *to_print, int width);
  void RightAlign(std::string *to_print, int width);

  Printer *const printer_;
  Scroller *first_line_scroller;
  Scroller *second_line_scroller;

  unsigned char blink_time;
  int volume_countdown;
  std::string previous_volume;

};

#endif  // UPNP_DISPLAY_DISPLAY_WRITER_H
