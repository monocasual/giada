/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * utils
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */

#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdio.h>
#include <cstdarg>
#include <sys/stat.h>   // stat (gDirExists)
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <limits.h>
#if defined(__APPLE__)
	#include <libgen.h>     // basename unix
#endif


/* gVector
 * lightweight template class. */

template <class T> class gVector {
public:


	/* gVector()
	 * default constructor, no parameters */

	gVector() : size(0), s(NULL) {}

	/* gVector(const &)
	 * copy-constructor, when gVector a = b (where b is gVector).
	 * Default constructor doesn't copy referenced ojbects, so we need
	 * to re-allocate the internal stack for the copied object */

	gVector(const gVector &other) {
		s = new T[other.size];
		for (unsigned i=0; i<other.size; i++)
			s[i] = other.s[i];
		size = other.size;
	}


	~gVector() {
		/// FIXME empty s with clear()?!?
	}


	void add(const T &item) {
		T *tmp = new T[size+1];  /// TODO: chunk increment (size+N), N ~= 16
		for (unsigned i=0; i<size; i++)
			tmp[i] = s[i];
		tmp[size] = item;
		delete[] s;
		s = tmp;
		size++;
	}


	int del(const T &item) {
		for (unsigned i=0; i<size; i++)
			if (s[i] == item)
				return del(i);
		return -1;
	}


	int del(unsigned p) {
		if (p > size-1) printf("[vector] del() outside! requested=%d, size=%d\n", p, size);
		T *tmp = new T[size-1];
		unsigned i=0;
		unsigned j=0;
		while (i<size) {
			if (i != p) {
				tmp[j] = s[i];
				j++;
			}
			i++;
		}
		delete[] s;
		s = tmp;
		size -= 1;
		return size;
	}


	void clear() {
		if (size > 0) {
			delete [] s;
			s = NULL;
			size = 0;
		}
	}


	void swap(unsigned x, unsigned y) {
		T tmp = s[x];
		s[x] = s[y];
		s[y] = tmp;
	}


	T &at(unsigned p) {
		if (p > size-1)	printf("[vector] at() outside! requested=%d, size=%d\n", p, size);
		return s[p];
	}


	T &last() {
		return s[size-1];
	}


	unsigned size;
	T *s;  				// stack (array of T)
};


/* ------------------------------------------------------------------ */


bool gFileExists(const char *filename);

bool gDirExists(const char *path);

bool gIsDir(const char *path);

bool gIsProject(const char *path);

bool gIsPatch(const char *path);

bool gMkdir(const char *path);

std::string gBasename(const char *path);

std::string gDirname(const char *path);

std::string gGetCurrentPath();

std::string gGetExt(const char *filename);

std::string gStripExt(const char *filename);

std::string gGetProjectName(const char *path);

#endif
