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
#ifndef UPNP_DISPLAY_RENDER_INFO_SUBSCRIBER_
#define UPNP_DISPLAY_RENDER_INFO_SUBSCRIBER_

#include <string>

#include "render-info.h"

// Interface for subscribers of RenderInfo
class RenderInfoSubscriber {
public:
  virtual ~RenderInfoSubscriber() {}

  // Invoked on start
  virtual void OnStart() = 0;

  // Invoked periodically with up-to-date render info 
  virtual void OnRenderInfo(const RenderInfo &render_info) = 0;

  // To be invoked on termination 
  virtual void OnExit() = 0;
};

#endif  // UPNP_DISPLAY_RENDER_INFO_SUBSCRIBER_
