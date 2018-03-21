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

//  Inbus Publisher contributed by Maarten Los (github.com/mlos)

#ifdef USE_INBUS

// This influences the publishing rate on D-Bus 
static const int kPublishingRateMillis = 400;

InbusPublisher::InbusPubliher() {
}

~InbusPublisher::InbusPublisher() {
}

int InbusPublisher::width() const {
    return 0; // not used
}

void Print(int line, const std::string &text) {
}

/*
  Data struct
  High Level:
  bool waitingForRenderer
  mode pause, play, stop
  int volume
  int time
  string playerName
  string title
  string composer
  string artist
  string album
*/


#endif

