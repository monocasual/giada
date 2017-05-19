/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geWaveform
 * An element which represents a waveform.
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
#include <FL/fl_draw.H>
#include <FL/Fl_Menu_Button.H>
#include "../../../core/wave.h"
#include "../../../core/conf.h"
#include "../../../core/const.h"
#include "../../../core/mixer.h"
#include "../../../core/waveFx.h"
#include "../../../core/sampleChannel.h"
#include "../../../glue/channel.h"
#include "../basics/boxtypes.h"
#include "waveTools.h"
#include "waveform.h"


using namespace giada::m;


geWaveform::geWaveform(int x, int y, int w, int h, SampleChannel *ch, const char *l)
: Fl_Widget(x, y, w, h, l),
  chan(ch),
  menuOpen(false),
  chanStart(0),
  chanStartLit(false),
  chanEnd(0),
  chanEndLit(false),
  ratio(0.0f),
  selectionA(0),
  selectionB(0),
  selectionA_abs(0),
  selectionB_abs(0)
{
  data.sup  = nullptr;
  data.inf  = nullptr;
  data.size = 0;

  grid.snap  = conf::sampleEditorGridOn;
  grid.level = conf::sampleEditorGridVal;

  stretchToWindow();
}


/* -------------------------------------------------------------------------- */


geWaveform::~geWaveform()
{
  freeData();
}


/* -------------------------------------------------------------------------- */


void geWaveform::freeData()
{
  if (data.sup != nullptr) {
    free(data.sup);
    free(data.inf);
    data.sup  = nullptr;
    data.inf  = nullptr;
    data.size = 0;
  }
  grid.points.clear();
}


/* -------------------------------------------------------------------------- */


int geWaveform::alloc(int datasize)
{
  ratio = chan->wave->size / (float) datasize;

  if (ratio < 2)
    return 0;

  freeData();

  data.size = datasize;
  data.sup  = (int*) malloc(data.size * sizeof(int));
  data.inf  = (int*) malloc(data.size * sizeof(int));

  int offset = h() / 2;
  int zero   = y() + offset; // center, zero amplitude (-inf dB)

  /* grid frequency: store a grid point every 'gridFreq' pixel. Must be
   * even, as always */

  int gridFreq = 0;
  if (grid.level != 0) {
    gridFreq = chan->wave->size / grid.level;
    if (gridFreq % 2 != 0)
      gridFreq--;
  }

  for (int i=0; i<data.size; i++) {

    int pp;  // point prev
    int pn;  // point next

    /* resampling the waveform, hardcore way. Many thanks to
     * http://fourier.eng.hmc.edu/e161/lectures/resize/node3.html
     * Note: we use
     *   p = j * (m-1 / n)
     * instead of
     *   p = j * (m-1 / n-1)
     * in order to obtain 'datasize' cells to parse (and not datasize-1) */

    pp = i * ((chan->wave->size - 1) / (float) datasize);
    pn = (i+1) * ((chan->wave->size - 1) / (float) datasize);

    if (pp % 2 != 0) pp -= 1;
    if (pn % 2 != 0) pn -= 1;

    float peaksup = 0.0f;
    float peakinf = 0.0f;

    /* scan the original data in chunks */

    int k = pp;
    while (k < pn) {

      if (chan->wave->data[k] > peaksup)
        peaksup = chan->wave->data[k];    // FIXME - Left data only
      else
      if (chan->wave->data[k] <= peakinf)
        peakinf = chan->wave->data[k];    // FIXME - Left data only

      /* print grid */

      if (gridFreq != 0)
        if (k % gridFreq == 0 && k != 0)
          grid.points.push_back(i);

      k += 2;
    }

    data.sup[i] = zero - (peaksup * chan->getBoost() * offset);
    data.inf[i] = zero - (peakinf * chan->getBoost() * offset);

    // avoid window overflow

    if (data.sup[i] < y())       data.sup[i] = y();
    if (data.inf[i] > y()+h()-1) data.inf[i] = y()+h()-1;
  }

  recalcPoints();
  return 1;
}


