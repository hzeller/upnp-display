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

class UIFormatter : public ControllerObserver {
  class Scroller {
    static const int kBorderWait = 4;  // ticks to wait at end-of-scroll
  public:
    Scroller() : pos_(0), scroll_timeout_(0) {}

    void SetValue(std::string &content, int width) {
      if (content != content_ || width != width_) {
        content_ = content;
        width_ = width;
        pos_ = 0;
        scroll_timeout_ = kBorderWait;
      }
    }

    std::string GetScrolledContent() {
      return content_.substr(pos_, width_);
    }

    void Advance() {
      if (scroll_timeout_ > 0) {
        scroll_timeout_--;
      } else {
        pos_++;
        const int display_portion = content_.length() - pos_;
        if (display_portion == width_) {
          scroll_timeout_ = kBorderWait;
        } else if (display_portion < width_) {
          pos_ = 0;
          scroll_timeout_ = kBorderWait;
        }
      }
    }

  private:
    int width_;
    std::string content_;
    int pos_;
    int scroll_timeout_;
  };

public:
  UIFormatter(const std::string &friendly_name, Printer *printer)
    : match_name_(friendly_name), printer_(printer), current_state_(NULL) {
    ithread_mutex_init(&mutex_, NULL);
    printer_->Print(1, "Waiting for");
    std::string to_print = match_name_.empty() ? "any Renderer" : match_name_;
    CenterAlign(&to_print, printer_->width());
    printer_->Print(2, to_print);
  }
  
  void Loop() {
    std::string title, composer, artist, album;
    std::string play_state = "STOPPED";
    int time = 0;
    Scroller first_line_scroller;
    Scroller second_line_scroller;
    std::string last_play_state;
    unsigned char blink_time = 0;
    for (;;) {
      usleep(150000);
      bool got_var = false;
      ithread_mutex_lock(&mutex_);
      if (current_state_ != NULL) {
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
      if (!got_var)
        continue;

      if (play_state != last_play_state) {
        blink_time = 0;
        last_play_state = play_state;
      }
      std::string print_line = composer;
      if (!print_line.empty()) print_line.append(":");
      print_line.append(title);
      if (print_line.empty()) {
        // Nothing else to display ? Say play-state.
        if (play_state == "STOPPED")         print_line = "[Stopped]";
        else if (play_state == "PAUSED_PLAYBACK") print_line = "[Paused]";
      }
      CenterAlign(&print_line, printer_->width());
      first_line_scroller.SetValue(print_line, printer_->width());
      printer_->Print(1, first_line_scroller.GetScrolledContent());

      std::string formatted_time;
      if (play_state != "STOPPED") {
        formatted_time = formatTime(time);
        // 'Blinking' time when paused.
        if (play_state == "PAUSED_PLAYBACK" && (blink_time / 3) % 2 == 0) {
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
      printer_->Print(2, formatted_time + " "
                      + second_line_scroller.GetScrolledContent());

      blink_time++;
      first_line_scroller.Advance();
      second_line_scroller.Advance();
    }
  }

  virtual void AddRenderer(const std::string &uuid,
                           const RendererState *state) {
    if (current_state_ == NULL
        && (match_name_.empty() || match_name_ == state->friendly_name())) {
      ithread_mutex_lock(&mutex_);
      uuid_ = uuid;
      current_state_ = state;
      ithread_mutex_unlock(&mutex_);
      printer_->Print(1, "Connected to");
      printer_->Print(2, state->friendly_name());
    }
  }

  virtual void RemoveRenderer(const std::string &uuid) {
    if (current_state_ != NULL && uuid == uuid_) {
      printer_->Print(1, "Disconnected");
      printer_->Print(2, current_state_->friendly_name());
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

  const std::string match_name_;
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

  ConsolePrinter printer;
  //LCDDisplay printer;

  UIFormatter ui(match_name, &printer);
  ControllerState controller(&ui);
  ui.Loop();
  return 0;
}
