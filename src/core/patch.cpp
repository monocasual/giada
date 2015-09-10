/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * patch
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


#include <stdint.h>
#include "../utils/log.h"
#include "../utils/utils.h"
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/elems/ge_keyboard.h"
#include "patch.h"
#include "init.h"
#include "recorder.h"
#include "conf.h"
#include "pluginHost.h"
#include "wave.h"
#include "mixer.h"
#include "channel.h"


extern Mixer 		     G_Mixer;
extern Conf 		     G_Conf;
#ifdef WITH_VST
extern PluginHost    G_PluginHost;
#endif
extern gdMainWindow *mainWin;



int Patch::write(const char *file, const char *name, bool project)
{
	FILE *fp = fopen(file, "w");
	if (fp == NULL) {
    gLog("[patch::write] unable to read file\n");
		return 0;
  }

  root = json_object();
  
  json_object_set_new(root, "header",         json_string("GIADAPTC"));
  json_object_set_new(root, "version",        json_string(VERSIONE));
  json_object_set_new(root, "version_float",  json_real(VERSIONE_FLOAT));
  json_object_set_new(root, "patch_name",     json_string(name));
  
#if 0
  json_object_set_new(root, "bpm",            json_integer(G_Mixer.bpm));
  json_object_set_new(root, "bars",           json_integer(G_Mixer.bars));
  json_object_set_new(root, "beats",          json_integer(G_Mixer.beats));
  json_object_set_new(root, "quantize",       json_integer(G_Mixer.beats));
  json_object_set_new(root, "master_vol_in",  json_integer(G_Mixer.outVol));
  json_object_set_new(root, "master_vol_out", json_integer(G_Mixer.inVol));
	json_object_set_new(root, "metronome",      json_integer(G_Mixer.metronome));
	json_object_set_new(root, "last_take_id",   json_integer(lastTakeId));
	json_object_set_new(root, "samplerate",     json_integer(G_Conf.samplerate)); // original samplerate when the patch was saved
	json_object_set_new(root, "channels",       json_integer(G_Mixer.channels.size));
	json_object_set_new(root, "columns",        json_integer(mainWin->keyboard->getTotalColumns()));
#endif

  char *out = json_dumps(root, 0);
  fputs(out, fp);
  fclose(fp);
  free(out);

#if 0  
  
	fprintf(fp, "# --- Giada patch file --- \n");
	fprintf(fp, "header=GIADAPTC\n");
	fprintf(fp, "version=%s\n",    VERSIONE);
	fprintf(fp, "versionf=%f\n",   VERSIONE_FLOAT);
	fprintf(fp, "patchname=%s\n",  name);
	fprintf(fp, "bpm=%f\n",        G_Mixer.bpm);
	fprintf(fp, "bars=%d\n",       G_Mixer.bars);
	fprintf(fp, "beats=%d\n",      G_Mixer.beats);
	fprintf(fp, "quantize=%d\n",   G_Mixer.quantize);
	fprintf(fp, "outVol=%f\n",     G_Mixer.outVol);
	fprintf(fp, "inVol=%f\n",      G_Mixer.inVol);
	fprintf(fp, "metronome=%d\n",  G_Mixer.metronome);
	fprintf(fp, "lastTakeId=%d\n", lastTakeId);
	fprintf(fp, "samplerate=%d\n", G_Conf.samplerate);	// original samplerate when the patch was saved
	fprintf(fp, "channels=%d\n",   G_Mixer.channels.size);
	fprintf(fp, "columns=%d\n",    mainWin->keyboard->getTotalColumns());

	for (unsigned i=0; i<G_Mixer.channels.size; i++) {
		fprintf(fp, "# --- channel %d --- \n", i);
		G_Mixer.channels.at(i)->writePatch(fp, i, project);
	}

	/* writing recs. Warning: channel index is not mixer.channels.at(chan),
	 * but mixer.channels.at(chan)->index! */

	fprintf(fp, "# --- actions --- \n");
	fprintf(fp, "numrecs=%d\n", recorder::global.size);
	for (unsigned i=0; i<recorder::global.size; i++) {
		fprintf(fp, "recframe%d=%d %d\n", i, recorder::frames.at(i), recorder::global.at(i).size);
		for (unsigned k=0; k<recorder::global.at(i).size; k++) {
			fprintf(fp, "f%da%d=%d|%d|%f|%u\n",
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
		Channel *ch = G_Mixer.channels.at(i);
		numPlugs    = G_PluginHost.countPlugins(PluginHost::CHANNEL, ch);
		fprintf(fp, "chan%dPlugins=%d\n", ch->index, numPlugs);

		for (int j=0; j<numPlugs; j++) {
			pPlugin = G_PluginHost.getPluginByIndex(j, PluginHost::CHANNEL, ch);
			if (!pPlugin->status) {
				gLog("[patch] Plugin %d is in a bad status, skip writing params\n", i);
				continue;
			}
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

#endif // if 0
}