/* -------------------------------------------------------------------------- */


void geWaveform::recalcPoints()
{
  selectionA = relativePoint(selectionA_abs);
  selectionB = relativePoint(selectionB_abs);
  chanStart  = relativePoint(chan->begin / 2);

  /* fix the rounding error when chanEnd is set on the very end of the
   * sample */

  if (chan->end == chan->wave->size)
    chanEnd = data.size - 2; // 2 px border
  else
    chanEnd = relativePoint(chan->end / 2);
}


/* -------------------------------------------------------------------------- */


void geWaveform::draw()
{
  /* blank canvas */

  fl_rectf(x(), y(), w(), h(), COLOR_BG_0);

  /* draw selection (if any) */

  if (selectionA != selectionB) {

    int a_x = selectionA + x() - BORDER; // - start;
    int b_x = selectionB + x() - BORDER; //  - start;

    if (a_x < 0)
      a_x = 0;
    if (b_x >= w()-1)
      b_x = w()-1;

    if (selectionA < selectionB)
      fl_rectf(a_x+BORDER, y(), b_x-a_x, h(), COLOR_BD_0);
    else
      fl_rectf(b_x+BORDER, y(), a_x-b_x, h(), COLOR_BD_0);
  }

  /* draw waveform from x1 (offset driven by the scrollbar) to x2
   * (width of parent window). We don't draw the entire waveform,
   * only the visibile part. */

  int offset = h() / 2;
  int zero   = y() + offset; // sample zero (-inf dB)

  int wx1 = abs(x() - ((geWaveTools*)parent())->x());
  int wx2 = wx1 + ((geWaveTools*)parent())->w();
  if (x()+w() < ((geWaveTools*)parent())->w())
    wx2 = x() + w() - BORDER;

  fl_color(0, 0, 0);
  for (int i=wx1; i<wx2; i++) {
    fl_line(i+x(), zero, i+x(), data.sup[i]);
    fl_line(i+x(), zero, i+x(), data.inf[i]);

    /* print grid */

    for (unsigned k=0; k<grid.points.size(); k++) {
      if (grid.points.at(k) == i) {
        //gu_log("draw grid line at %d\n", i);
        fl_color(fl_rgb_color(54, 54, 54));
        fl_line_style(FL_DASH, 0, nullptr);
        fl_line(i+x(), y(), i+x(), y()+h());
        fl_color(0, 0, 0);
        fl_line_style(FL_SOLID, 0, nullptr);
        break;
      }
    }
  }

  /* border box */

  fl_rect(x(), y(), w(), h(), COLOR_BD_0);

  /* print chanStart */

  int lineX = x()+chanStart+1;

  if (chanStartLit) fl_color(COLOR_BD_1);
  else              fl_color(COLOR_BD_0);

  /* vertical line */

  fl_line(lineX, y()+1, lineX, y()+h()-2);

  /* print flag and avoid overflow */

  if (lineX+FLAG_WIDTH > w()+x()-2)
    fl_rectf(lineX, y()+h()-FLAG_HEIGHT-1, w()-lineX+x()-1, FLAG_HEIGHT);
  else
    fl_rectf(lineX, y()+h()-FLAG_HEIGHT-1, FLAG_WIDTH, FLAG_HEIGHT);

  /* print chanEnd */

  lineX = x()+chanEnd;
  if (chanEndLit) fl_color(COLOR_BD_1);
  else            fl_color(COLOR_BD_0);

  fl_line(lineX, y()+1, lineX, y()+h()-2);

  if (lineX-FLAG_WIDTH < x())
    fl_rectf(x()+1, y()+1, lineX-x(), FLAG_HEIGHT);
  else
    fl_rectf(lineX-FLAG_WIDTH, y()+1, FLAG_WIDTH, FLAG_HEIGHT);
}


