/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#ifndef GE_BASE_ACTION_EDITOR_H
#define GE_BASE_ACTION_EDITOR_H


#include <FL/Fl_Group.H>


class Channel;


namespace giada {
namespace v
{
class gdBaseActionEditor;
class geBaseAction;

class geBaseActionEditor : public Fl_Group
{
private:
	
	/* drawVerticals
	Draws generic vertical lines (beats, bars, grid lines...). */
	
	void drawVerticals(int steps) const;
	
	int push();
	int drag();
	int release();

protected:

	Channel* m_ch;

	gdBaseActionEditor* m_base;

	/* m_action
	Selected action. Used while dragging. */

	geBaseAction* m_action;

	/* baseDraw
	Draws basic things like borders and grids. Optional background clear. */

  void baseDraw(bool clear=true) const;

	virtual void onAddAction()     = 0;
	virtual void onDeleteAction()  = 0;
	virtual void onMoveAction()    = 0;
	virtual void onResizeAction()  = 0;
	virtual void onRefreshAction() = 0;

public:

	geBaseActionEditor(Pixel x, Pixel y, Pixel w, Pixel h, Channel* ch);

  /* updateActions
  Rebuild the actions widgets from scratch. */
  
	virtual void rebuild() = 0;

	/* handle
	Override base FL_Group events. */
	
	int handle(int e) override;

	/* getActionAtCursor
	Returns the action under the mouse. nullptr if nothing found. Why not using
	Fl::belowmouse? It would require a boring dynamic_cast. */

	geBaseAction* getActionAtCursor() const;	
};
}} // giada::v::


#endif
