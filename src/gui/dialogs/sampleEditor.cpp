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


#include <cmath>
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>
#include "../../glue/channel.h"
#include "../../glue/sampleEditor.h"
#include "../../core/waveFx.h"
#include "../../core/conf.h"
#include "../../core/const.h"
#include "../../core/graphics.h"
#include "../../core/sampleChannel.h"
#include "../../core/mixer.h"
#include "../../core/wave.h"
#include "../../core/clock.h"
#include "../../utils/gui.h"
#include "../../utils/string.h"
#include "../elems/basics/button.h"
#include "../elems/basics/input.h"
#include "../elems/basics/choice.h"
#include "../elems/basics/dial.h"
#include "../elems/basics/box.h"
#include "../elems/basics/check.h"
#include "../elems/sampleEditor/waveform.h"
#include "../elems/sampleEditor/waveTools.h"
#include "../elems/sampleEditor/volumeTool.h"
#include "../elems/sampleEditor/boostTool.h"
#include "../elems/sampleEditor/panTool.h"
#include "../elems/sampleEditor/pitchTool.h"
#include "../elems/sampleEditor/rangeTool.h"
#include "../elems/mainWindow/keyboard/channel.h"
#include "gd_warnings.h"
#include "sampleEditor.h"


using namespace giada::m;
using namespace giada::c;


gdSampleEditor::gdSampleEditor(SampleChannel *ch)
  : gdWindow(640, 480),
    ch(ch)
{
  begin();

  /* top bar: grid and zoom tools */

  Fl_Group *bar = new Fl_Group(8, 8, w()-16, 20);
  bar->begin();
    grid    = new geChoice(bar->x(), bar->y(), 50, 20);
    snap    = new geCheck(grid->x()+grid->w()+4, bar->y(), 12, 12);
    sep1    = new geBox(snap->x()+snap->w()+4, bar->y(), 506, 20);
    zoomOut = new geButton(sep1->x()+sep1->w()+4, bar->y(), 20, 20, "", zoomOutOff_xpm, zoomOutOn_xpm);
    zoomIn  = new geButton(zoomOut->x()+zoomOut->w()+4, bar->y(), 20, 20, "", zoomInOff_xpm, zoomInOn_xpm);
  bar->end();
  bar->resizable(sep1);

  /* waveform */

  waveTools = new geWaveTools(8, bar->y()+bar->h()+8, w()-16, h()-128, ch);
  waveTools->end();

  /* other tools */

  Fl_Group *row1 = new Fl_Group(8, waveTools->y()+waveTools->h()+8, w()-16, 20);
  row1->begin();
    volumeTool = new geVolumeTool(row1->x(), row1->y(), ch);
    boostTool  = new geBoostTool(volumeTool->x()+volumeTool->w()+4, row1->y(), ch);
    panTool    = new gePanTool(boostTool->x()+boostTool->w()+4, row1->y(), ch);
  row1->end();
  row1->resizable(0);

  Fl_Group *row2 = new Fl_Group(8, row1->y()+row1->h()+8, 800, 20);
  row2->begin();
    pitchTool = new gePitchTool(row2->x(), row2->y(), ch);
  row2->end();
  row2->resizable(0);

  Fl_Group *row3 = new Fl_Group(8, row2->y()+row2->h()+8, w()-16, 20);
  row3->begin();
    rangeTool = new geRangeTool(row3->x(), row3->y(), ch);
    sep2      = new geBox(rangeTool->x()+rangeTool->w()+4, row3->y(), 246, 20);
    reload    = new geButton(sep2->x()+sep2->w()+4, row3->y(), 70, 20, "Reload");
  row3->end();
  row3->resizable(sep2);

  end();

  /* grid tool setup */

  grid->add("(off)");
  grid->add("2");
  grid->add("3");
  grid->add("4");
  grid->add("6");
  grid->add("8");
  grid->add("16");
  grid->add("32");
  grid->add("64");
  if (conf::sampleEditorGridVal == 0)
    grid->value(0);
  else 
    grid->value(grid->find_item(gu_itoa(conf::sampleEditorGridVal).c_str()));
  grid->callback(cb_changeGrid, (void*)this);

  snap->value(conf::sampleEditorGridOn);
  snap->callback(cb_enableSnap, (void*)this);

  /* TODO - redraw grid if != (off) */

  reload->callback(cb_reload, (void*)this);

  zoomOut->callback(cb_zoomOut, (void*)this);
  zoomIn->callback(cb_zoomIn, (void*)this);

  /* logical samples (aka takes) cannot be reloaded. So far. */

  if (ch->wave->isLogical())
    reload->deactivate();

  gu_setFavicon(this);
  size_range(640, 480);
  resizable(waveTools);

  label(ch->wave->getName().c_str());

  set_non_modal();

  if (conf::sampleEditorX)
    resize(conf::sampleEditorX, conf::sampleEditorY, conf::sampleEditorW, conf::sampleEditorH);

  show();
}


/* -------------------------------------------------------------------------- */


gdSampleEditor::~gdSampleEditor()
{
  conf::sampleEditorX = x();
  conf::sampleEditorY = y();
  conf::sampleEditorW = w();
  conf::sampleEditorH = h();
  conf::sampleEditorGridVal = atoi(grid->text());
  conf::sampleEditorGridOn  = snap->value();
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::cb_reload    (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_reload(); }
void gdSampleEditor::cb_zoomIn    (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_zoomIn(); }
void gdSampleEditor::cb_zoomOut   (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_zoomOut(); }
void gdSampleEditor::cb_changeGrid(Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_changeGrid(); }
void gdSampleEditor::cb_enableSnap(Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_enableSnap(); }


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_enableSnap()
{
  waveTools->waveform->setSnap(!waveTools->waveform->getSnap());
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_reload()
{
  if (!gdConfirmWin("Warning", "Reload sample: are you sure?"))
    return;

  /* no need for glue_loadChan, there's no gui to refresh */

  ch->load(ch->wave->getPath().c_str(), conf::samplerate, conf::rsmpQuality);

  glue_setBoost(ch, G_DEFAULT_BOOST);
  glue_setPitch(ch, G_DEFAULT_PITCH);
  glue_setPanning(ch, 1.0f);

  panTool->refresh();
  boostTool->refresh();

  waveTools->waveform->stretchToWindow();
  waveTools->updateWaveform();

  sampleEditor::setBeginEndChannel(ch, 0, ch->wave->getSize());

  redraw();
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_zoomIn()
{
  waveTools->waveform->setZoom(-1);
  waveTools->redraw();
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_zoomOut()
{
  waveTools->waveform->setZoom(0);
  waveTools->redraw();
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_changeGrid()
{
  waveTools->waveform->setGridLevel(atoi(grid->text()));
}
