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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/datarecords.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void TableIndexSetValueHS::readData(Common::SeekableReadStream &stream) {
	_tableIndex = stream.readUint16LE();
	_valueChangeType = stream.readByte();
	_entryCorrectFlagID = stream.readSint16LE();
	_allEntriesCorrectFlagID = stream.readSint16LE();

	_flags.readData(stream);
	_cursorType = stream.readUint16LE();
	uint16 numHotspots = stream.readUint16LE();
	_hotspots.resize(numHotspots);
	for (uint i = 0; i < numHotspots; ++i) {
		_hotspots[i].readData(stream);
	}
}

void TableIndexSetValueHS::execute() {
	switch (_state) {
	case kBegin:
		_state = kRun;
		// fall through
	case kRun:
		_hasHotspot = false;
		for (uint i = 0; i < _hotspots.size(); ++i) {
			if (_hotspots[i].frameID == NancySceneState.getSceneInfo().frameID) {
				_hasHotspot = true;
				_hotspot = _hotspots[i].coords;
			}
		}
		break;
	case kActionTrigger: {
		TableData *playerTable = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
		assert(playerTable);
		auto *tabl = GetEngineData(TABL);
		assert(tabl);

		// Edit table. Values start from 1!
		switch (_valueChangeType) {
		case kNoChangeTableValue:
			break;
		case kIncrementTableValue:
			++playerTable->singleValues[_tableIndex - 1];
			if (playerTable->singleValues[_tableIndex - 1] >= (int)playerTable->singleValues.size() + 1) {
				playerTable->singleValues[_tableIndex - 1] = 1;
			}
			break;
		case kDecrementTableValue:
			--playerTable->singleValues[_tableIndex - 1];
			if (playerTable->singleValues[_tableIndex - 1] == 0) {
				playerTable->singleValues[_tableIndex - 1] = playerTable->singleValues.size();
			}
			
			break;
		}

		// Check for correctness...

		// ...of current index only...
		if (playerTable->singleValues[_tableIndex] == tabl->correctIDs[_tableIndex]) {
			NancySceneState.setEventFlag(_entryCorrectFlagID, g_nancy->_true);
		} else {
			NancySceneState.setEventFlag(_entryCorrectFlagID, g_nancy->_false);
		}

		// ..and of all indices
		bool allCorrect = true;
		for (uint i = 0; i < tabl->correctIDs.size(); ++i) {
			if (playerTable->singleValues[i] != tabl->correctIDs[i]) {
				allCorrect = false;
				break;
			}
		}

		if (allCorrect) {
			NancySceneState.setEventFlag(_allEntriesCorrectFlagID, g_nancy->_true);
		} else {
			NancySceneState.setEventFlag(_allEntriesCorrectFlagID, g_nancy->_false);
		}

		_flags.execute();
		finishExecution();
	}
	}
}

void SetValue::readData(Common::SeekableReadStream &stream) {
	_index = stream.readByte();
	_shouldSet = stream.readByte();
	_value = stream.readSint16LE();
}

void SetValue::execute() {
	TableData *playerTable = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
	assert(playerTable);

	// nancy8 has 20 single & 20 combo values, later games have 30/10
	uint numSingleValues = g_nancy->getGameType() <= kGameTypeNancy8 ? 20 : 30;

	if (_index < numSingleValues) {
		// Single values
		int16 curValue = playerTable->getSingleValue(_index);
		if (_shouldSet || curValue == kNoTableValue) {
			playerTable->setSingleValue(_index, _value);
		} else {
			playerTable->setSingleValue(_index, curValue + _value);
		}
	} else {
		// Combo values
		float curValue = playerTable->getComboValue(_index - numSingleValues);
		if (_shouldSet || curValue == (float)kNoTableValue) {
			playerTable->setComboValue(_index - numSingleValues, _value);
		} else {
			playerTable->setComboValue(_index - numSingleValues, curValue + _value);
		}
	}

	finishExecution();
}

void EventFlags::readData(Common::SeekableReadStream &stream) {
	if (!_isTerse) {
		_flags.readData(stream);
	} else {
		// Terse version only has 2 flags
		_flags.descs[0].label = stream.readSint16LE();
		_flags.descs[0].flag = stream.readUint16LE();
		_flags.descs[1].label = stream.readSint16LE();
		_flags.descs[1].flag = stream.readUint16LE();
	}
}

void EventFlags::execute() {
	_flags.execute();
	_isDone = true;
}

void EventFlagsMultiHS::readData(Common::SeekableReadStream &stream) {
	EventFlags::readData(stream);

	if (_isCursor) {
		_hoverCursor = (CursorManager::CursorType)stream.readUint16LE();
	}

	uint16 numHotspots = stream.readUint16LE();

	_hotspots.reserve(numHotspots);
	for (uint16 i = 0; i < numHotspots; ++i) {
		_hotspots.push_back(HotspotDescription());
		HotspotDescription &newDesc = _hotspots[i];
		newDesc.readData(stream);
	}
}

void EventFlagsMultiHS::execute() {
	switch (_state) {
	case kBegin:
		// turn main rendering on
		_state = kRun;
		// fall through
	case kRun:
		_hasHotspot = false;

		for (uint i = 0; i < _hotspots.size(); ++i) {
			if (_hotspots[i].frameID == NancySceneState.getSceneInfo().frameID) {
				_hasHotspot = true;
				_hotspot = _hotspots[i].coords;
			}
		}

		break;
	case kActionTrigger:
		if (_hoverCursor != CursorManager::kCustom1 && _hoverCursor != CursorManager::kCustom2) {
			_hasHotspot = false;
			EventFlags::execute();
			finishExecution();
			break;
		} else {
			_state = kRun;
		}
	}
}

void DifficultyLevel::readData(Common::SeekableReadStream &stream) {
	_difficulty = stream.readUint16LE();
	_flag.label = stream.readSint16LE();
	_flag.flag = stream.readUint16LE();
}

void DifficultyLevel::execute() {
	NancySceneState.setDifficulty(_difficulty);
	NancySceneState.setEventFlag(_flag);
	_isDone = true;
}

} // End of namespace Action
} // End of namespace Nancy
