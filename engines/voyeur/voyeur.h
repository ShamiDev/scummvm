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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef VOYEUR_VOYEUR_H
#define VOYEUR_VOYEUR_H

#include "voyeur/debugger.h"
#include "voyeur/events.h"
#include "voyeur/files.h"
#include "voyeur/game.h"
#include "voyeur/graphics.h"
#include "voyeur/sound.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/random.h"
#include "common/util.h"
#include "engines/engine.h"
#include "graphics/surface.h"

/**
 * This is the namespace of the Voyeur engine.
 *
 * Status of this engine: In Development
 *
 * Games using this engine:
 * - Voyeur
 */
namespace Voyeur {

#define DEBUG_BASIC 1
#define DEBUG_INTERMEDIATE 2
#define DEBUG_DETAILED 3

#define MAX_RESOLVE 1000

enum VoyeurDebugChannels {
	kDebugPath      = 1 <<  0
};

struct VoyeurGameDescription;


class VoyeurEngine : public Engine {
private:
	const VoyeurGameDescription *_gameDescription;
	Common::RandomSource _randomSource;

	BoltFile *_bVoy;
	byte *_fontPtr;
	SVoy _voy;
	Common::Array<int> _resolves;

	void ESP_Init();
	void initialiseManagers();
	void globalInitBolt();
	void initBolt();
	void vInitInterrupts();
	void initInput();

	void doHeadTitle();
	void showConversionScreen();
	bool doLock();
protected:
	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
public:
	Debugger _debugger;
	EventsManager _eventsManager;
	FilesManager _filesManager;
	GraphicsManager _graphicsManager;
	SoundManager _soundManager;
public:
	VoyeurEngine(OSystem *syst, const VoyeurGameDescription *gameDesc);
	virtual ~VoyeurEngine();
	void GUIError(const Common::String &msg);

	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	uint16 getVersion() const;
	bool getIsDemo() const;

	int getRandomNumber(int maxNumber);
	Common::String generateSaveName(int slotNumber);
	virtual bool canLoadGameStateCurrently();
	virtual bool canSaveGameStateCurrently();
	virtual Common::Error loadGameState(int slot);
	virtual Common::Error saveGameState(int slot, const Common::String &desc);
};

} // End of namespace Voyeur

#endif /* VOYEUR_VOYEUR_H */
