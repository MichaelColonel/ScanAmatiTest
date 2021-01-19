#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <util/atomic.h>

#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>

#define SETBITS( x, y)                           ((x) |= (y))
#define CLEARBITS( x, y)                        ((x) &= ~(y))
#define SETBIT( x, y)                     SETBITS( x, _BV(y))
#define CLEARBIT( x, y)                 CLEARBITS( x, _BV(y))
#define BITSET( x, y)                       bit_is_set( x, y)
#define BITCLEAR( x, y)                   bit_is_clear( x, y)
#define BITSSET( x, y)                   (((x) & (y)) == (y))
#define BITSCLEAR( x, y)                   (((x) & (y)) == 0)
#define BITVAL( x, y)                      (((x) >> (y)) & 1)

#define SETVAL( x, y, value) { if (value) SETBIT( x, y); else CLEARBIT( x, y); }

#define ADC_MUX                                          0xC0 // Reference selection bits enabled
#define ADC_PF7_INPUT                                    0x07 // ADC7 (PF7) Input
#define ADC_SRA                                          0x87 // Frequency reference (ADC enabled, 128 division factor)

#define DETECTORS_A_CHIP                                  128

#define BUFFER_HEADER                                       2
#define BUFFER_ANSWER                                      16

#define TIMER1_DIV			                              200

#define DEFAULT_CAPACITY                                  '7'

#define AD8400_HEADER                                       2 // A0,A1 values
#define AD8400_BUFFER            (AD8400_HEADER) + (CHAR_BIT)

const char Welcome_string[] PROGMEM = "Welcome";
const char APRM003_string[] PROGMEM = "APRM003";
const char OK_string[] PROGMEM = "OK";

PGM_P const strings[] PROGMEM = {
		Welcome_string,
		APRM003_string,
		OK_string
};

union InputBuffer {
	volatile uint8_t buffer[BUFFER_HEADER + DETECTORS_A_CHIP + 1];
	struct {
		uint8_t code;
		uint8_t size;
		uint8_t data[DETECTORS_A_CHIP + 1];
	};
};

struct Buffers {
	union InputBuffer input;
	uint8_t output[BUFFER_ANSWER];
} command = {
		{ { 'N' } },
		{ }
};

volatile uint8_t timer0_data, timer0_cnt = 0;
volatile uint8_t timer1_data, timer1_inc, timer1_of_int;

ISR(TIMER0_OVF_vect)
{
	TCCR0 = 0x00; //stop T0
	TCNT0 = timer0_data; // load T0_data
	TCCR0 = 0x07; // start T0
	if (timer0_cnt == 4) {
	   timer0_cnt = 0;
	   if (BITVAL( PINB, PINB4))
		   CLEARBIT( PORTB, PB4);
	   else
		   SETBIT( PORTB, PB4);
	}
	timer0_cnt++;
}

ISR(TIMER1_OVF_vect)
{
	TCCR1B = 0x00;
	TCNT1 = timer1_data;

	if (timer1_of_int <= TIMER1_DIV) {
		timer1_data += timer1_inc;
		timer1_of_int++;
	}
	if (BITVAL( PINB, PINB4))
		CLEARBIT( PORTB, PB4);
	else
		SETBIT( PORTB, PB4);


	TCCR1B = 0x01;
}

ISR(INT0_vect)
{
	cli();

	SETBIT( PORTA, PA6); // memory HiZ

	PORTC = 0xFF; // Port C = in
	DDRC = 0x00;

	CLEARBIT( PORTG, PG1);
	SETBIT( PORTG, PG4);

	// Read command header
	for ( uint8_t i = 0; i < BUFFER_HEADER; ++i) {
		CLEARBIT( PORTG, PG2); // strobe
		_delay_us(10);
		command.input.buffer[i] = PINC;
	    SETBIT( PORTG, PG2); // strobe
	    _delay_us(10);
	}

	// Read command data
	for ( uint8_t i = 0; i < command.input.size; ++i) {
		CLEARBIT( PORTG, PG2); // strobe
		_delay_us(10);
		command.input.data[i] = PINC;
	    SETBIT( PORTG, PG2); // strobe
	    _delay_us(10);
	}

	PORTC = 0x00; // Port C = HiZ
	DDRC = 0x00;
	sei();
}

ISR(INT4_vect) // motor 1
{
	 if ((command.input.buffer[12] == '1') && (BITCLEAR( PINE, PINE4)))
		 command.input.code = 'N';
}


