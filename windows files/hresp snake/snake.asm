;========================================================================
; Notice: (C) Copyright 2025 by Kat Mwenesongole. All Rights Reserved. 
;========================================================================

; definitions
sei

.equ SRAM_STRUCT_X = 0x0100
.equ SRAM_STRUCT_Y = 0x0101

.def HEAD_X  = r2 ; 0000 0000
.def HEAD_Y  = r4 ; 0000 0000

.def STORAGE     = r26
.def LAST_BUTTON = r25
.def BUTTONS     = r24

; reset
sbi PORTC, 0   
sbi PORTC, 2
sbi PORTC, 3
sbi PORTC, 4
sbi PORTC, 5 

ldi STORAGE, 0b00010000
sts SRAM_STRUCT_X, STORAGE
sts SRAM_STRUCT_Y, STORAGE
																								
jmp start
start:
	call wait_button
	jmp start

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

update_row0:
	mov r23, HEAD_X
	ret
update_row1:
	mov r22, HEAD_X
	ret
update_row2:
	mov r21, HEAD_X
	ret
update_row3:
	mov r20, HEAD_X
	ret
update_row4:
	mov r19, HEAD_X
	ret
update_row5:
	mov r18, HEAD_X
	ret
update_row6:
	mov r17, HEAD_X
	ret
update_row7:
	mov r16, HEAD_X
	ret

update:
    call clear
	
	sts SRAM_STRUCT_X, HEAD_X  
	sts SRAM_STRUCT_Y, HEAD_Y

	sbrc HEAD_Y, 0
	call update_row0
    sbrc HEAD_Y, 1
	call update_row1
	sbrc HEAD_Y, 2
	call update_row2
	sbrc HEAD_Y, 3
	call update_row3
	sbrc HEAD_Y, 4
	call update_row4
	sbrc HEAD_Y, 5
	call update_row5
	sbrc HEAD_Y, 6
	call update_row6
	sbrc HEAD_Y, 7
	call update_row7
	ret

button_north_wait:
    in BUTTONS, PINC
    sbrs BUTTONS, 4         
    jmp button_north_wait
    call update
	ret
button_south_wait:
    in BUTTONS, PINC
    sbrs BUTTONS, 2       
    jmp button_south_wait
    call update
	ret
button_east_wait:
    in BUTTONS, PINC
    sbrs BUTTONS, 3       
    jmp button_east_wait
    call update
	ret
button_west_wait:
    in BUTTONS, PINC
    sbrs BUTTONS, 5       
    jmp button_west_wait
    call update
	ret
	
button_north:
    ror HEAD_Y
	ldi LAST_BUTTON, 0b00000001
	call button_north_wait
	ret
button_south:
    rol HEAD_Y
	ldi LAST_BUTTON, 0b00000010
	call button_south_wait
	ret
button_east:
    ror HEAD_X
	ldi LAST_BUTTON, 0b00000100
	call button_east_wait
	ret
button_west:
    rol HEAD_X
	ldi LAST_BUTTON, 0b00001000
	call button_west_wait
	ret

wait_button:
    lds HEAD_Y, SRAM_STRUCT_Y 
	lds HEAD_X, SRAM_STRUCT_X

	;mov r18, HEAD_Y ;test
	;mov r17, HEAD_X ;test

	in   BUTTONS, PINC
    sbrs BUTTONS, 2
    call button_south
    sbrs BUTTONS, 3
    call button_east
    sbrs BUTTONS, 4
    call button_north
    sbrs BUTTONS, 5
    call button_west
	ret

button_north_force:
	ror HEAD_Y
	ret
button_south_force:
	rol HEAD_Y
	ret
button_east_force:
	ror HEAD_X
	ret
button_west_force:
    rol HEAD_X
	ret

snake_update:
	sbrc LAST_BUTTON, 0 ; north
	call button_north_force
    sbrc LAST_BUTTON, 1 ; south
	call button_south_force
	sbrc LAST_BUTTON, 2 ; east
	call button_east_force
	sbrc LAST_BUTTON, 3 ; west
	call button_west_force
	call update
	ret