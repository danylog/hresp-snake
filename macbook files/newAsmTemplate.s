.global main
.equ DDRC, 0x07
.equ DDRD,  0x0A
.equ DDRB,  0x04
.equ PORTD, 0x0B
.equ PORTB, 0x05
.equ PORTC, 0x08
.equ PINC, 0x06

main:
    ; Configure PC2–PC5 as input, others as output
    in   r24, DDRC
    andi r24, 0b11000011
    out  DDRC, r24

    ; Enable pull-ups on PC2–PC5
    in   r24, PORTC
    ori  r24, 0b00111100
    out  PORTC, r24

    ; Set all rows and columns as output
    ldi  r16, 0xFF
    out  DDRD, r16
    out  DDRB, r16

    ; Initialize: row 3 HIGH, others LOW
    ldi  r17, 0x08           ; (1 << 3)
    out  PORTD, r17

    ; Initialize column and row indices
    ldi  r19, 0x00           ; Column index (0..7)
    ldi  r26, 0x00           ; Row index (start at 0)

add_pixel_loop:
    ; --- Set columns: all HIGH except selected column LOW ---
    ldi  r18, 0xFF           ; All columns HIGH
    ldi  r22, 1              ; Bit mask for column
    mov  r23, r19            ; Copy column index
shift_col:
    tst  r23
    breq col_done
    lsl  r22
    dec  r23
    rjmp shift_col
col_done:
    com  r22                 ; Only selected bit is 0
    and  r18, r22
    out  PORTB, r18

    ; --- Set rows: all LOW except selected row HIGH ---
    ldi  r17, 0x00
    ldi  r21, 1              ; Bit mask for row
    mov  r20, r26            ; Copy row index
shift_row:
    tst  r20
    breq row_done
    lsl  r21
    dec  r20
    rjmp shift_row
row_done:
    or   r17, r21
    out  PORTD, r17

    ; --- Wait for button press ---
wait_button:
    in   r25, PINC

    sbrs r25, 2
    rjmp button_pc2_pressed
    sbrs r25, 3
    rjmp button_pc3_pressed
    sbrs r25, 4
    rjmp button_pc4_pressed
    sbrs r25, 5
    rjmp button_pc5_pressed

    rjmp wait_button

button_pc3_pressed:
    inc  r19                 ; Next column
    rjmp wait_release

button_pc5_pressed:
    dec  r19                 ; Previous column
    rjmp wait_release

button_pc4_pressed:
    inc  r26                 ; Next row
    rjmp wait_release

button_pc2_pressed:
    dec  r26                 ; Previous row
    rjmp wait_release

wait_release:
    in   r25, PINC
    andi r25, 0b00111100
    cpi  r25, 0b00111100
    brne wait_release

    rjmp add_pixel_loop

done:
    rjmp done