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

#include "src/core/model/model.h"
#include <utility>

namespace giada::m::model
{
ModelView Model::read() const
{
	return ModelView(m_realtimeModel.getSnapshot());
}

/* -------------------------------------------------------------------------- */

void Model::write(SwapType type, DocumentWriteCallback callback)
{
	m_realtimeModel.writeDocument(type, [this, callback = std::move(callback)](Document& document)
	{
		const auto      assets = m_realtimeModel.getCurrentAssets();
		const ModelView view(document, *assets, m_realtimeModel.getParameters());
		DocumentEdit    edit(document, view);
		callback(edit);
	});
}

/* -------------------------------------------------------------------------- */

void Model::write(SwapType type, AssetsWriteCallback callback)
{
	m_realtimeModel.writeAssets(type, [this, callback = std::move(callback)](Assets& assets)
	{
		const auto      document = m_realtimeModel.getCurrentDocument();
		const ModelView view(*document, assets, m_realtimeModel.getParameters());
		AssetsEdit      edit(assets, view);
		callback(edit);
	});
}

/* -------------------------------------------------------------------------- */

void Model::write(SwapType type, ModelWriteCallback callback)
{
	m_realtimeModel.writeDocumentAndAssets(type, [this, callback = std::move(callback)](Document& document, Assets& assets)
	{
		const ModelView view(document, assets, m_realtimeModel.getParameters());
		ModelEdit       edit(document, assets, view);
		callback(edit);
	});
}

/* -------------------------------------------------------------------------- */

ModelView Model::readRealtime()
{
	const auto lock = m_realtimeModel.read();
	return ModelView(lock.getDocument(), lock.getAssets(), lock.getParameters());
}

/* -------------------------------------------------------------------------- */

void Model::flush()
{
	m_realtimeModel.flush();
}

/* -------------------------------------------------------------------------- */

SwapType Model::pollDocumentChanges()
{
	return m_realtimeModel.documentChanges.take();
}

/* -------------------------------------------------------------------------- */

SwapType Model::pollAssetsChanges()
{
	return m_realtimeModel.assetsChanges.take();
}
} // namespace giada::m::model
