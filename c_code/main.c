#include <avr/interrupt.h>
#include <util/delay.h>


uint8_t head_x = 0x10; // Start middle
uint8_t head_y = 0x10;

uint8_t last_button = 0;


// SNAKE

struct position
{
    uint8_t x;
    uint8_t y;
};
struct position snake_body[64] = { 0 };
uint8_t         snake_length = 3;

struct position rabbit = { 0 };

volatile uint8_t buffer[8] = { 0 };
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

        PORTB = rev;            // LED data
        PORTD = (1 << row);     // Select row
        _delay_us(50);          // Short hold
        PORTD = 0x00;           // Clear row
        _delay_us(50);          // Short off delay
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
    for(uint8_t i = 0; i < snake_length; i++)
    {
	if((x & snake_body[i].x) && (y & snake_body[i].y))
	{
	    return(1); // it collides
	}
    }
    return(0); // it does not collide
}

void snake_clear_body() 
{
    for(uint8_t i = 0; i < 64; i++)
    {
	snake_body[i].x = 0;
	snake_body[i].y = 0;
    }
}
void snake_reset()
{
    snake_length = 3;

    head_x = 0b00010000;
    head_y = 0b00010000;

    last_button = 0;

    rabbit.x = 0b00010000;
    rabbit.y = 0b01000000;

    snake_body[0].x = 0b00010000;
    snake_body[0].y = 0b00010000;
    
    snake_body[1].x = 0b00100000;
    snake_body[1].y = 0b00010000;

    snake_body[1].x = 0b01000000;
    snake_body[1].y = 0b00010000;


    snake_clear_body();
}
void snake_update_body_shift()
{
    struct position* b = &snake_body;
    for(uint8_t i = snake_length-1; i > 0; i--)
    {
	b[i] = b[i-1];
    }

    // write head.
    b[0].x = head_x;
    b[0].y = head_y;    
}
void snake_update_body(uint8_t x, uint8_t y)
{
    for (uint8_t row = 0; row < 8; row++)
    {
        if (y & (1<<row)) { buffer[row] |= x; }
    }
}
void snake_update_eat()
{
    if((snake_body[0].x & rabbit.x) && (snake_body[0].y & rabbit.y))
    {
	// despawn rabbit
	rabbit.x = 0;
	rabbit.y = 0;

	snake_length++;
    }
}

void rabbit_update()
{
    for (uint8_t row = 0; row < 8; row++)
    {
        if (rabbit.y & (1<<row)) { buffer[row] |= rabbit.x; }
    }
}

void snake_update(void)
{
    if (last_button & (1<<0)) // north
    {
	head_y = head_y << 1; // north (ROR)
    }
    if (last_button & (1<<1)) // south
    {
	head_y = head_y >> 1; // south (ROL)
    }
    if (last_button & (1<<2)) // east
    {
	head_x = head_x >> 1; // east
    }
    if (last_button & (1<<3)) // west
    {
	head_x = head_x << 1; // west
    }
    
    if(!head_x || !head_y)
    {
	snake_reset();
    }
    else
    {
	// render.
	clear_buffer();

	snake_update_body_shift();
	for(uint8_t b = 0; b < snake_length; b++)
	{
	    snake_update_body(snake_body[b].x, snake_body[b].y);
	}

	snake_update_eat();

	rabbit_update();
    }

    
}

void wait_button(void) {
    uint8_t btns = PINC;

    if (!(btns & (1<<2))) // south
    { 
	if(last_button != (1<<0)) // north
	{
	    last_button = (1<<1);
	}
    }
    if (!(btns & (1<<3))) // east
    {
	if(last_button != (1<<3)) // west
	{
	    last_button = (1<<2);
	}
    }
    if (!(btns & (1<<4))) // north
    {
	if(last_button != (1<<2)) // south
	{
	    last_button = (1<<0);
	}
    }
    if (!(btns & (1<<5))) // west
    {
	if(last_button != (1<<2) 
	{
	    last_button = (1<<3);
	}
    }
}


ISR(TIMER0_OVF_vect) {
    push_buffer();
}

ISR(TIMER1_OVF_vect) {
    snake_update();
}

int main(void) {

    // Port B & D outputs (LED matrix)
    DDRB = 0xFF;
    DDRD = 0xFF;

    // Port C buttons inputs w/ pull-ups
    DDRC &= ~((1<<2)|(1<<3)|(1<<4)|(1<<5));
    PORTC |= (1<<2)|(1<<3)|(1<<4)|(1<<5);

    // Timer1: enable overflow interrupt
    TCCR1B = (1<<CS11)|(1<<CS10); // clk/64
    TIMSK1 = (1<<TOIE1);

    // Timer0: enable overflow interrupt
    TCCR0B = (1<<CS00); // clk/1
    TIMSK0 = (1<<TOIE0);

    sei(); // Enable global interrupts

    // Initial snake state
    head_x = 0b00010000;
    head_y = 0b00010000;

    snake_body[0].x = 0b00010000;
    snake_body[0].y = 0b00010000;
    
    snake_body[1].x = 0b00100000;
    snake_body[1].y = 0b00010000;

    snake_body[1].x = 0b01000000;
    snake_body[1].y = 0b00010000;

    rabbit.x = 0b00010000;
    rabbit.y = 0b01000000;


    
    
    while (1) {
        wait_button(); 
    }
}
