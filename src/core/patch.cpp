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

#include "src/core/patch.h"

namespace geompp
{
void to_json(nlohmann::json& j, const giada::FrameRange& r)
{
	j = nlohmann::json{{"a", r.getA()}, {"b", r.getB()}};
}

void from_json(const nlohmann::json& j, giada::FrameRange& r)
{
	r.setA(j.value("a", 0));
	r.setB(j.value("b", 0));
}
} // namespace geompp

/* -------------------------------------------------------------------------- */

namespace mcl::utils
{
void to_json(nlohmann::json& j, const Id& id)
{
	j = id.getValue();
}

void from_json(const nlohmann::json& j, Id& id)
{
	id = Id(j.get<std::size_t>());
}
} // namespace mcl::utils

/* -------------------------------------------------------------------------- */

namespace giada
{
void to_json(nlohmann::json& j, const Tick& t) { j = t.value(); }
void from_json(const nlohmann::json& j, Tick& t) { t = Tick(j.get<Tick::Value>()); }
} // namespace giada
