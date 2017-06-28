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


#include <cassert>
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
#include "../../../glue/sampleEditor.h"
#include "../basics/boxtypes.h"
#include "waveTools.h"
#include "waveform.h"


using namespace giada::m;
using namespace giada::c;


geWaveform::geWaveform(int x, int y, int w, int h, SampleChannel* ch, const char* l)
: Fl_Widget   (x, y, w, h, l),
  selection   {},
  chan        (ch),
  chanStart   (0),
  chanStartLit(false),
  chanEnd     (0),
  chanEndLit  (false),
  ratio       (0.0f)
{
  data.sup  = nullptr;
  data.inf  = nullptr;
  data.size = 0;

  grid.snap  = conf::sampleEditorGridOn;
  grid.level = conf::sampleEditorGridVal;

  alloc(w);
}


/* -------------------------------------------------------------------------- */


geWaveform::~geWaveform()
{
  freeData();
}


/* -------------------------------------------------------------------------- */


void geWaveform::freeData()
{
  if (data.sup) {
    delete[] data.sup;
    delete[] data.inf;
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
  data.sup  = new (std::nothrow) int[data.size];
  data.inf  = new (std::nothrow) int[data.size];

  if (!data.sup || !data.inf)
    return 0;

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
  selection.aPixel = relativePoint(selection.aFrame);
  selection.bPixel = relativePoint(selection.bFrame);
  chanStart  = relativePoint(chan->begin / 2);

  /* fix the rounding error when chanEnd is set on the very end of the
   * sample */

  if (chan->end == chan->wave->size)
    chanEnd = data.size - 2; // 2 px border
  else
    chanEnd = relativePoint(chan->end / 2);
}


/* -------------------------------------------------------------------------- */


void geWaveform::drawSelection()
{
  if (!isSelected()) 
    return;

  int a_x = selection.aPixel + x(); // - start;
  int b_x = selection.bPixel + x(); //  - start;

  if (a_x < 0)
    a_x = 0;
  if (b_x >= w()-1)
    b_x = w()-1;

  if (selection.aPixel < selection.bPixel)
    fl_rectf(a_x, y(), b_x-a_x, h(), G_COLOR_GREY_4);
  else
    fl_rectf(b_x, y(), a_x-b_x, h(), G_COLOR_GREY_4);
}


/* -------------------------------------------------------------------------- */


void geWaveform::drawWaveform(int from, int to)
{
  int zero = y() + (h() / 2); // zero amplitude (-inf dB)

  fl_color(G_COLOR_BLACK);
  for (int i=from; i<to; i++) {
    fl_line(i+x(), zero, i+x(), data.sup[i]);
    fl_line(i+x(), zero, i+x(), data.inf[i]);
  }
}


/* -------------------------------------------------------------------------- */


void geWaveform::drawGrid(int from, int to)
{
  fl_color(G_COLOR_GREY_3);
  fl_line_style(FL_DASH, 1, nullptr);
  for (int i=from; i<to; i++) {
    for (unsigned k=0; k<grid.points.size(); k++) {
      if (grid.points.at(k) != i) 
        continue;
      fl_line(i+x(), y(), i+x(), y()+h());
      break;
    }
  }
  fl_line_style(FL_SOLID, 0, nullptr);
}


/* -------------------------------------------------------------------------- */


void geWaveform::drawStartEndPoints()
{
  /* print chanStart */

  int lineX = chanStart + x();

  if (chanStartLit) fl_color(G_COLOR_LIGHT_2);
  else              fl_color(G_COLOR_LIGHT_1);

  /* vertical line */

  fl_line(lineX, y()+1, lineX, y()+h()-2);

  /* print flag and avoid overflow */

  if (lineX+FLAG_WIDTH > w()+x()-2)
    fl_rectf(lineX, y()+h()-FLAG_HEIGHT-1, w()-lineX+x()-1, FLAG_HEIGHT);
  else
    fl_rectf(lineX, y()+h()-FLAG_HEIGHT-1, FLAG_WIDTH, FLAG_HEIGHT);

  /* print chanEnd */

  lineX = chanEnd + x();
  if (chanEndLit) fl_color(G_COLOR_LIGHT_2);
  else            fl_color(G_COLOR_LIGHT_1);

  /* vertical line */

  fl_line(lineX, y()+1, lineX, y()+h()-2);

  if (lineX-FLAG_WIDTH < x())
    fl_rectf(x()+1, y()+1, lineX-x(), FLAG_HEIGHT);
  else
    fl_rectf(lineX-FLAG_WIDTH, y()+1, FLAG_WIDTH, FLAG_HEIGHT);
}


/* -------------------------------------------------------------------------- */


void geWaveform::drawPlayHead()
{
  int p = ceilf(chan->tracker / ratio) + x();
  fl_color(G_COLOR_LIGHT_2);
  fl_line(p, y() + 1, p, y() + h() - 2);
}


/* -------------------------------------------------------------------------- */


void geWaveform::draw()
{
  assert(data.sup != nullptr);
  assert(data.inf != nullptr);

  fl_rectf(x(), y(), w(), h(), G_COLOR_GREY_2);  // blank canvas

  /* Draw things from 'from' (offset driven by the scrollbar) to 'to' (width of 
  parent window). We don't draw the entire waveform, only the visibile part. */

  int from = abs(x() - parent()->x());
  int to = from + parent()->w();
  if (x() + w() < parent()->w())
    to = x() + w() - BORDER;

  drawSelection();
  drawWaveform(from, to);
  drawGrid(from, to);
  drawStartEndPoints();
  drawPlayHead();

  fl_rect(x(), y(), w(), h(), G_COLOR_GREY_4);   // border box
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
        if (Fl::event_button3()) {  // let the parent (waveTools) handle this
          ret = 0;
          break;
        }
        if (mouseOnSelectionA() || mouseOnSelectionB())
          resized = true;
        else {
          dragged = true;
          selection.aPixel = Fl::event_x() - x();
          selection.bPixel = selection.aPixel;
        }
      }
      ret = 1;
      break;
    }

    case FL_RELEASE: {

      /* If selection has been done (dragged or resized), make sure that point A 
      is always lower than B. */

      if (dragged || resized)
        fixSelection();

      /* Handle begin/end markers interaction. */

      if (chanStartLit || chanEndLit) {
        int realChanStart = chan->begin;
        int realChanEnd   = chan->end;
        if (chanStartLit)
          realChanStart = absolutePoint(chanStart) * 2;
        else
        if (chanEndLit)
          realChanEnd = absolutePoint(chanEnd) * 2;
        sampleEditor::setBeginEndChannel(chan, realChanStart, realChanEnd);
      }

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

      if (mouseOnSelectionA() && isSelected())
        fl_cursor(FL_CURSOR_WE, FL_WHITE, FL_BLACK);
      else
      if (mouseOnSelectionB() && isSelected())
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

      /* Here the mouse is on the waveform, i.e. a new selection has started. */

      else
      if (dragged) {
        selection.bPixel = Fl::event_x() - x();

        if (selection.bPixel >= data.size)
          selection.bPixel = data.size;

        if (selection.bPixel <= 0)
          selection.bPixel = 0;

        if (grid.snap)
          selection.bPixel = applySnap(selection.bPixel);

        redraw();
      }

      /* here the mouse is on a selection boundary i.e. resize */

      else
      if (resized) {
        int pos = Fl::event_x() - x();
        if (mouseOnSelectionA())
          selection.aPixel = grid.snap ? applySnap(pos) : pos;
        else
        if (mouseOnSelectionB())
          selection.bPixel = grid.snap ? applySnap(pos) : pos;
        redraw();
      }
      mouseX = Fl::event_x() + x();
      ret = 1;
      break;
    }
  }
  return ret;
}


