#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>

// Function declarations
uint16_t read_adc(uint8_t channel);

uint8_t head_x = 0x10; // Start middle
uint8_t head_y = 0x10;

uint8_t last_button = 0;

// SNAKE

struct position
{
    uint8_t x;
    uint8_t y;
};
struct position snake_body[64] = {0};
uint8_t snake_length = 2;

struct position rabbit = {0};

volatile uint8_t buffer[8] = {0};
void push_buffer(void)
{
    for (uint8_t row = 0; row < 8; row++)
    {
        uint8_t row_data = buffer[row];

        row_data = ~row_data;

        uint8_t rev = 0;
        for (uint8_t i = 0; i < 8; i++)
        {
            rev <<= 1;
            rev |= (row_data & 1);
            row_data >>= 1;
        }

        PORTB = rev;        // LED data
        PORTD = (1 << row); // Select row
        _delay_us(50);      // Short hold
        PORTD = 0x00;       // Clear row
        _delay_us(50);      // Short off delay
    }
}
void clear_buffer()
{
    for (uint8_t i = 0; i < 8; i++)
    {
        buffer[i] = 0;
    }
}

uint8_t snake_check_collision(uint8_t x, uint8_t y)
{
    for (uint8_t i = 0; i < snake_length; i++)
    {
        if ((x & snake_body[i].x) && (y & snake_body[i].y))
        {
            return (1); // it collides
        }
    }
    return (0); // it does not collide
}

