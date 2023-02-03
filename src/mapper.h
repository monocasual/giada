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

#ifndef G_MAPPER_H
#define G_MAPPER_H

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace giada
{
class Mapper
{
public:
	/* getMapFilesFound
	Returns a reference to the list of maps found. */

	const std::vector<std::string>& getMapFilesFound() const;

	/* init
	Parses the map folder m_mapsPath and find the available maps. */

	void init();

protected:
	/* read
	Reads a map from file 'file' as a JSON object. */

	std::optional<nlohmann::json> read(const std::string& file) const;

	/* m_mapsPath
	Path to folder containing map files, different between OSes. */

	std::string m_mapsPath;

	/* m_mapFiles
	The available map files. Each element of the vector represents a map file 
    found in the map folder. */

	std::vector<std::string> m_mapFiles;
};
} // namespace giada

#endif
