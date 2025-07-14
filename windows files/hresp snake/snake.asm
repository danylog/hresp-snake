;========================================================================
; Notice: (C) Copyright 2025 by Kat Mwenesongole. All Rights Reserved. 
;========================================================================

; definitions
.def STRUCT_X = r29
.def STRUCT_Y = r28

.def STORAGE = r25
.def HEAD_X  = r2 ; 0000 0000
.def HEAD_Y  = r4 ; 0000 0000
.def BUTTONS = r24

; reset
sbi PORTC, 0   
sbi PORTC, 2
sbi PORTC, 3
sbi PORTC, 4
sbi PORTC, 5 

ldi STRUCT_Y, 0b00000001
ldi STRUCT_X, 0b00010000

																								
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
	jmp start
update_row1:
	mov r22, HEAD_X
	jmp start
update_row2:
	mov r21, HEAD_X
	jmp start
update_row3:
	mov r20, HEAD_X
	jmp start
update_row4:
	mov r19, HEAD_X
	jmp start
update_row5:
	mov r18, HEAD_X
	jmp start
update_row6:
	mov r17, HEAD_X
	jmp start
update_row7:
	mov r16, HEAD_X
	jmp start

update:
    call clear
	
	mov STRUCT_X, HEAD_X  
	mov STRUCT_Y, HEAD_Y

	sbrc HEAD_Y, 0
	jmp update_row0
    sbrc HEAD_Y, 1
	jmp update_row1
	sbrc HEAD_Y, 2
	jmp update_row2
	sbrc HEAD_Y, 3
	jmp update_row3
	sbrc HEAD_Y, 4
	jmp update_row4
	sbrc HEAD_Y, 5
	jmp update_row5
	sbrc HEAD_Y, 6
	jmp update_row6
	sbrc HEAD_Y, 7
	jmp update_row7
	jmp start

button_north_wait:
    in BUTTONS, PINC
    sbrs BUTTONS, 4         
    jmp button_north_wait
    jmp update
button_south_wait:
    in BUTTONS, PINC
    sbrs BUTTONS, 2       
    jmp button_south_wait
    jmp update
button_east_wait:
    in BUTTONS, PINC
    sbrs BUTTONS, 3       
    jmp button_east_wait
    jmp update
button_west_wait:
    in BUTTONS, PINC
    sbrs BUTTONS, 5       
    jmp button_west_wait
    jmp update

button_north:
    ror HEAD_Y
	jmp button_north_wait
button_south:
    rol HEAD_Y
	jmp button_south_wait
button_east:
    ror HEAD_X
	jmp button_east_wait
button_west:
    rol HEAD_X
	jmp button_west_wait

wait_button:
    mov HEAD_Y, STRUCT_Y 
	mov HEAD_X, STRUCT_X

	;mov r18, HEAD_Y ;test
	;mov r17, HEAD_X ;test

	in   BUTTONS, PINC
    sbrs BUTTONS, 2
    rjmp button_south
    sbrs BUTTONS, 3
    rjmp button_east
    sbrs BUTTONS, 4
    rjmp button_north
    sbrs BUTTONS, 5
    rjmp button_west
	ret