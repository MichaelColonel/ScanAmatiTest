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

#include <algorithm>
#include <cmath>

#include "defines.hpp"
#include "commands.hpp"
#include "builtin_chip_capacities.hpp"

#define COMMAND_BUFFER_HEADER 2

namespace {

class MemorySizeCodes {
public:
	MemorySizeCodes(size_t size) : size_(size) { find_codes(); }
	const guint8* codes() const { return code_; }
	bool check() const;
private:
	void find_codes();
	size_t size_;
	guint8 code_[3];
};

inline
bool
MemorySizeCodes::check() const
{
	size_t size = code_[0] * code_[1] * code_[2] * 256;
	return (size == size_);
}

void
MemorySizeCodes::find_codes()
{
	guint m = size_ >> 8;
	guint8& a = code_[0];
	guint8& b = code_[1];
	guint8& c = code_[2];

	a = 0;
	b = c = 1;
	while (a == 0 && b <= UCHAR_MAX && c <= UCHAR_MAX) {
		double v = double(m) / double(b * c); // memory_size / (b * c * 256)
		double f;
		double m = modf( v, &f);
		if (f <= UCHAR_MAX && ceil(m) == 0.) {
			a = (guint8)f;
			break;
		}
		else {
			b += 1;
			c += 1;
		}
	}
}

} // namespace

namespace ScanAmati {

namespace Scanner {

Command::~Command()
{
}

Commands::~Commands()
{
}

void
Commands::fill_buffer( guint8*, size_t& size)
{
	size = data_size_;
	size += COMMAND_BUFFER_HEADER;
}

class BaseCommand : public Commands {
public:
	BaseCommand( char com, size_t size = 0) : com_(com) { data_size_ = size; }

	virtual ~BaseCommand() {}
	virtual void fill_buffer( guint8* buf, size_t& size);

protected:
	char com_;
};

void
BaseCommand::fill_buffer( guint8* buf, size_t& size)
{
	Commands::fill_buffer( buf, size);

	buf[0] = com_;
	buf[1] = static_cast<guint8>(data_size_);
}

class ValueCommand : public BaseCommand {
public:
	ValueCommand( char com, guint8 value, size_t data_size = 1)
		: BaseCommand( com, data_size), value1_(value) {}
	virtual ~ValueCommand() {}
	virtual void fill_buffer( guint8* buf, size_t& size);

protected:
	guint8 value1_;
};

void
ValueCommand::fill_buffer( guint8* buf, size_t& size)
{
	BaseCommand::fill_buffer( buf, size);
	buf[2] = value1_;
}

class ThreeValueCommand : public ValueCommand {
public:
	ThreeValueCommand( char com, guint8 v1, guint8 v2, guint8 v3)
		: ValueCommand( com, v1, 3), value2_(v2), value3_(v3) {}
	virtual ~ThreeValueCommand() {}
	virtual void fill_buffer( guint8* buf, size_t& size);

protected:
	guint8 value2_;
	guint8 value3_;
};

void
ThreeValueCommand::fill_buffer( guint8* buf, size_t& size)
{
	ValueCommand::fill_buffer( buf, size);
	buf[3] = value2_;
	buf[4] = value3_;
}

class LiningCommand : public BaseCommand {
public:
	LiningCommand( char chip, const std::vector<guint8>& lining)
		:
		BaseCommand( 'D', SCANNER_STRIPS_PER_CHIP_REAL + 1),
		chip_(chip),
		lining_(lining)
		{}

	LiningCommand( char chip, guint8 value)
		:
		BaseCommand( 'D', SCANNER_STRIPS_PER_CHIP_REAL + 1),
		chip_(chip),
		lining_(SCANNER_STRIPS_PER_CHIP_REAL)
		{ std::fill( lining_.begin(), lining_.end(), value); }

