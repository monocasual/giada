/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/gui/dialogs/browser/browserLoad.h"
#include "src/core/engine.h"
#include "src/gui/dialogs/warnings.h"
#include <juce_core/juce_core.h>
#include "src/gui/elems/basics/imageButton.h"
#include "src/gui/elems/basics/input.h"
#include "src/gui/elems/basics/textButton.h"
#include "src/gui/elems/fileBrowser.h"
#include "src/gui/ui.h"
#include "src/utils/fs.h"

extern giada::v::Ui* g_ui;
extern giada::m::Engine* g_engine;

namespace giada::v
{
gdBrowserLoad::gdBrowserLoad(const std::string& title, const std::string& path,
    std::function<void(void*)> cb, ID channelId, const Model& model, bool audioPreview)
: gdBrowserBase(title, path, cb, channelId, model)
, m_audioPreview(audioPreview)
{
	hidePathName();

	ok->label(g_ui->getI18Text(LangMap::COMMON_LOAD));
	ok->onClick = [this]()
	{ stopPreview(); fireCallback(); };
	if (m_audioPreview)
	{
		preview->show();
		preview->onClick = [this]()
		{
			if (m_previewLoaded && g_engine->getSampleEditorApi().getPreviewStatus() == ChannelStatus::PLAY)
				stopPreview();
			else
			{
				stopPreview();
				const int result = g_engine->getSampleEditorApi().playPreviewFile(getSelectedItem());
				m_previewLoaded = result == G_RES_OK;
				if (!m_previewLoaded)
					gdAlert(g_ui->getI18Text(LangMap::BROWSER_PREVIEWERROR));
			}
			updatePreviewButton();
		};
		browser->onSelectItem = [this]() { stopPreview(); updatePreviewButton(); };
		const auto changeDir = browser->onChangeDir;
		browser->onChangeDir = [this, changeDir]()
		{ stopPreview(); changeDir(); updatePreviewButton(); };
		browser->onChooseItem = [this]() { stopPreview(); fireCallback(); };
		updatePreviewButton();
	}
}

gdBrowserLoad::~gdBrowserLoad() { stopPreview(); }

void gdBrowserLoad::stopPreview()
{
	if (m_previewLoaded)
		g_engine->getSampleEditorApi().freePreviewChannel();
	m_previewLoaded = false;
}

void gdBrowserLoad::updatePreviewButton()
{
	const juce::File file{juce::String::fromUTF8(getSelectedItem().c_str())};
	if (g_engine->isAudioReady() && file.existsAsFile() && file.hasFileExtension("wav;mp3;aif;aiff;flac;ogg"))
		preview->activate();
	else
		preview->deactivate();
	const bool playing = m_previewLoaded && g_engine->getSampleEditorApi().getPreviewStatus() == ChannelStatus::PLAY;
	preview->label(g_ui->getI18Text(playing ? LangMap::BROWSER_STOPPREVIEW : LangMap::BROWSER_PLAYPREVIEW));
}

void gdBrowserLoad::refresh()
{
	if (m_audioPreview)
		updatePreviewButton();
}
} // namespace giada::v
