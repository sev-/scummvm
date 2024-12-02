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
 * along with this program.  If not, see <http://www.gnu.org/licenses/ },.
 *
 */

#include "m4/riddle/rooms/section7/room704.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room704::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room704::init() {
	digi_preload("950_S40", -1);
	digi_preload("950_S40A", -1);
	_ripTrekMedReachHandPos1Series = series_load("RIP TREK MED REACH HAND POS1", -1, nullptr);

	if (_G(flags[V224])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #5", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #6", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #7", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #8", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", false);
	}

	hotspot_set_active(_G(currentSceneDef).hotspots, _G(flags[V218]) ? "EMPTY NICHE" : "PRAYER WHEEL #6", false);

	switch (_G(flags[V218])) {
	case 1:
		_prayerWheelMach = series_place_sprite("704 PEACE PRAYER WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 2:
		_prayerWheelMach = series_place_sprite("704 INSIGHT PRAYER WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 3:
		_prayerWheelMach = series_place_sprite("704 SERENITY PRAYER WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 4:
		_prayerWheelMach = series_place_sprite("704 TRUTH PRAYER WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 5:
		_prayerWheelMach = series_place_sprite("704 WISDOM PRAYER WHEEL", 0, 0, 0, 100, 1280);
		break;

	default:
		break;
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		// Same as default?
		digi_preload("950_s39", -1);
		break;

	case 703:
		player_first_walk(0, 300, 3, 130, 318, 3, true);
		break;

	case 705:
		MoveScreenDelta(_G(game_buff_ptr), -640, 0);
		ws_demand_location(_G(my_walker), 1299, 300);
		ws_demand_facing(_G(my_walker), 9);
		ws_walk(_G(my_walker), 1170, 318, nullptr, -1, 0, true);

		break;

	case 707:
		MoveScreenDelta(_G(game_buff_ptr), -320, 0);
		ws_demand_location(_G(my_walker), 645, 356);
		ws_demand_facing(_G(my_walker), 5);
		_ripGoesDownStairsSeries = series_load("704 RIP GOES DOWN STAIRS", -1, nullptr);
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 320, 0, 100, 0, 0, callback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _ripGoesDownStairsSeries, 4, 39, 140, _ripGoesDownStairsSeries, 39, 39, 0);

		break;

	default:
		digi_preload("950_s39", -1);
		break;
	}

	digi_play_loop("950_s39", 3, 255, -1, -1);
}

void Room704::pre_parser() {
	// No implementation
}

void Room704::parser() {
	// TODO
}

void Room704::daemon() {
	const bool takeFl = player_said("take");

	switch (_G(kernel).trigger) {
	case 70:
		player_set_commands_allowed(false);
		setGlobals1(_ripTrekMedReachHandPos1Series, 1, 5, 5, 5, 0, 5, 10, 10, 10, 0, 10, 1, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 71);

		break;

	case 71:
		kernel_timing_trigger(5, 72, nullptr);
		break;

	case 72:
		sendWSMessage_120000(_G(my_walker), 73);
		break;

	case 73:
		kernel_timing_trigger(5, 74, nullptr);
		break;

	case 74:
		switch (_G(flags[V218])) {
		case 1:
			inv_give_to_player("PEACE WHEEL");
			_G(flags[V218]) = 0;
			terminateMachine(_prayerWheelMach);
			player_update_info(_G(my_walker), &_G(player_info));
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING PEACE WHEEL", _G(master_palette), 5, 1, _G(player_info).camera_x + 353, 225, 75, nullptr, -1);

			break;

		case 2:
			inv_give_to_player("INSIGHT WHEEL");
			_G(flags[V218]) = 0;
			terminateMachine(_prayerWheelMach);
			player_update_info(_G(my_walker), &_G(player_info));
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING INSIGHT WHEEL", _G(master_palette), 5, 1, _G(player_info).camera_x + 353, 225, 75, nullptr, -1);

			break;

		case 3:
			inv_give_to_player("SERENITY WHEEL");
			_G(flags[V218]) = 0;
			terminateMachine(_prayerWheelMach);
			player_update_info(_G(my_walker), &_G(player_info));
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING SERENITY WHEEL", _G(master_palette), 5, 1, _G(player_info).camera_x + 353, 225, 75, nullptr, -1);

			break;

		case 4:
			inv_give_to_player("TRUTH WHEEL");
			_G(flags[V218]) = 0;
			terminateMachine(_prayerWheelMach);
			player_update_info(_G(my_walker), &_G(player_info));
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING TRUTH WHEEL", _G(master_palette), 5, 1, _G(player_info).camera_x + 353, 225, 75, nullptr, -1);

			break;

		case 5:
			inv_give_to_player("WISDOM WHEEL");
			_G(flags[V218]) = 0;
			terminateMachine(_prayerWheelMach);
			player_update_info(_G(my_walker), &_G(player_info));
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING WISDOM WHEEL", _G(master_palette), 5, 1, _G(player_info).camera_x + 353, 225, 75, nullptr, -1);

			break;

		default:
			break;
		}

		break;

	case 75:
		sendWSMessage_130000(_G(my_walker), 76);
		break;

	case 76:
		sendWSMessage_150000(_G(my_walker), 77);
		break;

	case 77:
		player_set_commands_allowed(true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #6", false);

		break;

	case 80:
		player_set_commands_allowed(false);
		setGlobals1(_ripTrekMedReachHandPos1Series, 1, 5, 5, 5, 0, 5, 10, 10, 10, 0, 10, 1, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 81);

		break;

	case 81:
		kernel_timing_trigger(5, 82, nullptr);
		break;

	case 82:
		sendWSMessage_120000(_G(my_walker), 83);
		break;

	case 83:
		if (takeFl && _G(player).click_y <= 374) {
			switch (imath_ranged_rand(1, 3)) {
			case 1:
				digi_play("com077", 1, 255, 84, -1);
				break;

			case 2:
				digi_play("com078", 1, 255, 84, -1);
				break;

			case 3:
				digi_play("com079", 1, 255, 84, -1);
				break;

			default:
				break;
			}
		}
		break;

	case 84:
		sendWSMessage_130000(_G(my_walker), 85);
		break;

	case 85:
		sendWSMessage_150000(_G(my_walker), 86);
		break;

	case 86:
	case 91:
		player_set_commands_allowed(true);
		break;

	case 123:
		if (_field58_mode != 124)
			break;

		switch (_field5C_should) {
		case 121:
			_704Eye2aSeries = series_load("704EYE2a", -1, nullptr);
			_field5C_should = 922;
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 256, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _704Eye2aSeries, 1, 5, 123, _704Eye2aSeries, 5, 5, 0);

			break;

		case 122:
			terminateMachine(_monkMach);
			series_unload(_704Eye2aSeries);
			conv_resume(conv_get_handle());

			break;

		case 125:
			conv_load("conv704a", 10, 10, 91);
			conv_export_value_curr(0, 0);
			conv_export_value_curr(_G(flags[V211]), 1);
			conv_export_value_curr(_G(flags[V213]), 2);
			conv_export_value_curr((_G(flags[V218]) == 3) ? 1 : 0, 0);
			conv_play(conv_get_handle());

			break;

		case 922:
			switch (imath_ranged_rand(1, 2)) {
			case 1:
				_field5C_should = 923;
				break;

			case 2:
				_field5C_should = 924;
				break;

			default:
				break;
			}

			kernel_timing_trigger(10, 123, nullptr);

			break;

		case 923:
			_field5C_should = 925;
			sendWSMessage_10000(1, _monkMach, _704Eye2aSeries, 6, 7, -1, _704Eye2aSeries, 11, 13, 4);
			sendWSMessage_1a0000(_monkMach, 9);
			digi_play(conv_sound_to_play(), 1, 255, 123, -1);

			break;

		case 924:
			_field5C_should = 925;
			sendWSMessage_10000(1, _monkMach, _704Eye2aSeries, 6, 10, -1, _704Eye2aSeries, 11, 13, 4);
			sendWSMessage_1a0000(_monkMach, 9);
			digi_play(conv_sound_to_play(), 1, 255, 123, -1);

			break;

		case 925:
			_field5C_should = 926;
			sendWSMessage_10000(1, _monkMach, _704Eye2aSeries, 14, 17, 123, _704Eye2aSeries, 17, 17, 0);

			break;

		case 926:
			_field5C_should = 122;
			kernel_timing_trigger(10, 123, nullptr);

			break;

		default:
			break;
		}

		break;

	case 127:
		if (_field58_mode != 124)
			break;

		switch (_field5C_should) {
		case 121:
		case 122:
		case 124:
		case 125:
		case 126:
		case 127:
		case 922:
		case 923:
		case 924:
		case 925:
		case 926:
			kernel_timing_trigger(1, 123, nullptr);
			break;
		default:
			break;
		}

		break;

	case 140:
		player_set_commands_allowed(true);
		ws_unhide_walker(_G(my_walker));
		terminateMachine(_ripStairsMach);
		kernel_timing_trigger(10, 141, nullptr);

		break;

	case 141:
		series_unload(_ripGoesDownStairsSeries);
		break;

	default:
		break;

	}
}


void Room704::callback(frac16 myMessage, machine *sender) {
	kernel_trigger_dispatchx(myMessage);
}



} // namespace Rooms
} // namespace Riddle
} // namespace M4
