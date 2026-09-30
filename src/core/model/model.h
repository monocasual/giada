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

#ifndef G_MODEL_MODEL_H
#define G_MODEL_MODEL_H

#include "src/core/model/assetsEdit.h"
#include "src/core/model/documentEdit.h"
#include "src/core/model/modelEdit.h"
#include "src/core/model/modelView.h"
#include <functional>

namespace giada::m::model
{
class Model final
{
public:
	using DocumentWriteCallback = std::function<void(DocumentEdit&)>;
	using AssetsWriteCallback   = std::function<void(AssetsEdit&)>;
	using ModelWriteCallback    = std::function<void(ModelEdit&)>;

	/* Returns a view of the current model. Use it only while this Model exists.
	Do not call from the audio thread. */

	[[nodiscard]] ModelView read() const;

	/* Queues an edit on the writer thread. Use the edit and references obtained
	from it only during the callback. Do not call from the audio thread. */

	void write(SwapType, DocumentWriteCallback callback);
	void write(SwapType, AssetsWriteCallback callback);
	void write(SwapType, ModelWriteCallback callback);

	/* Audio thread only. Use the view and references obtained from it only
	during this audio callback, before the next realtime read. */

	[[nodiscard]] ModelView readRealtime();

	/* Waits for earlier writes to finish. Never call from the audio thread or
	from inside a write callback. */

	void flush();

	/* Call only from the GUI thread. */

	SwapType pollDocumentChanges();
	SwapType pollAssetsChanges();

private:
	RealtimeModel m_realtimeModel;
};
} // namespace giada::m::model

#endif
