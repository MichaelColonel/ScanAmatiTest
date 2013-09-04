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
Commands::fill_buffer( guint8* , size_t& size)
{
	size = buffer_size_;
}

class BaseCommand : public Commands {
public:
	BaseCommand(char com) : com_(com) { buffer_size_ = SCANNER_BUFFER; }
	virtual ~BaseCommand() {}
	virtual void fill_buffer( guint8* buf, size_t& size);

protected:
	char com_;
};

void
BaseCommand::fill_buffer( guint8* buf, size_t& size)
{
	Commands::fill_buffer( buf, size);
	std::fill( buf, buf + size, '0');
	buf[0] = com_;
}

class ValueCommand : public BaseCommand {
public:
	ValueCommand( char com, guint8 value)
		: BaseCommand(com), value_(value) {}
	virtual ~ValueCommand() {}
	virtual void fill_buffer( guint8* buf, size_t& size);

protected:
	guint8 value_;
};

void
ValueCommand::fill_buffer( guint8* buf, size_t& size)
{
	BaseCommand::fill_buffer( buf, size);
	buf[1] = value_;
}

class PeltierCommand : public ValueCommand {
public:
	PeltierCommand(guint8 value) : ValueCommand( 'P', value) {}
	virtual ~PeltierCommand() {}
	virtual void fill_buffer( guint8* buf, size_t& size);
};

void
PeltierCommand::fill_buffer( guint8* buf, size_t& size)
{
	BaseCommand::fill_buffer( buf, size);
	buf[1] = '0';
	buf[2] = '0';
	buf[8] = value_;
}

class ThreeValueCommand : public BaseCommand {
public:
	ThreeValueCommand( char com, guint8 v1, guint8 v2, guint8 v3)
		: BaseCommand(com), value1_(v1), value2_(v2), value3_(v3) {}
	virtual ~ThreeValueCommand() {}
	virtual void fill_buffer( guint8* buf, size_t& size);

protected:
	guint8 value1_;
	guint8 value2_;
	guint8 value3_;
};

void
ThreeValueCommand::fill_buffer( guint8* buf, size_t& size)
{
	BaseCommand::fill_buffer( buf, size);
	buf[1] = value1_;
	buf[2] = value2_;
	buf[3] = value3_;
}

class CapacityCommand : public ThreeValueCommand {
public:
	CapacityCommand(guint8 v1, guint8 v2, guint8 v3)
		: ThreeValueCommand( 'C', v1, v2, v3) {}
	virtual ~CapacityCommand() {}
	virtual void fill_buffer( guint8* buf, size_t& size);
};

void
CapacityCommand::fill_buffer( guint8* buf, size_t& size)
{
	ThreeValueCommand::fill_buffer( buf, size);
}

class ReadMemoryCommand : public ThreeValueCommand {
public:
	ReadMemoryCommand(guint8 v1, guint8 v2, guint8 v3)
		: ThreeValueCommand( 'W', v1, v2, v3) {}
	virtual ~ReadMemoryCommand() {}
	virtual void fill_buffer( guint8* buf, size_t& size);
};

void
ReadMemoryCommand::fill_buffer( guint8* buf, size_t& size)
{
	ThreeValueCommand::fill_buffer( buf, size);
}

class LiningCommand : public Commands {
public:
	LiningCommand( char chip, const std::vector<guint8>& lining)
		: chip_(chip), lining_(lining) { buffer_size_ = SCANNER_BUFFER * 2; }
	LiningCommand( char chip, guint8 value) : chip_(chip),
		lining_(SCANNER_STRIPS_PER_CHIP_REAL)
		{ buffer_size_ = SCANNER_BUFFER * 2;
		std::fill( lining_.begin(), lining_.end(), value); }
	virtual ~LiningCommand() {}
	virtual void fill_buffer( guint8* buf, size_t& size);

protected:
	char chip_;
	std::vector<guint8> lining_;
};

