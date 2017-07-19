/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * wave
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


#include <cstring>  // memcpy
#include "../utils/fs.h"
#include "../utils/log.h"
#include "init.h"
#include "const.h"
#include "wave.h"


using std::string;


Wave::Wave()
: m_data   (nullptr),
	m_size   (0),
	m_logical(0),
	m_edited (0) {}


/* -------------------------------------------------------------------------- */


Wave::Wave(float* data, int size, int channels, int rate, const std::string& path)
: m_data    (data),
  m_size    (size),
  m_channels(channels),
  m_rate    (rate),
  m_logical (false),
  m_edited  (false),
  m_path    (path),
  m_name    (gu_stripExt(gu_basename(path)))
{
}	


/* -------------------------------------------------------------------------- */


Wave::~Wave()
{
	clear();
}


/* -------------------------------------------------------------------------- */


Wave::Wave(const Wave& other)
: m_data   (nullptr),
	m_size   (0),
	m_logical(true),
	m_edited (false)
{
	m_size = other.m_size;
	m_data = new float[m_size];
	memcpy(m_data, other.m_data, m_size * sizeof(float));
	//memcpy(&inHeader, &other.inHeader, sizeof(other.inHeader));
	m_path = other.m_path;
	m_name = other.m_name;
}


/* -------------------------------------------------------------------------- */


void Wave::clear()
{
	if (m_data == nullptr) 
		return;
	delete[] m_data;
	m_data = nullptr;
	m_path = "";
	m_size = 0;
}


/* -------------------------------------------------------------------------- */


string Wave::getBasename(bool ext) const
{
	return ext ? gu_basename(m_path) : gu_stripExt(gu_basename(m_path));
}


/* -------------------------------------------------------------------------- */


void Wave::setName(const string& n)
{
	string ext = gu_getExt(m_path);
	m_name = gu_stripExt(gu_basename(n));
	m_path = gu_dirname(m_path) + G_SLASH + m_name + "." + ext;
	m_logical  = true;

	/* A wave with updated m_name must become logical, since the underlying file
	does not exist yet. */
}


/* -------------------------------------------------------------------------- */


int Wave::getRate() const { return m_rate; }
int Wave::getChannels() const { return m_channels; }
std::string Wave::getPath() const { return m_path; }
std::string Wave::getName() const { return m_name; }
float* Wave::getData() const { return m_data; }
int Wave::getSize() const { return m_size; }
bool Wave::isLogical() const { return m_logical; }
bool Wave::isEdited() const { return m_edited; }


/* -------------------------------------------------------------------------- */


void Wave::setRate(int v) { m_rate = v; }
void Wave::setChannels(int v) { m_channels = v; }
void Wave::setPath(const string& p) { m_path = p; }
void Wave::setData(float* d) { m_data = d; }
void Wave::setSize(int s) { m_size = s; }
void Wave::setLogical(bool l) { m_logical = l; }
void Wave::setEdited(bool e) { m_edited = e; }