/* -------------------------------------------------------------------------- */


int geWaveform::handle(int e)
{
  int ret = 0;

  switch (e) {

    case FL_PUSH: {

      mouseX = Fl::event_x();
      pushed = true;

      if (!mouseOnEnd() && !mouseOnStart()) {

        /* right button? show the menu. Don't set selectionA,B,etc */

        if (Fl::event_button3()) {
          openEditMenu();
        }
        else
        if (mouseOnSelectionA() || mouseOnSelectionB()) {
          resized = true;
        }
        else {
          dragged = true;
          selectionA = Fl::event_x() - x();

          if (selectionA >= data.size) selectionA = data.size;

          selectionB = selectionA;
          selectionA_abs = absolutePoint(selectionA);
          selectionB_abs = selectionA_abs;
        }
      }

      ret = 1;
      break;
    }

    case FL_RELEASE: {

      int realChanStart = chan->begin;
      int realChanEnd   = chan->end;

      if (chanStartLit)
        realChanStart = absolutePoint(chanStart)*2;
      else
      if (chanEndLit)
        realChanEnd = absolutePoint(chanEnd)*2;

      glue_setBeginEndChannel(chan, realChanStart, realChanEnd);

      pushed  = false;
      dragged = false;

      redraw();
      ret = 1;
      break;
    }

    case FL_ENTER: {  // enables FL_DRAG
      ret = 1;
      break;
    }

    case FL_LEAVE: {
      if (chanStartLit || chanEndLit) {
        chanStartLit = false;
        chanEndLit   = false;
        redraw();
      }
      ret = 1;
      break;
    }

    case FL_MOVE: {
      mouseX = Fl::event_x();
      mouseY = Fl::event_y();

      if (mouseOnStart()) {
        chanStartLit = true;
        redraw();
      }
      else
      if (chanStartLit) {
        chanStartLit = false;
        redraw();
      }

      if (mouseOnEnd()) {
        chanEndLit = true;
        redraw();
      }
      else
      if (chanEndLit) {
        chanEndLit = false;
        redraw();
      }

      if (mouseOnSelectionA())
        fl_cursor(FL_CURSOR_WE, FL_WHITE, FL_BLACK);
      else
      if (mouseOnSelectionB())
        fl_cursor(FL_CURSOR_WE, FL_WHITE, FL_BLACK);
      else
        fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);

      ret = 1;
      break;
    }

    case FL_DRAG: {

      /* here the mouse is on the chanStart tool */

      if (chanStartLit && pushed) {

        chanStart = Fl::event_x() - x();

        if (grid.snap)
          chanStart = applySnap(chanStart);

        if (chanStart < 0)
          chanStart = 0;
        else
        if (chanStart >= chanEnd)
          chanStart = chanEnd-2;

        redraw();
      }
      else
      if (chanEndLit && pushed) {

        chanEnd = Fl::event_x() - x();

        if (grid.snap)
          chanEnd = applySnap(chanEnd);

        if (chanEnd >= data.size - 2)
          chanEnd = data.size - 2;
        else
        if (chanEnd <= chanStart)
          chanEnd = chanStart + 2;

        redraw();
      }

      /* here the mouse is on the waveform, i.e. a selection */

      else
      if (dragged) {

        selectionB = Fl::event_x() - x();

        if (selectionB >= data.size)
          selectionB = data.size;

        if (selectionB <= 0)
          selectionB = 0;

        if (grid.snap)
          selectionB = applySnap(selectionB);

        selectionB_abs = absolutePoint(selectionB);
        redraw();
      }

      /* here the mouse is on a selection boundary i.e. resize */

      else
      if (resized) {
        int pos = Fl::event_x() - x();
        if (mouseOnSelectionA()) {
          selectionA     = grid.snap ? applySnap(pos) : pos;
          selectionA_abs = absolutePoint(selectionA);
        }
        else
        if (mouseOnSelectionB()) {
          selectionB     = grid.snap ? applySnap(pos) : pos;
          selectionB_abs = absolutePoint(selectionB);
        }
        redraw();
      }
      mouseX = Fl::event_x();
      ret = 1;
      break;
    }
  }
  return ret;
}


