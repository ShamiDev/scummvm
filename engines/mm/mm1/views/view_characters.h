/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MM1_VIEWS_VIEW_CHARACTERS_H
#define MM1_VIEWS_VIEW_CHARACTERS_H

#include "common/array.h"
#include "mm/mm1/views/text_view.h"

namespace MM {
namespace MM1 {
namespace Views {

/**
 * Dialog for View All Characters
 */
class ViewCharacters : public TextView {
private:
	Common::Array<uint> _charIndexes;
public:
	ViewCharacters() : TextView("ViewCharacters") {}
	virtual ~ViewCharacters() {}

	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

/**
 * Base class for showing character information
 */
class CharacterStats : public TextView {
protected:
	void printStats();
	void printSummary();
	void printCondition();
	void printInventory();

public:
	CharacterStats(const Common::String &name) : TextView(name) {}
	~CharacterStats() {}
};

/**
 * Dialog for showing the stats for a single character,
 * from the View All Characters dialog
 */
class ViewCharacter : public CharacterStats {
	enum ViewState { DISPLAY = 0, RENAME = 1, DELETE = 2 };
	ViewState _state = DISPLAY;
	Common::String _newName;
public:
	ViewCharacter() : CharacterStats("ViewCharacter") {}
	virtual ~ViewCharacter() {}

	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
};


} // namespace Views
} // namespace MM1
} // namespace MM

#endif
