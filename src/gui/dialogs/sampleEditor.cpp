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


using std::string;
using namespace giada::m;
using namespace giada::c;


gdSampleEditor::gdSampleEditor(SampleChannel* ch)
  : gdWindow(640, 480),
    ch(ch)
{
  Fl_Group* upperBar = createUpperBar();
  
  waveTools = new geWaveTools(8, upperBar->y()+upperBar->h()+8, w()-16, h()-128, 
  	ch);
  
  Fl_Group* bottomBar = createBottomBar(8, waveTools->y()+waveTools->h()+8, 
  	h()-waveTools->h()-upperBar->h()-32);

  add(upperBar);
  add(waveTools);
  add(bottomBar);

  resizable(waveTools);

  gu_setFavicon(this);
  set_non_modal();
  copy_label(ch->wave->getName().c_str());

  size_range(720, 480);
  if (conf::sampleEditorX)
    resize(conf::sampleEditorX, conf::sampleEditorY, conf::sampleEditorW, 
    	conf::sampleEditorH);
  
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
  sampleEditor::setPreview(ch, G_PREVIEW_NONE);
}


/* -------------------------------------------------------------------------- */


Fl_Group* gdSampleEditor::createUpperBar()
{
  Fl_Group* g = new Fl_Group(8, 8, w()-16, 20);
  g->begin();
    grid    = new geChoice(g->x(), g->y(), 50, 20);
    snap    = new geCheck(grid->x()+grid->w()+4, g->y()+3, 12, 12, "Snap");
    sep1    = new geBox(snap->x()+snap->w()+4, g->y(), 506, 20);
    zoomOut = new geButton(sep1->x()+sep1->w()+4, g->y(), 20, 20, "", zoomOutOff_xpm, zoomOutOn_xpm);
    zoomIn  = new geButton(zoomOut->x()+zoomOut->w()+4, g->y(), 20, 20, "", zoomInOff_xpm, zoomInOn_xpm);
  g->end();
  g->resizable(sep1);

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
    grid->value(grid->find_item(gu_toString(conf::sampleEditorGridVal).c_str()));
  grid->callback(cb_changeGrid, (void*)this);

  snap->value(conf::sampleEditorGridOn);
  snap->callback(cb_enableSnap, (void*)this);

  /* TODO - redraw grid if != (off) */

  zoomOut->callback(cb_zoomOut, (void*)this);
  zoomIn->callback(cb_zoomIn, (void*)this);

  return g;
}


/* -------------------------------------------------------------------------- */


Fl_Group* gdSampleEditor::createOpTools(int x, int y, int h)
{
  Fl_Group* g = new Fl_Group(x, y, 572, h);
  g->begin();
  g->resizable(0);
    volumeTool = new geVolumeTool(g->x(), g->y(), ch);
    boostTool  = new geBoostTool(volumeTool->x()+volumeTool->w()+4, g->y(), ch);
    panTool    = new gePanTool(boostTool->x()+boostTool->w()+4, g->y(), ch);
   
    pitchTool = new gePitchTool(g->x(), panTool->y()+panTool->h()+8, ch);

    rangeTool = new geRangeTool(g->x(), pitchTool->y()+pitchTool->h()+8, ch);
    reload    = new geButton(g->x()+g->w()-70, rangeTool->y(), 70, 20, "Reload");
  g->end();

  if (ch->wave->isLogical()) // Logical samples (aka takes) cannot be reloaded.
    reload->deactivate();

  reload->callback(cb_reload, (void*)this);

  return g;
}


/* -------------------------------------------------------------------------- */


Fl_Group* gdSampleEditor::createPreviewBox(int x, int y, int h)
{
  Fl_Group* g = new Fl_Group(x, y, 110, h);
  g->begin();
    rewind = new geButton(g->x(), g->y()+(g->h()/2)-12, 25, 25, "", rewindOff_xpm, rewindOn_xpm);
    play   = new geButton(rewind->x()+rewind->w()+4, g->y()+(g->h()/2)-12, 25, 25, "", play_xpm, pause_xpm);
    loop   = new geCheck(play->x()+play->w()+6, g->y()+(g->h()/2)-6, 12, 12, "Loop");
  g->end();

  play->callback(cb_togglePreview, (void*)this);
  rewind->callback(cb_rewindPreview, (void*)this);

  ch->setOnEndPreviewCb([this] { 
  	play->value(0);
  });

  return g;
}


