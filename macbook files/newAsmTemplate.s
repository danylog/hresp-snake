.global main
.equ DDRC, 0x07
.equ DDRD,  0x0A
.equ DDRB,  0x04
.equ PORTD, 0x0B
.equ PORTB, 0x05
.equ PORTC, 0x08
.equ PINC, 0x06

main:
        in   r24, DDRC
        andi r24, 0b11000011    ; 0 = input for bits 2–5
        out  DDRC, r24

        ; Optionally enable pull-ups on PC2–PC5
        in   r24, PORTC
        ori  r24, 0b00111100    ; 1 = pull-up enabled for bits 2–5
        out  PORTC, r24
        ; Set rows/columns as output
        ldi r16, 0xFF
        out DDRD, r16
        out DDRB, r16

        ; Set only row 3 HIGH (others LOW)
        ldi r17, 0x00
        sbr r17, (1 << 3)    ; PD3 = 1, rest = 0
        out PORTD, r17

        ; Set columns: all HIGH except current column LOW
        ldi r18, 0xFF
        com r18
        ldi r22, 1
        mov r23, r19

; Set rows: all LOW except current row HIGH


        ; Set columns: all HIGH except column 0 LOW
        ldi r18, 0xFE        ; PB0 = 0, rest = 1
        ; r18 will be our columns byte

        ldi r19, 0x00        ; Current column bit index (0..7)
        ldi r26, 0x00        ; Start at row 3 (change to your preferred row)



add_pixel_loop:
        ; --- Compute column mask: all HIGH except selected column LOW ---
        ldi r18, 0xFF              ; Start with all columns HIGH
        ldi r22, 1                 ; Bit mask for column
        mov r23, r19               ; Copy column index
shift_col:
        tst r23
        breq col_done
        lsl r22
        dec r23
        rjmp shift_col
col_done:
        com r22                    ; Invert mask: only selected bit is 0
        and r18, r22               ; Set only selected column LOW
        out PORTB, r18             ; Output to columns

        ; --- Compute row mask: all LOW except selected row HIGH ---
        ldi r17, 0x00              ; Start with all rows LOW
        ldi r21, 1                 ; Bit mask for row
        mov r20, r26               ; Copy row index
shift_row:
        tst r20
        breq row_done
        lsl r21
        dec r20
        rjmp shift_row
row_done:
        or r17, r21                ; Set only selected row HIGH
        out PORTD, r17             ; Output to rows

        ; --- Wait for button press ---
        rjmp wait_button

wait_button:
        in   r25, PINC           ; Read input pins

        ; Check PC2 (bit 2)
        sbrs r25, 2              ; Skip next if bit 2 is set (not pressed)
        rjmp button_pc2_pressed

        ; Check PC3 (bit 3)
        sbrs r25, 3
        rjmp button_pc3_pressed

        ; Check PC4 (bit 4)
        sbrs r25, 4
        rjmp button_pc4_pressed

        ; Check PC5 (bit 5)
        sbrs r25, 5
        rjmp button_pc5_pressed

        rjmp wait_button         ; No button pressed, keep waiting

button_pc3_pressed:
        inc r19                      ; Increment column index (r19 = r19 + 1)
        mov r23, r19
        rjmp wait_release

button_pc5_pressed:
        dec r19                      ; Increment column index (r19 = r19 + 1)
        mov r23, r19
        ; --- handle PC3 button press here ---
        rjmp wait_release

button_pc4_pressed:
        inc r26
        mov r27, r26
        ; --- handle PC4 button press here ---
        rjmp wait_release

button_pc2_pressed:
        dec r26
        mov r27, r26
        ; --- handle PC5 button press here ---
        rjmp wait_release            ; Jump to wait for button release

wait_release:
    in   r25, PINC
    andi r25, 0b00111100
    cpi  r25, 0b00111100
    brne wait_release

    rjmp add_pixel_loop          ; Jump back to main loop to update display

done:
        rjmp done                    ; Infinite loop (program finished)