void
LiningCommand::fill_buffer( guint8* buf, size_t& size)
{
	const size_t half = SCANNER_STRIPS_PER_CHIP_REAL / 2;

	Commands::fill_buffer( buf, size);

	buf[0] = 'D';
	buf[1] = chip_;
	buf[2] = '0';

	for ( size_t i = 0; i < half; ++i)
		buf[i + SCANNER_HEADER] = lining_[i];

	buf[72] = 'D';
	buf[73] = chip_;
	buf[74] = '1';

	for ( size_t i = 0; i < half; ++i)
		buf[i + SCANNER_BUFFER + SCANNER_HEADER] = lining_[half + i];
}

class MovementCommand : public BaseCommand {
public:
	MovementCommand( const Movement& movement,
		MovementType movement_type,
		DirectionType direction_type)
			:
			BaseCommand('m'),
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

	buf[1] = '0';
	buf[2] = '0';

	switch (movement_type_) {
	case MOVEMENT_ARRAY:
		buf[6] = '1'; // array
		buf[7] = '0'; // xray
		break;
	case MOVEMENT_XRAY:
		buf[6] = '0'; // array
		buf[7] = '1'; // xray
		break;
	case MOVEMENT_BOTH:
		buf[6] = '1'; // array
		buf[7] = '1'; // xray
		break;
	case MOVEMENT_NONE:
	default:
		buf[6] = '0'; // array
		buf[7] = '0'; // xray
		break;
	}

	buf[8] = (movement_.steps >> 8) & 0xFF; // first byte
	buf[9] = movement_.steps & 0xFF; // second byte
	buf[10] = 0;
	buf[11] = (movement_.array_speed.freq >> 8) & 0xFF; // first byte (the only byte)
	buf[12] = movement_.array_speed.mode + '0';

	switch (direction_type_) {
	case DIRECTION_FORWARD:
		buf[13] = '1'; // array
		buf[16] = '1'; // xray
		break;
	case DIRECTION_REVERSE:
	default:
		buf[13] = '0'; // array
		buf[16] = '0'; // xray
		break;
	}

	buf[14] = movement_.xray_speed.freq;
	buf[15] = movement_.xray_speed.mode + '0';

	buf[17] = movement_.array_speed.freq & 0xFF;
	buf[18] = movement_.xray_delay;
	buf[19] = movement_.xray_array_delay;
}

Command*
Commands::create(CommandType com)
{
	Command* command = 0;

	switch (com) {
	case COMMAND_HANDSHAKE:
		command = new BaseCommand('M');
		break;
	case COMMAND_ID:
		command = new BaseCommand('I');
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
		command = new ReadMemoryCommand( 2, 16, 16);
		break;
	case COMMAND_READ_MEMORY_ONE_BANK:
		command = new ReadMemoryCommand( 2, 128, 128);
		break;
	case COMMAND_READ_MEMORY_TWO_BANKS:
		command = new ReadMemoryCommand( 4, 128, 128);
		break;
	case COMMAND_READ_MEMORY_THREE_BANKS:
		command = new ReadMemoryCommand( 6, 128, 128);
		break;
	case COMMAND_READ_MEMORY_FULL:
		command = new ReadMemoryCommand( 8, 128, 128);
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
	case COMMAND_PELTIER_ON:
		command = new PeltierCommand(value);
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
		command = new CapacityCommand( v1, v2, v3);
		break;
	case COMMAND_READ_MEMORY:
		command = new ReadMemoryCommand( v1, v2, v3);
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
	return (mem.check()) ? new ReadMemoryCommand( codes[0], codes[1], codes[2]) :
		new BaseCommand('R');
}

Command*
Commands::create(double capacity)
{
	const char* codes = BuiltinCapacities::capacity_code(capacity);
	return (codes) ? new CapacityCommand( codes[0], codes[1], codes[2]) :
		new BaseCommand('R');
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
