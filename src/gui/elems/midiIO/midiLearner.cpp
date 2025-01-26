/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "gui/elems/midiIO/midiLearner.h"
#include "core/const.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/textButton.h"
#include "gui/ui.h"
#include <cassert>
#include <fmt/core.h>

extern giada::v::Ui* g_ui;

namespace giada::v
{
geMidiLearner::geMidiLearner(int x, int y, int w, int h, std::string l, int param)
: geFlex(x, y, w, h, Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
, onStartLearn(nullptr)
, onStopLearn(nullptr)
, onClearLearn(nullptr)
, m_param(param)
{
	m_text     = new geBox(l.c_str());
	m_value    = new geBox("");
	m_learnBtn = new geTextButton(g_ui->getI18Text(LangMap::COMMON_LEARN));
	m_clearBtn = new geTextButton(g_ui->getI18Text(LangMap::COMMON_CLEAR));

	addWidget(m_text);
	addWidget(m_value, 80);
	addWidget(m_learnBtn, 50);
	addWidget(m_clearBtn, 50);
	end();

	m_text->box(G_CUSTOM_BORDER_BOX);

	m_value->box(G_CUSTOM_BORDER_BOX);

	m_learnBtn->setToggleable(true);
	m_learnBtn->onClick = [this]()
	{
		assert(onStartLearn != nullptr);
		assert(onStopLearn != nullptr);

		if (m_learnBtn->getValue() == 1)
			onStartLearn(m_param);
		else
			onStopLearn();
	};

	m_clearBtn->onClick = [this]()
	{
		onClearLearn(m_param);
	};
}

/* -------------------------------------------------------------------------- */

void geMidiLearner::update(uint32_t value)
{
	std::string tmp = g_ui->getI18Text(LangMap::COMMON_NOTSET);

	if (value != 0x0)
	{
		tmp = fmt::format("{:#x}", value);
		tmp.pop_back(); // Remove last two digits, useless in MIDI messages
		tmp.pop_back(); // Remove last two digits, useless in MIDI messages
	}

	m_value->copy_label(tmp.c_str());
	m_learnBtn->setValue(0);
}

/* -------------------------------------------------------------------------- */

void geMidiLearner::update(const std::string& s)
{
	m_value->copy_label(s.c_str());
	m_learnBtn->setValue(0);
}

/* -------------------------------------------------------------------------- */

void geMidiLearner::activate()
{
	Fl_Group::activate();
	m_text->activate();
	m_value->activate();
	m_learnBtn->activate();
	m_clearBtn->activate();
}

void geMidiLearner::deactivate()
{
	Fl_Group::deactivate();
	m_text->deactivate();
	m_value->deactivate();
	m_learnBtn->deactivate();
	m_clearBtn->deactivate();
}
} // namespace giada::v