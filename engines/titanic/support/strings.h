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

#ifndef TITANIC_STRINGS_H
#define TITANIC_STRINGS_H

#include "common/str-array.h"

namespace Titanic {

enum StringId {
	BLANK,
	STANDING_OUTSIDE_PELLERATOR,
	BOT_BLOCKING_PELLERATOR,
	SUCCUBUS_IS_IN_STANDBY,
	NOTHING_TO_DELIVER,
	NOTHING_IN_SUCCUBUS_TRAY,
	SUCCUBUS_SINGLE_DELIVERY,
	ONE_ALLOCATED_CHICKEN_PER_CUSTOMER,
	ONE_CHICKEN_PER_CUSTOMER,
	UPGRADED_TO_FIRST_CLASS,
	UPGRADED_TO_SECOND_CLASS,
	ROOM_RESERVED_FOR_FIRST_CLASS,
	NO_LOSERS,
	CLASS_NOT_PERMITTED_IN_AREA,
	EXIT_FROM_OTHER_SIDE,
	TRANSPORT_OUT_OF_ORDER,
	FAN_HAS_BLOWN_A_FUSE,
	POKE_WITH_LONG_STICK,
	TOO_SHORT_TO_REACH_BRANCHES,
	OUTSIDE_ELEVATOR_NUM,
	BOT_BLOCKING_ELEVATOR,
	ELEVATOR_NON_FUNCTIONAL,
	LIGHT_IS_LOOSE,
	LUMI_GLOW_LIGHTS,
	ALREADY_HAVE_STICK,
	GLASS_IS_UNBREAKABLE,
	FOR_STICK_BREAK_GLASS,
	DISPENSOR_HAS_UNBREAKABLE_GLASS,
	CHICKEN_IS_CLEAN,
	ADJUST_VIEWING_APPARATUS,
	CANNOT_TAKE_CAGE_LOCKED,
	ALREADY_AT_DESTINATION,
	CLASS_NOT_ALLOWED_AT_DEST,
	AT_LEAST_3RD_CLASS_FOR_HELP,
	NO_ROOM_ASSIGNED,
	ELEVATOR_NOT_BELOW_27,
	SELECT_GAME_TO_LOAD,
	SELECT_GAME_TO_SAVE,
	SUPPLY_GALACTIC_REFERENCE,
	LOCKED_MUSIC_SYSTEM,
	STUCK_TO_BRANCH,
	FROZEN_TO_BRANCH,
	CHECK_IN_AT_RECEPTION,
	FOODSTUFF_ALREADY_GARNISHED,
	DISPENSOR_IS_EMPTY,
	PUT_FOOD_UNDER_DISPENSOR,
	SEASONAL_SWITCH_NOT_WORKING,
	YOUR_STATEROOM,
	BED_NOT_SUPPORT_YOUR_WEIGHT,
	NOT_YOUR_ASSIGNED_ROOM,
	OUT_OF_REACH,
	SUCCUBUS_DESCRIPTION,
	CANAL_CLOSED_FOR_WINTER,
	AREA_OFF_LIMIT_TO_PASSENGERS,
	GO_WHERE,
	NICE_IF_TAKE_BUT_CANT,
	BOWL_OF_NUTS,
	NOT_A_BOWL_OF_NUTS,

