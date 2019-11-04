// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
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
#ifndef UPNP_DISPLAY_CONTROLLER_STATE_
#define UPNP_DISPLAY_CONTROLLER_STATE_

#include <upnp.h>
#include <upnp/ithread.h>

#include <string>
#include <map>

class ControllerObserver;
class RendererState;

class ControllerState {
public:
  ControllerState(ControllerObserver *observer);

private:
  void Register(const UpnpDiscovery *discovery);
  void Unregister(const UpnpDiscovery *discovery);
  void ReceiveEvent(const UpnpEvent *data);

  // Callback from upnp library.
  static int UpnpEventHandler(Upnp_EventType_e event, const void *event_data,
                              void *userdata);

  ControllerObserver *const observer_;

  UpnpClient_Handle device_;
  ithread_mutex_t mutex_;
  typedef std::map<std::string, RendererState *> RenderMap;
  RenderMap uuid2render_;
  RenderMap subscription2render_;
};

#endif  // UPNP_DISPLAY_CONTROLLER_STATE_
