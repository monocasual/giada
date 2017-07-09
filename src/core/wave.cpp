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
: data   (nullptr),
	size   (0),
	logical(0),
	edited (0) {}


/* -------------------------------------------------------------------------- */


Wave::~Wave()
{
	clear();
}


/* -------------------------------------------------------------------------- */


Wave::Wave(const Wave& other)
: data   (nullptr),
	size   (0),
	logical(false),
	edited (false)
{
	size = other.size;
	data = new float[size];
	memcpy(data, other.data, size * sizeof(float));
	memcpy(&inHeader, &other.inHeader, sizeof(other.inHeader));
	path = other.path;
	name = other.name;
	logical = true;
}

/* -------------------------------------------------------------------------- */


int Wave::open(const char* f)
{
	path = f;
	name     = gu_stripExt(gu_basename(f));
	fileIn   = sf_open(f, SFM_READ, &inHeader);

	if (fileIn == nullptr) {
		gu_log("[wave] unable to read %s. %s\n", f, sf_strerror(fileIn));
		path = "";
		name     = "";
		return 0;
	}

	logical = false;
	edited  = false;

	return 1;
}


/* -------------------------------------------------------------------------- */

/* how to read and write with libsndfile:
 *
 * a frame consists of all items (samples) that belong to the same
 * point in time. So in each frame there are as many items as there
 * are channels.
 *
 * Quindi:
 * 	frame  = [item, item, ...]
 * In pratica:
 *  frame1 = [itemLeft, itemRight]
 * 	frame2 = [itemLeft, itemRight]
 * 	...
 */

int Wave::readData()
{
	size = inHeader.frames * inHeader.channels;
	data = (float*) malloc(size * sizeof(float));
	if (data == nullptr) {
		gu_log("[wave] unable to allocate memory\n");
		return 0;
	}

	if (sf_read_float(fileIn, data, size) != size)
		gu_log("[wave] warning: incomplete read!\n");

	sf_close(fileIn);
	return 1;
}


/* -------------------------------------------------------------------------- */


int Wave::writeData(const char* f)
{
	/* prepare the header for output file */

	outHeader.samplerate = inHeader.samplerate;
	outHeader.channels   = inHeader.channels;
	outHeader.format     = inHeader.format;

	fileOut = sf_open(f, SFM_WRITE, &outHeader);
	if (fileOut == nullptr) {
		gu_log("[wave] unable to open %s for exporting\n", f);
		return 0;
	}

	int out = sf_write_float(fileOut, data, size);
	if (out != (int) size) {
		gu_log("[wave] error while exporting %s! %s\n", f, sf_strerror(fileOut));
		return 0;
	}

	logical = false;
	edited  = false;
	sf_close(fileOut);
	return 1;
}


/* -------------------------------------------------------------------------- */


void Wave::clear()
{
	if (data != nullptr) {
		free(data);
		data     = nullptr;
		path = "";
		size     = 0;
	}
}


/* -------------------------------------------------------------------------- */


int Wave::allocEmpty(unsigned __size, unsigned samplerate)
{
	/* the caller must pass a __size for stereo values */

	/// FIXME - this way if malloc fails size becomes wrong
	size = __size;
	data = (float*) malloc(size * sizeof(float));
	if (data == nullptr) {
		gu_log("[wave] unable to allocate memory\n");
		return 0;
	}

	memset(data, 0, sizeof(float) * size); /// FIXME - is it useful?

	inHeader.samplerate = samplerate;
	inHeader.channels   = 2;
	inHeader.format     = SF_FORMAT_WAV | SF_FORMAT_FLOAT; // wave only

	logical = true;
	return 1;
}


/* -------------------------------------------------------------------------- */


int Wave::resample(int quality, int newRate)
{
	float ratio = newRate / (float) inHeader.samplerate;
	int newSize = ceil(size * ratio);
	if (newSize % 2 != 0)   // libsndfile goes crazy with odd size in case of saving
		newSize++;

	float *tmp = (float *) malloc(newSize * sizeof(float));
	if (!tmp) {
		gu_log("[wave] unable to allocate memory for resampling\n");
		return -1;
	}

	SRC_DATA src_data;
	src_data.data_in       = data;
	src_data.input_frames  = size/2;     // in frames, i.e. /2 (stereo)
	src_data.data_out      = tmp;
	src_data.output_frames = newSize/2;  // in frames, i.e. /2 (stereo)
	src_data.src_ratio     = ratio;

	gu_log("[wave] resampling: new size=%d (%d frames)\n", newSize, newSize/2);

	int ret = src_simple(&src_data, quality, 2);
	if (ret != 0) {
		gu_log("[wave] resampling error: %s\n", src_strerror(ret));
		return 0;
	}

	free(data);
	data = tmp;
	size = newSize;
	inHeader.samplerate = newRate;
	return 1;
}


/* -------------------------------------------------------------------------- */


string Wave::getBasename(bool ext) const
{
	return ext ? gu_basename(path) : gu_stripExt(gu_basename(path));
}


/* -------------------------------------------------------------------------- */


void Wave::setName(const string& n)
{
	string ext = gu_getExt(path);
	name = gu_stripExt(gu_basename(n));
	path = gu_dirname(path) + G_SLASH + name + "." + ext;
	logical  = true;

	/* a wave with updated name must become logical, since the underlying
	 * file does not exist yet. */
}


/* -------------------------------------------------------------------------- */


int Wave::getRate() const { return inHeader.samplerate; }
int Wave::getChannels() const { return inHeader.channels; }
int Wave::getFrames() const { return inHeader.frames; }
std::string Wave::getPath() const { return path; }
std::string Wave::getName() const { return name; }
float* Wave::getData() const { return data; }
int Wave::getSize() const { return size; }
bool Wave::isLogical() const { return logical; }
bool Wave::isEdited() const { return edited; }


/* -------------------------------------------------------------------------- */


void Wave::setRate(int v) { inHeader.samplerate = v; }
void Wave::setChannels(int v) { inHeader.channels = v; }
void Wave::setFrames(int v) { inHeader.frames = v; }
void Wave::setPath(const string& p) { path = p; }
void Wave::setData(float* d) { data = d; }
void Wave::setSize(int s) { size = s; }
void Wave::setEdited(bool e) { edited = e; }
