/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * patch
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


#include <stdint.h>
#include <cstring>
#include "../utils/log.h"
#include "../utils/fs.h"
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/elems/mainWindow/keyboard/keyboard.h"
#include "patch_DEPR_.h"
#include "init.h"
#include "recorder.h"
#include "conf.h"
#include "pluginHost.h"
#include "plugin.h"
#include "wave.h"
#include "mixer.h"
#include "channel.h"


extern Mixer 		  G_Mixer;
extern Conf       G_Conf;
extern Recorder   G_Recorder;
#ifdef WITH_VST
extern PluginHost G_PluginHost;
#endif
extern gdMainWindow *mainWin;


int Patch_DEPR_::open(const char *file)
{
	fp = fopen(file, "r");
	if (fp == NULL)
		return PATCH_UNREADABLE;

	if (getValue("header") != "GIADAPTC")
		return PATCH_INVALID;

	version = atof(getValue("versionf").c_str());
	gu_log("[patch_DEPR_] open patch version %f\n", version);

	return PATCH_READ_OK;
}


/* -------------------------------------------------------------------------- */


void Patch_DEPR_::setDefault()
{
	name[0]    = '\0';
  lastTakeId = 0;
  samplerate = G_DEFAULT_SAMPLERATE;
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::close()
{
	return fclose(fp);
}


/* -------------------------------------------------------------------------- */


void Patch_DEPR_::getName()
{
	std::string out = getValue("patchname");
	strncpy(name, out.c_str(), G_MAX_PATCHNAME_LEN);
}


/* -------------------------------------------------------------------------- */


std::string Patch_DEPR_::getSamplePath(int c)
{
	char tmp[16];
	sprintf(tmp, "samplepath%d", c);
	return getValue(tmp);
}


/* -------------------------------------------------------------------------- */


float Patch_DEPR_::getPitch(int c)
{
	char tmp[16];
	sprintf(tmp, "chanPitch%d", c);
	float out = atof(getValue(tmp).c_str());
	if (out > 2.0f || out < 0.1f)
		return 1.0f;
	return out;
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getNumChans()
{
	if (version == 0.0)      // backward compatibility with version < 0.6.1
		return 32;
	return atoi(getValue("channels").c_str());
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getNumColumns()
{
	return atoi(getValue("columns").c_str());
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getColumn(int c)
{
	if (version == 0.0)      // backward compatibility with version < 0.6.1
		return 0;
	char tmp[16];
	sprintf(tmp, "chanColumn%d", c);
	return atoi(getValue(tmp).c_str());
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getIndex(int c)
{
	if (version == 0.0)      // backward compatibility with version < 0.6.1
		return c;

	char tmp[16];
	sprintf(tmp, "chanIndex%d", c);
	return atoi(getValue(tmp).c_str());
}


/* -------------------------------------------------------------------------- */


float Patch_DEPR_::getVol(int c)
{
	char tmp[16];
	sprintf(tmp, "chanvol%d", c);
	float out = atof(getValue(tmp).c_str());
	if (out > 1.0f || out < 0.0f)
		return G_DEFAULT_VOL;
	return out;
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getMode(int c)
{
	char tmp[16];
	sprintf(tmp, "chanmode%d", c);
	int out = atoi(getValue(tmp).c_str());
	if (out & (LOOP_ANY | SINGLE_ANY))
		return out;
	return G_DEFAULT_CHANMODE;
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getMute(int c)
{
	char tmp[16];
	sprintf(tmp, "chanMute%d", c);
	return atoi(getValue(tmp).c_str());
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getMute_s(int c)
{
	char tmp[16];
	sprintf(tmp, "chanMute_s%d", c);
	return atoi(getValue(tmp).c_str());
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getSolo(int c)
{
	char tmp[16];
	sprintf(tmp, "chanSolo%d", c);
	return atoi(getValue(tmp).c_str());
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getType(int c)
{
	char tmp[16];
	sprintf(tmp, "chanType%d", c);
	int out = atoi(getValue(tmp).c_str());
	if (out == 0)
		return CHANNEL_SAMPLE;
	return out;
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getBegin(int c)
{
	char tmp[16];
	if (version < 0.73f)
		sprintf(tmp, "chanstart%d", c);
	else
		sprintf(tmp, "chanBegin%d", c);
	int out = atoi(getValue(tmp).c_str());
	if (out < 0)
		return 0;
	return out;
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getEnd(int c, unsigned size)
{
	char tmp[16];
	sprintf(tmp, "chanend%d", c);

	/* if chanEnd doesn't exist, it returns an atoi(empty string) == 0.
	 * good in theory, a disaster in practice. */

	std::string val = getValue(tmp);
	if (val == "")
		return size;

	unsigned out = atoi(val.c_str());
	if (out <= 0 || out > size)
		return size;
	return out;
}


/* -------------------------------------------------------------------------- */


float Patch_DEPR_::getBoost(int c)
{
	char tmp[16];
	sprintf(tmp, "chanBoost%d", c);
	float out = atof(getValue(tmp).c_str());
	if (out < 1.0f)
		return G_DEFAULT_BOOST;
	return out;
}


/* -------------------------------------------------------------------------- */


float Patch_DEPR_::getPanLeft(int c)
{
	char tmp[16];
	sprintf(tmp, "chanPanLeft%d", c);
	std::string val = getValue(tmp);
	if (val == "")
		return 1.0f;

	float out = atof(val.c_str());
	if (out < 0.0f || out > 1.0f)
		return 1.0f;
	return out;
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getKey(int c)
{
	if (version == 0.0)      // backward compatibility with version < 0.6.1
		return 0;
	char tmp[16];
	sprintf(tmp, "chanKey%d", c);
	return atoi(getValue(tmp).c_str());
}


/* -------------------------------------------------------------------------- */


float Patch_DEPR_::getPanRight(int c)
{
	char tmp[16];
	sprintf(tmp, "chanPanRight%d", c);
	std::string val = getValue(tmp);
	if (val == "")
		return 1.0f;

	float out = atof(val.c_str());
	if (out < 0.0f || out > 1.0f)
		return 1.0f;
	return out;
}


/* -------------------------------------------------------------------------- */


bool Patch_DEPR_::getRecActive(int c)
{
	char tmp[16];
	sprintf(tmp, "chanRecActive%d", c);
	return atoi(getValue(tmp).c_str());
}


/* -------------------------------------------------------------------------- */


float Patch_DEPR_::getOutVol()
{
	return atof(getValue("outVol").c_str());
}


/* -------------------------------------------------------------------------- */


float Patch_DEPR_::getInVol()
{
	return atof(getValue("inVol").c_str());
}


/* -------------------------------------------------------------------------- */


float Patch_DEPR_::getBpm()
{
	float out = atof(getValue("bpm").c_str());
	if (out < 20.0f || out > 999.0f)
		return G_DEFAULT_BPM;
	return out;
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getBars()
{
	int out = atoi(getValue("bars").c_str());
	if (out <= 0 || out > 32)
		return G_DEFAULT_BARS;
	return out;
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getBeats()
{
	int out = atoi(getValue("beats").c_str());
	if (out <= 0 || out > 32)
		return G_DEFAULT_BEATS;
	return out;
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getQuantize()
{
	int out = atoi(getValue("quantize").c_str());
	if (out < 0 || out > 8)
		return G_DEFAULT_QUANTIZE;
	return out;
}


/* -------------------------------------------------------------------------- */


bool Patch_DEPR_::getMetronome()
{
	return atoi(getValue("metronome").c_str());
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getLastTakeId()
{
	return atoi(getValue("lastTakeId").c_str());
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::getSamplerate()
{
	int out = atoi(getValue("samplerate").c_str());
	if (out <= 0)
		return G_DEFAULT_SAMPLERATE;
	return out;
}


/* -------------------------------------------------------------------------- */


uint32_t Patch_DEPR_::getMidiValue(int i, const char *c)
{
	char tmp[32];
	sprintf(tmp, "chanMidi%s%d", c, i);
	return strtoul(getValue(tmp).c_str(), NULL, 10);
}


/* -------------------------------------------------------------------------- */


int Patch_DEPR_::readRecs()
{
	gu_log("[patch_DEPR_] Reading recs...\n");

	unsigned numrecs = atoi(getValue("numrecs").c_str());

	for (unsigned i=0; i<numrecs; i++) {
		int frame, recPerFrame;

		/* parsing 'dddddd d': [framenumber] [num. recs for that frame]  */

		char tmpbuf[16];
		sprintf(tmpbuf, "recframe%d", i);
		sscanf(getValue(tmpbuf).c_str(), "%d %d", &frame, &recPerFrame);

//gu_log("processing frame=%d, recPerFrame=%d\n", frame, recPerFrame);

		for (int k=0; k<recPerFrame; k++) {
			int      chan = 0;
			int      type = 0;
			float    fValue = 0.0f;
			int      iValue_fix = 0;
			uint32_t iValue = 0;

			/* reading info for each frame: %d|%d */

			char tmpbuf[16];
			sprintf(tmpbuf, "f%da%d", i, k);

			if (version < 0.61f)    // no float and int values
				sscanf(getValue(tmpbuf).c_str(), "%d|%d", &chan, &type);
			else
				if (version < 0.83f)  // iValues were stored as signed int (wrong)
					sscanf(getValue(tmpbuf).c_str(), "%d|%d|%f|%d", &chan, &type, &fValue, &iValue_fix);
				else
					sscanf(getValue(tmpbuf).c_str(), "%d|%d|%f|%u", &chan, &type, &fValue, &iValue);

//gu_log("  loading chan=%d, type=%d, fValue=%f, iValue=%u\n", chan, type, fValue, iValue);

			Channel *ch = G_Mixer.getChannelByIndex(chan);
			if (ch)
				if (ch->status & ~(STATUS_WRONG | STATUS_MISSING | STATUS_EMPTY)) {
					if (version < 0.83f)
						G_Recorder.rec(ch->index, type, frame, iValue_fix, fValue);
					else
						G_Recorder.rec(ch->index, type, frame, iValue, fValue);
          ch->hasActions = true;
				}
		}
	}
	return 1;
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST
int Patch_DEPR_::readPlugins()
{
	gu_log("[patch_DEPR_] Reading plugins...\n");

	int globalOut = 1;

	/* master plugins */

	globalOut &= readMasterPlugins(PluginHost::MASTER_IN);
	globalOut &= readMasterPlugins(PluginHost::MASTER_OUT);

	/* channel plugins */

	for (unsigned i=0; i<G_Mixer.channels.size(); i++) {
		Channel *ch = G_Mixer.channels.at(i);

		char tmp[MAX_LINE_LEN];
		sprintf(tmp, "chan%dPlugins", ch->index);
		int np = atoi(getValue(tmp).c_str());

		for (int j=0; j<np; j++) {
			sprintf(tmp, "chan%d_p%dpathfile", ch->index, j);
			Plugin *plugin = G_PluginHost.addPlugin(getValue(tmp).c_str(), PluginHost::CHANNEL, &G_Mixer.mutex_plugins, ch);
			if (plugin != NULL) {
				sprintf(tmp, "chan%d_p%dnumParams", ch->index, j);
				int nparam = atoi(getValue(tmp).c_str());
				Plugin *pPlugin = G_PluginHost.getPluginByIndex(j, PluginHost::CHANNEL, ch);
				sprintf(tmp, "chan%d_p%dbypass", ch->index, j);
				if (pPlugin) {
					pPlugin->setBypass(atoi(getValue(tmp).c_str()));
					for (int k=0; k<nparam; k++) {
						sprintf(tmp, "chan%d_p%dparam%dvalue", ch->index, j, k);
						float pval = atof(getValue(tmp).c_str());
						pPlugin->setParameter(k, pval);
					}
				}
				globalOut &= 1;
			}
			else
				globalOut &= 0;
		}
	}
	return globalOut;
}
#endif


/* -------------------------------------------------------------------------- */

#ifdef WITH_VST

int Patch_DEPR_::readMasterPlugins(int type)
{
	int  nmp;
	char chr;
	int  res = 1;

	if (type == PluginHost::MASTER_IN) {
		chr = 'I';
		nmp = atoi(getValue("masterIPlugins").c_str());
	}
	else {
		chr = 'O';
		nmp = atoi(getValue("masterOPlugins").c_str());
	}

	for (int i=0; i<nmp; i++) {
		char tmp[MAX_LINE_LEN];
		sprintf(tmp, "master%c_p%dpathfile", chr, i);
		Plugin *p = G_PluginHost.addPlugin(getValue(tmp).c_str(), type, &G_Mixer.mutex_plugins);
		if (p != NULL) {
			Plugin *pPlugin = G_PluginHost.getPluginByIndex(i, type);
			sprintf(tmp, "master%c_p%dbypass", chr, i);
			pPlugin->setBypass(atoi(getValue(tmp).c_str()));
			sprintf(tmp, "master%c_p%dnumParams", chr, i);
			int nparam = atoi(getValue(tmp).c_str());
			for (int j=0; j<nparam; j++) {
				sprintf(tmp, "master%c_p%dparam%dvalue", chr, i, j);
				float pval = atof(getValue(tmp).c_str());
				pPlugin->setParameter(j, pval);
			}
			res &= 1;
		}
		else
			res &= 0;
	}

	return res;
}

#endif
