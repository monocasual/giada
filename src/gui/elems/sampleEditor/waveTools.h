/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GE_WAVE_TOOLS_H
#define GE_WAVE_TOOLS_H

#include <FL/Fl_Scroll.H>

namespace giada::c::sampleEditor
{
struct Data;
}

namespace giada::v
{
class geWaveform_DEPR_;
class geWaveTools : public Fl_Scroll
{
public:
	geWaveTools(int x, int y, int w, int h, bool gridEnabled, int gridVal);

	void resize(int x, int y, int w, int h) override;
	int  handle(int e) override;

	/* rebuild
	Updates the waveform by realloc-ing new data (i.e. when the waveform has
	changed). */

	void rebuild(const c::sampleEditor::Data& d);

	/* refresh
	Redraws the waveform, called by the video thread. This is meant to be called
	repeatedly when you need to update the play head inside the waveform. The
	method is smart enough to skip painting if the channel is stopped. */

	void refresh();

	const c::sampleEditor::Data& getChannelData() const { return *m_data; }

	v::geWaveform_DEPR_* waveform_DEPR_;

private:
	void openMenu();

	const c::sampleEditor::Data* m_data;
};
} // namespace giada::v

#endif
