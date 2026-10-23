#ifndef G_MODEL_MODEL_VIEW_H
#define G_MODEL_MODEL_VIEW_H

#include "src/core/model/types.h"
#include <memory>

namespace giada::m::model
{
class ModelView;

/* makeModelView
Builds a view of the latest published Document and Assets, plus the live
Parameters. For non-realtime threads only. */

ModelView makeModelView(const Model&);

/* ModelView
Read-only bundle of Document, Assets and Parameters, so they can be passed
around as one object. Document and Assets are snapshots kept alive by the view;
Parameters is live and can change while the view exists. */

class ModelView final
{
public:
	const Document&   getDocument() const { return *m_document; }
	const Assets&     getAssets() const { return *m_assets; }
	const Parameters& getParameters() const { return m_parameters; }

private:
	friend ModelView makeModelView(const Model&);

	ModelView(
	    std::shared_ptr<const Document> document,
	    std::shared_ptr<const Assets>   assets,
	    const Parameters&               parameters);

	std::shared_ptr<const Document> m_document;
	std::shared_ptr<const Assets>   m_assets;
	const Parameters&               m_parameters;
};
} // namespace giada::m::model

#endif
