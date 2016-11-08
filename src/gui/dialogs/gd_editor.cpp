/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_editor
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#include "../../glue/channel.h"
#include "../../core/waveFx.h"
#include "../../core/conf.h"
#include "../../core/const.h"
#include "../../core/graphics.h"
#include "../../core/sampleChannel.h"
#include "../../core/mixer.h"
#include "../../core/wave.h"
#include "../elems/channel.h"
#include "../elems/ge_waveform.h"
#include "../elems/ge_mixed.h"
#include "../elems/ge_waveTools.h"
#include "gd_warnings.h"
#include "gd_editor.h"


extern Mixer G_Mixer;
extern Conf  G_Conf;


gdEditor::gdEditor(SampleChannel *ch)
  : gWindow(640, 480),
    ch(ch)
{
  set_non_modal();

  if (G_Conf.sampleEditorX)
    resize(G_Conf.sampleEditorX, G_Conf.sampleEditorY, G_Conf.sampleEditorW, G_Conf.sampleEditorH);

  /* top bar: grid and zoom tools */

  Fl_Group *bar = new Fl_Group(8, 8, w()-16, 20);
  bar->begin();
    grid    = new gChoice(bar->x(), bar->y(), 50, 20);
    snap    = new gCheck(grid->x()+grid->w()+4, bar->y()+4, 12, 12);
    zoomOut = new gClick(bar->x()+bar->w()-20, bar->y(), 20, 20, "", zoomOutOff_xpm, zoomOutOn_xpm);
    zoomIn  = new gClick(zoomOut->x()-24, bar->y(), 20, 20, "", zoomInOff_xpm, zoomInOn_xpm);
  bar->end();
  bar->resizable(new gBox(grid->x()+grid->w()+4, bar->y(), 80, bar->h()));

  /* waveform */

  waveTools = new gWaveTools(8, 36, w()-16, h()-120, ch);
  waveTools->end();

  /* other tools */

  Fl_Group *tools = new Fl_Group(8, waveTools->y()+waveTools->h()+8, w()-16, 130);
  tools->begin();
    volume        = new gDial (tools->x()+50,                    tools->y(), 20, 20, "Volume");
    volumeNum     = new gInput(volume->x()+volume->w()+4,        tools->y(), 46, 20, "dB");

    boost         = new gDial (volumeNum->x()+volumeNum->w()+108, tools->y(), 20, 20, "Boost");
    boostNum      = new gInput(boost->x()+boost->w()+4,           tools->y(), 44, 20, "dB");

    normalize     = new gClick(boostNum->x()+boostNum->w()+54,   tools->y(), 70, 20, "Normalize");
    pan           = new gDial (normalize->x()+normalize->w()+40, tools->y(), 20, 20, "Pan");
    panNum        = new gInput(pan->x()+pan->w()+4,              tools->y(), 45, 20, "%");

    pitch         = new gDial (tools->x()+50,                       volume->y()+volume->h()+4, 20, 20, "Pitch");
    pitchNum      = new gInput(pitch->x()+pitch->w()+4,             volume->y()+volume->h()+4, 46, 20);
    pitchToBar    = new gClick(pitchNum->x()+pitchNum->w()+4,       volume->y()+volume->h()+4, 60, 20, "To bar");
    pitchToSong   = new gClick(pitchToBar->x()+pitchToBar->w()+4,   volume->y()+volume->h()+4, 60, 20, "To song");
    pitchHalf     = new gClick(pitchToSong->x()+pitchToSong->w()+4, volume->y()+volume->h()+4, 20, 20, "", divideOff_xpm, divideOn_xpm);
    pitchDouble   = new gClick(pitchHalf->x()+pitchHalf->w()+4,     volume->y()+volume->h()+4, 20, 20, "", multiplyOff_xpm, multiplyOn_xpm);
    pitchReset    = new gClick(pitchDouble->x()+pitchDouble->w()+4, volume->y()+volume->h()+4, 46, 20, "Reset");
    reload        = new gClick(pitchReset->x()+pitchReset->w()+4,   volume->y()+volume->h()+4, 70, 20, "Reload");

    chanStart     = new gInput(tools->x()+60,                   pitch->y()+pitch->h()+4, 60, 20, "Range");
    chanEnd       = new gInput(chanStart->x()+chanStart->w()+4, pitch->y()+pitch->h()+4, 60, 20, "");
    resetStartEnd = new gClick(chanEnd->x()+chanEnd->w()+4,     pitch->y()+pitch->h()+4, 60, 20, "Reset");

  tools->end();
  tools->resizable(new gBox(panNum->x()+panNum->w()+4, tools->y(), 80, tools->h()));

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
  grid->value(G_Conf.sampleEditorGridVal);
  grid->callback(cb_changeGrid, (void*)this);

  snap->value(G_Conf.sampleEditorGridOn);
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

  volume->callback(cb_setVolume, (void*)this);
  volume->value(ch->guiChannel->vol->value());

  float dB = 20*log10(ch->volume);   // dB = 20*log_10(linear value)
  if (dB > -INFINITY) sprintf(buf, "%.2f", dB);
  else                sprintf(buf, "-inf");
  volumeNum->value(buf);
  volumeNum->align(FL_ALIGN_RIGHT);
  volumeNum->callback(cb_setVolumeNum, (void*)this);

  boost->range(1.0f, 10.0f);
  boost->callback(cb_setBoost, (void*)this);
  if (ch->boost > 10.f)
    boost->value(10.0f);
  else
    boost->value(ch->boost);
  boost->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);

  float boost = 20*log10(ch->boost); // dB = 20*log_10(linear value)
  sprintf(buf, "%.2f", boost);
  boostNum->value(buf);
  boostNum->align(FL_ALIGN_RIGHT);
  boostNum->callback(cb_setBoostNum, (void*)this);

  normalize->callback(cb_normalize, (void*)this);

  pan->range(0.0f, 2.0f);
  pan->callback(cb_panning, (void*)this);

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

  if (ch->panRight < 1.0f) {
    char buf[8];
    sprintf(buf, "%d L", (int) std::abs((ch->panRight * 100.0f) - 100));
    pan->value(ch->panRight);
    panNum->value(buf);
  }
  else if (ch->panRight == 1.0f && ch->panLeft == 1.0f) {
    pan->value(1.0f);
    panNum->value("C");
  }
  else {
    char buf[8];
    sprintf(buf, "%d R", (int) std::abs((ch->panLeft * 100.0f) - 100));
    pan->value(2.0f - ch->panLeft);
    panNum->value(buf);
  }

  panNum->align(FL_ALIGN_RIGHT);
  panNum->readonly(1);
  panNum->cursor_color(FL_WHITE);

  gu_setFavicon(this);
  size_range(640, 480);
  resizable(waveTools);

  label(ch->wave->name.c_str());

  show();
}