/* -------------------------------------------------------------------------- */


int geWaveform::applySnap(int pos)
{
  /* Pixel snap disances (SNAPPING) must be equal to those defined in 
  mouseOnSelectionA() and mouseOnSelectionB(). */

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
  return mouseX - (FLAG_WIDTH / 2) >  chanStart + x() - BORDER              &&
         mouseX - (FLAG_WIDTH / 2) <= chanStart + x() - BORDER + FLAG_WIDTH &&
         mouseY    >  h() + y() - FLAG_HEIGHT;
}


/* -------------------------------------------------------------------------- */


bool geWaveform::mouseOnEnd()
{
  return mouseX - (FLAG_WIDTH / 2) >= chanEnd + x() - BORDER - FLAG_WIDTH &&
         mouseX - (FLAG_WIDTH / 2) <= chanEnd + x() - BORDER              &&
         mouseY    <= y() + FLAG_HEIGHT + 1;
}


/* -------------------------------------------------------------------------- */

/* pixel boundaries (10px) must be equal to the snap factor distance
 * defined in geWaveform::applySnap() */

bool geWaveform::mouseOnSelectionA()
{
  //if (!isSelected())
  //  return false;
  return mouseX >= selection.aPixel - (FLAG_WIDTH / 2) + x() && 
         mouseX <= selection.aPixel + (FLAG_WIDTH / 2) + x();
}


