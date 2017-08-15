/*
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#pragma once

#include <glib.h>

#include <vector>
#include <tr1/memory>

#include "movement.hpp"

namespace ScanAmati {

namespace Scanner {

enum CommandType {
	COMMAND_HANDSHAKE,
	COMMAND_ID,
	COMMAND_TEMPERATURE,
	COMMAND_PELTIER_ON,
	COMMAND_PELTIER_OFF,
	COMMAND_SELECT_CAPACITY,
	COMMAND_SELECT_CHIP,
	COMMAND_ALTERA_COUNTS_START,
	COMMAND_ALTERA_COUNTS_STOP,
	COMMAND_ALTERA_RESET,
	COMMAND_ALTERA_START,
	COMMAND_ALTERA_START_IMAGE,
	COMMAND_ALTERA_START_PEDESTALS,
	COMMAND_ARRAY_RESET,
	COMMAND_CAPACITY,
	COMMAND_READ_MEMORY_LINING,
	COMMAND_READ_MEMORY_ONE_BANK,
	COMMAND_READ_MEMORY_TWO_BANKS,
	COMMAND_READ_MEMORY_THREE_BANKS,
	COMMAND_READ_MEMORY_FULL,
	COMMAND_READ_MEMORY,
	COMMAND_STEPPERS_STOP,
	COMMAND_XRAY_CHECK_ON,
	COMMAND_XRAY_CHECK_OFF
};

class Command {
public:
	virtual ~Command();
	virtual void fill_buffer( guint8* buf, size_t& size) = 0;
};

class Commands : public Command {
public:
	virtual ~Commands();
	virtual void fill_buffer( guint8* buf, size_t& size);

	static Command* create(CommandType com = COMMAND_ARRAY_RESET);
	static Command* create( CommandType, guint8);
	static Command* create( CommandType, guint8, guint8, guint8);
	static Command* create(size_t);
	static Command* create(double);
	static Command* create( char, const std::vector<guint8>&);
	static Command* create( char, guint8);
	static Command* create( const Movement& movement,
		MovementType movement_type = MOVEMENT_BOTH,
		DirectionType direction_type = DIRECTION_FORWARD);
protected:
	size_t data_size_;
};

typedef std::tr1::shared_ptr<Command> CommandSharedPtr;

} // namespace Scanner

} // namespace ScanAmati
