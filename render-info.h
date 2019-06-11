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
#ifndef UPNP_DISPLAY_RENDER_INFO_
#define UPNP_DISPLAY_RENDER_INFO_

#include <string>

enum PlayState {
  Paused,
  Playing,
  Stopped
};

// Contains info about what is being rendered
struct RenderInfo {
  // If true, playerName contains the name of the renderer
  // for which to wait and all other variables are meaningless.
  bool is_waiting_for_renderer;

  PlayState play_state;
  std::string volume;
  int time;
  std::string player_name; 
  std::string title;
  std::string composer;
  std::string artist;
  std::string album;
};

#endif  // UPNP_DISPLAY_RENDER_INFO_
