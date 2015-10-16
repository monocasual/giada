/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * dataStorageIni
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2015 Giovanni A. Zuliani | Monocasual
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


#include <string>
#include "../utils/log.h"
#include "dataStorageJson.h"


using std::string;


bool DataStorageJson::setString(json_t *jRoot, const char *key, string &output)
{
  json_t *jObject = json_object_get(jRoot, key);
  if (!json_is_string(jObject)) {
    gLog("[dataStorageJson::setString] %s is not a string!\n", key);
    json_decref(jRoot);
    return false;
  }
  output = json_string_value(jObject);
  return true;
}


/* -------------------------------------------------------------------------- */


bool DataStorageJson::setFloat(json_t *jRoot, const char *key, float &output)
{
  json_t *jObject = json_object_get(jRoot, key);
  if (!json_is_real(jObject)) {
    gLog("[dataStorageJson::setString] %s is not a float!\n", key);
    json_decref(jRoot);
    return false;
  }
  output = json_real_value(jObject);
  return true;
}


/* -------------------------------------------------------------------------- */


bool DataStorageJson::setUint32(json_t *jRoot, const char *key, uint32_t &output)
{
  json_t *jObject = json_object_get(jRoot, key);
  if (!json_is_integer(jObject)) {
    gLog("[dataStorageJson::setString] %d is not an integer!\n", key);
    json_decref(jRoot);
    return false;
  }
  output = json_integer_value(jObject);
  return true;
}


/* -------------------------------------------------------------------------- */


bool DataStorageJson::setInt(json_t *jRoot, const char *key, int &output)
{
  return setUint32(jRoot, key, (uint32_t&) output);
}


/* -------------------------------------------------------------------------- */


bool DataStorageJson::checkObject(json_t *jRoot, const char *key)
{
  if (!json_is_object(jRoot)) {
    gLog("[DataStorageJson::checkObject] malformed json: %s is not an object!\n", key);
    json_decref(jRoot);
    return false;
  }
  return true;
}


/* -------------------------------------------------------------------------- */


bool DataStorageJson::checkArray(json_t *jRoot, const char *key)
{
  if (!json_is_array(jRoot)) {
    gLog("[DataStorageJson::checkObject] malformed json: %s is not an array!\n", key);
    json_decref(jRoot);
    return false;
  }
  return true;
}
