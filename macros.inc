PRTTEXT .MACRO text,xpos,ypos,length
        lda #(xpos+<$8000)      ;X=11 Y=13
        sta tmp
        lda #(ypos+>$8000)
        sta tmp+1
        ldy #0

	ldx #0       
dotitl#
        lda title,x      ;Title text is just bitmap saying 'PRESS FIRE'
	sta (tmp),y
        tya
        clc
        adc #$20
        tay
        bne noinc#
        inc tmp 
noinc#
 	inx
	cpx #(length*8)
	bne dotitl#
.MACEND       

PLOTSPRITE .MACRO nr,xpos,ypos
	lda #nr       
	ldx #xpos
	ldy #ypos
	jsr drawspr
.MACEND
