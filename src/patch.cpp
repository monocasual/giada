/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * patch
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2011 Giovanni A. Zuliani | Monocasual
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


#include "patch.h"
#include "init.h"
#include "recorder.h"
#include "utils.h"
#include "conf.h"
#include "pluginHost.h"
#include "wave.h"
#include "mixer.h"
#include "channel.h"


extern Mixer 		  G_Mixer;
extern Conf 		  G_Conf;
#ifdef WITH_VST
extern PluginHost G_PluginHost;
#endif


int Patch::open(const char *file) {
	fp = fopen(file, "r");
	if (fp == NULL)
		return PATCH_UNREADABLE;

	if (getValue("header") != "GIADAPTC")
		return PATCH_INVALID;

	version = atof(getValue("versionf").c_str());
	printf("[PATCH] open patch version %f\n", version);
	if (version == 0.0)
		puts("[PATCH] patch < 0.6.1, backward compatibility mode");

	return PATCH_OPEN_OK;
}


/* ------------------------------------------------------------------ */


void Patch::setDefault() {
	name[0]    = '\0';
  lastTakeId = 0;
  samplerate = DEFAULT_SAMPLERATE;
}


/* ------------------------------------------------------------------ */


int Patch::close() {
	return fclose(fp);
}


/* ------------------------------------------------------------------ */


void Patch::getName() {
	std::string out = getValue("patchname");
	strncpy(name, out.c_str(), MAX_PATCHNAME_LEN);
}


/* ------------------------------------------------------------------ */


std::string Patch::getSamplePath(int c) {
	char tmp[16];
	sprintf(tmp, "samplepath%d", c);
	return getValue(tmp);
}


/* ------------------------------------------------------------------ */


float Patch::getPitch(int c) {
	char tmp[16];
	sprintf(tmp, "chanPitch%d", c);
	float out = atof(getValue(tmp).c_str());
	if (out > 2.0f || out < 0.1f)
		return 1.0f;
	return out;
}


/* ------------------------------------------------------------------ */


int Patch::getNumChans() {
	if (version == 0.0)      // backward compatibility with version < 0.6.1
		return 32;
	return atoi(getValue("channels").c_str());
}


/* ------------------------------------------------------------------ */


char Patch::getSide(int c) {
	if (version == 0.0)      // backward compatibility with version < 0.6.1
		return 0;
	char tmp[16];
	sprintf(tmp, "chanSide%d", c);
	return atoi(getValue(tmp).c_str());
}

/* ------------------------------------------------------------------ */


int Patch::getIndex(int c) {
	if (version == 0.0)      // backward compatibility with version < 0.6.1
		return c;

	char tmp[16];
	sprintf(tmp, "chanIndex%d", c);
	return atoi(getValue(tmp).c_str());
}


/* ------------------------------------------------------------------ */


float Patch::getVol(int c) {
	char tmp[16];
	sprintf(tmp, "chanvol%d", c);
	float out = atof(getValue(tmp).c_str());
	if (out > 1.0f || out < 0.0f)
		return DEFAULT_VOL;
	return out;
}


/* ------------------------------------------------------------------ */


int Patch::getMode(int c) {
	char tmp[16];
	sprintf(tmp, "chanmode%d", c);
	int out = atoi(getValue(tmp).c_str());
	if (out & (LOOP_ANY | SINGLE_ANY))
		return out;
	return DEFAULT_CHANMODE;
}


/* ------------------------------------------------------------------ */


int Patch::getMute(int c) {
	char tmp[16];
	sprintf(tmp, "chanmute%d", c);
	return atoi(getValue(tmp).c_str());
}


/* ------------------------------------------------------------------ */


int Patch::getMute_s(int c) {
	char tmp[16];
	sprintf(tmp, "chanMute_s%d", c);
	return atoi(getValue(tmp).c_str());
}


/* ------------------------------------------------------------------ */


int Patch::getSolo(int c) {
	char tmp[16];
	sprintf(tmp, "chanSolo%d", c);
	return atoi(getValue(tmp).c_str());
}


/* ------------------------------------------------------------------ */


int Patch::getType(int c) {
	char tmp[16];
	sprintf(tmp, "chanType%d", c);
	int out = atoi(getValue(tmp).c_str());
	if (out == 0)
		return CHANNEL_SAMPLE;
	return out;
}


/* ------------------------------------------------------------------ */


int Patch::getStart(int c) {
	char tmp[16];
	sprintf(tmp, "chanstart%d", c);
	int out = atoi(getValue(tmp).c_str());
	if (out < 0)
		return 0;
	return out;
}


