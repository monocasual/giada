/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


class geResizerBar;


namespace giada {
namespace v
{
class geSampleActionEditor;
class geEnvelopeEditor;

class gdSampleActionEditor : public gdBaseActionEditor
{  
public:

    gdSampleActionEditor(ID channelId);

    void rebuild() override;

private:

	geSampleActionEditor* m_ae;
    geResizerBar*         m_aer;

	geEnvelopeEditor*     m_ee;
    geResizerBar*         m_eer;

	bool canChangeActionType();
};
}} // giada::v::


#endif
