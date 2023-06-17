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
#ifndef RENDERER_STATE_H
#define RENDERER_STATE_H

#include <map>
#include <string>
#include <vector>

#include <pthread.h>
#include <ixml.h>
#include <time.h>
#include <upnp.h>

// Representing the state for a particular renderer.
class RendererState {
public:
  typedef std::map<std::string, RendererState *> SubscriptionMap;

  RendererState(const char *uuid);
  ~RendererState();

  // -- method calls interesting for users.
  // Returns the human readable name of the renderer (e.g. "Living Room")
  const std::string friendly_name() const { return friendly_name_; }

  // Get variable with given name. Text is encoded in UTF-8.
  // Thread safe.
  std::string GetVar(const std::string &name) const;

  time_t last_event_update() const;

  // -- method calls used for internal upnp subscription management.

  // Initialize from descriptor url that points to an XML file describing
  // the renderer web-service.
  bool InitDescription(const char *descriptior_url);

  // Register interest in variables.
  // TODO: this needs to be handled by a subscription manager or something.
  // It breaks the encapsulation that this stores itself in the subscription_map.
  bool SubscribeTo(UpnpClient_Handle upnp_controller,
                   SubscriptionMap *subscription_map);

  // Callback from controller when changed variables arrive.
  void ReceiveEvent(const UpnpEvent *data);

private:
  bool Subscribe(UpnpClient_Handle upnp_controller,
                 const char *service_type,
                 const char *event_url);

  // Decode DIDL data and insert as Meta_Title, Meta_Artist, Meta_Composer.
  // requires variable_mutex_ to be locked.
  void DecodeMetaAndInsertData_Locked(const char *xml);

  const std::string uuid_;
  std::string friendly_name_;
  std::string base_url_;

  IXML_Document *descriptor_;      // owned. Initialized in InitDescription()
  SubscriptionMap *subscriptions_; // not owned. Initialized in SubscribeTo()

  std::vector<std::string> subscription_ids_;

  mutable pthread_mutex_t variable_mutex_;
  typedef std::map<std::string, std::string> VariableMap;
  time_t last_event_update_;
  VariableMap variables_;
};
#endif // RENDERER_STATE_H
