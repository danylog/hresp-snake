#include <avr/interrupt.h>
#include <util/delay.h>

volatile uint8_t fb[8] = {0};

volatile uint8_t head_x = 0x10; // Start middle
volatile uint8_t head_y = 0x10;

volatile uint8_t last_button = 0;

void display_matrix(void) {
    for (uint8_t row = 0; row < 8; row++) {
        uint8_t row_data = fb[row];

        row_data = ~row_data;

        uint8_t rev = 0;
        for (uint8_t i = 0; i < 8; i++) {
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

// === Snake update ===
void snake_update(void) {
    if (last_button & (1<<0)) head_y = (head_y >> 1) | (head_y << 7); // north (ROR)
    if (last_button & (1<<1)) head_y = (head_y << 1) | (head_y >> 7); // south (ROL)
    if (last_button & (1<<2)) head_x = (head_x >> 1) | (head_x << 7); // east
    if (last_button & (1<<3)) head_x = (head_x << 1) | (head_x >> 7); // west

    for (uint8_t i=0; i<8; i++) fb[i] = 0x00;

    for (uint8_t row=0; row<8; row++) {
        if (head_y & (1<<row)) {
            fb[row] = head_x;
        }
    }
}

void wait_button(void) {
    uint8_t btns = PINC;

    if (!(btns & (1<<2))) { // south
        head_y = (head_y << 1) | (head_y >> 7);
        last_button = (1<<1);
    }
    if (!(btns & (1<<3))) { // east
        head_x = (head_x >> 1) | (head_x << 7);
        last_button = (1<<2);
    }
    if (!(btns & (1<<4))) { // north
        head_y = (head_y >> 1) | (head_y << 7);
        last_button = (1<<0);
    }
    if (!(btns & (1<<5))) { // west
        head_x = (head_x << 1) | (head_x >> 7);
        last_button = (1<<3);
    }
}

// === Timer interrupts ===

ISR(TIMER0_OVF_vect) {
    display_matrix();
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

    while (1) {
        wait_button(); 
    }
}
