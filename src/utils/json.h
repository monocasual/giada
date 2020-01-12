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


#ifndef G_UTILS_JSON_H
#define G_UTILS_JSON_H


#include <string>
#include <jansson.h>


namespace giada {
namespace u     {
namespace json 
{
std::string readString(json_t* j, const char* key, const std::string& def="");
uint32_t    readInt(json_t* j, const char* key, uint32_t def=0);
float       readFloat(json_t* j, const char* key, float def=0.0f);
bool        readBool(json_t* j, const char* key, bool def=false);

bool isArray(json_t* j);
bool isObject(json_t* j);

json_t* load(const std::string& file);
}}}  // giada::u::json::


#endif
