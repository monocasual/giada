#include "flex.h"
#include <numeric>

namespace giada::v
{
geFlex::Elem::Elem(Fl_Widget& w, geFlex& parent, Direction d, int size)
: size(size)
, m_w(w)
, m_parent(parent)
, m_dir(d)
{
}

/* -------------------------------------------------------------------------- */

int geFlex::Elem::getSize() const
{
	if (isFixed())
		return size;
	return m_dir == Direction::VERTICAL ? m_w.h() : m_w.w();
}

/* -------------------------------------------------------------------------- */

bool geFlex::Elem::isFixed() const
{
	return size != -1;
}

/* -------------------------------------------------------------------------- */

void geFlex::Elem::resize(int pos, int newSize)
{
	if (m_dir == Direction::VERTICAL)
		m_w.resize(m_parent.x(), pos, m_parent.w(), newSize);
	else
		m_w.resize(pos, m_parent.y(), newSize, m_parent.h());
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geFlex::geFlex(int x, int y, int w, int h, Direction d, int gutter)
: Fl_Group(x, y, w, h, 0)
, m_direction(d)
, m_gutter(gutter)
, m_numFixed(0)
{
	Fl_Group::end();
}

/* -------------------------------------------------------------------------- */

geFlex::geFlex(geompp::Rect<int> r, Direction d, int gutter)
: geFlex(r.x, r.y, r.w, r.h, d, gutter)
{
}

/* -------------------------------------------------------------------------- */

geFlex::geFlex(Direction d, int gutter)
: geFlex(0, 0, 0, 0, d, gutter)
{
}

/* -------------------------------------------------------------------------- */

void geFlex::add(Fl_Widget& w, int size)
{
	Fl_Group::add(w);
	m_elems.push_back({w, *this, m_direction, size});
	if (size != -1)
		m_numFixed++;
}

void geFlex::add(Fl_Widget* w, int size)
{
	geFlex::add(*w, size);
}

/* -------------------------------------------------------------------------- */

void geFlex::resize(int X, int Y, int W, int H)
{
	Fl_Group::resize(X, Y, W, H);

	const size_t numAllElems    = m_elems.size();
	const size_t numLiquidElems = numAllElems - m_numFixed;

	const int pos  = m_direction == Direction::VERTICAL ? y() : x();
	const int size = m_direction == Direction::VERTICAL ? h() : w();

	/* No fancy computations if there are no liquid elements. Just lay children
	according to their fixed size. */

	if (numLiquidElems == 0)
	{
		layWidgets(pos);
		return;
	}

	const int fixedElemsSize = std::accumulate(m_elems.begin(), m_elems.end(), 0, [](int acc, const Elem& e) {
		return e.isFixed() ? acc + e.getSize() : acc;
	});
	const int availableSize  = size - (m_gutter * (numAllElems - 1)); // Total size - gutters
	const int liquidElemSize = (availableSize - fixedElemsSize) / numLiquidElems;

	layWidgets(pos, liquidElemSize);
}

/* -------------------------------------------------------------------------- */

void geFlex::layWidgets(int startPos, int sizeIfLiquid)
{
	int nextElemPos = startPos;
	for (Elem& e : m_elems)
	{
		e.resize(nextElemPos, e.isFixed() ? e.size : sizeIfLiquid);
		nextElemPos += e.getSize() + m_gutter;
	}
}

/* -------------------------------------------------------------------------- */

void geFlex::end()
{
	Fl_Group::end();
	resize(x(), y(), w(), h());
}
} // namespace giada::v