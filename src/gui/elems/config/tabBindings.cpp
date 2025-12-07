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

#include "src/gui/elems/config/tabBindings.h"
#include "src/gui/elems/basics/liquidScroll.h"
#include "src/gui/elems/keyBinder.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"
#include "src/utils/log.h"
#include <format>
#include <ranges>

extern giada::v::Ui* g_ui;

namespace giada::v
{
geTabBindings::geTabBindings(geompp::Rect<int> bounds, const Model& model)
: Fl_Group(bounds.x, bounds.y, bounds.w, bounds.h, g_ui->getI18Text(LangMap::CONFIG_BINDINGS_TITLE))
{
	end();

	geFlex* body = new geFlex(bounds.reduced(G_GUI_OUTER_MARGIN), Direction::VERTICAL, G_GUI_INNER_MARGIN);
	{
		play          = new geKeyBinder(g_ui->getI18Text(LangMap::CONFIG_BINDINGS_PLAY), model.keyBindPlay);
		rewind        = new geKeyBinder(g_ui->getI18Text(LangMap::CONFIG_BINDINGS_REWIND), model.keyBindRewind);
		recordActions = new geKeyBinder(g_ui->getI18Text(LangMap::CONFIG_BINDINGS_RECORDACTIONS), model.keyBindRecordActions);
		recordInput   = new geKeyBinder(g_ui->getI18Text(LangMap::CONFIG_BINDINGS_RECORDAUDIO), model.keyBindRecordInput);
		exit          = new geKeyBinder(g_ui->getI18Text(LangMap::CONFIG_BINDINGS_EXIT), model.keyBindExit);

		for (const auto [index, scene] : std::views::enumerate(scenes))
			scene = new geKeyBinder(std::format("{} {}", g_ui->getI18Text(LangMap::COMMON_SCENE), index + 1), model.keyBindScenes[index]);

		body->addWidget(play, G_GUI_UNIT);
		body->addWidget(rewind, G_GUI_UNIT);
		body->addWidget(recordActions, G_GUI_UNIT);
		body->addWidget(recordInput, G_GUI_UNIT);
		body->addWidget(exit, G_GUI_UNIT);
		for (geKeyBinder* scene : scenes)
			body->addWidget(scene, G_GUI_UNIT);
		body->end();
	}

	add(body);
	resizable(body);

	play->onKeyBound = [](int key)
	{ g_ui->model.keyBindPlay = key; };
	play->onKeyClear = []
	{ g_ui->model.keyBindPlay = 0; };

	rewind->onKeyBound = [](int key)
	{ g_ui->model.keyBindRewind = key; };
	rewind->onKeyClear = []
	{ g_ui->model.keyBindRewind = 0; };

	recordActions->onKeyBound = [](int key)
	{ g_ui->model.keyBindRecordActions = key; };
	recordActions->onKeyClear = []
	{ g_ui->model.keyBindRecordActions = 0; };

	recordInput->onKeyBound = [](int key)
	{ g_ui->model.keyBindRecordInput = key; };
	recordInput->onKeyClear = []
	{ g_ui->model.keyBindRecordInput = 0; };

	exit->onKeyBound = [](int key)
	{ g_ui->model.keyBindExit = key; };
	exit->onKeyClear = []
	{ g_ui->model.keyBindExit = 0; };

	for (const auto [index, scene] : std::views::enumerate(scenes))
	{
		scene->onKeyBound = [index](int key)
		{ g_ui->model.keyBindScenes[index] = key; };
		scene->onKeyClear = [index]
		{ g_ui->model.keyBindScenes[index] = 0; };
	}
}
} // namespace giada::v