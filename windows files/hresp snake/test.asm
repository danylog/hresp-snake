;========================================================================
; Notice: (C) Copyright 2025 by Kat Mwenesongole. All Rights Reserved. 
;========================================================================

; Each bit      corresponds to a LED in the 8x8 LED array. 
; Each register corresponds to a row in the 8x8 LED array.
;
; right to left   => (0x01) - (0x80)
; top   to bottom => r23 - r16
;

; SMILE TEST

 ldi r22, 0x66 
 ldi r21, 0x66
 ldi r19, 0x42
 ldi r18, 0x24
 ldi r17, 0x18
