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


#include <cassert>
#include <cstring>  // memcpy
#include "../utils/fs.h"
#include "../utils/log.h"
#include "../utils/string.h"
#include "const.h"
#include "wave.h"


using std::string;


Wave::Wave()
: m_data   (nullptr),
	m_size   (0),
	m_logical(0),
	m_edited (0) {}


/* -------------------------------------------------------------------------- */


Wave::Wave(float* data, int size, int channels, int rate, int bits, 
	const std::string& path)
: m_data    (data),
  m_size    (size),
  m_channels(channels),
  m_rate    (rate),
  m_bits    (bits),
  m_logical (false),
  m_edited  (false),
  m_path    (path)
{
}	


/* -------------------------------------------------------------------------- */


Wave::~Wave()
{
	clear();
}


/* -------------------------------------------------------------------------- */


Wave::Wave(const Wave& other)
: m_data    (nullptr),
	m_size    (other.m_size),
	m_channels(other.m_channels),
  m_rate    (other.m_rate),
  m_bits    (other.m_bits),	
	m_logical (true),   // a cloned wave does not exist on disk
	m_edited  (false),
	m_path    (other.m_path)
{
	m_data = new float[m_size];
	memcpy(m_data, other.m_data, m_size * sizeof(float));
}


/* -------------------------------------------------------------------------- */


void Wave::clear()
{
	free();
	m_path = "";
	m_size = 0;
}


/* -------------------------------------------------------------------------- */


void Wave::free()
{
	if (m_data == nullptr) 
		return;
	delete[] m_data;
	m_data = nullptr;
}


/* -------------------------------------------------------------------------- */


string Wave::getBasename(bool ext) const
{
	return ext ? gu_basename(m_path) : gu_stripExt(gu_basename(m_path));
}


/* -------------------------------------------------------------------------- */


int Wave::getRate() const { return m_rate; }
int Wave::getChannels() const { return m_channels; }
std::string Wave::getPath() const { return m_path; }
float* Wave::getData() const { return m_data; }
int Wave::getSize() const { return m_size / m_channels; }
int Wave::getBits() const { return m_bits; }
bool Wave::isLogical() const { return m_logical; }
bool Wave::isEdited() const { return m_edited; }


/* -------------------------------------------------------------------------- */


int Wave::getDuration() const
{
	return m_size / m_channels / m_rate;
}


/* -------------------------------------------------------------------------- */


std::string Wave::getExtension() const
{
	return gu_getExt(m_path);
}


/* -------------------------------------------------------------------------- */


float* Wave::getFrame(int f) const
{
	assert(f >= 0);
	assert(f < getSize());	

	f *= m_channels;    // convert frame to sample
	return m_data + f;  // i.e. a pointer to m_data[f]
}


/* -------------------------------------------------------------------------- */


void Wave::setRate(int v) { m_rate = v; }
void Wave::setChannels(int v) { m_channels = v; }
void Wave::setLogical(bool l) { m_logical = l; }
void Wave::setEdited(bool e) { m_edited = e; }


/* -------------------------------------------------------------------------- */


void Wave::setPath(const string& p, int id) 
{ 
	if (id == -1)
		m_path = p; 
	else 
		m_path = gu_stripExt(p) + "-" + gu_iToString(id) + "." + gu_getExt(p);


}


/* -------------------------------------------------------------------------- */


void Wave::setData(float* d, int size) 
{ 
	m_data = d; 
	m_size = size;
}
