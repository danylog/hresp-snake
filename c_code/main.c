#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>

// Position structure using bit masks for efficient LED matrix addressing
struct position
{
    uint8_t x; // X coordinate as bit mask (1 bit set in 8-bit value)
    uint8_t y; // Y coordinate as bit mask (1 bit set in 8-bit value)
};

// LED MATRIX DISPLAY FUNCTIONS

// Display buffer for 8x8 LED matrix (one byte per row)
volatile uint8_t buffer[8] = {0};

// GAME SELECT

uint8_t selected_game = 1; // 0: snake 1: asteriods

// SNAKE

// Snake head position (bit masks for 8x8 matrix)
uint8_t head_x = 0x10; // Start at column 4 (0b00010000)
uint8_t head_y = 0x10; // Start at row 4    (0b00010000)

// Last button pressed (stored as bit flags)
uint8_t last_button = 0;

volatile int8_t initialized = 1; // Flag to check if game is initialized

// SNAKE DATA STRUCTURES

// Snake body array - stores all segments of the snake
struct position snake_body[64] = {0}; // Max 64 segments (8x8 matrix)
uint8_t snake_length = 2;             // Current snake length

// Rabbit/food position
struct position rabbit = {0};

// ASTERIODS DATA STRUCTURES

struct asteroid
{
    struct position center;
    struct position chunks[9];
    uint8_t chunk_count;
};
struct outer_space
{
    struct asteroid next_asteroids[3];
    struct asteroid asteroids[3];
};
struct outer_space space;

struct position spaceship;

// GENERAL

// Read analog-to-digital converter value from specified channel
uint16_t read_adc(uint8_t channel)
{
    // Select ADC channel (0-7) while preserving reference voltage bits
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

    // Start ADC conversion
    ADCSRA |= (1 << ADSC);

    // Wait for conversion to complete
    while (ADCSRA & (1 << ADSC))
        ;

    // Return 10-bit ADC result
    return ADC;
}

// Multiplexed display function - rapidly cycles through rows
void push_buffer(void)
{
    for (uint8_t row = 0; row < 8; row++)
    {
        uint8_t row_data = buffer[row];

        // Invert bits for common cathode display
        row_data = ~row_data;

        // Bit reversal for proper LED orientation
        uint8_t rev = 0;
        for (uint8_t i = 0; i < 8; i++)
        {
            rev <<= 1;             // Shift left to make room for next bit
            rev |= (row_data & 1); // Add LSB of row_data to rev
            row_data >>= 1;        // Shift row_data right for next bit
        }

        PORTB = rev;        // Output LED column data
        PORTD = (1 << row); // Select current row (active high)
        _delay_us(50);      // Brief display time
        PORTD = 0x00;       // Turn off row
        _delay_us(50);      // Brief off time before next row
    }
}

// Clear the display buffer
void clear_buffer()
{
    for (uint8_t i = 0; i < 8; i++)
    {
        buffer[i] = 0; // Clear each row
    }
}

// INPUT HANDLING

// Read button inputs and update movement direction
void wait_button(void)
{
    uint8_t buttons = PINC; // Read button port

    if(selected_game == 0)
    {
	// Check each button (active low with pull-ups)
	if (!(buttons & (1 << 4))) // North button pressed
	{
	    // Only allow if not currently going south (prevent reverse)
	    if (last_button != (1 << 1))
	    {
		last_button = (1 << 0); // Set north direction
	    }
	}
	else if (!(buttons & (1 << 2))) // South button pressed
	{
	    // Only allow if not currently going north
	    if (last_button != (1 << 0))
	    {
		last_button = (1 << 1); // Set south direction
	    }
	}
	else if (!(buttons & (1 << 3))) // East button pressed
	{
	    // Only allow if not currently going west
	    if (last_button != (1 << 3))
	    {
		last_button = (1 << 2); // Set east direction
	    }
	}
	else if (!(buttons & (1 << 5))) // West button pressed
	{
	    // Only allow if not currently going east
	    if (last_button != (1 << 2))
	    {
		last_button = (1 << 3); // Set west direction
	    }
	}
    }
}

// INTERRUPT SERVICE ROUTINES

// Timer0 overflow - handles display refresh (fast)
ISR(TIMER0_OVF_vect)
{
    push_buffer(); // Refresh LED matrix display
}

// Timer1 overflow - handles game logic updates (slower)
ISR(TIMER1_OVF_vect)
{
    snake_update(); // Update game state
}

