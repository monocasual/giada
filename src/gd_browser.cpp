/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_browser
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


#include "gd_browser.h"
#include "ge_browser.h"
#include "gd_pluginList.h"
#include "gd_mainWindow.h"
#include "gd_warnings.h"
#include "mixer.h"
#include "graphics.h"
#include "wave.h"
#include "glue.h"
#include "pluginHost.h"
#include "channel.h"
#include "sampleChannel.h"
#include "patch.h"
#include "conf.h"


extern Patch         G_Patch;
extern Conf	         G_Conf;
extern Mixer         G_Mixer;
#ifdef WITH_VST
extern PluginHost    G_PluginHost;
#endif
extern gdMainWindow	*mainWin;


gdBrowser::gdBrowser(const char *title, const char *initPath, Channel *ch, int type, int stackType)
	:	gWindow  (396, 302, title),
		ch       (ch),
		type     (type),
		stackType(stackType)
{
	set_non_modal();

	browser = new gBrowser(8, 36, 380, 230);
	Fl_Group *group_btn = new Fl_Group(8, 274, 380, 20);
		gBox *b = new gBox(8, 274, 204, 20); 					        // spacer window border <-> buttons
		ok  	  = new gClick(308, 274, 80, 20);
		cancel  = new gClick(220, 274, 80, 20, "Cancel");
		status  = new gProgress(8, 274, 204, 20);
		status->minimum(0);
		status->maximum(1);
		status->hide();   // show the bar only if necessary
	group_btn->resizable(b);
	group_btn->end();

	Fl_Group *group_upd = new Fl_Group(8, 8, 380, 25);
		if (type == BROWSER_SAVE_PATCH || type == BROWSER_SAVE_SAMPLE || type == BROWSER_SAVE_PROJECT)  /// bitmask please!
			name = new gInput(208, 8, 152, 20);
		if (type == BROWSER_SAVE_PATCH || type == BROWSER_SAVE_SAMPLE || type == BROWSER_SAVE_PROJECT)  /// bitmask please!
			where = new gInput(8, 8, 192, 20);
		else
			where = new gInput(8, 8, 352, 20);
		updir	= new gClick(368, 8, 20, 20, "", updirOff_xpm, updirOn_xpm);
	group_upd->resizable(where);
	group_upd->end();

	end();

	resizable(browser);
	size_range(w(), h(), 0, 0);

	where->readonly(true);
	where->cursor_color(COLOR_BG_DARK);

	if (type == BROWSER_SAVE_PATCH || type == BROWSER_SAVE_SAMPLE || type == BROWSER_SAVE_PROJECT)  /// bitmask please!
		ok->label("Save");
	else
		ok->label("Load");

	if (type == BROWSER_LOAD_PATCH)
		ok->callback(cb_load_patch, (void*)this);
	else
	if (type == BROWSER_LOAD_SAMPLE)
		ok->callback(cb_load_sample, (void*)this);
	else
	if (type == BROWSER_SAVE_PATCH) {
		ok->callback(cb_save_patch, (void*)this);
		name->value(G_Patch.name[0] == '\0' ? "my_patch.gptc" : G_Patch.name);
		name->maximum_size(MAX_PATCHNAME_LEN+5); // +5 for ".gptc"
	}
	else
	if (type == BROWSER_SAVE_SAMPLE) {
		ok->callback(cb_save_sample, (void*)this);
		name->value(((SampleChannel*)ch)->wave->name.c_str());
	}
	else
	if (type == BROWSER_SAVE_PROJECT) {
		ok->callback(cb_save_project, (void*)this);
		name->value(gStripExt(G_Patch.name).c_str());
	}
#ifdef WITH_VST
	else
	if (type == BROWSER_LOAD_PLUGIN) {
		ok->callback(cb_loadPlugin, (void*)this);
	}
#endif

	ok->shortcut(FL_Enter);

	updir->callback(cb_up, (void*)this);
	cancel->callback(cb_close, (void*)this);
	browser->callback(cb_down, this);
	browser->path_obj = where;
	browser->init(initPath);

	if (G_Conf.browserW)
		resize(G_Conf.browserX, G_Conf.browserY, G_Conf.browserW, G_Conf.browserH);

	gu_setFavicon(this);
	show();
}


