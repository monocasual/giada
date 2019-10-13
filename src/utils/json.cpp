/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include <jansson.h>
#include "utils/log.h"
#include "json.h"


namespace giada {
namespace u     {
namespace json 
{
namespace
{
/* jsonIs, jsonGet
Tiny wrappers around the old C-style macros provided by Jansson. This way we can
pass them as template parameters. */

bool jsonIsString_(json_t* j) { return json_is_string(j); }
bool jsonIsInt_   (json_t* j) { return json_is_integer(j); }
bool jsonIsFloat_ (json_t* j) { return json_is_real(j); }
bool jsonIsBool_  (json_t* j) { return json_is_boolean(j); }
bool jsonIsArray_ (json_t* j) { return json_is_array(j); }
bool jsonIsObject_(json_t* j) { return json_is_object(j); }

std::string jsonGetString_(json_t* j) { return json_string_value(j); }
uint32_t    jsonGetInt_   (json_t* j) { return json_integer_value(j); }
float       jsonGetFloat_ (json_t* j) { return json_real_value(j); }
bool        jsonGetBool_  (json_t* j) { return json_boolean_value(j); }


/* -------------------------------------------------------------------------- */


template<typename F>
bool is_(json_t* j, F f)
{
	if (!f(j)) {
		gu_log("[patch::is_] malformed json!\n");
		json_decref(j);
		return false;
	}
	return true;
}


/* -------------------------------------------------------------------------- */


template<typename O, typename FC, typename FG>
O read_(json_t* j, const char* key, FC checker, FG getter, O def)
{
	json_t* jo = json_object_get(j, key);
	if (jo == nullptr) {
		gu_log("[patch::read_] key '%s' not found, using default value\n", key);
		return def;
	}
	if (!checker(jo)) {
		gu_log("[patch::read_] key '%s' is of the wrong type, using default value\n", key);
		return def;
	}
	return getter(jo);
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


std::string readString(json_t* j, const char* key, const std::string& def)
{
	return read_(j, key, jsonIsString_, jsonGetString_, def);
}

uint32_t readInt(json_t* j, const char* key, uint32_t def)
{
	return read_(j, key, jsonIsInt_, jsonGetInt_, def);
}

float readFloat(json_t* j, const char* key, float def)
{
	return read_(j, key, jsonIsFloat_, jsonGetFloat_, def);
}

bool readBool(json_t* j, const char* key, bool def)
{
	return read_(j, key, jsonIsBool_, jsonGetBool_, def);
}


/* -------------------------------------------------------------------------- */


bool isArray(json_t* j)  { return is_(j, jsonIsArray_); };
bool isObject(json_t* j) { return is_(j, jsonIsObject_); };


/* -------------------------------------------------------------------------- */


json_t* load(const std::string& file)
{
	json_error_t jerr;
	json_t* j = json_load_file(file.c_str(), 0, &jerr);
	if (j == nullptr)
		gu_log("[u::json::load] unable to read json file! Error on line %d: %s\n",
			jerr.line, jerr.text);
	return j;
}

}}}  // giada::u::json::
