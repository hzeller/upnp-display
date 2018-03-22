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

#include "upnp-display.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>

#include <upnp/ithread.h>

#include "render-info.h"
#include "render-info-subscriber.h"
#include "renderer-state.h"

// Time between display updates.
// This influences scroll speed and 'pause' blinking.
// Note, too fast scrolling looks blurry on cheap displays.
static const int kDisplayUpdateMillis = 400;

// Number of periods, a changed volume flashes up.
static const int kVolumeFlashTime = 3;

// We do the signal receiving the classic static way, as creating callbacks to
// c functions is more readable than with c++ methods :)
volatile bool signal_received = false;
static void SigReceiver(int) {
  signal_received = true;
}

UPnPDisplay::UPnPDisplay(const std::string &friendly_name, RenderInfoSubscriber *subscriber)
  : player_match_name_(friendly_name),
    subscriber_(subscriber), current_state_(NULL) {
  ithread_mutex_init(&mutex_, NULL);
  signal(SIGTERM, &SigReceiver);
  signal(SIGINT, &SigReceiver);
}
  
void UPnPDisplay::Loop() {

  RenderInfo render_info;
  signal_received = false;
  subscriber_->OnStart();

  while (!signal_received) {
    usleep(kDisplayUpdateMillis * 1000);

    render_info.is_waiting_for_renderer = true;
    ithread_mutex_lock(&mutex_);
    if (current_state_ != NULL) {
      render_info.is_waiting_for_renderer = false;
      render_info.player_name = current_state_->friendly_name();
      render_info.title = current_state_->GetVar("Meta_Title");
      render_info.composer = current_state_->GetVar("Meta_Composer");
      render_info.artist = current_state_->GetVar("Meta_Artist");
      std::string creator = current_state_->GetVar("Meta_Creator");
      if (render_info.artist == render_info.composer 
        && !creator.empty() && creator != render_info.artist) {
        render_info.artist = creator;
      }
      render_info.album = current_state_->GetVar("Meta_Album");
      render_info.play_state = parsePlayState(current_state_->GetVar("TransportState"));
      render_info.time = parseTime(current_state_->GetVar("RelativeTimePosition"));
      //duration = parseTime(current_state_->GetVar("CurrentTrackDuration"));
      render_info.volume = current_state_->GetVar("Volume");
    }
    ithread_mutex_unlock(&mutex_);

    subscriber_->OnRenderInfo(render_info);
  }

  subscriber_->OnExit();
}

void UPnPDisplay::AddRenderer(const std::string &uuid,
                              const RendererState *state) {
  printf("%s: connected (uuid=%s)\n",            // not to LCD, different thread
         state->friendly_name().c_str(), uuid.c_str());
  ithread_mutex_lock(&mutex_);
  if (current_state_ == NULL
      && (player_match_name_.empty()
          || player_match_name_ == uuid
          || player_match_name_ == state->friendly_name())) {
    uuid_ = uuid;
    current_state_ = state;
  }
  ithread_mutex_unlock(&mutex_);
}

void UPnPDisplay::RemoveRenderer(const std::string &uuid) {
  printf("disconnect (uuid=%s)\n", uuid.c_str()); // not to LCD, different thread
  ithread_mutex_lock(&mutex_);
  if (current_state_ != NULL && uuid == uuid_) {
    current_state_ = NULL;
  }
  ithread_mutex_unlock(&mutex_);
}
                                     
int UPnPDisplay::parseTime(const std::string &upnp_time) {
  int hour = 0;
  int minute = 0;
  int second = 0;
  if (sscanf(upnp_time.c_str(), "%d:%02d:%02d", &hour, &minute, &second) == 3)
    return hour * 3600 + minute * 60 + second;
  return 0;
}

PlayState UPnPDisplay::parsePlayState(const std::string &play_state) {
  if(play_state == "STOPPED")
    return Stopped;
  else if(play_state == "PAUSED_PLAYBACK")
    return Paused;
  else if(play_state == "PLAYING")
    return Playing;

  return Stopped;
}

