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

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <upnp/ithread.h>

#include "observer.h"
#include "controller-state.h"
#include "printer.h"
#include "renderer-state.h"
#include "lcd-display.h"
#include "scroller.h"

// Comment this out, if you don't run this on a Raspberry Pi; then it will
// do some basic printing on the console.
#define USE_RASPBERRY_LCD

// Width of your display. Usually this is just 16 wide, but you can get 24 or
// even 40 wide displays.
#define LCD_DISPLAY_WIDTH 16

// Time between display updates. This influences scroll speed. Note, too fast
// scrolling looks blurry on cheap displays.
static const int kDisplayUpdateMillis = 200;

// The actual 'UI': wait for matching
class UIFormatter : public ControllerObserver {
public:
  UIFormatter(const std::string &friendly_name, Printer *printer)
    : player_match_name_(friendly_name),
      printer_(printer), current_state_(NULL) {
    ithread_mutex_init(&mutex_, NULL);
  }
  
  void Loop() {
    std::string player_name;
    std::string title, composer, artist, album;
    std::string play_state = "STOPPED";
    int time = 0;
    Scroller first_line_scroller;
    Scroller second_line_scroller;
    std::string last_play_state;
    unsigned char blink_time = 0;
    for (;;) {
      usleep(kDisplayUpdateMillis * 1000);
      bool got_var = false;
      ithread_mutex_lock(&mutex_);
      if (current_state_ != NULL) {
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
        got_var = true;
      }
      ithread_mutex_unlock(&mutex_);
      if (!got_var) {
        printer_->Print(0, "Waiting for");
        std::string to_print = (player_match_name_.empty()
                                ? "any Renderer"
                                : player_match_name_);
        CenterAlign(&to_print, printer_->width());
        printer_->Print(1, to_print);
        continue;
      }

      if (play_state != last_play_state) {
        blink_time = 0;
        last_play_state = play_state;
      }
      std::string print_line = composer;
      if (!print_line.empty()) print_line.append(":");
      print_line.append(title);

      if (print_line.empty()) {
        // Nothing else to display ? Say play-state.
        print_line = player_name;
        CenterAlign(&print_line, printer_->width());
        printer_->Print(0, print_line);

        print_line = play_state;
        if (play_state == "STOPPED")              print_line = "[Stopped]";
        else if (play_state == "PAUSED_PLAYBACK") print_line = "[Paused]";
        CenterAlign(&print_line, printer_->width());
        printer_->Print(1, print_line);
        continue;
      }

      // Alright, we have a title, print that.
      CenterAlign(&print_line, printer_->width());
      first_line_scroller.SetValue(print_line, printer_->width());
      printer_->Print(0, first_line_scroller.GetScrolledContent());

      std::string formatted_time;
      if (play_state != "STOPPED") {
        formatted_time = formatTime(time);
        // 'Blinking' time when paused.
        if (play_state == "PAUSED_PLAYBACK" && (blink_time / 2) % 2 == 0) {
          formatted_time = std::string(formatted_time.size(), ' ');
        }
      }

      int remaining_len = printer_->width() - formatted_time.length() - 1;
      print_line = album;
      if (!artist.empty() && artist != album) {
        if (!print_line.empty()) print_line.append("/");
        print_line.append(artist);
      }
      RightAlign(&print_line, remaining_len);
      second_line_scroller.SetValue(print_line, remaining_len);
      printer_->Print(1, formatted_time + " "
                      + second_line_scroller.GetScrolledContent());

      blink_time++;
      first_line_scroller.NextTick();
      second_line_scroller.NextTick();
    }
  }

  virtual void AddRenderer(const std::string &uuid,
                           const RendererState *state) {
    printf("%s: connected (uuid=%s)\n",
           state->friendly_name().c_str(), uuid.c_str());
    if (current_state_ == NULL
        && (player_match_name_.empty()
            || player_match_name_ == state->friendly_name())) {
      ithread_mutex_lock(&mutex_);
      uuid_ = uuid;
      current_state_ = state;
      ithread_mutex_unlock(&mutex_);
      // This is a different thread than main(); don't print anything here.
    }
  }

  virtual void RemoveRenderer(const std::string &uuid) {
    printf("disconnect (uuid=%s)\n", uuid.c_str());
    if (current_state_ != NULL && uuid == uuid_) {
      ithread_mutex_lock(&mutex_);
      current_state_ = NULL;
      ithread_mutex_unlock(&mutex_);
    }
  }

                                     
private:
  int parseTime(const std::string &upnp_time) {
    int hour = 0;
    int minute = 0;
    int second = 0;
    if (sscanf(upnp_time.c_str(), "%d:%02d:%02d", &hour, &minute, &second) == 3)
      return hour * 3600 + minute * 60 + second;
    return 0;
  }

  std::string formatTime(int time) {
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

  void CenterAlign(std::string *to_print, int width) {
    if ((int)to_print->length() < width) {
      to_print->insert(0, std::string((width - to_print->length()) / 2, ' '));
    }
  }

  void RightAlign(std::string *to_print, int width) {
    if ((int)to_print->length() < width) {
      to_print->insert(0, std::string((width - to_print->length()), ' '));
    }
  }

  const std::string player_match_name_;
  Printer *const printer_;
  ithread_mutex_t mutex_;

  std::string uuid_;
  const RendererState *current_state_;
};

int main(int argc, char *argv[]) {
  std::string match_name;
  int opt;
  while ((opt = getopt(argc, argv, "n:")) != -1) {
    switch (opt) {
    case 'n':
      if (optarg != NULL) match_name = optarg;
      break;
    default: /* '?' */
      fprintf(stderr, "Usage: %s [-n name]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

#ifdef USE_RASPBERRY_LCD
  LCDDisplay printer(LCD_DISPLAY_WIDTH);
  if (!printer.Init()) {
    fprintf(stderr, "You need to run this as root to have access to GPIO pins. "
            "Run with sudo.\n");
    return 1;
  }
#else
  ConsolePrinter printer;
#endif

  // TODO: drop priviliges

  UIFormatter ui(match_name, &printer);
  ControllerState controller(&ui);
  ui.Loop();
  return 0;
}
