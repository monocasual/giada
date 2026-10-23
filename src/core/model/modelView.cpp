#include "src/core/model/modelView.h"
#include <cassert>
#include <utility>

namespace giada::m::model
{
ModelView::ModelView(
    std::shared_ptr<const Document> document,
    std::shared_ptr<const Assets>   assets,
    const Parameters&               parameters)
: m_document(std::move(document))
, m_assets(std::move(assets))
, m_parameters(parameters)
{
}

/* -------------------------------------------------------------------------- */

ModelView makeModelView(const Model& model)
{
	/* Same order as RealtimeModel::read(): Document first, then Assets, so every
	asset the Document refers to is already in the Assets we get. */

	auto document = model.getCurrentDocument();
	auto assets   = model.getCurrentAssets();

	assert(document != nullptr);
	assert(assets != nullptr);

	return ModelView(std::move(document), std::move(assets), model.getParameters());
}
} // namespace giada::m::model
