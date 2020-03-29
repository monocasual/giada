/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */


#include <FL/fl_draw.H>
#include "../../../core/conf.h"
#include "../../../core/const.h"
#include "dial.h"

// drag distance to map widget value range, 10% of screen height
const int geDial::dragDistanceToMax = Fl::h() * 0.1f;

namespace {
constexpr float numberOfMouseWheelSteps = 30;
constexpr float mouseWheelDelta = 1.0f / numberOfMouseWheelSteps;
}

geDial::geDial(int x, int y, int w, int h, const char *l)
: Fl_Dial(x, y, w, h, l)
{
  labelsize(G_GUI_FONT_SIZE_BASE);
  labelcolor(G_COLOR_LIGHT_2);
  align(FL_ALIGN_LEFT);
  type(FL_FILL_DIAL);
  angles(0, 360);
  color(G_COLOR_GREY_2);            // background
  selection_color(G_COLOR_GREY_4);   // selection
}


/* -------------------------------------------------------------------------- */


void geDial::draw()
{
  double angle = (angle2()-angle1())*(value()-minimum())/(maximum()-minimum()) + angle1();

  fl_color(G_COLOR_GREY_2);
  fl_pie(x(), y(), w(), h(), 270-angle1(), angle > angle1() ? 360+270-angle : 270-360-angle);

  fl_color(G_COLOR_GREY_4);
  fl_arc(x(), y(), w(), h(), 0, 360);
  fl_pie(x(), y(), w(), h(), 270-angle, 270-angle1());
}

int geDial::handle(int event)
{
  if(giada::m::conf::conf.linearDialControl) {
    switch(event) {
      case FL_PUSH: {
        if ( Fl::event_state(FL_BUTTON1) ) {
          dragBeginY = Fl::event_y();
          valueBeginDrag = value();
        }
        return 1;
      }
      case FL_DRAG: {
        if ( Fl::event_state(FL_BUTTON1) ) {
          if ( dragging == false ) { // catch the "click" event
            dragging = true;
          }

          float deltaY = dragBeginY - Fl::event_y();
          const auto newValue = std::min(1.0f, std::max(0.0f, valueBeginDrag + deltaY / dragDistanceToMax));

          set_value( newValue );

          redraw();
          do_callback(); // makes FLTK call "extra" code entered in FLUID
          return 1;
        }
      }
      case FL_RELEASE: {
        dragging = false;
        return 1;
      }
      case FL_MOUSEWHEEL: {
        const auto newValue = std::min(1.0, std::max(0.0, value() - Fl::event_dy() * mouseWheelDelta));
        set_value( newValue );
        redraw();
        do_callback();
        return 1;
      }
    }
  }
	return Fl_Dial::handle(event);
}