/* -------------------------------------------------------------------------- */


Fl_Group* gdSampleEditor::createInfoBox(int x, int y, int h)
{
  Fl_Group* g = new Fl_Group(x, y, 400, h);
  g->begin();
    info = new geBox(g->x(), g->y(), g->w(), g->h());
  g->end();	

  info->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_TOP);
  
  updateInfo();

  return g;
}


/* -------------------------------------------------------------------------- */


Fl_Group* gdSampleEditor::createBottomBar(int x, int y, int h)
{
  Fl_Group* g = new Fl_Group(8, waveTools->y()+waveTools->h()+8, w()-16, h);
  g->begin();
    Fl_Group* previewBox = createPreviewBox(g->x(), g->y(), g->h());

    geBox* divisor1 = new geBox(previewBox->x()+previewBox->w()+8, g->y(), 1, g->h());
    divisor1->box(FL_BORDER_BOX);

    Fl_Group* opTools = createOpTools(divisor1->x()+divisor1->w()+12, g->y(), g->h());

    geBox* divisor2 = new geBox(opTools->x()+opTools->w()+8, g->y(), 1, g->h());
    divisor2->box(FL_BORDER_BOX);

    createInfoBox(divisor2->x()+divisor2->w()+8, g->y(), g->h());

  g->end();
  g->resizable(0);

  return g;
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::cb_reload       (Fl_Widget* w, void* p) { ((gdSampleEditor*)p)->__cb_reload(); }
void gdSampleEditor::cb_zoomIn       (Fl_Widget* w, void* p) { ((gdSampleEditor*)p)->__cb_zoomIn(); }
void gdSampleEditor::cb_zoomOut      (Fl_Widget* w, void* p) { ((gdSampleEditor*)p)->__cb_zoomOut(); }
void gdSampleEditor::cb_changeGrid   (Fl_Widget* w, void* p) { ((gdSampleEditor*)p)->__cb_changeGrid(); }
void gdSampleEditor::cb_enableSnap   (Fl_Widget* w, void* p) { ((gdSampleEditor*)p)->__cb_enableSnap(); }
void gdSampleEditor::cb_togglePreview(Fl_Widget* w, void* p) { ((gdSampleEditor*)p)->__cb_togglePreview(); }
void gdSampleEditor::cb_rewindPreview(Fl_Widget* w, void* p) { ((gdSampleEditor*)p)->__cb_rewindPreview(); }


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_enableSnap()
{
  waveTools->waveform->setSnap(!waveTools->waveform->getSnap());
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_togglePreview()
{
	if (play->value())
  	sampleEditor::setPreview(ch, G_PREVIEW_NONE);
	else
  	sampleEditor::setPreview(ch, loop->value() ? G_PREVIEW_LOOP : G_PREVIEW_NORMAL);
}


void gdSampleEditor::__cb_rewindPreview()
{
	sampleEditor::rewindPreview(ch);
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_reload()
{
  if (!gdConfirmWin("Warning", "Reload sample: are you sure?"))
    return;

  if (glue_loadChannel(ch, ch->wave->getPath()) != G_RES_OK)
    return;

  glue_setBoost(ch, G_DEFAULT_BOOST);
  glue_setPitch(ch, G_DEFAULT_PITCH);
  glue_setPanning(ch, 0.5f);

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


/* -------------------------------------------------------------------------- */


void gdSampleEditor::updateInfo()
{
	string bitDepth = ch->wave->getBits() != 0 ? gu_toString(ch->wave->getBits()) : "(unknown)";
	string infoText = 
		"File: "  + ch->wave->getPath() + "\n"
		"Size: " + gu_toString(ch->wave->getSize()) + " frames\n"
		"Duration: " + gu_toString(ch->wave->getDuration()) + " seconds\n"
		"Bit depth: " + bitDepth + "\n"
		"Frequency: " + gu_toString(ch->wave->getRate()) + " Hz\n";
	info->copy_label(infoText.c_str());
}
