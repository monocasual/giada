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



int Patch::write(const char *file, const char *name, bool isProject)
{
	FILE *fp = fopen(file, "w");
	if (fp == NULL) {
    gLog("[patch::write] unable to read file\n");
		return 0;
  }

  jRoot = json_object();
  
  json_object_set_new(jRoot, "header",         json_string(header));
  json_object_set_new(jRoot, "version",        json_string(version));
  json_object_set_new(jRoot, "version_float",  json_real(versionFloat));
  json_object_set_new(jRoot, "name",           json_string(name));
  json_object_set_new(jRoot, "bpm",            json_integer(bpm));
  json_object_set_new(jRoot, "bars",           json_integer(bars));
  json_object_set_new(jRoot, "beats",          json_integer(beats));
  json_object_set_new(jRoot, "quantize",       json_integer(quantize));
  json_object_set_new(jRoot, "master_vol_in",  json_integer(masterVolIn));
  json_object_set_new(jRoot, "master_vol_out", json_integer(masterVolOut));
	json_object_set_new(jRoot, "metronome",      json_integer(metronome));
	json_object_set_new(jRoot, "last_take_id",   json_integer(lastTakeId));
	json_object_set_new(jRoot, "samplerate",     json_integer(samplerate)); // original samplerate when the patch was saved

  /* columns */
  
  json_t *jColumns = json_array();
  for (unsigned i=0; i<columns.size; i++) {
    json_t *jColumn = json_object();
    json_object_set_new(jColumn, "index", json_integer(columns.at(i).index));
    json_object_set_new(jColumn, "width", json_integer(columns.at(i).width));
    
    /* channels */
    json_t *jChannels = json_array();
    for (unsigned k=0; k<columns.at(i).channels.size; k++) {
      json_array_append_new(jChannels, json_integer(columns.at(i).channels.at(k)));
    }
    json_object_set_new(jColumn, "channels", jChannels);
    json_array_append_new(jColumns, jColumn);
  }
  json_object_set_new(jRoot, "columns", jColumns);
  
  /* channels */
  
  json_t *jChannels = json_array();
  for (unsigned i=0; i<channels.size; i++) {
    json_t *jChannel = json_object();
  }
  
  char *out = json_dumps(jRoot, 0);
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
