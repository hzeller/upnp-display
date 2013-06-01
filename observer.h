//  -*- c++ -*-
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
#ifndef UPNP_OBSERVER_
#define UPNP_OBSERVER_

#include <string>

class RendererState;

// An observer, to be implemented by objects that want to know when
// new renderers become available on the network or are removed.
// Observers are allowed to use the RenderState object they get in AddRenderer()
// until RemoveRenderer() is called for that uuid. After that, the object is
// not available anymore.
class ControllerObserver {
public:
  virtual ~ControllerObserver() {}
  virtual void AddRenderer(const std::string &uuid,
			   const RendererState *state) = 0;
  virtual void RemoveRenderer(const std::string &uuid) = 0;
};

#endif // UPNP_OBSERVER_
