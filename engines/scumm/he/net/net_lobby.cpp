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

#include "common/config-manager.h"

#include "scumm/he/intern_he.h"
#include "scumm/he/net/net_lobby.h"

namespace Scumm {

Lobby::Lobby(ScummEngine_v90he *vm) : _vm(vm) {
	_gameName = _vm->_game.gameid;
	_socket = nullptr;

	_userId = 0;
}

Lobby::~Lobby() {
	if (_socket)
		delete _socket;
}

void Lobby::writeStringArray(int array, Common::String string) {
	int newArray = 0;
	byte *data = _vm->defineArray(array, ScummEngine_v90he::kStringArray, 0, 0, 0, strlen(string.c_str()), true, &newArray);
	memcpy(data, string.c_str(), strlen(string.c_str()));
	_vm->writeVar(array, newArray);
}

void Lobby::doNetworkOnceAFrame() {
	if (!_socket)
		return;

	int ready = _socket->ready();
	if (ready) {
		receiveData();
	}
}

void Lobby::send(Common::JSONObject data) {
	if (!_socket) {
		warning("LOBBY: Attempted to send data while not connected to server");
		return;
	}
	Common::JSONValue value(data);
	Common::String valueString = Common::JSON::stringify(&value);
	// Add new line.
	valueString += "\n";

	debug(1, "LOBBY: Sending data: %s", valueString.c_str());
	_socket->send(valueString.c_str(), strlen(valueString.c_str()));
}

void Lobby::receiveData() {
	if (!_socket)
		return;

	char data[1024];
	size_t len = _socket->recv(data, 1024);
	if (!len) {
		// Assume disconnection.
	}

	Common::String data_str(data, len);
	_buffer += data_str;

	while (_buffer.contains("\n")) {
		int pos = _buffer.findFirstOf('\n');
		processLine(_buffer.substr(0, pos));
		_buffer = _buffer.substr(pos + 1);
	}
}

void Lobby::processLine(Common::String line) {
	debug(1, "LOBBY: Received Data: %s", line.c_str());
	Common::JSONValue *json = Common::JSON::parse(line.c_str());
	if (!json) {
		warning("LOBBY: Received trunciated data from server! %s", line.c_str());
		return;
	}
	if (!json->isObject()){
		warning("LOBBY: Received non JSON object from server! %s", line.c_str());
		return;
	}

	Common::JSONObject root = json->asObject();
	if (root.find("cmd") != root.end() && root["cmd"]->isString()) {
		Common::String command = root["cmd"]->asString();
		if (command == "heartbeat") {
			handleHeartbeat();
		} else if (command == "login_resp") {
			int errorCode = root["error_code"]->asIntegerNumber();
			int userId = root["id"]->asIntegerNumber();
			Common::String response = root["response"]->asString();
			handleLoginResp(errorCode, userId, response);
		} else if (command == "profile_info") {
			Common::JSONArray profile = root["profile"]->asArray();
			handleProfileInfo(profile);
		}
	}
}

void Lobby::handleHeartbeat() {
	Common::JSONObject heartbeat;
	heartbeat.setVal("cmd", new Common::JSONValue("heartbeat"));
	send(heartbeat);
}

void Lobby::openUrl(const char *url) {
	debug(1, "LOBBY: openURL: %s", url);
	Common::String urlString = Common::String(url);

	if (urlString == "http://www.jrsn.com/c_corner/cc_regframe.asp") {
		if (_vm->displayMessageYesNo("Online Play for this game is provided by Backyard Sports Online, which is a\nservice provided by the ScummVM project.\nWould you like to go to their registration page?")) {
			if (!g_system->openUrl("https://backyardsports.online/register")) {
				_vm->displayMessage(0, "Failed to open registration URL.  Please navigate to this page manually.\n\n\"https://backyardsports.online/register\"");
			}
		}
	} else {
		warning("LOBBY: URL not handled: %s", url);
	}
}

bool Lobby::connect() {
	if (_socket)
		return true;

	_socket = new Networking::CurlSocket();

	// NOTE: Even though the protocol starts with http(s), this is an entirely
	// different protocol.  This is done so we can achieve communicating over
	// TLS/SSL sockets.
	// TODO: custom url
	Common::String url = "http://127.0.0.1:9130";

	debug(1, "LOBBY: Connecting to %s", url.c_str());

	if (_socket->connect(url)) {
		debug(1, "LOBBY: Successfully connected to %s", url.c_str());
		return true;
	} else {
		disconnect();
		writeStringArray(109, "Unable to contact server");
		_vm->writeVar(108, -99);
	}
	return false;
}

void Lobby::disconnect() {
	if (!_socket)
		return;

	debug(1, "LOBBY: Disconnecting connection to server.");
	delete _socket;
	_socket = nullptr;
}

void Lobby::login(const char *userName, const char *password) {
	Common::JSONObject loginRequestParameters;
	loginRequestParameters.setVal("cmd", new Common::JSONValue("login"));
	loginRequestParameters.setVal("user", new Common::JSONValue((Common::String)userName));
	loginRequestParameters.setVal("pass", new Common::JSONValue((Common::String)password));
	loginRequestParameters.setVal("game", new Common::JSONValue((Common::String)_gameName));

	send(loginRequestParameters);
}

void Lobby::handleLoginResp(int errorCode, int userId, Common::String response) {
	if (errorCode > 0) {
		writeStringArray(109, response);
		_vm->writeVar(108, -99);
		return;
	}
	_userId = userId;
	_vm->writeVar(108, 99);
}

void Lobby::getUserProfile(int userId) {
	if (!_socket)
		return;

	Common::JSONObject getProfileRequest;
	getProfileRequest.setVal("cmd", new Common::JSONValue("get_profile"));
	if (userId) {
		getProfileRequest.setVal("user_id", new Common::JSONValue((long long int)userId));
	}
	send(getProfileRequest);
}

void Lobby::handleProfileInfo(Common::JSONArray profile) {
	int newArray = 0;
	_vm->defineArray(108, ScummEngine_v90he::kDwordArray, 0, 0, 0, profile.size(), true, &newArray);
	_vm->writeVar(108, newArray);

	for (uint i = 0; i < profile.size(); i++) {
		if (profile[i]->isIntegerNumber()) {
			_vm->writeArray(108, 0, i, profile[i]->asIntegerNumber());
		} else {
			warning("BYOnline: Value for profile index %d is not an integer!", i);
		}
	}
	_vm->writeVar(111, 1);
}

} // End of namespace Scumm


