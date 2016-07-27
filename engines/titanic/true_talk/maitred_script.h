/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TITANIC_MAITRED_SCRIPT_H
#define TITANIC_MAITRED_SCRIPT_H

#include "titanic/true_talk/tt_npc_script.h"

namespace Titanic {

class MaitreDScript : public TTnpcScript {
private:
	/**
	 * Setup sentence data
	 */
	void setupSentences();
public:
	MaitreDScript(int val1, const char *charClass, int v2,
			const char *charName, int v3, int val2);

	/**
	 * Chooses and adds a conversation response based on a specified tag Id.
	 */
	virtual int chooseResponse(TTroomScript *roomScript, TTsentence *sentence, uint tag);

	/**
	 * Does NPC specific processing of the parsed sentence
	 */
	virtual int process(TTroomScript *roomScript, TTsentence *sentence);

	/**
	 * Called when the script/id changes
	 */
	virtual ScriptChangedResult scriptChanged(TTscriptBase *roomScript, uint id);

	virtual int handleQuote(TTroomScript *roomScript, TTsentence *sentence,
		uint val, uint tagId, uint remainder);

	virtual int proc21(int v1, int v2, int v3);
	virtual int proc22(int id) const;
	virtual int proc23() const;
	virtual int proc25(int val1, const int *srcIdP, TTroomScript *roomScript, TTsentence *sentence);
	virtual void proc26(int v1, const TTsentenceEntry *entry, TTroomScript *roomScript, TTsentence *sentence);
};

} // End of namespace Titanic

#endif /* TITANIC_MAITRED_SCRIPT_H */
