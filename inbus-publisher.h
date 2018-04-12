// - -*- c++ -*-
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

#ifndef UPNP_DISPLAY_INBUS_PUBLISHER_H
#define UPNP_DISPLAY_INBUS_PUBLISHER_H

#include "render-info-subscriber.h"

// Only publishes a message if one of the following 
// render-info fields changes:
//  - waiting for renderer
//  - play_state
//  - title
//  - composer
//  - artist
//  - album
//
// Publishes to Inbus in the following JSON format.
// {
//     "ready" : <int>,
//     "playstate" : <int>,
//     "title" : <string>,
//     "composer" : <string>,
//     "artist" : <string>,
//     "album" : <string>
// }
// 
// where 
//    <ready>: 0=waiting for renderer 1=renderer available
//    <play-state>: 0=paused, 1=playing, 2=stopped
// 
class Publisher;

class InbusPublisher : public RenderInfoSubscriber {

public:
  InbusPublisher(const std::string& app_key, int app_type);
  virtual ~InbusPublisher();

  virtual void OnStart();
  virtual void OnRenderInfo(const RenderInfo &render_info);
  virtual void OnExit();

private:
  Publisher* inbusPublisher_; 
  RenderInfo lastRenderInfo_;
  std::string appKey_;
  int appType_;
    
  bool HasNewRenderInfo(const RenderInfo &render_info);
  std::string CreateJSONMessage(const RenderInfo &render_info);
  
};

#endif  // UPNP_DISPLAY_INBUS_PUBLISHER_H
