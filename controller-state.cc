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
#include <stdio.h>

#include "observer.h"
#include "renderer-state.h"

#include <upnptools.h>

// Prefix, as they can be followed by version number.
static const char kMediaRendererDevicePrefix[] =
  "urn:schemas-upnp-org:device:MediaRenderer:";

ControllerState::ControllerState(ControllerObserver *observer,
                                 Printer *printer)
  : observer_(observer) {
  assert(observer != NULL);  // without, it wouldn't make much sense.
  pthread_mutex_init(&mutex_, NULL);
  // If network is not up yet, UpnpInit2() fails. Retry.
  // This can happen if system just booted and DHCP is not settled yet.
  int rc = UpnpInit2(NULL, 0);
  int retries_left = 60;
  static const int kRetryTimeMs = 1000;
  while (rc != UPNP_E_SUCCESS && retries_left--) {
    usleep(kRetryTimeMs * 1000);
    char buffer[40];
    snprintf(buffer, sizeof(buffer), "Network...%d", retries_left);
    printer->Print(0, buffer);
    fprintf(stderr, "UpnpInit2() Error: %s (%d). Retrying...(%ds)",
            UpnpGetErrorMessage(rc), rc, retries_left);
    rc = UpnpInit2(NULL, 0);
  }
  if (rc != UPNP_E_SUCCESS) {
    fprintf(stderr, "UpnpInit2() Error: %s (%d).", UpnpGetErrorMessage(rc), rc);
  }
  UpnpRegisterClient(&UpnpEventHandler, this, &device_);
}

static bool prefixMatch(const char *str, const char *prefix) {
  return strncmp(str, prefix, strlen(prefix)) == 0;
}
void ControllerState::Register(const UpnpDiscovery *discovery) {
  if (!prefixMatch(UpnpDiscovery_get_DeviceType_cstr(discovery),
                   kMediaRendererDevicePrefix)) {
    return;
  }

  const std::string uuid = UpnpDiscovery_get_DeviceID_cstr(discovery);
  RendererState *renderer = NULL;

  pthread_mutex_lock(&mutex_);
  renderer = uuid2render_[uuid];
  if (renderer == NULL) {
    renderer = new RendererState(UpnpDiscovery_get_Location_cstr(discovery));
    uuid2render_[uuid] = renderer;
    if (renderer->InitDescription(UpnpDiscovery_get_Location_cstr(discovery))) {
      renderer->SubscribeTo(device_, &subscription2render_);
    }
    observer_->AddRenderer(uuid, renderer);
  }
  pthread_mutex_unlock(&mutex_);
}

void ControllerState::Unregister(const UpnpDiscovery *discovery) {
  const std::string uuid = UpnpDiscovery_get_DeviceID_cstr(discovery);

  pthread_mutex_lock(&mutex_);
  RenderMap::iterator found = uuid2render_.find(uuid);
  if (found != uuid2render_.end()) {
    observer_->RemoveRenderer(uuid);
    delete found->second;
    uuid2render_.erase(uuid);
  }
  pthread_mutex_unlock(&mutex_);
}

void ControllerState::ReceiveEvent(const UpnpEvent *data) {
  const std::string sid = UpnpEvent_get_SID_cstr(data);
  pthread_mutex_lock(&mutex_);
  RenderMap::iterator found = subscription2render_.find(sid);
  if (found != subscription2render_.end()) {
    found->second->ReceiveEvent(data);
  }
  pthread_mutex_unlock(&mutex_);
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