ISR(INT5_vect) // motor 1
{
	 if ((command.input.buffer[12] == '0') && (BITCLEAR( PINE, PINE5)))
		 command.input.code = 'N';
}

ISR(INT6_vect) // motor 2
{
	if ((command.input.buffer[9] == '0') && (BITCLEAR( PINE, PINE6)))
		CLEARBIT( PORTB, PB6);
}

ISR(INT7_vect) // motor 2
{
	if ((command.input.buffer[9] == '0') && (BITCLEAR( PINE, PINE7)))
		CLEARBIT( PORTB, PB6);
}

void init() __attribute__((naked,section(".init3")));
static void chip_reset();
static void chip_select(uint8_t);
static void chip_select_clock(uint8_t);
static void chip_data();
static void altera_start(int8_t);
static void altera_reset();
static void adc_value( /* uint8_t input, */ uint8_t *low, uint8_t *high);
static void time_delay_10us(uint16_t);
static void time_delay_ms(uint8_t);

static void output_rom_string(PGM_P str);
static void output_data( uint8_t *buf, uint8_t size);

int
main()
{
	uint8_t with_xray = 0;
	uint8_t start_altera;
	uint16_t delay;
	uint16_t steps;
	uint16_t acceleration_step;
	uint16_t acceleration_delay;

	uint8_t previous_chip, current_chip = DEFAULT_CAPACITY; // During initiation - capacity 9.5 pF (0111b code)

	while (1) {
		uint8_t cmd;
		// Wait for interrupt
		do {
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
				cmd = command.input.code;
			}
		} while (cmd == 'N');

		switch (cmd) {
        case 'C': // Capacities
        	previous_chip = current_chip;
        	CLEARBIT( PORTD, PD2);
        	// value from '1' to '7'
        	current_chip = command.input.data[0];
         	chip_select(current_chip); // capacity select actually
            SETBIT( PORTD, PD2); // Write capacities
            CLEARBIT( PORTD, PD2); // Block capacity selection

            _delay_ms(200);
            // Restore previous chip
            current_chip = previous_chip;

            SETBIT( PORTG, PG4); // Integration mode
            chip_select(current_chip);
            SETBIT( PORTA, PA1); // Turn on altera
            chip_reset();

            command.input.code = 'N';
            break;
		case 'I': // Welcome command : "Welcome", Info command : "APRMXXX"
            {
            	PGM_P answer = (command.input.data[0] == '0') ? (PGM_P)pgm_read_word(&(strings[0])) :
            			(PGM_P)pgm_read_word(&(strings[1]));
            	output_rom_string(answer);
            }
            command.input.code = 'N';
			break;
        case 'D': // Write chip codes
        	previous_chip = current_chip;
        	current_chip = command.input.data[0];
        	DDRD = 0x04;
        	CLEARBIT( PORTA, PA1); // OFF altera_clk
            CLEARBIT( PORTA, PA4);
            SETBIT( PORTA, PA4); // rst_n altera
            CLEARBIT( PORTA, PA0); // Deny MX0-3 selection
      	    chip_select(command.input.data[0]);
      	    SETBIT( PORTA, PA0); // Deny MX0-3 selection
      	    CLEARBIT( PORTA, PA1); // Allow read_clk
       	    chip_data();
       	    SETBIT( PORTA, PA1); // ON altera_clk
       	    chip_reset();
   			output_rom_string((PGM_P)pgm_read_word(&(strings[2])));
       	    command.input.code = 'N';
            break;
        case 'G': // Chip select
        	SETBIT( PORTG, PG4); // Integration mode
        	current_chip = command.input.data[0];
            chip_select(current_chip);
            SETBIT( PORTA, PA1); // Turn on altera
            command.input.code = 'N';
            break;
		case 'P': // Peltier command (AD8400 256 position digitally controlled variable resistor (VR))
			DDRF = 0x3F;
			PORTF = 0x01;
			_delay_us(50);

   	       	// Set digital resistor value (A0,A1,bits of a byte_code)
   	        for ( uint8_t i = 0; i < AD8400_BUFFER; ++i) {
   	        	if (i < AD8400_HEADER) { // A0=0 and A1=0 value
   	    	        PORTF = 0x04; // A0,A1
   	    	        _delay_us(10);
   	    	        PORTF = 0x00; //strobe
   	    	        _delay_us(10);
   	    	        continue;
   	        	}

   	        	// bit position from 7 to 0
   	        	bool bit = (command.input.data[0] >> (AD8400_BUFFER - i - 1)) & 0x01;

   	          	PORTF = bit << 3 | 0x04;
   	          	_delay_us(10);
   	          	PORTF = 0x00; //strobe
   	          	_delay_us(10);
   	        }
   	        PORTF = 0x02;
   	        _delay_us(50);

   	        command.input.code = 'N';
   	        break;
		case 'p': // Stop peltier command
			PORTF = 0x01;
			command.input.code = 'N';
		break;
      	case 'R': // reset chip
      		chip_reset();
      		command.input.code = 'N';
      		break;
        case 'r': // reset Altera
        	altera_reset();
        	command.input.code = 'N';
      	    break;
        case 's': // start Altera for full or part amount of memory
        	altera_start(command.input.data[0] - '0');
        	command.input.code = 'N';
      	    break;
        case 'M': // Stepper motors command
      		CLEARBIT( PORTA, PA3);
      		start_altera = 0;

      		// motor1
      		steps = command.input.data[2] << CHAR_BIT | command.input.data[3]; // number of steps
      	    delay = command.input.data[4] << CHAR_BIT | command.input.data[5]; // delay frequency

      	    SETVAL( PORTB, PB0, command.input.data[6] - '0'); // motor HALF=1 / FULL=0 - mode
      	    SETVAL( PORTB, PB5, command.input.data[10] - '0'); // motor CW=1 / CCW=0 - direction

      	    // motor2
      	    timer1_data = command.input.data[8] << CHAR_BIT | command.input.data[11];

      	    SETVAL( PORTE, PE2, command.input.data[9] - '0'); // motor HALF=1 / FULL=0 - mode
      	    SETVAL( PORTB, PB3, command.input.data[7] - '0'); // motor CW=1 / CCW=0 - direction

      	    SETVAL( PORTB, PB2, command.input.data[0] - '0');
      	    SETVAL( PORTB, PB6, command.input.data[1] - '0');

      	    if ((BITSET( PINE, PINE6) && (command.input.data[7] == '0')) || (BITSET( PINE, PINE7) && (command.input.data[7] == '1'))) {
      	        timer1_inc = (UINT16_MAX - timer1_data) / TIMER1_DIV;
      	        timer1_of_int = 0;
      	        timer1_data = (timer1_data << 1) - UINT16_MAX;

      	        TCNT1 = timer1_data;
                TCCR1A = 0;
                TCCR1B = 1;
      	    }

      	    time_delay_ms(command.input.data[12]);

      	    acceleration_step = TIMER1_DIV;

      	    if (with_xray)  // x-ray on
               SETBIT( PORTB, PB7);

      	    time_delay_ms(command.input.data[13]);


      	    if ((BITSET( PINE, PINE4) && (command.input.data[10] == '1')) || (BITSET( PINE, PINE5) && (command.input.data[10] == '0'))) {
      	    	uint16_t i = 0;
      	    	while ((command.input.code == 'M') && (i < steps)) {
      	    		CLEARBIT( PORTB, PB1);

      	    		if (i <= acceleration_step) {
      	    			acceleration_delay = delay * (1 + (acceleration_step - i) / acceleration_step);
      	    			time_delay_10us(acceleration_delay);
      	            }
      	            else {
      	            	if (!start_altera) {
      	            		altera_start(0);
      	                    start_altera = 1;
      	                }
              	        time_delay_10us(delay);
      	            }

      	    		SETBIT( PORTB, PB1);

      	    		if (i <= acceleration_step) {
      	    			acceleration_delay = delay * (1 + (acceleration_step - i) / acceleration_step);
      	    			time_delay_10us(acceleration_delay);
      	    		}
      	    		else
      	    			time_delay_10us(delay);
      	    		i++;
      	    	}
      	    }

      	    TCCR1B = 0x00;

      	    CLEARBIT( PORTB, PB2);
      	    CLEARBIT( PORTB, PB6);
     	    CLEARBIT( PORTB, PB7); // x-ray off

     	    CLEARBIT( PORTG, PG3);

     	    with_xray = 0;
        	command.input.code = 'N';
        	break;
        case 'T':
      	    adc_value( /* ADC_PF7_INPUT, */ command.output, command.output + 1);
            output_data( command.output, 2);
            command.input.code = 'N';
        	break;
        case 'W': // Readout data
     		PORTC = 0x00; // Port С = HiZ
            DDRC = 0x00;

            CLEARBIT( PORTA, PA6); // memory out
            CLEARBIT( PORTA, PA7);
            CLEARBIT( PORTA, PA2);
            for ( uint8_t i = command.input.data[0]; i != 0; --i) {
            	for ( uint8_t j = command.input.data[1]; j != 0; --j) {
            		for ( uint8_t k = command.input.data[2]; k != 0; --k) {
            			for ( uint8_t l = SCHAR_MAX + 1; l != 0; --l) { // send two bytes
            				loop_until_bit_is_clear( PIND, PIND1); // FT245 ready to read

            				SETBIT( PORTA, PA7); // ft_strob
            				CLEARBIT( PORTA, PA7);
            				SETBIT( PORTA, PA2);

            				SETBIT( PORTA, PA7);
            				CLEARBIT( PORTA, PA7);
                            CLEARBIT( PORTA, PA2);
            			}
            		}
            	}
            }
            SETBIT( PORTA, PA6); // memory HiZ
            PORTC = 0x00; // Port С = HiZ
            DDRC = 0x00;
            command.input.code = 'N';
            break;
     	case 'X': // X-Ray on/off
     		CLEARBIT( PORTB, PB7);
     		SETVAL( PORTG, PG3, command.input.data[0] -'0');
     		command.input.code = 'N';
       	    break;
		case 'N':
		default:
			break;
		}
	}
}