/* -------------------------------------------------------------------------- */


gdEditor::~gdEditor()
{
  G_Conf.sampleEditorX = x();
  G_Conf.sampleEditorY = y();
  G_Conf.sampleEditorW = w();
  G_Conf.sampleEditorH = h();
  G_Conf.sampleEditorGridVal = grid->value();
  G_Conf.sampleEditorGridOn  = snap->value();
}


/* -------------------------------------------------------------------------- */


void gdEditor::cb_setChanPos      (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setChanPos(); }
void gdEditor::cb_resetStartEnd   (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_resetStartEnd(); }
void gdEditor::cb_setVolume       (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setVolume(); }
void gdEditor::cb_setVolumeNum    (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setVolumeNum(); }
void gdEditor::cb_setBoost        (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setBoost(); }
void gdEditor::cb_setBoostNum     (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setBoostNum(); }
void gdEditor::cb_normalize       (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_normalize(); }
void gdEditor::cb_panning         (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_panning(); }
void gdEditor::cb_reload          (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_reload(); }
void gdEditor::cb_setPitch        (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setPitch(); }
void gdEditor::cb_setPitchToBar   (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setPitchToBar(); }
void gdEditor::cb_setPitchToSong  (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setPitchToSong(); }
void gdEditor::cb_setPitchHalf    (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setPitchHalf(); }
void gdEditor::cb_setPitchDouble  (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setPitchDouble(); }
void gdEditor::cb_resetPitch      (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_resetPitch(); }
void gdEditor::cb_setPitchNum     (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_setPitchNum(); }
void gdEditor::cb_zoomIn          (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_zoomIn(); }
void gdEditor::cb_zoomOut         (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_zoomOut(); }
void gdEditor::cb_changeGrid      (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_changeGrid(); }
void gdEditor::cb_enableSnap      (Fl_Widget *w, void *p) { ((gdEditor*)p)->__cb_enableSnap(); }


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_enableSnap()
{
  waveTools->waveform->setSnap(!waveTools->waveform->getSnap());
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_setPitchToBar()
{
  glue_setPitch(this, ch, ch->end/(float)G_Mixer.framesPerBar, true);
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_setPitchToSong()
{
  glue_setPitch(this, ch, ch->end/(float)G_Mixer.totalFrames, true);
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_resetPitch()
{
  glue_setPitch(this, ch, 1.0f, true);
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_setChanPos()
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


void gdEditor::__cb_resetStartEnd()
{
  glue_setBeginEndChannel(this, ch, 0, ch->wave->size, true);
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_setVolume()
{
  glue_setVolEditor(this, ch, volume->value(), false);
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_setVolumeNum()
{
  glue_setVolEditor(this, ch, atof(volumeNum->value()), true);
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_setBoost()
{
  if (Fl::event() == FL_DRAG)
    glue_setBoost(this, ch, boost->value(), false);
  else if (Fl::event() == FL_RELEASE) {
    glue_setBoost(this, ch, boost->value(), false);
  waveTools->updateWaveform();
  }
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_setBoostNum()
{
  glue_setBoost(this, ch, atof(boostNum->value()), true);
  waveTools->updateWaveform();
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_normalize()
{
  float val = wfx_normalizeSoft(ch->wave);
  glue_setBoost(this, ch, val, false); // we pretend that a fake user turns the dial (numeric=false)
  if (val < 0.0f)
    boost->value(0.0f);
  else
  if (val > 20.0f) // a dial > than it's max value goes crazy
    boost->value(10.0f);
  else
    boost->value(val);
  waveTools->updateWaveform();
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_panning()
{
  glue_setPanning(this, ch, pan->value());
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_reload()
{
  if (!gdConfirmWin("Warning", "Reload sample: are you sure?"))
    return;

  /* no need for glue_loadChan, there's no gui to refresh */

  ch->load(ch->wave->pathfile.c_str(), G_Conf.samplerate, G_Conf.rsmpQuality);

  glue_setBoost(this, ch, DEFAULT_BOOST, true);
  glue_setPitch(this, ch, gDEFAULT_PITCH, true);
  glue_setPanning(this, ch, 1.0f);
  pan->value(1.0f);  // glue_setPanning doesn't do it
  pan->redraw();     // glue_setPanning doesn't do it

  waveTools->waveform->stretchToWindow();
  waveTools->updateWaveform();

  glue_setBeginEndChannel(this, ch, 0, ch->wave->size, true);

  redraw();
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_setPitch()
{
  glue_setPitch(this, ch, pitch->value(), false);
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_setPitchNum()
{
  glue_setPitch(this, ch, atof(pitchNum->value()), true);
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_setPitchHalf()
{
  glue_setPitch(this, ch, pitch->value()/2, true);
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_setPitchDouble()
{
  glue_setPitch(this, ch, pitch->value()*2, true);
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_zoomIn()
{
  waveTools->waveform->setZoom(-1);
  waveTools->redraw();
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_zoomOut()
{
  waveTools->waveform->setZoom(0);
  waveTools->redraw();
}


/* -------------------------------------------------------------------------- */


void gdEditor::__cb_changeGrid()
{
  waveTools->waveform->setGridLevel(atoi(grid->text()));
}
