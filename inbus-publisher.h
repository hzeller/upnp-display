// -*- c++ -*-
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

#ifndef UPNP_DISPLAY_INBUS_PUBLISHER_H
#define UPNP_DISPLAY_INBUS_PUBLISHER_H

#include "printer.h"

class InbusPublisher : public Printer {

  virtual ~InbusPublisher();
  virtual int width() const;
  virtual void Print(int line, const std::string &text);
};

#endif // UPNP_DISPLAY_PRINTER_ 
};

#endif  // UPNP_DISPLAY_INBUS_PUBLISHER_H
