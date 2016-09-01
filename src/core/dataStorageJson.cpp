/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * dataStorageIni
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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
    gu_log("[dataStorageJson::setString] key '%s' is not a string!\n", key);
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
  if (!jObject) {
    gu_log("[dataStorageJson::setFloat] key '%s' not found, using default value\n", key);
    output = 0.0f;
    return true;
  }
  if (!json_is_real(jObject)) {
    gu_log("[dataStorageJson::setFloat] key '%s' is not a float!\n", key);
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
  if (!jObject) {
    gu_log("[dataStorageJson::setUint32] key '%s' not found, using default value\n", key);
    output = 0;
    return true;
  }
  if (!json_is_integer(jObject)) {
    gu_log("[dataStorageJson::setUint32] key '%s' is not an integer!\n", key);
    json_decref(jRoot);
    return false;
  }
  output = json_integer_value(jObject);
  return true;
}


/* -------------------------------------------------------------------------- */


bool DataStorageJson::setBool(json_t *jRoot, const char *key, bool &output)
{
  json_t *jObject = json_object_get(jRoot, key);
  if (!jObject) {
    gu_log("[dataStorageJson::setBool] key '%s' not found, using default value\n", key);
    output = false;
    return true;
  }
  if (!json_is_boolean(jObject)) {
    gu_log("[dataStorageJson::setBool] key '%s' is not a boolean!\n", key);
    json_decref(jRoot);
    return false;
  }
  output = json_boolean_value(jObject);
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
    gu_log("[DataStorageJson::checkObject] malformed json: %s is not an object!\n", key);
    json_decref(jRoot);
    return false;
  }
  return true;
}


/* -------------------------------------------------------------------------- */


bool DataStorageJson::checkArray(json_t *jRoot, const char *key)
{
  if (!json_is_array(jRoot)) {
    gu_log("[DataStorageJson::checkObject] malformed json: %s is not an array!\n", key);
    json_decref(jRoot);
    return false;
  }
  return true;
}
