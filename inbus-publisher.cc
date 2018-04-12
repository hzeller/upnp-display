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

#ifdef USE_INBUS

#include "inbus-publisher.h"

#include <sstream>

#include <inbus/publisher.h>

InbusPublisher::InbusPublisher(const std::string& app_key, int app_type) 
    : appKey_(app_key)
    , appType_(app_type)
{
}

InbusPublisher::~InbusPublisher() {
}

void InbusPublisher::OnStart() {
  lastRenderInfo_.is_waiting_for_renderer = true;
  inbusPublisher_ = new Publisher(appKey_);
  inbusPublisher_->publish(CreateJSONMessage(lastRenderInfo_), appType_);
}

void InbusPublisher::OnRenderInfo(const RenderInfo &render_info) {

  if(HasNewRenderInfo(render_info)) {
    inbusPublisher_->publish(CreateJSONMessage(render_info));
  }
  lastRenderInfo_ = render_info;
}

void InbusPublisher::OnExit() {
  delete inbusPublisher_;
}

bool InbusPublisher::HasNewRenderInfo(const RenderInfo &render_info) {

  return (
    (render_info.is_waiting_for_renderer != lastRenderInfo_.is_waiting_for_renderer)
    || (render_info.play_state != lastRenderInfo_.play_state)
    || (render_info.title != lastRenderInfo_.title)
    || (render_info.composer != lastRenderInfo_.composer)
    || (render_info.artist != lastRenderInfo_.artist)
    || (render_info.album != lastRenderInfo_.album)
  );
}

std::string InbusPublisher::CreateJSONMessage(const RenderInfo &render_info) {

  std::ostringstream message;
  message << "{"
    << "\"ready\":" << (render_info.is_waiting_for_renderer ? 0 : 1)
    << ",\"playstate\":" << (int)render_info.play_state
    << ",\"title\": \"" << render_info.title << "\""
    << ",\"composer\":\"" << render_info.composer << "\""
    << ",\"artist\":\"" << render_info.artist << "\""
    << ",\"album\":\"" << render_info.album << "\""
    << "}";

  return message.str();
}



#endif // USE_INBUS

