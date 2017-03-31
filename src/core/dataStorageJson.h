/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * dataStorageIni
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#ifndef G_DATA_STORAGE_JSON_H
#define G_DATA_STORAGE_JSON_H


#include <jansson.h>


namespace giada {
namespace storager
{
bool setString(json_t *jRoot, const char *key, std::string &output);
bool setFloat(json_t *jRoot, const char *key, float &output);
bool setUint32(json_t *jRoot, const char *key, uint32_t &output);
bool setInt(json_t *jRoot, const char *key, int &output);
bool setBool(json_t *jRoot, const char *key, bool &output);

/* checkObject
check whether the jRoot object is a valid json object {} */

bool checkObject(json_t *jRoot, const char *key);

/* checkArray
check whether the jRoot object is a valid json array [] */

bool checkArray(json_t *jRoot, const char *key);
}}; // giada::storager::


class DataStorageJson
{
protected:

  json_t       *jRoot;
  json_error_t  jError;

  bool setString(json_t *jRoot, const char *key, std::string &output);
  bool setFloat(json_t *jRoot, const char *key, float &output);
  bool setUint32(json_t *jRoot, const char *key, uint32_t &output);
  bool setInt(json_t *jRoot, const char *key, int &output);
  bool setBool(json_t *jRoot, const char *key, bool &output);

  /* checkObject
  check whether the jRoot object is a valid json object {} */

  bool checkObject(json_t *jRoot, const char *key);

  /* checkArray
  check whether the jRoot object is a valid json array [] */

  bool checkArray(json_t *jRoot, const char *key);
};

#endif