void snake_score(uint8_t score)
{
    // score = 64
    uint8_t d1=score/10;      // 64/10     => 6
    uint8_t d0=score-(d1*10); // 64-(6*10) => 4 

    if(d1)
    {
	switch(d0)
	{
	case 0:
	    buffer[6] |= 0b00001110;
	    buffer[5] |= 0b00001010;
	    buffer[4] |= 0b00001010;
	    buffer[3] |= 0b00001010;
	    buffer[2] |= 0b00001110;
	    break;
	case 1:
	    buffer[6] |= 0b00001100;
	    buffer[5] |= 0b00000100;
	    buffer[4] |= 0b00000100;
	    buffer[3] |= 0b00000100;
	    buffer[2] |= 0b00001110;
	    break;
	case 2:
	    buffer[6] |= 0b00001110;
	    buffer[5] |= 0b00000010;
	    buffer[4] |= 0b00001110;
	    buffer[3] |= 0b00001000;
	    buffer[2] |= 0b00001110;
	    break;
	case 3:
	    buffer[6] |= 0b00001110;
	    buffer[5] |= 0b00000010;
	    buffer[4] |= 0b00001110;
	    buffer[3] |= 0b00000010;
	    buffer[2] |= 0b00001110;
	    break;
	case 4:
	    buffer[6] |= 0b00000010;
	    buffer[5] |= 0b00000110;
	    buffer[4] |= 0b00001010;
	    buffer[3] |= 0b00001110;
	    buffer[2] |= 0b00000010;
	    break;
	case 5:
	    buffer[6] |= 0b00001110;
	    buffer[5] |= 0b00001000;
	    buffer[4] |= 0b00001110;
	    buffer[3] |= 0b00000010;
	    buffer[2] |= 0b00001110;
	    break;
	case 6:
	    buffer[6] |= 0b00000100;
	    buffer[5] |= 0b00001000;
	    buffer[4] |= 0b00001110;
	    buffer[3] |= 0b00001010;
	    buffer[2] |= 0b00001110;
	    break;
	case 7:
	    buffer[6] |= 0b00001110;
	    buffer[5] |= 0b00000010;
	    buffer[4] |= 0b00000100;
	    buffer[3] |= 0b00001000;
	    buffer[2] |= 0b00001000;
	    break;
	case 8:
	    buffer[6] |= 0b00001110;
	    buffer[5] |= 0b00001010;
	    buffer[4] |= 0b00001110;
	    buffer[3] |= 0b00001010;
	    buffer[2] |= 0b00001110;
	    break;
	case 9:
	    buffer[6] |= 0b00001110;
	    buffer[5] |= 0b00001010;
	    buffer[4] |= 0b00001110;
	    buffer[3] |= 0b00000010;
	    buffer[2] |= 0b00000100;
	    break;
	}
	switch(d1)
	{
	case 0:
	    buffer[6] |= 0b11100000;
	    buffer[5] |= 0b10100000;
	    buffer[4] |= 0b10100000;
	    buffer[3] |= 0b10100000;
	    buffer[2] |= 0b11100000;
	    break;
	case 1:
	    buffer[6] |= 0b11000000;
	    buffer[5] |= 0b01000000;
	    buffer[4] |= 0b01000000;
	    buffer[3] |= 0b01000000;
	    buffer[2] |= 0b11100000;
	    break;
	case 2:
	    buffer[6] |= 0b11100000;
	    buffer[5] |= 0b00100000;
	    buffer[4] |= 0b11100000;
	    buffer[3] |= 0b10000000;
	    buffer[2] |= 0b11100000;
	    break;
	case 3:
	    buffer[6] |= 0b11100000;
	    buffer[5] |= 0b00100000;
	    buffer[4] |= 0b11100000;
	    buffer[3] |= 0b00100000;
	    buffer[2] |= 0b11100000;
	    break;
	case 4:
	    buffer[6] |= 0b00100000;
	    buffer[5] |= 0b01100000;
	    buffer[4] |= 0b10100000;
	    buffer[3] |= 0b11100000;
	    buffer[2] |= 0b00100000;
	    break;
	case 5:
	    buffer[6] |= 0b11100000;
	    buffer[5] |= 0b10000000;
	    buffer[4] |= 0b11100000;
	    buffer[3] |= 0b00100000;
	    buffer[2] |= 0b11100000;
	    break;
	case 6:
	    buffer[6] |= 0b01000000;
	    buffer[5] |= 0b10000000;
	    buffer[4] |= 0b11100000;
	    buffer[3] |= 0b10100000;
	    buffer[2] |= 0b11100000;
	    break;
	}
    }
    else
    {
	switch(d0)
	{
	case 0:
	    buffer[6] |= 0b11100000;
	    buffer[5] |= 0b10100000;
	    buffer[4] |= 0b10100000;
	    buffer[3] |= 0b10100000;
	    buffer[2] |= 0b11100000;
	    break;
	case 1:
	    buffer[6] |= 0b11000000;
	    buffer[5] |= 0b01000000;
	    buffer[4] |= 0b01000000;
	    buffer[3] |= 0b01000000;
	    buffer[2] |= 0b11100000;
	    break;
	case 2:
	    buffer[6] |= 0b11100000;
	    buffer[5] |= 0b00100000;
	    buffer[4] |= 0b11100000;
	    buffer[3] |= 0b10000000;
	    buffer[2] |= 0b11100000;
	    break;
	case 3:
	    buffer[6] |= 0b11100000;
	    buffer[5] |= 0b00100000;
	    buffer[4] |= 0b11100000;
	    buffer[3] |= 0b00100000;
	    buffer[2] |= 0b11100000;
	    break;
	case 4:
	    buffer[6] |= 0b00100000;
	    buffer[5] |= 0b01100000;
	    buffer[4] |= 0b10100000;
	    buffer[3] |= 0b11100000;
	    buffer[2] |= 0b00100000;
	    break;
	case 5:
	    buffer[6] |= 0b11100000;
	    buffer[5] |= 0b10000000;
	    buffer[4] |= 0b11100000;
	    buffer[3] |= 0b00100000;
	    buffer[2] |= 0b11100000;
	    break;
	case 6:
	    buffer[6] |= 0b01000000;
	    buffer[5] |= 0b10000000;
	    buffer[4] |= 0b11100000;
	    buffer[3] |= 0b10100000;
	    buffer[2] |= 0b11100000;
	    break;
	case 7:
	    buffer[6] |= 0b11100000;
	    buffer[5] |= 0b00100000;
	    buffer[4] |= 0b01000000;
	    buffer[3] |= 0b10000000;
	    buffer[2] |= 0b10000000;
	    break;
	case 8:
	    buffer[6] |= 0b11100000;
	    buffer[5] |= 0b10100000;
	    buffer[4] |= 0b11100000;
	    buffer[3] |= 0b10100000;
	    buffer[2] |= 0b11100000;
	    break;
	case 9:
	    buffer[6] |= 0b11100000;
	    buffer[5] |= 0b10100000;
	    buffer[4] |= 0b11100000;
	    buffer[3] |= 0b00100000;
	    buffer[2] |= 0b01000000;
	    break;
	}
    }

    push_buffer();
    _delay_us(20000);      // Short hold
}
void snake_death()
{    
    for(uint8_t j = 0; j < 3; j++)
    {
	for (uint8_t i = 0; i < 8; i++)
	{
	    if(j%2)
	    {
		if(i%2)
		{
		    buffer[i] = 0b10101010;
		}
		else
		{
		    buffer[i] = 0b01010101;
		}
	    }
	    else
	    {
		if(i%2)
		{
		    buffer[i] = 0b01010101;
		
		}
		else
		{
		    buffer[i] = 0b10101010;
		}	
	    }
	}
	push_buffer();
	_delay_us(10000);      // Short hold
    }
    
}

