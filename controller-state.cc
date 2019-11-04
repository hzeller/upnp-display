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

#include "controller-state.h"

#include <assert.h>
#include <string.h>

#include "observer.h"
#include "renderer-state.h"

static const char kMediaRendererDevice[] =
  "urn:schemas-upnp-org:device:MediaRenderer:1";

ControllerState::ControllerState(ControllerObserver *observer)
  : observer_(observer) {
  assert(observer != NULL);  // without, it wouldn't make much sense.
  ithread_mutex_init(&mutex_, NULL);
  UpnpInit(NULL, 0);
  UpnpRegisterClient(&UpnpEventHandler, this, &device_);
}

void ControllerState::Register(const UpnpDiscovery *discovery) {
  if (strcmp(UpnpDiscovery_get_DeviceType_cstr(discovery),
             kMediaRendererDevice) != 0) {
    return;
  }

  const std::string uuid = UpnpDiscovery_get_DeviceID_cstr(discovery);
  RendererState *renderer = NULL;

  ithread_mutex_lock(&mutex_);
  renderer = uuid2render_[uuid];
  if (renderer == NULL) {
    renderer = new RendererState(UpnpDiscovery_get_Location_cstr(discovery));
    uuid2render_[uuid] = renderer;
    if (renderer->InitDescription(UpnpDiscovery_get_Location_cstr(discovery))) {
      renderer->SubscribeTo(device_, &subscription2render_);
    }
    observer_->AddRenderer(uuid, renderer);
  }
  ithread_mutex_unlock(&mutex_);
}

void ControllerState::Unregister(const UpnpDiscovery *discovery) {
  const std::string uuid = UpnpDiscovery_get_DeviceID_cstr(discovery);

  ithread_mutex_lock(&mutex_);
  RenderMap::iterator found = uuid2render_.find(uuid);
  if (found != uuid2render_.end()) {
    observer_->RemoveRenderer(uuid);
    delete found->second;
    uuid2render_.erase(uuid);
  }
  ithread_mutex_unlock(&mutex_);
}

void ControllerState::ReceiveEvent(const UpnpEvent *data) {
  const std::string sid = UpnpEvent_get_SID_cstr(data);
  ithread_mutex_lock(&mutex_);
  RenderMap::iterator found = subscription2render_.find(sid);
  if (found != subscription2render_.end()) {
    found->second->ReceiveEvent(data);
  }
  ithread_mutex_unlock(&mutex_);
}

int ControllerState::UpnpEventHandler(Upnp_EventType_e event,
                                      const void *event_data,
                                      void *userdata) {
  ControllerState *state = static_cast<ControllerState*>(userdata);
  switch (event) {
  case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
    state->Register(static_cast<const UpnpDiscovery*>(event_data));
    break;

  case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
  case UPNP_EVENT_AUTORENEWAL_FAILED:
    state->Unregister(static_cast<const UpnpDiscovery*>(event_data));
    break;

  case UPNP_EVENT_RECEIVED:
    state->ReceiveEvent(static_cast<const UpnpEvent*>(event_data));
    break;

  default:
    // don't care.
    ;
  }
  return UPNP_E_SUCCESS;
}