/* -------------------------------------------------------------------------- */

/* pixel snap disances (10px) must be equal to those defined in
 * geWaveform::mouseOnSelectionA() and gWaverfrom::mouseOnSelectionB() */
/* TODO - use constant for 10px */

int geWaveform::applySnap(int pos)
{
  for (unsigned i=0; i<grid.points.size(); i++) {
    if (pos >= grid.points.at(i) - SNAPPING &&
        pos <= grid.points.at(i) + SNAPPING)
    {
      return grid.points.at(i);
    }
  }
  return pos;
}


/* -------------------------------------------------------------------------- */


bool geWaveform::mouseOnStart()
{
  return mouseX-10 >  chanStart + x() - BORDER              &&
         mouseX-10 <= chanStart + x() - BORDER + FLAG_WIDTH &&
         mouseY    >  h() + y() - FLAG_HEIGHT;
}


/* -------------------------------------------------------------------------- */


bool geWaveform::mouseOnEnd()
{
  return mouseX-10 >= chanEnd + x() - BORDER - FLAG_WIDTH &&
         mouseX-10 <= chanEnd + x() - BORDER              &&
         mouseY    <= y() + FLAG_HEIGHT + 1;
}


/* -------------------------------------------------------------------------- */

/* pixel boundaries (10px) must be equal to the snap factor distance
 * defined in geWaveform::applySnap() */

bool geWaveform::mouseOnSelectionA()
{
  if (selectionA == selectionB)
    return false;
  return mouseX >= selectionA-10+x() && mouseX <= selectionA+10+x();
}


bool geWaveform::mouseOnSelectionB()
{
  if (selectionA == selectionB)
    return false;
  return mouseX >= selectionB-10+x() && mouseX <= selectionB+10+x();
}


/* -------------------------------------------------------------------------- */


int geWaveform::absolutePoint(int p)
{
  if (p <= 0)
    return 0;

  if (p > data.size)
    return chan->wave->size / 2;

  return (p * ratio) / 2;
}


/* -------------------------------------------------------------------------- */


int geWaveform::relativePoint(int p)
{
  return (ceil(p / ratio)) * 2;
}


/* -------------------------------------------------------------------------- */


