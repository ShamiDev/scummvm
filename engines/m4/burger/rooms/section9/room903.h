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

#ifndef M4_BURGER_ROOMS_SECTION9_ROOM903_H
#define M4_BURGER_ROOMS_SECTION9_ROOM903_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room903 : public Rooms::Room {
private:
	int _val1 = 0, _val2 = 0, _val3 = 0;
	int _val4 = 0, _val5 = 0;
	void *_ptr1 = nullptr;
	const char *_ptr2 = nullptr;
	const char *_pt3 = nullptr;

public:
	Room903() : Rooms::Room(903) {}

	void preload() override;
	void init() override;
	void pre_parser() override {}
	void daemon() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
