/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_MODEL_ASSETS_EDIT_H
#define G_MODEL_ASSETS_EDIT_H

namespace giada::m::model
{
struct Assets;
class Model;
class ModelView;

class AssetsEdit final
{
public:
	Assets&          get() { return m_assets; }
	const Assets&    get() const { return m_assets; }
	const ModelView& view() const { return m_view; }

private:
	friend class Model;

	AssetsEdit(Assets& assets, const ModelView& view)
	: m_assets(assets)
	, m_view(view)
	{
	}

	Assets&          m_assets;
	const ModelView& m_view;
};
} // namespace giada::m::model

#endif
