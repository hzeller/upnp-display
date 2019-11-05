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
#ifndef UPNP_DISPLAY_H
#define UPNP_DISPLAY_H

#include <string>

#include "observer.h"
#include <ithread.h>

class Printer;

class UPnPDisplay : public ControllerObserver {
public:
  // Creates upnp display that waits for a renderer with the given
  // registered name (if empty string, waits for the first available).
  // Outputs to "printer".
  UPnPDisplay(const std::string &renderer_registered_name, Printer *printer,
              int screensave_timeout);

  // Main Loop. Only exits on catching SIGTERM or SIGINT (Ctrl-c)
  void Loop();

  // -- Implementation of ControllerObserver interface.

  // Receive notification of new renderer added.
  virtual void AddRenderer(const std::string &uuid,
                           const RendererState *state);
  // Receive notification of renderer removed.
  virtual void RemoveRenderer(const std::string &uuid);

private:
  // Parse time from UPnP variable.
  int parseTime(const std::string &upnp_time);

  // Format time for display.
  std::string formatTime(int time);

  // Text formatting utility.
  void CenterAlign(std::string *to_print, int width);
  void RightAlign(std::string *to_print, int width);

  const std::string player_match_name_;
  Printer *const printer_;
  const int screensave_timeout_;
  ithread_mutex_t mutex_;

  std::string uuid_;
  const RendererState *current_state_;
};

#endif  // UPNP_DISPLAY_H
