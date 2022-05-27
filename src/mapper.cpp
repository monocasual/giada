/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "mapper.h"
#include "utils/fs.h"
#include <filesystem>
#include <fstream>

namespace nl = nlohmann;

namespace giada
{
void Mapper::init()
{
	if (!std::filesystem::exists(m_mapsPath))
		return;

	for (const auto& d : std::filesystem::directory_iterator(m_mapsPath))
	{
		if (!d.is_regular_file()) // TODO - better mechanism to check if it's a valid map
			continue;
		m_mapFiles.push_back(d.path().filename().string());
	}
}

/* -------------------------------------------------------------------------- */

std::optional<nlohmann::json> Mapper::read(const std::string& file) const
{
	if (file.empty())
		return {};

	std::ifstream ifs(u::fs::join(m_mapsPath, file));
	if (!ifs.good())
		return {};

	nl::json j = nl::json::parse(ifs, nullptr, /*exceptions=*/false);
	if (j.is_discarded())
		return {};

	return {j};
}

/* -------------------------------------------------------------------------- */

const std::vector<std::string>& Mapper::getMapFilesFound() const
{
	return m_mapFiles;
}
} // namespace giada
