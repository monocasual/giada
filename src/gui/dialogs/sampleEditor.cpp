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
#include <FL/Fl_Pack.H>
#include <FL/fl_draw.H>
#include "../../glue/channel.h"
#include "../../core/waveFx.h"
#include "../../core/conf.h"
#include "../../core/const.h"
#include "../../core/graphics.h"
#include "../../core/sampleChannel.h"
#include "../../core/mixer.h"
#include "../../core/wave.h"
#include "../../core/clock.h"
#include "../../utils/gui.h"
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
#include "../elems/mainWindow/keyboard/channel.h"
#include "gd_warnings.h"
#include "sampleEditor.h"


using namespace giada;


gdSampleEditor::gdSampleEditor(SampleChannel *ch)
  : gdWindow(640, 480),
    ch(ch)
{
  set_non_modal();

  if (conf::sampleEditorX)
    resize(conf::sampleEditorX, conf::sampleEditorY, conf::sampleEditorW, conf::sampleEditorH);

  /* top bar: grid and zoom tools */

  Fl_Group *bar = new Fl_Group(8, 8, w()-16, 20);
  bar->begin();
    grid    = new geChoice(bar->x(), bar->y(), 50, 20);
    snap    = new geCheck(grid->x()+grid->w()+4, bar->y()+4, 12, 12);
    zoomOut = new geButton(bar->x()+bar->w()-20, bar->y(), 20, 20, "", zoomOutOff_xpm, zoomOutOn_xpm);
    zoomIn  = new geButton(zoomOut->x()-24, bar->y(), 20, 20, "", zoomInOff_xpm, zoomInOn_xpm);
  bar->end();
  bar->resizable(new geBox(grid->x()+grid->w()+4, bar->y(), 80, bar->h()));

  /* waveform */

  waveTools = new geWaveTools(8, 36, w()-16, h()-130, ch);
  waveTools->end();

  /* other tools */

  Fl_Pack *row1 = new Fl_Pack(8, waveTools->y()+waveTools->h()+8, 200, 20);
  row1->spacing(4);
  row1->type(Fl_Pack::HORIZONTAL);
  row1->begin();
    volumeTool = new geVolumeTool(0, 0, ch);
    boostTool  = new geBoostTool(0, 0, ch);
    panTool    = new gePanTool(0, 0, ch);
    reload     = new geButton(0, 0, 70, 20, "Reload");
  row1->end();

  Fl_Pack *row2 = new Fl_Pack(8, row1->y()+row1->h()+8, 200, 20);
  row2->spacing(4);
  row2->type(Fl_Pack::HORIZONTAL);
  row2->begin();
                  new geBox  (0, 0, gu_getStringWidth("Pitch"), 20, "Pitch", FL_ALIGN_RIGHT);
    pitch       = new geDial(0, 0, 20, 20);
    pitchNum    = new geInput(0, 0, 70, 20);
    pitchToBar  = new geButton(0, 0, 70, 20, "To bar");
    pitchToSong = new geButton(0, 0, 70, 20, "To song");
    pitchHalf   = new geButton(0, 0, 20, 20, "", divideOff_xpm, divideOn_xpm);
    pitchDouble = new geButton(0, 0, 20, 20, "", multiplyOff_xpm, multiplyOn_xpm);
    pitchReset  = new geButton(0, 0, 70, 20, "Reset");
  row2->end();

  Fl_Pack *row3 = new Fl_Pack(8, row2->y()+row2->h()+8, 200, 20);
  row3->spacing(4);
  row3->type(Fl_Pack::HORIZONTAL);
  row3->begin();
                    new geBox  (0, 0, gu_getStringWidth("Range"), 20, "Range", FL_ALIGN_RIGHT);
    chanStart     = new geInput(0, 0, 70, 20);
    chanEnd       = new geInput(0, 0, 70, 20);
    resetStartEnd = new geButton(0, 0, 70, 20, "Reset");
  row3->end();

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
  grid->value(conf::sampleEditorGridVal);
  grid->callback(cb_changeGrid, (void*)this);

  snap->value(conf::sampleEditorGridOn);
  snap->callback(cb_enableSnap, (void*)this);

  /* TODO - redraw grid if != (off) */

  char buf[16];
  sprintf(buf, "%d", ch->begin / 2); // divided by 2 because stereo
  chanStart->value(buf);
  chanStart->type(FL_INT_INPUT);
  chanStart->callback(cb_setChanPos, this);

  /* inputs callback: fire when they lose focus or Enter is pressed. */

  chanStart->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);
  chanEnd  ->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);

  sprintf(buf, "%d", ch->end / 2);  // divided by 2 because stereo
  chanEnd->value(buf);
  chanEnd->type(FL_INT_INPUT);
  chanEnd->callback(cb_setChanPos, this);

  resetStartEnd->callback(cb_resetStartEnd, this);

  pitch->range(0.01f, 4.0f);
  pitch->value(ch->pitch);
  pitch->callback(cb_setPitch, (void*)this);
  pitch->when(FL_WHEN_RELEASE);

  sprintf(buf, "%.4f", ch->pitch); // 4 digits
  pitchNum->value(buf);
  pitchNum->align(FL_ALIGN_RIGHT);
  pitchNum->callback(cb_setPitchNum, (void*)this);
  pitchNum->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);

  pitchToBar->callback(cb_setPitchToBar, (void*)this);
  pitchToSong->callback(cb_setPitchToSong, (void*)this);
  pitchHalf->callback(cb_setPitchHalf, (void*)this);
  pitchDouble->callback(cb_setPitchDouble, (void*)this);
  pitchReset->callback(cb_resetPitch, (void*)this);

  reload->callback(cb_reload, (void*)this);

  zoomOut->callback(cb_zoomOut, (void*)this);
  zoomIn->callback(cb_zoomIn, (void*)this);

  /* logical samples (aka takes) cannot be reloaded. So far. */

  if (ch->wave->isLogical)
    reload->deactivate();

  gu_setFavicon(this);
  size_range(640, 480);
  resizable(waveTools);

  label(ch->wave->name.c_str());

  show();
}