void
init()
{
	// Ports initialization
	PORTA = 0xFF;
	DDRA = 0xFF;

	PORTB = 0x00;
	DDRB = 0xFF;

	PORTC = 0xFF;
    DDRC = 0x00;

    PORTD = 0x03;
    DDRD = 0x04;

    PORTE = 0xF0;
    DDRE = 0x03;

    PORTF = 0x0B;
    DDRF = 0x3F;

    PORTG = 0x14;
    DDRG = 0x1F;

    // Timer/Counter 0 initialization
    ASSR = 0x00;
    TCCR0 = 0x00;
    TCNT0 = 0x00;
    OCR0 = 0x00;

    // Timer/Counter 1 initialization
    TCCR1A = 0x00;
    TCCR1B = 0x00;
    TCNT1H = 0x00;
    TCNT1L = 0x00;
    ICR1H = 0x00;
    ICR1L = 0x00;
    OCR1AH = 0x00;
    OCR1AL = 0x00;
    OCR1BH = 0x00;
    OCR1BL = 0x00;
    OCR1CH = 0x00;
    OCR1CL = 0x00;

    // Timer/Counter 2 initialization
    TCCR2 = 0x00;
    TCNT2 = 0x00;
    OCR2 = 0x00;

    // Timer/Counter 3 initialization
    TCCR3A = 0x00;
    TCCR3B = 0x00;
    TCNT3H = 0x00;
    TCNT3L = 0x00;
    ICR3H = 0x00;
    ICR3L = 0x00;
    OCR3AH = 0x00;
    OCR3AL = 0x00;
    OCR3BH = 0x00;
    OCR3BL = 0x00;
    OCR3CH = 0x00;
    OCR3CL = 0x00;

    // USART0 initialization
    UCSR0A = 0x20;
    UCSR0B = 0x00;
    UCSR0C = 0x00;
    UBRR0H = 0x00;
    UBRR0L = 0x00;

    // External Interrupt(s) initialization
    EICRA = 0x00; // On INT0 Mode Low level
    EICRB = 0xAA; // INT4,5,6,7:  Falling Edge
    EIMSK = 0xF1; // INT0,4,5,6,7: On, INT1,2,3: Off
    EIFR = 0x01;

    // Timer(s)/Counter(s) Interrupt(s) initialization
    TIMSK = 0x04; // T1 int ON
    ETIMSK = 0x00;

    ADMUX = ADC_MUX | ADC_PF7_INPUT; // ADC initialization
    ADCSRA = ADC_SRA; // ADC Clock frequency: 62,500 kHz// ADC Voltage Reference: Int., cap. on AREF

    // Set 9.5 pF capacity
	SETBIT( PORTA, PA1); // Turn on altera sense_clk
	CLEARBIT( PORTD, PD2); // Deny capacity selection

	chip_select(DEFAULT_CAPACITY); // 9.5 pF (0111b code)

	SETBIT( PORTD, PD2); // Set 9.5 pF Capacity (Maximum 1 pF + 2.5 pF + 6 pF)
	CLEARBIT( PORTD, PD2); // Deny capacity selection

    // Global enable interrupts
    sei();
}

