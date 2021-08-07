/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#ifndef GD_SAMPLE_ACTION_EDITOR_H
#define GD_SAMPLE_ACTION_EDITOR_H

#include "baseActionEditor.h"
#include "gui/elems/actionEditor/envelopeEditor.h"
#include "gui/elems/actionEditor/sampleActionEditor.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/choice.h"

namespace giada::v
{
class gdSampleActionEditor : public gdBaseActionEditor
{
public:
	gdSampleActionEditor(ID channelId, m::conf::Conf& c);
	~gdSampleActionEditor();

	void rebuild() override;

	int getActionType() const;

private:
	bool canChangeActionType();

	geBox                m_barPadding;
	geSampleActionEditor m_sampleActionEditor;
	geEnvelopeEditor     m_envelopeEditor;
	geChoice             m_actionType;
};
} // namespace giada::v

#endif