// MAIN FUNCTION



// FUNCTIONS (SNAKE)

// COLLISION DETECTION

// Check if given position collides with snake body
uint8_t snake_check_collision(uint8_t x, uint8_t y)
{
    for (uint8_t i = 0; i < snake_length; i++)
    {
        // Use bitwise AND to check if positions overlap
        if ((x & snake_body[i].x) && (y & snake_body[i].y))
        {
            return (1); // Collision detected
        }
    }
    return (0); // No collision
}

// SCORE DISPLAY

// Display score as 2-digit number on LED matrix
void snake_score(uint8_t score)
{
    // Disable Timer0 to prevent interference
    TIMSK0 &= ~(1 << TOIE0);
    clear_buffer();
    // Extract tens and ones digits
    uint8_t d1 = score / 10;        // Tens digit (e.g., 64/10 = 6)
    uint8_t d0 = score - (d1 * 10); // Ones digit (e.g., 64-(6*10) = 4)

    // If score >= 10, display both digits
    if (d1)
    {
        // Display ones digit (right side of matrix)
        switch (d0)
        {
        case 0:
            // Draw "0" in 3x5 pixel font
            buffer[6] |= 0b00001110; // Top row
            buffer[5] |= 0b00001010; // Side pixels
            buffer[4] |= 0b00001010; // Side pixels
            buffer[3] |= 0b00001010; // Side pixels
            buffer[2] |= 0b00001110; // Bottom row
            break;
        case 1:
            // Draw "1" in 3x5 pixel font
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
        // Display tens digit (left side of matrix)
        switch (d1)
        {
        case 0:
            // Draw "0" in 3x5 pixel font on left side
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
        // For single digit scores, display in center
        switch (d0)
        {
        case 0:
            // Draw "0" centered
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

    // Now display the pattern many times to make it visible
    for (uint16_t k = 0; k < 10000; k++)
    {
        push_buffer(); // Manual display refresh
    }

    // Brief pause between flashes

    // Re-enable Timer0
    TIMSK0 |= (1 << TOIE0);
}

// GAME OVER ANIMATION

// Flash checkerboard pattern when snake dies
void snake_death()
{
    TIMSK0 &= ~(1 << TOIE0);
    clear_buffer();

    for (uint8_t j = 0; j < 3; j++) // Flash 3 times
    {
        for (uint8_t i = 0; i < 8; i++) // For each row (0-7)
        {
            if (j % 2) // Odd flash cycles (j=1)
            {
                if (i % 2) // Odd rows
                {
                    buffer[i] = 0b10101010; // Pattern: ■□■□■□■□
                }
                else // Even rows
                {
                    buffer[i] = 0b01010101; // Pattern: □■□■□■□■
                }
            }
            else // Even flash cycles (j=0,2)
            {
                if (i % 2) // Odd rows
                {
                    buffer[i] = 0b01010101; // Pattern: □■□■□■□■
                }
                else // Even rows
                {
                    buffer[i] = 0b10101010; // Pattern: ■□■□■□■□
                }
            }
        }
        // Now display the pattern many times to make it visible
        for (uint16_t k = 0; k < 2000; k++)
        {
            push_buffer(); // Manual display refresh
        }

        // Brief pause between flashes
        _delay_us(50000); // 50ms pause
    }

    // Re-enable Timer0
    TIMSK0 |= (1 << TOIE0);
}

// SNAKE MANAGEMENT

// Clear all snake body segments
void snake_clear_body()
{
    for (uint8_t i = 0; i < 64; i++)
    {
        snake_body[i].x = 0; // Clear X position
        snake_body[i].y = 0; // Clear Y position
    }
}

// Reset game to initial state
void snake_reset()
{
    last_button = 0; // Clear button state

    if (!initialized)
    {
        snake_score(snake_length - 2); // Show final score
    }
    initialized = 0;

    snake_death(); // Show death animation

    clear_buffer(); // Clear display

    snake_length = 2; // Reset to initial length

    snake_clear_body(); // Clear all body segments

    // Initialize snake starting position (2 segments)
    snake_body[0].x = 0b00010000; // Head at column 4
    snake_body[0].y = 0b00001000; // Head at row 3
    snake_body[1].x = 0b00100000; // Tail at column 5
    snake_body[1].y = 0b00001000; // Tail at row 3

    // Initialize rabbit position
    rabbit.x = 0b00000100; // Column 2
    rabbit.y = 0b00001000; // Row 3
    rabbit_update();       // Draw rabbit

    // Update head position variables
    head_x = snake_body[0].x;
    head_y = snake_body[0].y;

    // Draw initial snake
    snake_update_body(head_x, head_y);
    snake_update_body(snake_body[1].x, snake_body[1].y);
}

// Shift snake body segments when moving
void snake_update_body_shift()
{
    struct position *b = &snake_body[0]; // Pointer to snake body array

    // Shift all segments backward (tail follows head)
    for (uint8_t i = snake_length - 1; i > 0; i--)
    {
        b[i] = b[i - 1]; // Copy previous segment position
    }

    // Update head position
    b[0].x = head_x;
    b[0].y = head_y;
}

// Draw snake body segment on display buffer
void snake_update_body(uint8_t x, uint8_t y)
{
    for (uint8_t row = 0; row < 8; row++)
    {
        if (y & (1 << row)) // If this row bit is set in y
        {
            buffer[row] |= x; // Set the x bits in this row
        }
    }
}

// Handle snake eating rabbit
void snake_update_eat()
{
    // Check if snake head overlaps with rabbit
    if ((snake_body[0].x & rabbit.x) && (snake_body[0].y & rabbit.y))
    {
        uint8_t valid_position = 0;

        // Find new rabbit position that doesn't collide with snake
        while (!valid_position)
        {
            // Generate random position using ADC noise
            uint16_t adc6_val = read_adc(6); // Read ADC channel 6
            uint16_t adc7_val = read_adc(7); // Read ADC channel 7

            // Convert ADC values to matrix positions (0-7)
            uint8_t x_pos = adc6_val % 8;
            uint8_t y_pos = adc7_val % 8;

            // Convert positions to bit masks
            rabbit.x = (1 << x_pos);
            rabbit.y = (1 << y_pos);

            // Check if new position is valid (doesn't collide with snake)
            if (!snake_check_collision(rabbit.x, rabbit.y))
            {
                valid_position = 1; // Valid position found
            }
        }
        snake_length++; // Increase snake length
    }
}

// Draw rabbit on display buffer
void rabbit_update()
{
    for (uint8_t row = 0; row < 8; row++)
    {
        if (rabbit.y & (1 << row)) // If this row bit is set in rabbit.y
        {
            buffer[row] |= rabbit.x; // Set the rabbit.x bits in this row
        }
    }
}

// GAME LOGIC
 
// Main game update function
void snake_update(void)
{
    if (last_button) // Only update if a button was pressed
    {
        // Move snake head based on button pressed
        if (last_button & (1 << 0)) // North button
        {
            head_y = head_y << 1; // Shift Y bit left (move up)
        }
        else if (last_button & (1 << 1)) // South button
        {
            head_y = head_y >> 1; // Shift Y bit right (move down)
        }
        else if (last_button & (1 << 2)) // East button
        {
            head_x = head_x >> 1; // Shift X bit right (move right)
        }
        else if (last_button & (1 << 3)) // West button
        {
            head_x = head_x << 1; // Shift X bit left (move left)
        }

        // Check for wall collision (head_x or head_y becomes 0)
        if (!head_x || !head_y)
        {
	    snake_reset(); // Hit wall, reset game
        }
        if (snake_check_collision(head_x, head_y)) // Check for self-collision
        {
            snake_reset(); // Hit self, reset game
        }
        else
        {
            // Valid move - update game state
            clear_buffer(); // Clear display

            snake_update_body_shift(); // Move snake body

            // Draw all snake segments
            for (uint8_t b = 0; b < snake_length; b++)
            {
                snake_update_body(snake_body[b].x, snake_body[b].y);
            }

            snake_update_eat(); // Check if rabbit was eaten

            rabbit_update(); // Draw rabbit
        }
    }
}


void asteroids_push_chunk(uint8_t x, uint8_t y)
{
    for (uint8_t row = 0; row < 8; row++)
    {
        if (y & (1 << row)) // If this row bit is set in y
        {
            buffer[row] |= x; // Set the x bits in this row
        }
    }
}
void asteroids_push(struct asteroid* a)
{
    for(uint8_t c = 0; c < 9; c++)
    {
	asteroids_push_chunk(a->chunks[c].x, a->chunks[c].y);
    }
    
}

uint8_t asteroids_check_collision(struct asteroid* a)
{
    for(uint8_t i = 0; i < 3; i++)
    {
	if(a == &space.asteroids[i])
	{
	    continue;
	}
	    
	if((a->center.x & space.asteroids[i].center.x) &&
	   (a->center.y & space.asteroids[i].center.y))
	{
	    return(1);
	}
    }
    return(0);
}

void asteroids_create()
{
    // (1) pick random spot in 8x8 (center)
    // (2) asteroids are 3x3 (but don't fill the entire 9 cells, center is always filled)
    // (3) pick random number of outer cells to be filled (0-8)

    for(uint8_t i = 0; i < 3; i++)
    {
	struct asteroid* a = &space.asteroids[i];
	
	// Generate random position using ADC noise
	uint16_t adc6_val = read_adc(6); // Read ADC channel 6
	uint16_t adc7_val = read_adc(7); // Read ADC channel 7

	// Convert ADC values to matrix positions (0-7)
	a->center.x = adc6_val  % 8;
	a->center.y = ((adc6_val << 8) >> 8) % 8;
	// Convert positions to bit masks
	a->center.x = (1 << a->center.x);
	a->center.y = (1 << a->center.y);
	a->chunk_count = 1 + (adc7_val % 8);

	if(asteroids_check_collision(a))
	{
	    i--;
	    
	}
	else
	{
	

	    // build asteriod
	    a->chunks[4].x = a->center.x;
	    a->chunks[4].y = a->center.y;
	    if(a->chunk_count > 4)
	    {
		a->chunks[1].x = a->center.x;
		a->chunks[1].y = a->center.y >> 1;

		a->chunks[3].x = a->center.x << 1;
		a->chunks[3].y = a->center.y;

		a->chunks[5].x = a->center.x >> 1;
		a->chunks[5].y = a->center.y;

		a->chunks[7].x = a->center.x;
		a->chunks[7].y = a->center.y << 1;

		switch(a->chunk_count)
		{
		case 8:
		    a->chunks[8].x = a->center.x >> 1;
		    a->chunks[8].y = a->center.y << 1;
		case 7:
		    a->chunks[6].x = a->center.x << 1;
		    a->chunks[6].y = a->center.y << 1;
		case 6:
		    a->chunks[2].x = a->center.x >> 1;
		    a->chunks[2].y = a->center.y >> 1;
		case 5:
		    a->chunks[0].x = a->center.x << 1;
		    a->chunks[0].y = a->center.y >> 1;
		    break;
		}
	    }
	    else
	    {
		switch(a->chunk_count)
		{
		case 3:
		    a->chunks[2].x = a->center.x >> 1;
		    a->chunks[2].y = a->center.y >> 1;
		case 2:
		    a->chunks[1].x = a->center.x;
		    a->chunks[1].y = a->center.y >> 1;
		case 1:
		    a->chunks[5].x = a->center.x >> 1;
		    a->chunks[5].y = a->center.y;
		    break;
		}
	    }

	    asteroids_push(a);
	}
    }

    
    
    push_buffer();
}

void asteroids_spaceship(uint8_t x, uint8_t y)
{
    for (uint8_t row = 0; row < 8; row++)
    {
        if (y & (1 << row)) // If this row bit is set in y
        {
            buffer[row] |= x; // Set the x bits in this row
        }
    }
}

int main(void)
{
    // Configure GPIO ports
    DDRB = 0xFF; // Port B all outputs (LED columns)
    DDRD = 0xFF; // Port D all outputs (LED rows)

    // Configure button inputs with pull-up resistors
    DDRC &= ~((1 << 2) | (1 << 3) | (1 << 4) | (1 << 5)); // Set as inputs
    PORTC |= (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);   // Enable pull-ups

    // Configure ADC for random number generation
    ADMUX = (1 << REFS0);                                              // Use AVcc as reference voltage
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC, prescaler 128

    // Configure Timer1 for game logic timing
    TCCR1B = (1 << CS11) | (1 << CS10); // Clock/64 prescaler
    TIMSK1 = (1 << TOIE1);              // Enable overflow interrupt

    // Configure Timer0 for display refresh timing
    TCCR0B = (1 << CS00);  // Clock/1 prescaler (fastest)
    TIMSK0 = (1 << TOIE0); // Enable overflow interrupt

    sei(); // Enable global interrupts

    
    // Initialize game
    if(selected_game == 0)
    {
	snake_reset();
    }
    else
    {
	asteroids_create();
    }

    // Main game loop
    while (1)
    {
        wait_button(); // Continuously check for button presses
    }
}