bool geWaveform::mouseOnSelectionB()
{
  //if (!isSelected())
  //  return false;
  return mouseX >= selection.bPixel - (FLAG_WIDTH / 2) + x() && 
         mouseX <= selection.bPixel + (FLAG_WIDTH / 2) + x();
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


void geWaveform::fixSelection()
{
  if (selection.aPixel > selection.bPixel) // inverted selection
    std::swap(selection.aPixel, selection.bPixel);
  selection.aFrame = absolutePoint(selection.aPixel);
  selection.bFrame = absolutePoint(selection.bPixel);
}


/* -------------------------------------------------------------------------- */


void geWaveform::clearSel()
{
  selection.aPixel = 0;
  selection.bPixel = 0;
  selection.aFrame = 0;
  selection.bFrame = 0;  
}


/* -------------------------------------------------------------------------- */


void geWaveform::setZoom(int type)
{
  int newSize = type == ZOOM_IN ? data.size * 2 : data.size / 2;

  if (!alloc(newSize)) 
    return;

  size(newSize, h());

  /* zoom to pointer */

  int shift;
  if (x() > 0)
    shift = Fl::event_x() - x();
  else
  if (type == ZOOM_IN)
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

  int  parentW = parent()->w();
  int  thisW   = x() + w() - BORDER;           // visible width, not full width

  if (thisW < parentW)
    position(x() + parentW - thisW, y());
  if (smaller())
    stretchToWindow();

  redraw();
}


/* -------------------------------------------------------------------------- */


void geWaveform::stretchToWindow()
{
  int s = parent()->w();
  alloc(s);
  position(BORDER, y());
  size(s, h());
}


/* -------------------------------------------------------------------------- */


void geWaveform::refresh()
{
  alloc(data.size);
  redraw();
}


/* -------------------------------------------------------------------------- */


bool geWaveform::smaller()
{
  return w() < parent()->w();
}


/* -------------------------------------------------------------------------- */


void geWaveform::setGridLevel(int l)
{
  grid.points.clear();
  grid.level = l;
  alloc(data.size);
  redraw();
}


/* -------------------------------------------------------------------------- */


bool geWaveform::isSelected()
{
  return selection.aPixel != selection.bPixel;
}


/* -------------------------------------------------------------------------- */


int geWaveform::getSelectionA()
{
  return selection.aFrame;
}


int geWaveform::getSelectionB()
{
  return selection.bFrame;
}