void
chip_reset()
{
	SETBIT( PORTG, PG1);

    _delay_us(5);

    CLEARBIT( PORTG, PG1);
}

void
chip_select(uint8_t chip)
{
	CLEARBIT( PORTA, PA0); // Block strobes into MX0-3
	CLEARBIT( PORTA, PA4);
	SETBIT( PORTA, PA4); // rst_n

	uint8_t value = 0;
	switch (chip) {
	case '0':
		break;
	case '1':
		value = 4;
		break;
	case '2':
		value = 8;
		break;
	case '3':
		value = 12;
		break;
	case '4':
		value = 16;
		break;
	case '5':
		value = 20;
		break;
	case '6':
		value = 24;
		break;
	case '7':
		value = 28;
		break;
	case '8':
		value = 32;
		break;
	case '9':
		value = 36;
		break;
	case 'A':
		value = 40;
		break;
	case 'B':
		value = 44;
		break;
	case 'C':
		value = 48;
		break;
	case 'D':
		value = 52;
		break;
	case 'E':
		value = 56;
		break;
	case 'F':
		value = 60;
		break;
	default:
		break;
	}

	if (value)
		chip_select_clock(value);

	SETBIT( PORTA, PA0); // Unblock strobes into MX0-3
}

void
chip_select_clock(uint8_t value)
{
	for ( uint8_t i = value; i != 0; --i) {
		SETBIT( PORTA, PA2);
		CLEARBIT( PORTA, PA2); // read_clk
	}
}

