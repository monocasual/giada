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


#include <cstdio>
#include <cstdlib>
#include <cstring>  // memcpy
#include <cmath>
#include <samplerate.h>
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


Wave::Wave(float* data, int size, int channels, const std::string& path)
: m_data    (data),
  m_size    (size),
  m_channels(channels),
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
	m_logical(false),
	m_edited (false)
{
	m_size = other.m_size;
	m_data = new float[m_size];
	memcpy(m_data, other.m_data, m_size * sizeof(float));
	memcpy(&inHeader, &other.inHeader, sizeof(other.inHeader));
	m_path = other.m_path;
	m_name = other.m_name;
	m_logical = true;
}

/* -------------------------------------------------------------------------- */


int Wave::open(const char* f)
{
	m_path = f;
	m_name = gu_stripExt(gu_basename(f));
	fileIn = sf_open(f, SFM_READ, &inHeader);

	if (fileIn == nullptr) {
		gu_log("[wave] unable to read %s. %s\n", f, sf_strerror(fileIn));
		m_path = "";
		m_name = "";
		return 0;
	}

	m_logical = false;
	m_edited  = false;

	return 1;
}


/* -------------------------------------------------------------------------- */


int Wave::readData()
{
	/* Libsndfile's frame structure:

	frame1 = [leftChannel, rightChannel]
	frame2 = [leftChannel, rightChannel]
	... */
	
	int newSize = inHeader.frames * inHeader.channels;
	m_data = new (std::nothrow) float[newSize];
	if (m_data == nullptr) {
		gu_log("[wave] unable to allocate memory\n");
		return 0;
	}

	m_size = newSize;

	if (sf_read_float(fileIn, m_data, m_size) != m_size)
		gu_log("[wave] warning: incomplete read!\n");

	sf_close(fileIn);
	return 1;
}


/* -------------------------------------------------------------------------- */


int Wave::writeData(const char* f)
{
	/* Prepare the header for output file. */

	outHeader.samplerate = inHeader.samplerate;
	outHeader.channels   = inHeader.channels;
	outHeader.format     = inHeader.format;

	fileOut = sf_open(f, SFM_WRITE, &outHeader);
	if (fileOut == nullptr) {
		gu_log("[wave] unable to open %s for exporting\n", f);
		return 0;
	}

	int out = sf_write_float(fileOut, m_data, m_size);
	if (out != (int) m_size) {
		gu_log("[wave] error while exporting %s! %s\n", f, sf_strerror(fileOut));
		return 0;
	}

	m_logical = false;
	m_edited  = false;
	sf_close(fileOut);
	return 1;
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


int Wave::allocEmpty(unsigned size, unsigned samplerate)
{
	/* The caller must pass a __size for stereo values. */

	m_data = new (std::nothrow) float[size];
	if (m_data == nullptr) {
		gu_log("[wave] unable to allocate memory\n");
		return 0;
	}

	m_size = size;

	memset(m_data, 0, sizeof(float) * m_size); /// FIXME - is it useful?

	inHeader.samplerate = samplerate;
	inHeader.channels   = 2;
	inHeader.format     = SF_FORMAT_WAV | SF_FORMAT_FLOAT; // wave only

	m_logical = true;
	return 1;
}


/* -------------------------------------------------------------------------- */


int Wave::resample(int quality, int newRate)
{
	float ratio = newRate / (float) inHeader.samplerate;
	int newSize = ceil(m_size * ratio);
	if (newSize % 2 != 0)   // libsndfile goes crazy with odd size in case of saving
		newSize++;

	float *tmp = new (std::nothrow) float[newSize];
	if (tmp == nullptr) {
		gu_log("[wave] unable to allocate memory for resampling\n");
		return -1;
	}

	SRC_DATA src_data;
	src_data.data_in       = m_data;
	src_data.input_frames  = m_size/2;     // in frames, i.e. /2 (stereo)
	src_data.data_out      = tmp;
	src_data.output_frames = newSize/2;  // in frames, i.e. /2 (stereo)
	src_data.src_ratio     = ratio;

	gu_log("[wave] resampling: new size=%d (%d frames)\n", newSize, newSize/2);

	int ret = src_simple(&src_data, quality, 2);
	if (ret != 0) {
		gu_log("[wave] resampling error: %s\n", src_strerror(ret));
		delete[] tmp;
		return 0;
	}

	delete[] m_data;
	m_data = tmp;
	m_size = newSize;
	inHeader.samplerate = newRate;
	return 1;
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


int Wave::getRate() const { return inHeader.samplerate; }
int Wave::getChannels() const { return inHeader.channels; }
int Wave::getFrames() const { return inHeader.frames; }
std::string Wave::getPath() const { return m_path; }
std::string Wave::getName() const { return m_name; }
float* Wave::getData() const { return m_data; }
int Wave::getSize() const { return m_size; }
bool Wave::isLogical() const { return m_logical; }
bool Wave::isEdited() const { return m_edited; }


/* -------------------------------------------------------------------------- */


void Wave::setRate(int v) { inHeader.samplerate = v; }
void Wave::setChannels(int v) { inHeader.channels = v; }
void Wave::setFrames(int v) { inHeader.frames = v; }
void Wave::setPath(const string& p) { m_path = p; }
void Wave::setData(float* d) { m_data = d; }
void Wave::setSize(int s) { m_size = s; }
void Wave::setLogical(bool l) { m_logical = l; }
void Wave::setEdited(bool e) { m_edited = e; }