/* ------------------------------------------------------------------ */


gdBrowser::~gdBrowser() {
	G_Conf.browserX = x();
	G_Conf.browserY = y();
	G_Conf.browserW = w();
	G_Conf.browserH = h();
}


/* ------------------------------------------------------------------ */


void gdBrowser::cb_load_patch  (Fl_Widget *v, void *p)  { ((gdBrowser*)p)->__cb_load_patch();  }
void gdBrowser::cb_load_sample (Fl_Widget *v, void *p)  { ((gdBrowser*)p)->__cb_load_sample(); }
void gdBrowser::cb_save_sample (Fl_Widget *v, void *p)  { ((gdBrowser*)p)->__cb_save_sample(); }
void gdBrowser::cb_save_patch  (Fl_Widget *v, void *p)  { ((gdBrowser*)p)->__cb_save_patch(); }
void gdBrowser::cb_save_project(Fl_Widget *v, void *p)  { ((gdBrowser*)p)->__cb_save_project(); }
void gdBrowser::cb_down        (Fl_Widget *v, void *p)  { ((gdBrowser*)p)->__cb_down(); }
void gdBrowser::cb_up          (Fl_Widget *v, void *p)  { ((gdBrowser*)p)->__cb_up(); }
void gdBrowser::cb_close       (Fl_Widget *v, void *p)  { ((gdBrowser*)p)->__cb_close(); }
#ifdef WITH_VST
void gdBrowser::cb_loadPlugin  (Fl_Widget *v, void *p)  { ((gdBrowser*)p)->__cb_loadPlugin(); }
#endif


/* ------------------------------------------------------------------ */


void gdBrowser::__cb_load_patch() {

	if (browser->text(browser->value()) == NULL)
		return;

	/* patchFile is the file to open.
	 * For patches:  browser->get_selected_item()
	 * for projects: browser->get_selected_item() without extention +
	 *               patch name appended */

	std::string patchFile = browser->get_selected_item();;
	bool        isProject;

	if (gIsProject(browser->get_selected_item())) {
		std::string patchName = gGetProjectName(browser->get_selected_item());
#if defined(__linux__) || defined(__APPLE__)
		patchFile = patchFile+"/"+patchName+".gptc";
#elif defined(_WIN32)
		patchFile = patchFile+"\\"+patchName+".gptc";
#endif
		isProject = true;
	}
	else
		isProject = false;

	int res = glue_loadPatch(patchFile.c_str(),	browser->path_obj->value(),	status, isProject);

	if (res == PATCH_UNREADABLE) {
		status->hide();
		if (isProject)
			gdAlert("This project is unreadable.");
		else
			gdAlert("This patch is unreadable.");
	}
	else if (res == PATCH_INVALID) {
		status->hide();
		if (isProject)
			gdAlert("This project is not valid.");
		else
			gdAlert("This patch is not valid.");
	}
	else
		do_callback();
}


/* ------------------------------------------------------------------ */


void gdBrowser::__cb_save_sample() {

	if (strcmp(name->value(), "") == 0) {    /// FIXME glue business
		gdAlert("Please choose a file name.");
		return;
	}

	/* bruteforce check extension. */

	std::string filename = gStripExt(name->value());
	char fullpath[PATH_MAX];
	sprintf(fullpath, "%s/%s.wav", where->value(), filename.c_str());

	if (gFileExists(fullpath))
		if (!gdConfirmWin("Warning", "File exists: overwrite?"))
			return;

	if (((SampleChannel*)ch)->save(fullpath))
		do_callback();
	else
		gdAlert("Unable to save this sample!");
}


/* ------------------------------------------------------------------ */