	CANT_SUMMON_DOORBOT,
	CANT_SUMMON_BELLBOT,
	NO_ONE_TO_TALK_TO,
	TALKING_TO,
	DOORBOT_NAME,
	DESKBOT_NAME,
	LIFTBOT_NAME,
	PARROT_NAME,
	BARBOT_NAME,
	CHATTERBOT_NAME,
	BELLBOT_NAME,
	MAITRED_NAME,
	SUCCUBUS_NAME,
	UNKNOWN_NAME,
	ARM_ALREADY_HOLDING,
	YOU_CANT_GET_THIS,
	DOESNT_DO_ANYTHING,
	DOESNT_WANT_THIS,
	DOES_NOT_REACH,
	CHICKEN_ALREADY_CLEAN,
	HOSE_INCOMPATIBLE,
	INCORRECTLY_CALIBRATED,
	GONDOLIERS_FIRST_CLASS_ONLY,
	NOTHING_ON_CHANNEL,
	TELEVISION_CONTROL,
	OPERATE_ENTERTAINMENT,
	OPERATE_LIGHTS,
	DEPLOY_FLORAL_ENHANCEMENT,
	DEPLOY_FULLY_RELAXATION,
	DEPLOY_COMFORT_WORKSTATION,
	DEPLOY_MINOR_STORAGE,
	DEPLOY_MAJOR_RELAXATION,
	INFLATE_RELAXATION_DEVICE,
	DEPLOY_MAINTENANCE_HUB,
	DEPLOY_EXECUTIVE_SURFACE,
	DEPLOY_MINOR_RELAXATION,
	DEPLOY_SINK,
	DEPLOY_MAJOR_STORAGE,
	SUCCUBUS_DELIVERY_SYSTEM,
	NAVIGATION_CONTROLLER,
	SUMMON_ELEVATOR,
	SUMMON_PELLERATOR,
	GO_TO_BOTTOM_OF_WELL,
	GO_TO_TOP_OF_WELL,
	GO_TO_STATEROOM,
	GO_TO_BAR,
	GO_TO_PROMENADE_DECK,
	GO_TO_ARBORETUM,
	GO_TO_MUSIC_ROOM,
	GO_TO_1ST_CLASS_RESTAURANT,
	THE_PARROT_LOBBY,
	THE_CREATORS_CHAMBER,
	THE_BRIDGE,
	THE_BILGE_ROOM,
	THE_SCULPTURE_CHAMBER,
	THE_ARBORETUM,
	THE_BOTTOM_OF_THE_WELL,
	THE_PROMENADE_DECK,
	RESTAURANT_1ST_CLASS,
	TITANIAS_ROOM,
	THE_BAR,
	THE_EMBARKATION_LOBBY,
	THE_MUSIC_ROOM,
	UNKNOWN_ROOM,
	THE_SERVICE_ELEVATOR,
	SGT_LEISURE_LOUNGE,
	THE_ELEVATOR,
	THE_DOME,
	THE_PELLERATOR,
	THE_TOP_OF_THE_WELL,
	NOWHERE_TO_GO,
	CLASS_1,
	CLASS_2,
	CLASS_3,
	CLASS_NONE,
	YOUR_ASSIGNED_ROOM,
	PREVIOUSLY_ASSIGNED_ROOM,
	SAVED_CHEVRON,
	CURRENT_LOCATION,

	// German version only
	DE_SUMMER,
	DE_AUTUMN,
	DE_WINTER,
	DE_SPRING,
	DE_SUMMER_ARBORETUM,
	DE_AUTUMN_ARBORETUM,
	DE_WINTER_ARBORETUM,
	DE_SPRING_ARBORETUM,
	DE_ARBORETUM_MSG1,
	DE_ARBORETUM_MSG2,
	DE_BRIDGE_MSG1,
	DE_BRIDGE_MSG2,
	DE_BRIDGE_MSG3,
	DE_BRIDGE_MSG4,
	DE_BRIDGE_MSG5,
	DE_BRIDGE_MSG6,
	DE_BRIDGE_MSG7,
	DE_BRIDGE_MSG8,
	DE_BRIDGE_MSG9,
	DE_BRIDGE_MSG10,
	DE_PROMENADE_DECK_MSG1,
	DE_PROMENADE_DECK_MSG2,
	DE_PROMENADE_DECK_MSG3,
	DE_PROMENADE_DECK_MSG4,
	DE_PROMENADE_DECK_MSG5,
	DE_PROMENADE_DECK_MSG6,
	DE_PROMENADE_DECK_MSG7,
	DE_PROMENADE_DECK_MSG8,
	DE_PROMENADE_DECK_MSG9,
	DE_PROMENADE_DECK_MSG10,
	DE_SGTLOBBY_MSG1,
	DE_SGTLOBBY_MSG2,
	DE_SGTLOBBY_MSG3,
	DE_SGTLOBBY_MSG4,
	DE_SGTLOBBY_MSG5,
	DE_SGTLOBBY_MSG6,
	DE_SGTLOBBY_MSG7,
	DE_SGTLOBBY_MSG8,
	DE_TITANIA_MSG1,
	DE_TITANIA_MSG2,
	DE_TITANIA_MSG3,
	DE_TITANIA_MSG4,
	DE_TITANIA_MSG5,
	DE_TITANIA_MSG6,
	DE_TITANIA_MSG7,
	DE_TITANIA_MSG8

};

class Strings : public Common::StringArray {
public:
	void load();
};

} // End of namespace Titanic

#endif /* TITANIC_STRINGS_H */
