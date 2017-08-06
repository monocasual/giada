/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include <FL/Fl.H>
#include "../../../core/sampleChannel.h"
#include "../../../core/wave.h"
#include "../../../glue/channel.h"
#include "../../../glue/sampleEditor.h"
#include "../../../utils/gui.h"
#include "../../../utils/string.h"
#include "../../dialogs/sampleEditor.h"
#include "../basics/input.h"
#include "../basics/box.h"
#include "../basics/button.h"
#include "waveTools.h"
#include "rangeTool.h"


using namespace giada::c;


geRangeTool::geRangeTool(int x, int y, SampleChannel* ch)
  : Fl_Group(x, y, 300, 20),
    ch      (ch)
{
  begin();
    label  = new geBox  (x, y, gu_getStringWidth("Range"), 20, "Range", FL_ALIGN_RIGHT);
    begin_ = new geInput(label->x()+label->w()+4, y, 70, 20);
    end_   = new geInput(begin_->x()+begin_->w()+4, y, 70, 20);
    reset  = new geButton(end_->x()+end_->w()+4, y, 70, 20, "Reset");
  end();

  begin_->type(FL_INT_INPUT);
  begin_->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
  begin_->callback(cb_setChanPos, this);
  
  end_->type(FL_INT_INPUT);
  end_->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
  end_->callback(cb_setChanPos, this);

  reset->callback(cb_resetStartEnd, this);

  refresh();
}


/* -------------------------------------------------------------------------- */


void geRangeTool::refresh()
{
  begin_->value(gu_itoa(ch->getBegin()).c_str());
  end_->value(gu_itoa(ch->getEnd()).c_str());
}


/* -------------------------------------------------------------------------- */


void geRangeTool::cb_setChanPos   (Fl_Widget* w, void* p) { ((geRangeTool*)p)->__cb_setChanPos(); }
void geRangeTool::cb_resetStartEnd(Fl_Widget* w, void* p) { ((geRangeTool*)p)->__cb_resetStartEnd(); }


/* -------------------------------------------------------------------------- */


void geRangeTool::__cb_setChanPos()
{
  sampleEditor::setBeginEndChannel(ch, atoi(begin_->value()), atoi(end_->value()));
  static_cast<gdSampleEditor*>(window())->waveTools->updateWaveform();
}


/* -------------------------------------------------------------------------- */


void geRangeTool::__cb_resetStartEnd()
{
  sampleEditor::setBeginEndChannel(ch, 0, ch->wave->getSize());
  static_cast<gdSampleEditor*>(window())->waveTools->updateWaveform();
}
