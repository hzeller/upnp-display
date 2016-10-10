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

#include "dbus-publisher.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>

#include <upnp/ithread.h>

#include "renderer-state.h"

// This influences the publishing rate on D-Bus 
static const int kPublishingRateMillis = 400;

/*
  Data struct
  High Level:
  bool waitingForRenderer
  mode pause, play, stop
  int volume
  int time
  string waitingForRendererName - the name for which to wait
  string playerName
  string title
  string composer
  string artist
  string album
*/

// We do the signal receiving the classic static way, as creating callbacks to
// c functions is more readable than with c++ methods :)
volatile bool dbus_publisher_signal_received = false;
static void SigReceiver(int signo) {
  dbus_publisher_signal_received = true;
}

DBusPublisher::DBusPublisher(const std::string &friendly_name)
  : player_match_name_(friendly_name), current_state_(NULL) {
  ithread_mutex_init(&mutex_, NULL);
  signal(SIGTERM, &SigReceiver);
  signal(SIGINT, &SigReceiver);
}
  
void DBusPublisher::Loop() {
  std::string player_name;
  std::string title, composer, artist, album;
  std::string play_state = "STOPPED";
  std::string volume, previous_volume;
  int time = 0;

  dbus_publisher_signal_received = false;
  while (!dbus_publisher_signal_received) {
    usleep(kPublishingRateMillis * 1000);

    bool renderer_available = false;
    ithread_mutex_lock(&mutex_);
    if (current_state_ != NULL) {
      renderer_available = true;
      player_name = current_state_->friendly_name();
      title = current_state_->GetVar("Meta_Title");
      composer = current_state_->GetVar("Meta_Composer");
      artist = current_state_->GetVar("Meta_Artist");
      std::string creator = current_state_->GetVar("Meta_Creator");
      if (artist == composer && !creator.empty() && creator != artist) {
        artist = creator;
      }
      album = current_state_->GetVar("Meta_Album");
      play_state = current_state_->GetVar("TransportState");
      time = parseTime(current_state_->GetVar("RelativeTimePosition"));
      //duration = parseTime(current_state_->GetVar("CurrentTrackDuration"));
      volume = current_state_->GetVar("Volume");
    }
    ithread_mutex_unlock(&mutex_);

    printf("Available: %s\n", renderer_available ? "yes" : "no");
    printf("Player: %s - Title: %s - Composer: %s - Artist: %s - Album: %s\n", 
      player_name.c_str(), title.c_str(), composer.c_str(), artist.c_str(), album.c_str());
    printf("PlayState: %s - Volume: %s - Time: %d\n", play_state.c_str(), volume.c_str(), time);
  }

  // XXX TODO: Publish "goodbye" signal
}

void DBusPublisher::AddRenderer(const std::string &uuid,
                              const RendererState *state) {
  printf("%s: connected (uuid=%s)\n",            // not to DBus publisher, different thread
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

void DBusPublisher::RemoveRenderer(const std::string &uuid) {
  printf("disconnect (uuid=%s)\n", uuid.c_str()); // not to DBus publisher, different thread
  ithread_mutex_lock(&mutex_);
  if (current_state_ != NULL && uuid == uuid_) {
    current_state_ = NULL;
  }
  ithread_mutex_unlock(&mutex_);
}
                                     
int DBusPublisher::parseTime(const std::string &upnp_time) {
  int hour = 0;
  int minute = 0;
  int second = 0;
  if (sscanf(upnp_time.c_str(), "%d:%02d:%02d", &hour, &minute, &second) == 3)
    return hour * 3600 + minute * 60 + second;
  return 0;
}