void
chip_data()
{
	CLEARBIT( PORTG, PG4); // M1 Signal

	chip_reset();

	// Write chip data from the input buffer in a current chip
	for ( uint8_t i = 0; i < DETECTORS_A_CHIP; ++i) {
         // write bits into chip
         for ( uint8_t j = 0; j < CHAR_BIT; ++j) {
        	 // DATA_out
        	 SETVAL( PORTG, PG0, BITVAL( command.input.data[i + 1], CHAR_BIT - 1 - j));
        	 SETBIT( PORTA, PA2);
        	 _NOP();
        	 CLEARBIT( PORTA, PA2); // read_clk
        	 SETVAL( PORTG, PG0, 0);
         }
	}
	CLEARBIT( PORTG, PG1);
	SETBIT( PORTG, PG4);
	SETBIT( PORTA, PA1);
}

void
altera_reset()
{
	SETBIT( PORTG, PG1);
    CLEARBIT( PORTA, PA4);
	SETBIT( PORTA, PA4);
	_delay_us(4);
	CLEARBIT( PORTG, PG1);
}

void
altera_start(int8_t state)
{
	SETVAL( PORTA, PA3, state);

	PORTD = 0x03;
	DDRD = 0x04;

	SETBIT( PORTA, PA0); // Altera MX0-3 ON
	SETBIT( PORTA, PA1); // Altera sence_clk ON
	CLEARBIT( PORTA, PA4); // reset
	SETBIT( PORTA, PA4);
	CLEARBIT( PORTA, PA5); // start
	SETBIT( PORTA, PA5);
}

void
adc_value( /* uint8_t input, */ uint8_t *low, uint8_t *high)
{
	// ADMUX =  ADC_MUX | input; // see init function for initiation

    // Delay is needed for the stabilization of the ADC input voltage
	// _delay_us(10);

    // Start the AD conversion
    SETBIT( ADCSRA, ADSC);

    // Wait for the AD conversion to complete
    loop_until_bit_is_set( ADCSRA, ADIF);

    *low = ADCL;
    *high = ADCH;
}

void
time_delay_10us(uint16_t t)
{
	for ( ; t != 0; --t)
		_delay_us(10);
}

void
time_delay_ms(uint8_t t)
{
	for ( ; t != 0; --t)
		_delay_ms(1);
}

void
output_rom_string(PGM_P str)
{
    PORTC = 0x00; // Port C - output
    DDRC = 0xFF;

    for ( size_t i = strlen_P(str); i != 0; --i) {
    	loop_until_bit_is_clear( PIND, PIND1); // FT245 ready to read
    		SETBIT( PORTA, PA7);
    		PORTC = pgm_read_byte(&(str[i]));
    		CLEARBIT( PORTA, PA7);
    }
    PORTC = 0x00; // Port C = HiZ
    DDRC = 0x00;
}

void
output_data( uint8_t *buf, uint8_t size)
{
    PORTC = 0x00; // Port C - output
    DDRC = 0xFF;

    for ( uint8_t i = size; i != 0; --i) {
    	loop_until_bit_is_clear( PIND, PIND1); // FT245 ready to read
    	PORTC = buf[i];
    	SETBIT( PORTA, PA7);
    	CLEARBIT( PORTA, PA7);
    }
    PORTC = 0x00; // Port C = HiZ
    DDRC = 0x00;
}