void geWaveform::openEditMenu()
{
  if (selectionA == selectionB)
    return;

  menuOpen = true;

  Fl_Menu_Item menu[] = {
    {"Cut"},
    {"Trim"},
    {"Silence"},
    {"Fade in"},
    {"Fade out"},
    {"Smooth edges"},
    {"Set start/end here"},
    {0}
  };

  if (chan->status == STATUS_PLAY) {
    menu[0].deactivate();
    menu[1].deactivate();
  }

  Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
  b->box(G_CUSTOM_BORDER_BOX);
  b->textsize(GUI_FONT_SIZE_BASE);
  b->textcolor(COLOR_TEXT_0);
  b->color(COLOR_BG_0);

  const Fl_Menu_Item *m = menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, b);
  if (!m) {
    menuOpen = false;
    return;
  }

  /* straightSel() to ensure that point A is always lower than B */

  straightSel();

  if (strcmp(m->label(), "Silence") == 0) {
    wfx_silence(chan->wave, absolutePoint(selectionA), absolutePoint(selectionB));

    selectionA = 0;
    selectionB = 0;

    stretchToWindow();
    redraw();
    menuOpen = false;
    return;
  }

  if (strcmp(m->label(), "Set start/end here") == 0) {

    glue_setBeginEndChannel(chan, absolutePoint(selectionA) * 2, 
      absolutePoint(selectionB) * 2); // stereo values

    selectionA     = 0;
    selectionB     = 0;
    selectionA_abs = 0;
    selectionB_abs = 0;

    recalcPoints();
    redraw();
    menuOpen = false;
    return;
  }

  if (strcmp(m->label(), "Cut") == 0) {
    wfx_cut(chan->wave, absolutePoint(selectionA), absolutePoint(selectionB));

    /* for convenience reset start/end points */

    glue_setBeginEndChannel(chan, 0, chan->wave->size);

    selectionA     = 0;
    selectionB     = 0;
    selectionA_abs = 0;
    selectionB_abs = 0;

    setZoom(0);

    menuOpen = false;
    return;
  }

  if (strcmp(m->label(), "Trim") == 0) {
    wfx_trim(chan->wave, absolutePoint(selectionA), absolutePoint(selectionB));

    glue_setBeginEndChannel(chan, 0, chan->wave->size);

    selectionA     = 0;
    selectionB     = 0;
    selectionA_abs = 0;
    selectionB_abs = 0;

    stretchToWindow();
    menuOpen = false;
    redraw();
    return;
  }

  if (!strcmp(m->label(), "Fade in") || !strcmp(m->label(), "Fade out")) {

    int type = !strcmp(m->label(), "Fade in") ? 0 : 1;
    wfx_fade(chan->wave, absolutePoint(selectionA), absolutePoint(selectionB), type);

    selectionA = 0;
    selectionB = 0;

    stretchToWindow();
    redraw();
    menuOpen = false;
    return;
  }

  if (!strcmp(m->label(), "Smooth edges")) {

    wfx_smooth(chan->wave, absolutePoint(selectionA), absolutePoint(selectionB));

    selectionA = 0;
    selectionB = 0;

    stretchToWindow();
    redraw();
    menuOpen = false;
    return;
  }
}


/* -------------------------------------------------------------------------- */


void geWaveform::straightSel()
{
  if (selectionA > selectionB) {
    unsigned tmp = selectionB;
    selectionB = selectionA;
    selectionA = tmp;
  }
}


/* -------------------------------------------------------------------------- */


void geWaveform::setZoom(int type)
{
  int newSize;
  if (type == -1) newSize = data.size*2;  // zoom in
  else            newSize = data.size/2;  // zoom out

  if (alloc(newSize)) {
    size(data.size, h());

    /* zoom to pointer */

    int shift;
    if (x() > 0)
      shift = Fl::event_x() - x();
    else
    if (type == -1)
      shift = Fl::event_x() + abs(x());
    else
      shift = (Fl::event_x() + abs(x())) / -2;

    if (x() - shift > BORDER)
      shift = 0;

    position(x() - shift, y());


    /* avoid overflow when zooming out with scrollbar like that:
     * |----------[scrollbar]|
     *
     * offset vs smaller:
     * |[wave------------| offset > 0  smaller = false
     * |[wave----]       | offset < 0, smaller = true
     * |-------------]   | offset < 0, smaller = false  */

    int  parentW = ((geWaveTools*)parent())->w();
    int  thisW   = x() + w() - BORDER;           // visible width, not full width

    if (thisW < parentW)
      position(x() + parentW - thisW, y());
    if (smaller())
      stretchToWindow();

    redraw();
  }
}


/* -------------------------------------------------------------------------- */


void geWaveform::stretchToWindow()
{
  int s = ((geWaveTools*)parent())->w();
  alloc(s);
  position(BORDER, y());
  size(s, h());
}


/* -------------------------------------------------------------------------- */


bool geWaveform::smaller()
{
  return w() < ((geWaveTools*)parent())->w();
}


/* -------------------------------------------------------------------------- */


float geWaveform::displayRatio()
{
    return 1.0f / (data.size / (float) w());
};

/* -------------------------------------------------------------------------- */


void geWaveform::setGridLevel(int l)
{
  grid.points.clear();
  grid.level = l;
  alloc(data.size);
  redraw();
}