	virtual ~LiningCommand() {}
	virtual void fill_buffer( guint8* buf, size_t& size);

protected:
	char chip_;
	std::vector<guint8> lining_;
};

void
LiningCommand::fill_buffer( guint8* buf, size_t& size)
{
	BaseCommand::fill_buffer( buf, size);

	buf[2] = chip_;

	for ( size_t i = 0; i < SCANNER_STRIPS_PER_CHIP_REAL; ++i)
		buf[COMMAND_BUFFER_HEADER + 1 + i] = lining_[i];
}

class MovementCommand : public BaseCommand {
public:
	MovementCommand( const Movement& movement,
		MovementType movement_type,
		DirectionType direction_type)
			:
			BaseCommand( 'M', 14),
			movement_(movement),
			movement_type_(movement_type),
			direction_type_(direction_type)
			{}
	virtual ~MovementCommand() {}
	virtual void fill_buffer( guint8* buf, size_t& size);

protected:
	const Movement& movement_;
	MovementType movement_type_;
	DirectionType direction_type_;
};

void
MovementCommand::fill_buffer( guint8* buf, size_t& size)
{
	BaseCommand::fill_buffer( buf, size);

	switch (movement_type_) {
	case MOVEMENT_ARRAY:
		buf[2] = '1'; // array
		buf[3] = '0'; // xray
		break;
	case MOVEMENT_XRAY:
		buf[2] = '0'; // array
		buf[3] = '1'; // xray
		break;
	case MOVEMENT_BOTH:
		buf[2] = '1'; // array
		buf[3] = '1'; // xray
		break;
	case MOVEMENT_NONE:
	default:
		buf[2] = '0'; // array
		buf[3] = '0'; // xray
		break;
	}

	buf[4] = (movement_.steps >> 8) & 0xFF; // first byte
	buf[5] = movement_.steps & 0xFF; // second byte
	buf[6] = 0;
	buf[7] = (movement_.array_speed.freq >> 8) & 0xFF; // first byte (the only byte)
	buf[8] = movement_.array_speed.mode + '0';

	switch (direction_type_) {
	case DIRECTION_FORWARD:
		buf[9] = '1'; // array
		buf[12] = '1'; // xray
		break;
	case DIRECTION_REVERSE:
	default:
		buf[9] = '0'; // array
		buf[12] = '0'; // xray
		break;
	}

	buf[10] = movement_.xray_speed.freq;
	buf[11] = movement_.xray_speed.mode + '0';

	buf[13] = movement_.array_speed.freq & 0xFF;
	buf[14] = movement_.xray_delay;
	buf[15] = movement_.xray_array_delay;
}

Command*
Commands::create(CommandType com)
{
	Command* command = 0;

	switch (com) {
	case COMMAND_HANDSHAKE:
		command = new ValueCommand( 'I', '0');
		break;
	case COMMAND_ID:
		command = new ValueCommand( 'I', '1');
		break;
	case COMMAND_TEMPERATURE:
		command = new BaseCommand('T');
		break;
	case COMMAND_ALTERA_RESET:
		command = new BaseCommand('r');
		break;
	case COMMAND_PELTIER_OFF:
		command = new BaseCommand('p');
		break;
	case COMMAND_ALTERA_START:
	case COMMAND_ALTERA_START_IMAGE:
		command = new ValueCommand( 's', '0');
		break;
	case COMMAND_ALTERA_START_PEDESTALS:
		command = new ValueCommand( 's', '1');
		break;
	case COMMAND_ALTERA_COUNTS_START:
		command = new ValueCommand( 'G', '0');
		break;
	case COMMAND_ALTERA_COUNTS_STOP:
		command = new BaseCommand('g');
		break;
	case COMMAND_ARRAY_RESET:
		command = new BaseCommand('R');
		break;
	case COMMAND_STEPPERS_STOP:
		command = new BaseCommand('N');
		break;
	case COMMAND_READ_MEMORY_LINING:
		command = new ThreeValueCommand( 'W', 2, 16, 16);
		break;
	case COMMAND_READ_MEMORY_ONE_BANK:
		command = new ThreeValueCommand( 'W', 2, 128, 128);
		break;
	case COMMAND_READ_MEMORY_TWO_BANKS:
		command = new ThreeValueCommand( 'W', 4, 128, 128);
		break;
	case COMMAND_READ_MEMORY_THREE_BANKS:
		command = new ThreeValueCommand( 'W', 6, 128, 128);
		break;
	case COMMAND_READ_MEMORY_FULL:
		command = new ThreeValueCommand( 'W', 8, 128, 128);
		break;
	case COMMAND_XRAY_CHECK_ON:
		command = new ValueCommand( 'X', '1');
		break;
	case COMMAND_XRAY_CHECK_OFF:
		command = new ValueCommand( 'X', '0');
		break;
	default:
		command = new BaseCommand('R');
		break;
	}

	return command;
}

Command*
Commands::create( CommandType com, guint8 value)
{
	Command* command = 0;

	switch (com) {
	case COMMAND_CAPACITY:
		command = new ValueCommand( 'C', value);
		break;
	case COMMAND_PELTIER_ON:
		command = new ValueCommand( 'P', value);
		break;
	case COMMAND_SELECT_CHIP:
		command = new ValueCommand( 'G', value);
		break;
	case COMMAND_ALTERA_START:
		command = new ValueCommand( 's', value);
		break;
	default:
		command = new BaseCommand('R');
		break;
	}

	return command;
}

Command*
Commands::create( CommandType com, guint8 v1, guint8 v2, guint8 v3)
{
	Command* command = 0;

	switch (com) {
	case COMMAND_SELECT_CAPACITY:
		command = new ThreeValueCommand( 'C', v1, v2, v3);
		break;
	case COMMAND_READ_MEMORY:
		command = new ThreeValueCommand( 'W', v1, v2, v3);
		break;
	default:
		command = new BaseCommand('R');
		break;
	}

	return command;
}

Command*
Commands::create(size_t memory_size)
{
	MemorySizeCodes mem(memory_size);
	const guint8* codes = mem.codes();
	return (mem.check()) ? new ThreeValueCommand( 'W', codes[0], codes[1], codes[2]) :
		new BaseCommand('R');
}

Command*
Commands::create(double capacity)
{
	char value = BuiltinCapacities::capacity_char_code(capacity);

	return new ValueCommand( 'C', value);
}

Command*
Commands::create( char chip, const std::vector<guint8>& lining)
{	
	return new LiningCommand( chip, lining);
}

Command*
Commands::create( char chip, guint8 value)
{	
	return new LiningCommand( chip, value);
}

Command*
Commands::create( const Movement& mov, MovementType movement,
	DirectionType direction)
{
	return new MovementCommand( mov, movement, direction);
}

} // namespace Scanner

} // namespace ScanAmati
