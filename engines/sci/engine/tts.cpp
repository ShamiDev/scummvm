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

#include "sci/sci.h"
#include "sci/engine/tts.h"
#include "common/system.h"
#include "common/text-to-speech.h"
#include "common/config-manager.h"

namespace Sci {

SciTTS::SciTTS() : _message("") {
	_ttsMan = ConfMan.getBool("tts_enabled") ? g_system->getTextToSpeechManager() : nullptr;
	if (_ttsMan != nullptr)
		_ttsMan->setLanguage(ConfMan.get("language"));
}

void SciTTS::button(const Common::String &text) {
	if (_ttsMan != nullptr)
		_ttsMan->say(getMessage(text), Common::TextToSpeechManager::QUEUE_NO_REPEAT);
}

void SciTTS::text(const Common::String &text) {
	if (_ttsMan != nullptr)
		_ttsMan->say(getMessage(text), Common::TextToSpeechManager::INTERRUPT);
}

void SciTTS::stop() {
	if (_ttsMan != nullptr)
		_ttsMan->stop();
}

void SciTTS::setMessage(const Common::String &text) {
	if (text.size() > 0)
		_message = text;
}

Common::String SciTTS::getMessage(const Common::String &text) {
	// If the current message contains a substring of the text to be displayed,
	// minus the first letter, prefer the message instead. The first letter is
	// chopped off in messages in games such as KQ6 and is replaced with tabs in
	// KQ6 or spaces in KQ5, so that a calligraphic first letter is drawn instead.
	if (_message.size() > 0 && text.size() > 0 && text.hasSuffix(_message.substr(1)))
		return _message;
	else
		return text;
}

} // End of namespace Sci
