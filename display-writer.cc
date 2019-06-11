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

#include "display-writer.h"

#include <stdio.h>
#include <stdlib.h>

#include "printer.h"
#include "scroller.h"
#include "utf8.h"

// Number of periods, a changed volume flashes up.
static const int kVolumeFlashTime = 3;

#define STOP_SYMBOL "\u2b1b"   // ⬛
#define PLAY_SYMBOL "\u25b6"   // ▶
#define PAUSE_SYMBOL "]["      // TODO: add symbol in private unicode range.

DisplayWriter::DisplayWriter(Printer *printer) :
  printer_(printer),
  blink_time(0),
  volume_countdown(0) {

  first_line_scroller = new Scroller("  -  ");
  second_line_scroller = new Scroller("  -  ");
}

DisplayWriter::~DisplayWriter() {

  delete second_line_scroller;
  delete first_line_scroller;
}
  
void DisplayWriter::OnStart() {
}

void DisplayWriter::OnRenderInfo(const RenderInfo &render_info) {

  if (render_info.is_waiting_for_renderer) {
    printer_->Print(0, "Waiting for");
    std::string to_print = (render_info.player_name.empty()
                            ? "any Renderer"
                            : render_info.player_name);
    CenterAlign(&to_print, printer_->width());
    printer_->Print(1, to_print);
    return;
  }

  // First line is "[composer: ]Title"
  std::string print_line = render_info.composer;
  if (!print_line.empty()) print_line.append(": ");
    print_line.append(render_info.title);

  const bool no_title_to_display = (print_line.empty() && render_info.album.empty());
  if (no_title_to_display) {
    // No title, so show at least player name.
    print_line = render_info.player_name;
    CenterAlign(&print_line, printer_->width());
    printer_->Print(0, print_line);
  }

  // Second line: if there is a volume change, display it for kVolumeFlashTime
  if (render_info.volume != previous_volume || volume_countdown > 0) {
    if (!previous_volume.empty()) {
      if (render_info.volume != previous_volume) {
        volume_countdown = kVolumeFlashTime;
      } else {
        --volume_countdown;
      }
      std::string volume_line = "Volume " + render_info.volume;
      CenterAlign(&volume_line, printer_->width());
      printer_->Print(1, volume_line);
    }
    previous_volume = render_info.volume;
    return;
  }

  if (no_title_to_display) {
    // Nothing really to display ? Show play-state.
    print_line = formatPlayState(render_info.play_state);
    if (render_info.play_state == Stopped)
      print_line = STOP_SYMBOL " [Stopped]";
    else if (render_info.play_state == Paused)
      print_line = PAUSE_SYMBOL" [Paused]";
    else if (render_info.play_state == Playing)
      print_line = PLAY_SYMBOL " [Playing]";

    CenterAlign(&print_line, printer_->width());
    printer_->Print(1, print_line);
    return;
  }

  // Alright, we have a title. If short enough, center, otherwise scroll.
  CenterAlign(&print_line, printer_->width());
  first_line_scroller->SetValue(print_line, printer_->width());
  printer_->Print(0, first_line_scroller->GetScrolledContent());

  std::string formatted_time;
  if (render_info.play_state == Stopped) {
    formatted_time = "  " STOP_SYMBOL " ";
  } else {
    formatted_time = formatTime(render_info.time);
    // 'Blinking' time when paused.
    if (render_info.play_state == Paused && blink_time % 2 == 0) {
      formatted_time = std::string(formatted_time.size(), ' ');
    }
  }

  // Assemble second line from album and artist.
  print_line = render_info.album;
  if (!render_info.artist.empty() && render_info.artist != render_info.album) {
    if (!print_line.empty()) print_line.append("/");
    print_line.append(render_info.artist);
  }

  // Show album/artist right aligned in space next to time. Or scroll if long.
  const int remaining_len = printer_->width() - formatted_time.length() - 1;
  RightAlign(&print_line, remaining_len);
  second_line_scroller->SetValue(print_line, remaining_len);
  printer_->Print(1, formatted_time + " "
                  + second_line_scroller->GetScrolledContent());

  blink_time++;
  first_line_scroller->NextTick();
  second_line_scroller->NextTick();
}

void DisplayWriter::OnExit() {

  std::string msg = "Goodbye!";
  CenterAlign(&msg, printer_->width());
  printer_->Print(0, msg);
  // Show off unicode :)
  msg = "\u2192 \u266a\u266b\u266a\u2669 \u2190";  // → ♪♫♩ ←
  CenterAlign(&msg, printer_->width());
  printer_->Print(1, msg);
}

std::string DisplayWriter::formatTime(int time) {
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

std::string DisplayWriter::formatPlayState(PlayState play_state) {
  if(play_state == Paused)
    return "PAUSED_PLAYBACK";
  else if(play_state == Stopped)
    return "STOPPED";
  if(play_state == Playing)
    return "PLAYING";
  return "???";
}

void DisplayWriter::CenterAlign(std::string *to_print, int width) {
  const int len = utf8_character_count(to_print->begin(), to_print->end());
  if (len < width) {
    to_print->insert(0, std::string((width - len) / 2, ' '));
  }
}

void DisplayWriter::RightAlign(std::string *to_print, int width) {
  const int len = utf8_character_count(to_print->begin(), to_print->end());
  if (len < width) {
    to_print->insert(0, std::string(width - len, ' '));
  }
}