void snake_clear_body()
{
    for (uint8_t i = 0; i < 64; i++)
    {
        snake_body[i].x = 0;
        snake_body[i].y = 0;
    }
}
void snake_reset()
{
    // button state.
    last_button = 0;

    snake_death();

    // snake and rabbit state.
    clear_buffer();
    
    snake_length = 2;

    snake_clear_body();
    
    snake_body[0].x = 0b00010000; // head x
    snake_body[0].y = 0b00001000; // head y
    snake_body[1].x = 0b00100000;
    snake_body[1].y = 0b00001000;

    rabbit.x = 0b00000100;
    rabbit.y = 0b00001000;
    rabbit_update();
    
    head_x = snake_body[0].x;
    head_y = snake_body[0].y;

    snake_update_body(head_x, head_y);
    snake_update_body(snake_body[1].x, snake_body[1].y);
}
void snake_update_body_shift()
{
    struct position *b = &snake_body;
    for (uint8_t i = snake_length - 1; i > 0; i--)
    {
        b[i] = b[i - 1];
    }

    // write head.
    b[0].x = head_x;
    b[0].y = head_y;
}
void snake_update_body(uint8_t x, uint8_t y)
{
    for (uint8_t row = 0; row < 8; row++)
    {
        if (y & (1 << row))
        {
            buffer[row] |= x;
        }
    }
}
void snake_update_eat()
{
    if ((snake_body[0].x & rabbit.x) && (snake_body[0].y & rabbit.y))
    {
	uint8_t valid_position = 0;
	while(!valid_position)
	{
	    // despawn rabbit - generate new position using ADC noise
	    uint16_t adc6_val = read_adc(6);
	    uint16_t adc7_val = read_adc(7);

	    // Convert to bit positions (0-7)
	    uint8_t x_pos = adc6_val % 8;
	    uint8_t y_pos = adc7_val % 8;

	    // Convert to bit masks
	    rabbit.x = (1 << x_pos);
	    rabbit.y = (1 << y_pos);

	    if(!snake_check_collision(rabbit.x, rabbit.y))
	    {
		valid_position = 1;
	    }
	}
        snake_length++;
    }
}

void rabbit_update()
{
    for (uint8_t row = 0; row < 8; row++)
    {
        if (rabbit.y & (1 << row))
        {
            buffer[row] |= rabbit.x;
        }
    }
}

void snake_update(void)
{
    if(last_button)
    {
	if      (last_button & (1 << 0)) { head_y = head_y << 1; } // north, shift [head y] right 
	else if (last_button & (1 << 1)) { head_y = head_y >> 1; } // south, shift [head y] left
	else if (last_button & (1 << 2)) { head_x = head_x >> 1; } // east,  shift [head x] right
	else if (last_button & (1 << 3)) { head_x = head_x << 1; } // west,  shift [head x] left

	if (!head_x || !head_y)
	{
	    snake_reset();
	}
	else if(snake_check_collision(head_x, head_y))
	{
	    snake_reset();
	}
	else
	{
	    // render.
	    clear_buffer();

	    snake_update_body_shift();
	    for (uint8_t b = 0; b < snake_length; b++)
	    {
		snake_update_body(snake_body[b].x, snake_body[b].y);
	    }

	    snake_update_eat();

	    rabbit_update();
	}
	
    }
}

void wait_button(void)
{
    uint8_t buttons = PINC;

    if (!(buttons & (1 << 4))) // if [north] button is pressed (4)
    {
        if (last_button != (1 << 1)) { last_button = (1 << 0); } // if snake is not going [south], go [north]
    }
    else if (!(buttons & (1 << 2))) // if [south] button is pressed (2)
    {
        if (last_button != (1 << 0)) { last_button = (1 << 1); } // if snake is not going [north], go [south]
    }
    else if (!(buttons & (1 << 3))) // if [east] button is pressed (3)
    {
        if (last_button != (1 << 3)) { last_button = (1 << 2); } // if snake is not going [west], go [east]
    }
    else if (!(buttons & (1 << 5))) // if [west] button is pressed (5)
    {
        if (last_button != (1 << 2)) { last_button = (1 << 3); }  // if snake is not going [east], go [west]
    }
}

ISR(TIMER0_OVF_vect)
{
    push_buffer();
}

ISR(TIMER1_OVF_vect)
{
    snake_update();
}

// ADC reading function
uint16_t read_adc(uint8_t channel)
{
    // Select ADC channel (0-7)
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

    // Start conversion
    ADCSRA |= (1 << ADSC);

    // Wait for conversion to complete
    while (ADCSRA & (1 << ADSC))
        ;

    // Return ADC result
    return ADC;
}

int main(void)
{
    // Port B & D outputs (LED matrix)
    DDRB = 0xFF;
    DDRD = 0xFF;

    // Port C buttons inputs w/ pull-ups
    DDRC &= ~((1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));
    PORTC |= (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);

    // ADC configuration
    ADMUX = (1 << REFS0);                                              // Use AVcc as reference voltage
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC, prescaler 128 (16MHz/128 = 125kHz)

    // Timer1: enable overflow interrupt
    TCCR1B = (1 << CS11) | (1 << CS10); // clk/64
    TIMSK1 = (1 << TOIE1);

    // Timer0: enable overflow interrupt
    TCCR0B = (1 << CS00); // clk/1
    TIMSK0 = (1 << TOIE0);

    sei(); // Enable global interrupts

    snake_score(64);
    // snake state.
    snake_reset();

    while (1)
    {
        wait_button();
    }
}
