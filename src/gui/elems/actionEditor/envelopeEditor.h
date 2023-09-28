/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GE_ENVELOPE_EDITOR_H
#define GE_ENVELOPE_EDITOR_H

#include "baseActionEditor.h"

namespace giada::v
{
class geEnvelopeEditor : public geBaseActionEditor
{
public:
	geEnvelopeEditor(Pixel x, Pixel y, gdBaseActionEditor*);
	void draw() override;

	void rebuild(c::actionEditor::Data& d) override;

private:
	void onAddAction() override;
	void onDeleteAction() override;
	void onMoveAction() override;
	void onResizeAction() override{}; // Nothing to do here
	void onRefreshAction() override;

	Pixel frameToX(Frame frame) const;
	Pixel valueToY(int value) const;
	int   yToValue(Pixel pixel, Pixel offset = 0) const;

	bool isFirstPoint() const;
	bool isLastPoint() const;
};
} // namespace giada::v

#endif
