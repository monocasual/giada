/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#ifndef G_MODEL_RECORDER_H
#define G_MODEL_RECORDER_H

#include "core/weakAtomic.h"

namespace giada::m::model
{
class Recorder
{
	friend class Model;

public:
	bool a_isRecordingAction() const;
	bool a_isRecordingInput() const;
	void a_setRecordingAction(bool) const;
	void a_setRecordingInput(bool) const;

private:
	struct State
	{
		WeakAtomic<bool> isRecordingAction = false;
		WeakAtomic<bool> isRecordingInput  = false;
	};

	State* state = nullptr;
};
} // namespace giada::m::model

#endif
