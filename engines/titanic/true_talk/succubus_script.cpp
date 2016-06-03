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

#include "common/textconsole.h"
#include "titanic/true_talk/succubus_script.h"

namespace Titanic {

int SuccUBusScript::chooseResponse(TTroomScript *roomScript, TTsentence *sentence, uint tag) {
	uint dialogueId = tag;

	switch (tag) {
	case MKTAG('S', 'L', 'O', 'W'):
	case MKTAG('T', 'H', 'R', 'T'):
		dialogueId = 70021;

	case MKTAG('S', 'U', 'C', '1'):
		dialogueId = getDialogueId(230009);
		break;

	case MKTAG('S', 'U', 'C', '2'):
		dialogueId = 70117;
		break;

	case MKTAG('S', 'W', 'E', 'R'):
		dialogueId = getRandomNumber(100) > 40 ? 70103 : getDialogueId(230030);
		break;

	default:
		break;
	}

	if (dialogueId) {
		addResponse(dialogueId);
		applyResponse();
		return 2;
	} else {
		return 1;
	}
}

void SuccUBusScript::proc7(int v1, int v2) {
	warning("TODO");
}

int SuccUBusScript::proc10() const {
	warning("TODO");
	return 0;
}

bool SuccUBusScript::proc17() const {
	warning("TODO");
	return 0;
}

bool SuccUBusScript::proc18() const {
	warning("TODO");
	return 0;
}

int SuccUBusScript::proc21(int v1, int v2, int v3) {
	warning("TODO");
	return 0;
}

int SuccUBusScript::proc23() const {
	warning("TODO");
	return 0;
}

const int *SuccUBusScript::getTablePtr(int id) {
	warning("TODO");
	return nullptr;
}

int SuccUBusScript::proc25(int val1, int val2, TTroomScript *roomScript, TTsentence *sentence) const {
	warning("TODO");
	return 0;
}

void SuccUBusScript::proc26(int v1, const TTsentenceEntry *entry, TTroomScript *roomScript, TTsentence *sentence) {
}

} // End of namespace Titanic