/* ------------------------------------------------------------------ */


int Patch::getEnd(int c, unsigned size) {
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


/* ------------------------------------------------------------------ */


float Patch::getBoost(int c) {
	char tmp[16];
	sprintf(tmp, "chanBoost%d", c);
	float out = atof(getValue(tmp).c_str());
	if (out < 1.0f)
		return DEFAULT_BOOST;
	return out;
}


/* ------------------------------------------------------------------ */


float Patch::getPanLeft(int c) {
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


/* ------------------------------------------------------------------ */


int Patch::getKey(int c) {
	if (version == 0.0)      // backward compatibility with version < 0.6.1
		return 0;
	char tmp[16];
	sprintf(tmp, "chanKey%d", c);
	return atoi(getValue(tmp).c_str());
}


/* ------------------------------------------------------------------ */


float Patch::getPanRight(int c) {
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


/* ------------------------------------------------------------------ */


bool Patch::getRecActive(int c) {
	char tmp[16];
	sprintf(tmp, "chanRecActive%d", c);
	return atoi(getValue(tmp).c_str());
}


/* ------------------------------------------------------------------ */


float Patch::getOutVol() {
	return atof(getValue("outVol").c_str());
}


/* ------------------------------------------------------------------ */


float Patch::getInVol() {
	return atof(getValue("inVol").c_str());
}


/* ------------------------------------------------------------------ */


float Patch::getBpm() {
	float out = atof(getValue("bpm").c_str());
	if (out < 20.0f || out > 999.0f)
		return DEFAULT_BPM;
	return out;
}


/* ------------------------------------------------------------------ */


int Patch::getBars() {
	int out = atoi(getValue("bars").c_str());
	if (out <= 0 || out > 32)
		return DEFAULT_BARS;
	return out;
}


/* ------------------------------------------------------------------ */


int Patch::getBeats() {
	int out = atoi(getValue("beats").c_str());
	if (out <= 0 || out > 32)
		return DEFAULT_BEATS;
	return out;
}


/* ------------------------------------------------------------------ */


int Patch::getQuantize() {
	int out = atoi(getValue("quantize").c_str());
	if (out < 0 || out > 8)
		return DEFAULT_QUANTIZE;
	return out;
}


/* ------------------------------------------------------------------ */


bool Patch::getMetronome() {
	bool out = atoi(getValue("metronome").c_str());
	if (out != true || out != false)
		return false;
	return out;
}


/* ------------------------------------------------------------------ */


int Patch::getLastTakeId() {
	return atoi(getValue("lastTakeId").c_str());
}


/* ------------------------------------------------------------------ */


int Patch::getSamplerate() {
	int out = atoi(getValue("samplerate").c_str());
	if (out <= 0)
		return DEFAULT_SAMPLERATE;
	return out;
}


/* ------------------------------------------------------------------ */


int Patch::readRecs() {

	puts("[PATCH] Reading recs...");

	unsigned numrecs = atoi(getValue("numrecs").c_str());

	for (unsigned i=0; i<numrecs; i++) {
		int frame, recPerFrame;

		/* parsing 'dddddd d': [framenumber] [num. recs for that frame]  */
		char tmpbuf[16];
		sprintf(tmpbuf, "recframe%d", i);
		sscanf(getValue(tmpbuf).c_str(), "%d %d", &frame, &recPerFrame);

//printf("processing frame=%d, recPerFrame=%d\n", frame, recPerFrame);

		for (int k=0; k<recPerFrame; k++) {
			int   chan = 0;
			int   type = 0;
			float fValue = 0.0f;
			int   iValue = 0;

			/* reading info for each frame: %d|%d */

			char tmpbuf[16];
			sprintf(tmpbuf, "f%da%d", i, k);

			if (version < 0.61f) // no float and int values
				sscanf(getValue(tmpbuf).c_str(), "%d|%d", &chan, &type);
			else
				sscanf(getValue(tmpbuf).c_str(), "%d|%d|%f|%d", &chan, &type, &fValue, &iValue);

//printf("  loading chan=%d, type=%d, fValue=%f, iValue=%d\n", chan, type, fValue, iValue);

			/* backward compatibility < 0.4.1 */

			if (frame % 2 != 0)
				frame++;

			channel *ch = G_Mixer.getChannelByIndex(chan);
			if (ch)
				if (ch->status & ~(STATUS_WRONG | STATUS_MISSING | STATUS_EMPTY))
					recorder::rec(ch->index, type, frame, iValue, fValue);
		}
	}
	return 1;
}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST
int Patch::readPlugins() {
	puts("[PATCH] Reading plugins...");

	int globalOut = 1;

	/* master plugins */

	globalOut &= readMasterPlugins(PluginHost::MASTER_IN);
	globalOut &= readMasterPlugins(PluginHost::MASTER_OUT);

	/* channel plugins */

	for (unsigned i=0; i<G_Mixer.channels.size; i++) {
		channel *ch = G_Mixer.channels.at(i);

		char tmp[MAX_LINE_LEN];
		sprintf(tmp, "chan%dPlugins", ch->index);
		int np = atoi(getValue(tmp).c_str());

		for (int j=0; j<np; j++) {
			sprintf(tmp, "chan%d_p%dpathfile", ch->index, j);
			int out = G_PluginHost.addPlugin(getValue(tmp).c_str(), PluginHost::CHANNEL, ch);
			if (out != 0) {
				sprintf(tmp, "chan%d_p%dnumParams", ch->index, j);
				int nparam = atoi(getValue(tmp).c_str());
				Plugin *pPlugin = G_PluginHost.getPluginByIndex(j, PluginHost::CHANNEL, ch);
				sprintf(tmp, "chan%d_p%dbypass", ch->index, j);
				pPlugin->bypass = atoi(getValue(tmp).c_str());
				for (int k=0; k<nparam; k++) {
					sprintf(tmp, "chan%d_p%dparam%dvalue", ch->index, j, k);
					float pval = atof(getValue(tmp).c_str());
					pPlugin->setParam(k, pval);
				}
			}
			globalOut &= out;
		}
	}
	return globalOut;
}
#endif


/* ------------------------------------------------------------------ */


int Patch::write(const char *file, const char *name, bool project) {
	fp = fopen(file, "w");
	if (fp == NULL)
		return 0;

	fprintf(fp, "# --- Giada patch file --- \n");
	fprintf(fp, "header=GIADAPTC\n");
	fprintf(fp, "version=%s\n", VERSIONE);
	fprintf(fp, "versionf=%f\n", VERSIONE_FLOAT);
	fprintf(fp, "patchname=%s\n", name);
	fprintf(fp, "channels=%d\n", G_Mixer.channels.size);

	for (unsigned i=0; i<G_Mixer.channels.size; i++) {

		fprintf(fp, "# --- channel %d --- \n", i);
		channel *ch = G_Mixer.channels.at(i);

		const char *path = "";
		if (ch->wave != NULL) {
			path = ch->wave->pathfile.c_str();
			if (project)
				path = gBasename(path).c_str();  // make it portable
		}

		fprintf(fp, "samplepath%d=%s\n",    i, path);
		fprintf(fp, "chanSide%d=%d\n",      i, ch->side);
		fprintf(fp, "chanType%d=%d\n",      i, ch->type);
		fprintf(fp, "chanKey%d=%d\n",       i, ch->key);
		fprintf(fp, "chanIndex%d=%d\n",     i, ch->index);
		fprintf(fp, "chanmute%d=%d\n",      i, ch->mute);
		fprintf(fp, "chanMute_s%d=%d\n",    i, ch->mute_s);
		fprintf(fp, "chanSolo%d=%d\n",      i, ch->solo);
		fprintf(fp, "chanvol%d=%f\n",       i, ch->volume);
		fprintf(fp, "chanmode%d=%d\n",      i, ch->mode);
		fprintf(fp, "chanstart%d=%d\n",     i, ch->startTrue);       // true values, not pitched
		fprintf(fp, "chanend%d=%d\n",       i, ch->endTrue);         // true values, not pitched
		fprintf(fp, "chanBoost%d=%f\n",     i, ch->boost);
		fprintf(fp, "chanPanLeft%d=%f\n",   i, ch->panLeft);
		fprintf(fp, "chanPanRight%d=%f\n",  i, ch->panRight);
		fprintf(fp, "chanRecActive%d=%d\n", i, ch->readActions);
		fprintf(fp, "chanPitch%d=%f\n",     i, ch->pitch);
	}

	fprintf(fp, "bpm=%f\n",        G_Mixer.bpm);
	fprintf(fp, "bars=%d\n",       G_Mixer.bars);
	fprintf(fp, "beats=%d\n",      G_Mixer.beats);
	fprintf(fp, "quantize=%d\n",   G_Mixer.quantize);
	fprintf(fp, "outVol=%f\n",     G_Mixer.outVol);
	fprintf(fp, "inVol=%f\n",      G_Mixer.inVol);
	fprintf(fp, "metronome=%d\n",  G_Mixer.metronome);
	fprintf(fp, "lastTakeId=%d\n", lastTakeId);
	fprintf(fp, "samplerate=%d\n", G_Conf.samplerate);	// original samplerate when the patch was saved

	/* writing recs. Warning: channel index is not mixer.channels.at(chan),
	 * but mixer.channels.at(chan)->index! */

	fprintf(fp, "# --- actions --- \n");
	fprintf(fp, "numrecs=%d\n", recorder::global.size);
	for (unsigned i=0; i<recorder::global.size; i++) {
		fprintf(fp, "recframe%d=%d %d\n", i, recorder::frames.at(i), recorder::global.at(i).size);
		for (unsigned k=0; k<recorder::global.at(i).size; k++) {
			fprintf(fp, "f%da%d=%d|%d|%f|%d\n",
				i, k,
				recorder::global.at(i).at(k)->chan,
				recorder::global.at(i).at(k)->type,
				recorder::global.at(i).at(k)->fValue,
				recorder::global.at(i).at(k)->iValue);
		}
	}

#ifdef WITH_VST

	/* writing master VST parameters */

	writeMasterPlugins(PluginHost::MASTER_IN);
	writeMasterPlugins(PluginHost::MASTER_OUT);

	/* writing VST parameters, channels. chan%d is mixer::channels.at(%d)->index,
	 * not mixer::chanels.at(%d)! */

	int numPlugs;
	int numParams;
	Plugin *pPlugin;

	fprintf(fp, "# --- VST / channels --- \n");
	for (unsigned i=0; i<G_Mixer.channels.size; i++) {
		channel *ch = G_Mixer.channels.at(i);
		numPlugs    = G_PluginHost.countPlugins(PluginHost::CHANNEL, ch);
		fprintf(fp, "chan%dPlugins=%d\n", ch->index, numPlugs);

		for (int j=0; j<numPlugs; j++) {
			pPlugin = G_PluginHost.getPluginByIndex(j, PluginHost::CHANNEL, ch);
			fprintf(fp, "chan%d_p%dpathfile=%s\n", ch->index, j, pPlugin->pathfile);
			fprintf(fp, "chan%d_p%dbypass=%d\n",   ch->index, j, pPlugin->bypass);
			numParams = pPlugin->getNumParams();
			fprintf(fp, "chan%d_p%dnumParams=%d\n", ch->index, j, numParams);

			for (int k=0; k<numParams; k++)
				fprintf(fp, "chan%d_p%dparam%dvalue=%f\n", ch->index, j, k, pPlugin->getParam(k));
		}
	}

#endif

	fclose(fp);
	return 1;
}


/* ------------------------------------------------------------------ */

#ifdef WITH_VST

int Patch::readMasterPlugins(int type) {

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
		int out = G_PluginHost.addPlugin(getValue(tmp).c_str(), type);
		if (out != 0) {
			Plugin *pPlugin = G_PluginHost.getPluginByIndex(i, type);
			sprintf(tmp, "master%c_p%dbypass", chr, i);
			pPlugin->bypass = atoi(getValue(tmp).c_str());
			sprintf(tmp, "master%c_p%dnumParams", chr, i);
			int nparam = atoi(getValue(tmp).c_str());
			for (int j=0; j<nparam; j++) {
				sprintf(tmp, "master%c_p%dparam%dvalue", chr, i, j);
				float pval = atof(getValue(tmp).c_str());
				pPlugin->setParam(j, pval);
			}
		}
		res &= out;
	}

	return res;
}


/* ------------------------------------------------------------------ */


void Patch::writeMasterPlugins(int type) {

	char chr;

	if (type == PluginHost::MASTER_IN) {
		fprintf(fp, "# --- VST / master in --- \n");
		chr = 'I';
	}
	else {
		fprintf(fp, "# --- VST / master out --- \n");
		chr = 'O';
	}

	int nmp = G_PluginHost.countPlugins(type);
	fprintf(fp, "master%cPlugins=%d\n", chr, nmp);

	for (int i=0; i<nmp; i++) {
		Plugin *pPlugin = G_PluginHost.getPluginByIndex(i, type);
		fprintf(fp, "master%c_p%dpathfile=%s\n", chr, i, pPlugin->pathfile);
		fprintf(fp, "master%c_p%dbypass=%d\n", chr, i, pPlugin->bypass);
		int numParams = pPlugin->getNumParams();
		fprintf(fp, "master%c_p%dnumParams=%d\n", chr, i, numParams);

		for (int j=0; j<numParams; j++)
			fprintf(fp, "master%c_p%dparam%dvalue=%f\n", chr, i, j, pPlugin->getParam(j));
	}
}

#endif
