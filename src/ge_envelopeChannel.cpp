/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_envelopeWidget
 *
 * parent class of any envelope controller, from volume to VST parameter
 * automations.
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#include "ge_envelopeChannel.h"
#include "gd_actionEditor.h"


gEnvelopeChannel::gEnvelopeChannel(int x, int y, int w, int h, gdActionEditor *parent, channel *ch, char type)
	:	gActionWidget(x, y, w, h, parent), ch(ch), type(type) {}


/* ------------------------------------------------------------------ */


gEnvelopeChannel::~gEnvelopeChannel() {
	clearPoints();
}


/* ------------------------------------------------------------------ */


void gEnvelopeChannel::addPoint(int frame, int value) {
	point p;
	p.frame = frame;
	p.value = value;
	p.x = p.frame / parent->zoom;
	p.y = p.value / h();
	points.add(p);
}


/* ------------------------------------------------------------------ */


void gEnvelopeChannel::draw() {
	baseDraw();
}