void gdBrowser::__cb_load_sample() {
	if (browser->text(browser->value()) == NULL)
		return;

	int res = glue_loadChannel((SampleChannel*) ch, browser->get_selected_item(), browser->path_obj->value());

	if (res == SAMPLE_LOADED_OK) {
		do_callback();
		mainWin->delSubWindow(WID_SAMPLE_EDITOR); // if editor is open
	}
	else if (res == SAMPLE_NOT_VALID)
		gdAlert("This is not a valid WAVE file.");
	else if (res == SAMPLE_MULTICHANNEL)
		gdAlert("Multichannel samples not supported.");
	else if (res == SAMPLE_WRONG_BIT)
		gdAlert("This sample has an\nunsupported bit-depth (> 32 bit).");
	else if (res == SAMPLE_WRONG_ENDIAN)
		gdAlert("This sample has a wrong\nbyte order (not little-endian).");
	else if (res == SAMPLE_WRONG_FORMAT)
		gdAlert("This sample is encoded in\nan unsupported audio format.");
	else if (res == SAMPLE_READ_ERROR)
		gdAlert("Unable to read this sample.");
	else if (res == SAMPLE_PATH_TOO_LONG)
		gdAlert("File path too long.");
	else
		gdAlert("Unknown error.");
}


/* ------------------------------------------------------------------ */


void gdBrowser::__cb_down() {
	const char *path = browser->get_selected_item();
	if (!path)  // when click on an empty area
		return;
	if (!gIsDir(path)) {

		/* set the name of the patch/sample/project as the selected item */

		if (type == BROWSER_SAVE_PATCH || type == BROWSER_SAVE_SAMPLE || type == BROWSER_SAVE_PROJECT) {
			if (gIsProject(path)) {
				std::string tmp = browser->text(browser->value());
				tmp.erase(0, 4);
				name->value(tmp.c_str());
			}
			else
				name->value(browser->text(browser->value()));
		}
		return;
	}
	browser->clear();
	browser->down_dir(path);
	browser->sort();
}


/* ------------------------------------------------------------------ */


void gdBrowser::__cb_up() {
	browser->clear();
	browser->up_dir();
	browser->sort();
}


/* ------------------------------------------------------------------ */


void gdBrowser::__cb_save_patch() {

	if (strcmp(name->value(), "") == 0) {  /// FIXME glue business
		gdAlert("Please choose a file name.");
		return;
	}

	/* if name->value() contains ".gptc" */

	char ext[6] = ".gptc";
	if (strstr(name->value(), ".gptc") != NULL)
		ext[0] = '\0';

	char fullpath[PATH_MAX];
	sprintf(fullpath, "%s/%s%s", where->value(), name->value(), ext);
	if (gFileExists(fullpath))
		if (!gdConfirmWin("Warning", "File exists: overwrite?"))
			return;

	if (glue_savePatch(fullpath, name->value(), false)) // false == not a project
		do_callback();
	else
		gdAlert("Unable to save the patch!");
}


/* ------------------------------------------------------------------ */


void gdBrowser::__cb_save_project() {

	if (strcmp(name->value(), "") == 0) {    /// FIXME glue business
		gdAlert("Please choose a project name.");
		return;
	}

	/* check if name->value() contains ".gprj" */

	char ext[6] = ".gprj";
	if (strstr(name->value(), ".gprj") != NULL)
		ext[0] = '\0';

	char fullpath[PATH_MAX];
#if defined(_WIN32)
	sprintf(fullpath, "%s\\%s%s", where->value(), name->value(), ext);
#else
	sprintf(fullpath, "%s/%s%s", where->value(), name->value(), ext);
#endif

	if (gIsProject(fullpath) && !gdConfirmWin("Warning", "Project exists: overwrite?"))
		return;

	if (glue_saveProject(fullpath, name->value()))
		do_callback();
	else
		gdAlert("Unable to save the project!");
}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST
void gdBrowser::__cb_loadPlugin() {

	if (browser->text(browser->value()) == NULL)
		return;

	int res = G_PluginHost.addPlugin(browser->get_selected_item(), stackType, ch);

	/* store the folder path inside G_Conf, in order to reuse it the
	 * next time. */

	G_Conf.setPath(G_Conf.pluginPath, where->value());

	if (res)
		do_callback();
	else
		gdAlert("Unable to load the selected plugin!");
}
#endif


/* ------------------------------------------------------------------ */


void gdBrowser::__cb_close() {
	do_callback();
}