/* -------------------------------------------------------------------------- */


gdSampleEditor::~gdSampleEditor()
{
  conf::sampleEditorX = x();
  conf::sampleEditorY = y();
  conf::sampleEditorW = w();
  conf::sampleEditorH = h();
  conf::sampleEditorGridVal = grid->value();
  conf::sampleEditorGridOn  = snap->value();
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::cb_setChanPos      (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_setChanPos(); }
void gdSampleEditor::cb_resetStartEnd   (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_resetStartEnd(); }
void gdSampleEditor::cb_reload          (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_reload(); }
void gdSampleEditor::cb_setPitch        (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_setPitch(); }
void gdSampleEditor::cb_setPitchToBar   (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_setPitchToBar(); }
void gdSampleEditor::cb_setPitchToSong  (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_setPitchToSong(); }
void gdSampleEditor::cb_setPitchHalf    (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_setPitchHalf(); }
void gdSampleEditor::cb_setPitchDouble  (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_setPitchDouble(); }
void gdSampleEditor::cb_resetPitch      (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_resetPitch(); }
void gdSampleEditor::cb_setPitchNum     (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_setPitchNum(); }
void gdSampleEditor::cb_zoomIn          (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_zoomIn(); }
void gdSampleEditor::cb_zoomOut         (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_zoomOut(); }
void gdSampleEditor::cb_changeGrid      (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_changeGrid(); }
void gdSampleEditor::cb_enableSnap      (Fl_Widget *w, void *p) { ((gdSampleEditor*)p)->__cb_enableSnap(); }


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_enableSnap()
{
  waveTools->waveform->setSnap(!waveTools->waveform->getSnap());
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_setPitchToBar()
{
  glue_setPitch(this, ch, ch->end / (float) clock::getFramesPerBar(), true);
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_setPitchToSong()
{
  glue_setPitch(this, ch, ch->end / (float) clock::getTotalFrames(), true);
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_resetPitch()
{
  glue_setPitch(this, ch, 1.0f, true);
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_setChanPos()
{
  glue_setBeginEndChannel(
    this,
    ch,
    atoi(chanStart->value())*2,  // glue halves printed values
    atoi(chanEnd->value())*2,
    true
  );
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_resetStartEnd()
{
  glue_setBeginEndChannel(this, ch, 0, ch->wave->size, true);
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_reload()
{
  if (!gdConfirmWin("Warning", "Reload sample: are you sure?"))
    return;

  /* no need for glue_loadChan, there's no gui to refresh */

  ch->load(ch->wave->pathfile.c_str(), conf::samplerate, conf::rsmpQuality);

  glue_setBoost(ch, G_DEFAULT_BOOST);
  glue_setPitch(this, ch, G_DEFAULT_PITCH, true);
  glue_setPanning(ch, 1.0f);

  panTool->refresh();
  boostTool->refresh();

  waveTools->waveform->stretchToWindow();
  waveTools->updateWaveform();

  glue_setBeginEndChannel(this, ch, 0, ch->wave->size, true);

  redraw();
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_setPitch()
{
  glue_setPitch(this, ch, pitch->value(), false);
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_setPitchNum()
{
  glue_setPitch(this, ch, atof(pitchNum->value()), true);
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_setPitchHalf()
{
  glue_setPitch(this, ch, pitch->value()/2, true);
}


/* -------------------------------------------------------------------------- */


void gdSampleEditor::__cb_setPitchDouble()
{
  glue_setPitch(this, ch, pitch->value()*2, true);
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
