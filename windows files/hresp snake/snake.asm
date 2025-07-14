;========================================================================
; Notice: (C) Copyright 2025 by Kat Mwenesongole. All Rights Reserved. 
;========================================================================

; reset
sbi PORTC, 0    
.equ BUTTON_MASK = 0b00000001   ; PC0

 in   r25, DDRC
 andi r25, 0b11000011
 out  DDRC, r25

jmp start
start:
	jmp wait_button
	jmp start

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

; tools

clear:
    clr r23 
	clr r22 
	clr r21
	clr r20
	clr r19
	clr r18
	clr r17
	clr r16
	ret

button_north:
	call clear
	ldi r23, 0x10
	rjmp wait_release

button_south:
    call clear
	ldi r16, 0x10
	rjmp wait_release

button_east:
    call clear
	ldi r20, 0x01
	rjmp wait_release

button_west:
    call clear
	ldi r20, 0x80
	rjmp wait_release

wait_release:
    in   r24, PINC
    andi r24, BUTTON_MASK
    cpi  r24, 0           ; still pressed?
    breq wait_release   
	jmp start

wait_button:
	in   r24, PINC

    sbrs r24, 2
    rjmp button_south
    sbrs r24, 3
    rjmp button_east
    sbrs r24, 4
    rjmp button_north
    sbrs r24, 5
    rjmp button_west

	jmp start


