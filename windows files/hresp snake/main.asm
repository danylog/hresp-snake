;========================================================================
; Notice: (C) Copyright 2025 by Kat Mwenesongole. All Rights Reserved. 
;========================================================================

.include "m328pdef.inc"

.def REG_8 = r16
.def REG_7 = r17
.def REG_6 = r18
.def REG_5 = r19
.def REG_4 = r20
.def REG_3 = r21
.def REG_2 = r22
.def REG_1 = r23

.def tmp  = r24
.def tmp2 = r25
.def tmp3 = r26

.org 0x0000 jmp RESET
.org 0x001A jmp TIMER_OVF1
.org 0x0020 jmp TIMER_OVF0

// Display function (draws the contents of r0 to r7 to the screen)
display:
	push tmp			; Save registers to stack
	push tmp2
	push tmp3
	ldi tmp, 0			; Initialize Z-Pointer at r16
	mov ZH,tmp
	ldi tmp, 16
	mov ZL,tmp
	ldi tmp, 1			; Start row selector at bottom (LSB of PORTD)
next_line:
	ld tmp2, Z			; Load row data from Register (0-7)
	com tmp2			; Invert row data due to P-MOS (Active-LOW)
	ldi tmp3,0			; Invert bit order MSB - LSB
	lsl tmp2
	ror tmp3
	lsl tmp2
	ror tmp3
	lsl tmp2
	ror tmp3
	lsl tmp2
	ror tmp3
	lsl tmp2
	ror tmp3
	lsl tmp2
	ror tmp3
	lsl tmp2
	ror tmp3
	lsl tmp2
	ror tmp3
	out PORTB,tmp3		; Push out row data (to PORTB)
	out PORTD,tmp		; Push out row selector
	rcall wait			; Wait for drawing (LED now on)
	ldi tmp2,0			; Clear row selector
	out PORTD, tmp2
	rcall wait			; Wait for clearing (LED now off)
	lsl tmp				; Rotate row selector to next row (moving LSB to MSB)
	inc ZL				; Increase pointer to next row data
	cpi ZL,24			; Check if all 8 rows were displayed
	brne next_line		; Do next line
	pop tmp3			; Restore data
	pop tmp2			
	pop tmp
	ret

// Wait function (waits a defined time)
wait:
	push tmp
	ldi tmp,100			; initialize counter
wait1:
	dec tmp				; decrease counter
	brne wait1			; repeat if counter not zero
	pop tmp
	ret

// Timer interrupt
TIMER_OVF1:
	rcall snake_update
	; com STORAGE      ;test
	; mov r17, STORAGE ;test
	reti

// 8-Bit Timer 0 OVerFlow Interrupt -- validated
TIMER_OVF0:
	rcall display
	reti
	
RESET:
	// Stack initialization -- validated
	ldi	tmp,LOW(RAMEND)		
	out	SPL,tmp				
	ldi	tmp,HIGH(RAMEND)	
	out	SPH,tmp	

	// Port B and D as LED Output -- validated
	ldi tmp, 0b11111111
	out DDRB, tmp
	out DDRD, tmp

	// Button pins as input with pullup LED pin as output -- validated
	ldi tmp,0b00000010
	out DDRC, tmp
	ldi tmp, 0b00111100
	out PORTC, tmp

	// Initialize 16-Bit Counter1 for OVF interrupt
	ldi tmp, 0b00000011
	sts TCCR1B, tmp
	ldi tmp, 0b00000001
	sts TIMSK1, tmp

	// Initialize  8-Bit Counter0 for OVF interrupt with 512 prescaler -- validated
	ldi tmp, 0b00000001
	out TCCR0B, tmp
	ldi tmp, 0b00000001
	sts TIMSK0, tmp	
	sei
	
// TEST
//.include "test.asm"

// SNAKE
.include "snake.asm"




