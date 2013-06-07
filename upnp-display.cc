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

#include "printer.h"
#include "renderer-state.h"
#include "scroller.h"
#include "utf8.h"

// Time between display updates.
// This influences scroll speed and 'pause' blinking.
// Note, too fast scrolling looks blurry on cheap displays.
static const int kDisplayUpdateMillis = 400;

// Number of periods, a changed volume flashes up.
static const int kVolumeFlashTime = 3;

// We do the signal receiving the classic static way, as creating callbacks to
// c functions is more readable than with c++ methods :)
volatile bool signal_received = false;
static void SigReceiver(int signo) {
  signal_received = true;
}

#define STOP_SYMBOL "\u2b1b"   // ⬛
#define PLAY_SYMBOL "\u25b6"   // ▶
#define PAUSE_SYMBOL "]["      // TODO: add symbol in private unicode range.

UPnPDisplay::UPnPDisplay(const std::string &friendly_name, Printer *printer)
  : player_match_name_(friendly_name),
    printer_(printer), current_state_(NULL) {
  ithread_mutex_init(&mutex_, NULL);
  signal(SIGTERM, &SigReceiver);
  signal(SIGINT, &SigReceiver);
}
  
void UPnPDisplay::Loop() {
  std::string player_name;
  std::string title, composer, artist, album;
  std::string play_state = "STOPPED";
  std::string volume, previous_volume;
  int time = 0;

  Scroller first_line_scroller("  -  ");
  Scroller second_line_scroller("  -  ");
  unsigned char blink_time = 0;
  int volume_countdown = 0;

  signal_received = false;
  while (!signal_received) {
    usleep(kDisplayUpdateMillis * 1000);

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

    if (!renderer_available) {
      printer_->Print(0, "Waiting for");
      std::string to_print = (player_match_name_.empty()
                              ? "any Renderer"
                              : player_match_name_);
      CenterAlign(&to_print, printer_->width());
      printer_->Print(1, to_print);
      continue;
    }

    // First line is "[composer: ]Title"
    std::string print_line = composer;
    if (!print_line.empty()) print_line.append(": ");
    print_line.append(title);

    const bool no_title_to_display = (print_line.empty() && album.empty());
    if (no_title_to_display) {
      // No title, so show at least player name.
      print_line = player_name;
      CenterAlign(&print_line, printer_->width());
      printer_->Print(0, print_line);
    }

    // Second line: if there is a volume change, display it for kVolumeFlashTime
    if (volume != previous_volume || volume_countdown > 0) {
      if (!previous_volume.empty()) {
        if (volume != previous_volume) {
          volume_countdown = kVolumeFlashTime;
        } else {
          --volume_countdown;
        }
        std::string volume_line = "Volume " + volume;
        CenterAlign(&volume_line, printer_->width());
        printer_->Print(1, volume_line);
      }
      previous_volume = volume;
      continue;
    }

    if (no_title_to_display) {
      // Nothing really to display ? Show play-state.
      print_line = play_state;
      if (play_state == "STOPPED")
        print_line = STOP_SYMBOL " [Stopped]";
      else if (play_state == "PAUSED_PLAYBACK")
        print_line = PAUSE_SYMBOL" [Paused]";
      else if (play_state == "PLAYING")
        print_line = PLAY_SYMBOL " [Playing]";

      CenterAlign(&print_line, printer_->width());
      printer_->Print(1, print_line);
      continue;
    }

    // Alright, we have a title. If short enough, center, otherwise scroll.
    CenterAlign(&print_line, printer_->width());
    first_line_scroller.SetValue(print_line, printer_->width());
    printer_->Print(0, first_line_scroller.GetScrolledContent());

    std::string formatted_time;
    if (play_state == "STOPPED") {
      formatted_time = "  " STOP_SYMBOL " ";
    } else {
      formatted_time = formatTime(time);
      // 'Blinking' time when paused.
      if (play_state == "PAUSED_PLAYBACK" && blink_time % 2 == 0) {
        formatted_time = std::string(formatted_time.size(), ' ');
      }
    }

    // Assemble second line from album and artist.
    print_line = album;
    if (!artist.empty() && artist != album) {
      if (!print_line.empty()) print_line.append("/");
      print_line.append(artist);
    }

    // Show album/artist right aligned in space next to time. Or scroll if long.
    const int remaining_len = printer_->width() - formatted_time.length() - 1;
    RightAlign(&print_line, remaining_len);
    second_line_scroller.SetValue(print_line, remaining_len);
    printer_->Print(1, formatted_time + " "
                    + second_line_scroller.GetScrolledContent());

    blink_time++;
    first_line_scroller.NextTick();
    second_line_scroller.NextTick();
  }

  std::string msg = "Goodbye!";
  CenterAlign(&msg, printer_->width());
  printer_->Print(0, msg);
  // Show off unicode :)
  msg = "\u2192 \u266a\u266b\u266a\u2669 \u2190";  // → ♪♫♩ ←
  CenterAlign(&msg, printer_->width());
  printer_->Print(1, msg);
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

std::string UPnPDisplay::formatTime(int time) {
  const bool is_neg = (time < 0);
  time = abs(time);
  const int hour = time / 3600; time %= 3600;
  const int minute = time / 60; time %= 60;
  const int second = time;
  char buf[32];
  char *pos = buf;
  if (is_neg) *pos++ = '-';
  if (hour > 0) {
    snprintf(pos, sizeof(buf)-1, "%dh%02d:%02d", hour, minute, second);
  } else {
    snprintf(pos, sizeof(buf)-1, "%d:%02d", minute, second);
  }
  return buf;
}

void UPnPDisplay::CenterAlign(std::string *to_print, int width) {
  const int len = utf8_character_count(to_print->begin(), to_print->end());
  if (len < width) {
    to_print->insert(0, std::string((width - len) / 2, ' '));
  }
}

void UPnPDisplay::RightAlign(std::string *to_print, int width) {
  const int len = utf8_character_count(to_print->begin(), to_print->end());
  if (len < width) {
    to_print->insert(0, std::string(width - len, ' '));
  }
}
