;----------------------------------------------------------------------------------------
;JET SET MINER (formally PLAT 3064)
;
;Written by Tom Walker
;Original in 2003 (for Minigame 2003 4K competition) for C64
;BBC/Electron conversion in 2009
;Atom conversion in 2010 by Kees van Oss
;
;To be assembled with P65 and probably Swift
;
;I was 15 when I wrote this, so don't complain about the horrible coding
;This was written to fit in 4K, however some of the coding seems a bit 'big'
;Could probably be improved. If I'd made the more obvious size optimisations,
;it could have had twice as many levels as it does and still been 4K.
;
;The entire game has been made OS legal. This means it runs on any BBC or Electron
;with 32k of RAM. Older versions weren't OS legal, this meant they could run on
;a Model A.
;
;Suggestions for improvements :
;
; - More levels (obv). You could fit ~50 levels into an extra 4k.
; - Move enemies (would need an extra entry in the enemy tables, unless it's one
;   type per level)
; - More types of block per level. This would reduce the efficiency of the compression. 
; - Colour. Mode 5 should be easy, though some changes would be needed to collision
;   detection. Mode 1 or 2 would need more changes to the drawing routines
; - Sound. Should be fairly easy.  
;----------------------------------------------------------------------------------------


gamedata:
SCANKEY  equ $FE71
WAITSYNC equ $FE66

TMPVAR   equ $80
WORKSPC  equ $C0
VARS     equ $D0

;Game variables
PLX      equ WORKSPC    ;Player position
PLY      equ WORKSPC+1
PLF      equ WORKSPC+2  ;Player falling?
PLD      equ WORKSPC+3  ;Direction player is facing
PLJ      equ WORKSPC+4  ;Player jumping?
KEYSLEFT equ WORKSPC+5  ;Number of keys left in level
COLLIDE  equ WORKSPC+6  ;Collision detected?
CHKCOL   equ WORKSPC+7  ;Check collisions when drawing sprites
LEVEL    equ WORKSPC+8  ;Level number (0-3)
DEAD     equ WORKSPC+9  ;Player dead?
LEVCOMP  equ WORKSPC+$A ;Level completed?
LIVES    equ WORKSPC+$B ;Player lives left
LCOMP    equ WORKSPC+$C ;Current level (not limited to 0-3)
VSYNCCNT equ WORKSPC+$F ;Vsync count

LEVELOVR equ VARS+4     ;Level over?
TIME     equ VARS+5     ;Time left (2)
TIMEL    equ VARS+7     ;Time limit for current round of levels
tmp      equ VARS+8     ; (2)

;lev1     equ $400       ;Level data is decompressed to here
;ENMY     equ $900       ;Byte 0 - X pos  Byte 1 - Y pos  Byte 2 - X min  Byte 3 - X max  Byte 4 - direction

;--- Main routine ---

main:
        lda #0           ;Clear game variables
        ldx #15
l1      sta WORKSPC,x
        dex
        bpl l1
        
        sta LEVELOVR     ;Reset level over flag
	jsr clear        ;Clear screen

;Set up title text
        lda #<$8d0b      ;X=11 Y=13
        sta tmp
        lda #>$8d0b
        sta tmp+1
        ldy #0

	ldx #0       
dotitl:
        lda title,x      ;Title text is just bitmap saying 'PRESS FIRE'
	sta (tmp),y
        tya
        clc
        adc #$20
        tay
        bne noinc
        inc tmp 
noinc:
 	inx
	cpx #80
	bne dotitl

;Draw player sprites on title screen
	lda #0       
	ldx #0
	ldy #0
	jsr drawspr
	lda #0
	ldx #18
	ldy #0
	jsr drawspr
	lda #0
	ldx #36
	ldy #0
	jsr drawspr
	lda #0
	ldx #54
	ldy #0
	jsr drawspr

	lda #1
	ldx #240
	ldy #0
	jsr drawspr
	lda #1
	ldx #222
	ldy #0
	jsr drawspr
	lda #1
	ldx #204
	ldy #0
	jsr drawspr
	lda #1
	ldx #186
	ldy #0
	jsr drawspr

wait:
        lda $b001        ;Wait for 'SHIFT'
        and #$80
	bne wait

;Start game level 0
init:
        lda #0           ;Start on level 0
	sta LEVEL
	sta LCOMP
	lda #3           ;3 lives
	sta LIVES
	lda #9           ;Time starts at 999
	sta TIMEL
dothelv:
        lda #0           ;Not dead!
	sta DEAD
	lda #$99         ;Set time to (TIMEL*100)+99
	sta TIME
	lda TIMEL
	sta TIME+1
	sta LEVCOMP

	jsr start        ;Run level

	cmp #1           ;Did we die?
	beq dodie        ;Then jump to death code
nxtlev:
       	jmp dothelv      ;Otherwise run next level

dodie:  dec LIVES        ;Decrease our lives
	bmi endgame      ;End the game if out
	jmp nxtlev       ;Otherwise start level again

endgame:
        jmp main         ;Restart game if out of lives

;--- Start level ---

start:
        jsr clear        ;Clear screen

        lda #0
        sta LEVELOVR
        
	lda LEVEL        ;Get level number
	asl a
	asl a
	tax
	stx TMPVAR
	lda levtab+1,x   ;Put address into X,Y
	tay
	lda levtab,x
	tax
	jsr decomp       ;And decompress

	ldx TMPVAR
	lda levtab+3,x
	tay
	lda levtab+2,x
	tax
	jsr dolh         ;Copy level header to current level
        jsr drawlev      ;Draw level

	jsr mkdispb      ;Make status text ('display bottom')

        ldx #0
enmyl1:
        lda levhead+10,x ;Copy enemy data from level header into enemy structure
        sta ENMY,x
        inx
        cpx #20
        bne enmyl1

        lda #4           ;Reset number of keys left
        sta KEYSLEFT
	lda #0
	sta COLLIDE

        ldx #8           ;Player always starts in same position
        stx PLX
        ldy #136
        sty PLY
        lda #0           ;Neither dropping nor jumping
        sta PLD
        sta PLJ
        jsr drawspr      ;Place initial player sprite on screen (at X,Y)
        
        ldx ENMY         ;Get first enemy coordinates
        ldy ENMY+1
        lda ENMY+4       ;Get enemy sprite number
	cmp #$ff         ;Does enemy actually exist?
	beq en21         ;Jump if not
        ora #2        
        jsr drawspr      ;Otherwise draw
en21:
        ldx ENMY+5       ;Repeat for other enemies (could do with a loop)
        ldy ENMY+6
        lda ENMY+9
	cmp #$ff
	beq en31
        ora #2
        jsr drawspr
en31:
        ldx ENMY+10
        ldy ENMY+11
        lda ENMY+14
	cmp #$ff
	beq en41
        ora #2
        jsr drawspr
en41:
        ldx ENMY+15
        ldy ENMY+16
        lda ENMY+19
	cmp #$ff
	beq en51
        ora #2
        jsr drawspr
en51:
        lda #0           ;Disable collision detection
	sta CHKCOL
loop:
        jsr vsync        ;Wait 5 vsyncs (locks to 10 fps - fairly arbitary)
        lda PLD
        ldx PLX
        ldy PLY
        jsr drawspr      ;Erase player
        ldx ENMY
        ldy ENMY+1
        lda ENMY+4
	cmp #$ff
	beq en22
        ora #2
        jsr drawspr      ;Erase enemies
en22:
        ldx ENMY+5
        ldy ENMY+6
        lda ENMY+9
	cmp #$ff
	beq en32
        ora #2
        jsr drawspr
en32:
        ldx ENMY+10
        ldy ENMY+11
        lda ENMY+14
	cmp #$ff
	beq en42
        ora #2
        jsr drawspr
en42:
        ldx ENMY+15
        ldy ENMY+16
        lda ENMY+19
	cmp #$ff
	beq en52
        ora #2
        jsr drawspr
en52:
        jsr guymove     ;Check input and move player
	jsr chkspk      ;See if we hit any spikes
        jsr doenemy     ;Move enemies
        lda PLD
        ldx PLX
        ldy PLY
        jsr drawspr     ;Draw player
	lda #1          ;Re-enable collision detection
	sta CHKCOL
        ldx ENMY        ;Draw enemies
        ldy ENMY+1
        lda ENMY+4
	cmp #$ff
	beq en2
        ora #2
        jsr drawspr
en2:
        ldx ENMY+5
        ldy ENMY+6
        lda ENMY+9
	cmp #$ff
	beq en3
        ora #2
        jsr drawspr
en3:
        ldx ENMY+10
        ldy ENMY+11
        lda ENMY+14
	cmp #$ff
	beq en4
        ora #2
        jsr drawspr
en4:
        ldx ENMY+15
        ldy ENMY+16
        lda ENMY+19
	cmp #$ff
	beq en5
        ora #2
        jsr drawspr
en5:
        lda #0          ;Disable collision detection
	sta CHKCOL
	lda COLLIDE     ;And test results of previous checks
	bne die         ;Jump if a collision - ie monster collided with player
        jsr chkover     ;Check if level completed
	lda DEAD        ;Redundant? die: should sort this
	bne endital
	lda LEVCOMP     ;If LEVCOMP=FF then we've completed the level, exit loop
	eor #$ff        ;This ensures A=0 when we exit
	beq endital
	jsr updb        ;Update status ('display bottom')
	jsr uptime      ;Update time
        jmp loop        ;Jump back to start

endital:
        rts

die:
        lda #1          ;Set DEAD flag and return (A=1)
	sta DEAD
	rts

chkover:
        lda LEVELOVR    ;If level is over
        bne nextlev     ;Then enter next one
        rts

chkexit:
        rts             ;Redundant - needed in C64 version

nextlev:
        lda #0          ;Reset completion flags
        sta LEVELOVR
        lda #$ff
	sta LEVCOMP
	sed
	lda LCOMP       ;Increment level number (used in status display)
	clc
	adc #1
	sta LCOMP
	cld
	inc LEVEL       ;Increment actual level number (AND 3 cos there are 4 levels)
	lda LEVEL
	and #3
	sta LEVEL
	beq chgtime     ;If back to level 0 then we need to change the time limit
	rts

chgtime:
        dec TIMEL       ;Decrease time limit
	rts             ;No checks as when time gets to 099 no-one will be able to complete a level

;makexit - puts an exit in the level
makexit:
        lda levhead+8   ;Get exit position from level header
        sta  TMPVAR+6
        lda levhead+9
        clc
        adc #$80
        sta TMPVAR+7
        
        ldy #0
        ldx #0          ;Copy top half of door sprite onto screen
l2:
        lda doorspr,x
        sta (TMPVAR+6),y
        inx
        tya
        clc
        adc #$20
        tay
        lda TMPVAR+7
        adc #0
        sta TMPVAR+7
        cpx #16
        bne l2
        
        inc TMPVAR+6    ;Move down one character line
        dec TMPVAR+7
        dec TMPVAR+7

        ldy #0
        ldx #0          ;And copy bottom half
l3: 
        lda doorspr+16,x
        sta (TMPVAR+6),y
        inx
        tya
        clc
        adc #$20
        tay
        lda TMPVAR+7
        adc #0
        sta TMPVAR+7
        cpx #16
        bne l3
        
        lda levhead+8   ;Use door position to lookup level data
        sta TMPVAR+2
        lda levhead+9
        sta TMPVAR+3
        lda #0

        jsr savespc

        ldy TMPVAR+2
        lda #$80        ;Store $80 at locations door occupies
        sta (TMPVAR+3),y;Once all keys are gone, scanning rountine will see this as exit
        iny
        sta (TMPVAR+3),y
        lda TMPVAR+3
        clc
        adc #32
        sta TMPVAR+3
        dey
        lda #$80        
        sta (TMPVAR+3),y
        iny
        sta (TMPVAR+3),y                

        rts

chkspk:
        lda PLX         ;Get player location
	lsr a           ;And shift down to use as level index
	lsr a
	lsr a
	tax
	lda PLY
	lsr a
	lsr a
	lsr a
	tay
	stx TMPVAR+$e   ;Store
	sty TMPVAR+$f
	jsr getblk1     ;Get block
	cmp #3          ;Is is 3? (spike)
	beq hitspk      ;Jump if we hit
	ldx TMPVAR+$e   ;Repeat for all surrounding blocks
	ldy TMPVAR+$f
	inx
	jsr getblk1
	cmp #3
	beq hitspk
	ldx TMPVAR+$e
	ldy TMPVAR+$f
	iny
	jsr getblk1
	cmp #3
	beq hitspk
	ldx TMPVAR+$e
	ldy TMPVAR+$f
	inx
	iny
	jsr getblk1
	cmp #3
	beq hitspk
	ldx TMPVAR+$e
	ldy TMPVAR+$f
	iny
	iny
	sty TMPVAR+$f
	jsr getblk1
	cmp #3
	beq hitspk
	ldx TMPVAR+$e
	ldy TMPVAR+$f
	inx
	jsr getblk1
	cmp #3
	beq hitspk
	rts

hitspk:
        jmp die        ;If you hit a spike you die!

doenemy:
        lda #<ENMY     ;Get enemy data address into $fe (for indirect Y mode)
        sta VARS
        lda #>ENMY
        sta VARS+1
        ldx #0
enmyl2:
        ldy #4         ;Get index 4 (direction)
        lda (VARS),y
	cmp #$ff
	beq enmynxt    ;If $FF then enemy doesn't exist
	lda (VARS),y
        bne enemyl     ;If it's not 0 then it's left

enemyr:
        ldy #0         ;Else it's right
        lda (VARS),y   ;Get index 0 (X position)
        clc
        adc #2         ;Move him right
        sta (VARS),y
        ldy #3         ;Get index 3 (rightmost position)
        cmp (VARS),y 
        bne enmynxt    ;If enemy isn't there then jump
        ldy #4         ;Otherwise reverse direction
        lda #1
        sta (VARS),y
        jmp enmynxt

enemyl:
        ldy #0         ;Get index 0 (X position)
        lda (VARS),y
        sec
        sbc #2         ;Move him left
        sta (VARS),y
        ldy #2         ;Get index 2 (leftmost position)
        cmp (VARS),y
        bne enmynxt    ;If enemy isn't there then jump
        ldy #4         ;Overwise reverse direction
        lda #0
        sta (VARS),y

enmynxt:
        lda VARS       ;Move $FE pointer on 5 bytes (next enemy)
	clc
	adc #5
	sta VARS
	inx
        cpx #5        ;Are we at 20 (4 enemies * 5 bytes each)?
	bne enmyl2     ;No, then jump to top of loop
	rts            ;Otherwise return

drawlev:
        lda #<lev1     ;Put level address into 2/3
        sta TMPVAR+2
        lda #>lev1
        sta TMPVAR+3

        lda #20        ;Number of rows to draw (20)
        sta TMPVAR+8
        lda #$80       ;Starting screen address (4 columns from start of screen)
        sta TMPVAR+5
        lda #0
        sta TMPVAR+4

drawvl:
        ldy #0         ;Start at horizontal position 0
drawhl:
        lda (TMPVAR+2),y;Get tile number     
                        ;(2) = map (6) = tile addr  (4) = screen addr
          asl a         ;Multiply by 8, to use as an index into tiles
          asl a
          asl a
          clc
          adc #<tiles   ;Add tile address
          sta TMPVAR+6
          lda #0
          adc #>tiles
          sta TMPVAR+7

          tya
          pha
          ldx #7
          stx TMPVAR+$A
tileloop: ldy TMPVAR+$A 
          lda (TMPVAR+6),y     ;Copy tile (8 bytes, 8x8 pixels in mode 4)
          ldy scrptr,x 
           sta (TMPVAR+4),y
           dex
           dec TMPVAR+$A
           lda TMPVAR+$A
           bpl tileloop
   
          pla
          tay

          inc TMPVAR+4     ;Move screen address on 8

          iny        ;We at end of row?
          cpy #32
          bne drawhl ;Loop if not

         inc TMPVAR+5     ;Move screen down a line and back 32 characters
         lda #0
         sta TMPVAR+4

         lda TMPVAR+2     ;Move level data address down a row
         clc
         adc #32
         sta TMPVAR+2
         lda TMPVAR+3
         adc #0
         sta TMPVAR+3

         dec TMPVAR+8      ;Decrease rows left count
         bne drawvl ;and jump back

;Draw key sprites onto screen
        ldy #0 ;Start with key 0
keysprl:
        lda levhead,y ;Get location out of level header

        sta TMPVAR+6
        lda levhead+1,y
        clc
        adc #$80
        sta TMPVAR+7

        tya
        pha
                
        ldy #0
        ldx #0       ;Copy key graphic (was a sprite on C64) onto screen
l4      lda keyspr,x
        sta (TMPVAR+6),y
        inx
        tya
        clc
        adc #$20
        tay
        bne l4 
        
        pla
        pha
        tay
        
        lda levhead,y   ;Get key location again
        sta TMPVAR+2           ;and index into level data
        lda levhead+1,y
        sta TMPVAR+3
        lda #0

        jsr savespc

        ldy TMPVAR+2
        lda #$80     ;$80 means key when there are still keys left
        sta (TMPVAR+3),y
        
        pla
        tay
                 
        iny          ;Increment key pointer
        iny
        cpy #8       ;At the end? (4 keys * 2 bytes each)
        bne keysprl

        rts


vsync:  jsr WAITSYNC
        inc VSYNCCNT
        lda VSYNCCNT
        cmp #3        ;Wait til vsync count >=5
        bcc vsync
        lda #0
        sta VSYNCCNT  ;Clear VSYNCCNT
        rts

falldie: lda #0
        sta PLF 
        jmp die

guyfall: lda PLX     ;Update falling player
        lsr a         ;Get player position and shift down to block addresses
        lsr a
        lsr a
        tax
        lda PLY
        clc
        adc #16+7
        lsr a
        lsr a
        lsr a
        tay
        jsr getblk   ;Get blocks immediately below player
        bne stopf    ;If it's not air then stop falling
        inc PLY      ;Otherwise move player down
        inc PLY
        inc PLY
        inc PLY
	inc PLF      ;Increase fall distance
        rts

stfall:  lda #1      ;Start fall - set PLF flag to one 
        sta PLF
        jmp guyfall  ;then process fall

stopf:  lda PLF     ;Stop falling - see how far we've fallen
        cmp #12     ;If we've fallen far enough (6 blocks)
        bcs falldie ;then kill player
        lda #0      ;clear PLF flag
        sta PLF
        lda PLY      ;Then adjust player Y so we're directly on a block
        clc
        adc #7
        and #$F8
        sta PLY
        rts

startj:  lda #7      ;Start jump - set maximum upwards momentum in PLJ
        sta PLJ
guyjump: jmp guyj    ;Then process jump

guymove: lda PLJ     ;Are we jumping?
        bne guyjump  ;Then process jump
        lda PLF      ;Are we falling?
        bne guyfall  ;Then process fall
        lda PLX      ;Get player position
        lsr a         ;Shift down to block numbers
        lsr a
        lsr a
        tax
	stx $340     ;And store
        lda PLY
        clc
        adc #16      ;+16 - the block immediately under the player's feet
        lsr a
        lsr a
        lsr a
        tay
	sty $341
        jsr getblk   ;Check blocks immediately under player
        beq stfall   ;If it's just air then start falling

        lda #0
        sta $f
        
        ;Get Z and X keys
;        lda #$81
;        ldx #$9E ;'Z'
;        ldy #255
;	jsr OSBYTE ;If we get them now then player can't change direction during jump
;	cpx #255
        jsr SCANKEY
        cpy #$3a  ; 'Z'        
	bne l5
	lda #1
	sta $f    ;Store for later

l5
;        lda #$81
;        ldx #$BD ;'X'
;        ldy #255
;        jsr OSBYTE
;        cpx #255
        jsr SCANKEY
        cpy #$38 ; 'X'
	bne l6
	lda $f
	ora #2
	sta $f
	
l6      
;        lda #$81
;        ldx #$B6     ;Check RETURN key
;        ldy #255
;        jsr OSBYTE
;        cpx #255
        lda $b001
        and #$80 ;'SHIFT'
        beq startj  ;Start jumping if RETURN down
        
chkdir: lda $f
	and #1 ;'Z'
	bne guyleft ;Move left if Z held down
        
        lda $f
        and #2 ;'X'
        bne guyright ;Move right if X held down

        rts

guyleft: dec PLX   ;Move player left
        dec PLX
        lda #1     ;Set direction to left (so player faces that direction)
        sta PLD
        lda PLX    ;Shift coordinates down again
        lsr a
        lsr a
        lsr a
        tax
        lda PLY
        lsr a
        lsr a
        lsr a
        tay
        stx $70     ;We now check blocks immediately to the left of player 
        sty $71     ;to see if player could actually move that direction
        jsr getblk1 ;Check just one block
        cmp #2      ;If it's a wall
        beq noleft  ;Then jump to move player back again
        ldx $70     ;Also check block below the one we just checked
        ldy $71
        iny
        jsr getblk1
        cmp #2
        beq noleft
        rts

noleft:  inc PLX
        inc PLX
        rts

guyright: inc PLX   ;Much the same as above, just right instead of left!
        inc PLX
        lda #0
        sta PLD
        lda PLX
        clc         ;Player coordinates are the left half of the player, so
        adc #8      ;we need add a block to get the block at his right      
        lsr a
        lsr a
        lsr a
        tax
        lda PLY
        lsr a
        lsr a
        lsr a
        tay
        stx $70
        sty $71
        jsr getblk1
        cmp #2
        beq noright
        ldx $70
        ldy $71
        iny
        jsr getblk1
        cmp #2
        beq noright
        rts

noright: dec PLX
        dec PLX
        rts

;stopj4:  jmp stopj5  ;Branch instructions on 6502 are annoyingly limited
;Handle jumping
guyj:    lda PLY
        sec          ;Subtract PLJ from PLY to move player up/down
        sbc PLJ      ;(PLJ can be negative, which is how the player moves down again)
        sta PLY
        ldx PLJ      ;Update PLJ
        dex
        bne ndecj    ;Skip over 0, as PLJ=0 means not jumping!
        dex          ;move straight to -1 instead
ndecj:   stx PLJ
        cpx #$F7     ;Have we reached -8?
        beq stopj    ;Then stop jumping (player will probably start falling)
;        lda PLY      
;        cmp #200     ;Make sure player doesn't move off top or bottom of screen
;        bcs stopj4   ;Stop jump in that case
        jsr chkdir   ;Keep player moving left/right
        lda PLJ      ;Need to check if we hit ceiling/floor
        bpl chkc     ;Check jump sign to see which
;Check floor
        lda PLX      ;For floor we check immediately underneath player
        lsr a
        lsr a
        lsr a
        tax
        lda PLY
        clc
        adc #16+7
        lsr a
        lsr a
        lsr a
        tay
        jsr getblk
        bne stopj2
        rts
;Check ceiling
chkc:   lda PLX     ;For ceiling we check immediately above
        lsr a
        lsr a
        lsr a
        tax

        lda PLY
        lsr a
        lsr a
        lsr a
        tay
        stx $70
        sty $71
        jsr getblk1
        cmp #2
        beq stopj3
        ldx $70
        inx
        ldy $71
        jsr getblk1
        cmp #2
        beq stopj3
        rts

stopj2: lda PLY      ;Stop jumping if we've hit floor
        and #$F8     ;Update player Y so he's immediately above the floor
        sta PLY      ;(he could be partway through)
        lda PLX
        lsr a
        lsr a
        lsr a
        tax
        lda PLY
        clc
        adc #16+7
        lsr a
        lsr a
        lsr a
        tay
        jsr getblk  ;Not sure why we need to check this, I'm loathe to change it
        bne stopj
        lda PLY
        clc
        adc #4
        sta PLY
stopj:  lda #0     ;Clear PLJ flag
        sta PLJ
        rts

stopj5: lda #0     ;Stop jumping if hit the top of the screen
        sta PLY     ;Limit player to 0
        jmp stopj

stopj3: inc PLJ    ;Stop jumping if we hit ceiling
        lda PLY
        clc
        adc PLJ
        and #$F8    ;Make sure player isn't partway through the ceiling
        sta PLY
        jmp stopj

blkzero: lda #0 ;Used if player jumps off the top of the screen
        cpx #0
        beq l7
        cpx #31
        beq l7
        rts     ;just returns empty air

l7      lda #2 ;But return a brick wall if about to jump off the edge
        rts
                
;Return block number at block X,Y
getblk: cpy #20
        bcs blkzero
        stx TMPVAR+2 ;Actually this returns the block at X,Y ORed with X+1,Y 
        sty TMPVAR+3  ;This is needed for ceiling/floor calculations
        lda #0 ;as player is 2 blocks wide

        jsr savespc

        ldy TMPVAR+2
        lda (TMPVAR+3),y
        iny
        ora (TMPVAR+3),y
        and #$7f
        rts

;Return block number at block X,Y
getblk1: cpy #20
        bcs blkzero 
        stx TMPVAR+2   ;This only checks 1 block
        sty TMPVAR+3    ;But also checks for keys/exit at the same time
        stx $70
        sty $72
        lda #0

        jsr savespc

        ldy TMPVAR+2
        lda (TMPVAR+3),y
        bmi gotkey ;If block number is negative then it's either key or door
       
        rts

setlo:  lda #1       ;Set level over flag
        sta LEVELOVR
        lda #0       ;Then return 0, for empty air
        rts
        
gotkey: lda KEYSLEFT ;Do we have any keys left?
        beq setlo    ;If not then it's a door - set level over
        lda #0       ;Else it's a key - remove it from the map
        sta (TMPVAR+3),y
        
        sta $71      ;Remove it from the screen as well

        lda $70      ;This is just a repeat of the plotting code
        sta TMPVAR+6
        lda $72
        clc
        adc #$80
        sta TMPVAR+7
         
        ldy #0
        ldx #0       ;Copy key graphic (was a sprite on C64) onto screen
l8      lda keyspr,x
        eor (TMPVAR+6),y
        sta (TMPVAR+6),y
        inx
        tya
        clc
        adc #$20
        tay
        bne l8 

        dec KEYSLEFT  ;Decrease the key count
        beq mkexit2   ;If that's the last key then put the door in place        
        
        lda #0        ;Return empty air
        rts
        
mkexit2: jsr makexit  ;Put exit on screen
        lda #0        ;Return empty air
        rts
        



;Drawsprite routine
; A=spritenummer
;  x&7=0/1 -> subsprite 0
;  x&7=2/3 -> subsprite 1
;  x&7=4/5 -> subsprite 2
;  x&7=6/7 -> subsprite 3
;
;A=sprite num, X=X, Y=Y
; Spritedata    = $a/b
; Screenrowcount= $5
; Screenoffset  = $6/7
; Screenaddress = $8/9

drawspr:
        sta TMPVAR+2
        stx TMPVAR+3
        sty TMPVAR+4

;Screen base spriteaddress = $8000 + y*$100 + X
;Calculate screenaddress $6/7
        lda TMPVAR+3 
        lsr a
        lsr a
        lsr a
        sta TMPVAR+6
        lda #$80
        sta TMPVAR+7

;Set spriteline counter
        tya
        tax
        lda #15
        sta TMPVAR+5

;Base spriteaddress = sprites + nr*128
        lda TMPVAR+2         ;Get sprite start
        sta TMPVAR+$C
        lda #0
        sta TMPVAR+$D
        asl TMPVAR+$C       ;x2 - max 8
        asl TMPVAR+$C       ;x4 - max 16
        asl TMPVAR+$C       ;x8 - max 32
        asl TMPVAR+$C       ;x16 - max 64
        asl TMPVAR+$C       ;x32 - max 128
        asl TMPVAR+$C       ;x64 - max 256
        rol TMPVAR+$D
        asl TMPVAR+$C       ;x128 - max 512
        rol TMPVAR+$D

        lda TMPVAR+$C        ;Get full sprite address
        clc
        adc #<sprites
        sta TMPVAR+$A         ;And put in $A/$B
        lda TMPVAR+$D
        adc #>sprites
        sta TMPVAR+$B

;Add x%6 *32 to get subsprite
        lda TMPVAR+3         ;Sprites are preshifted, 4 based on X position
        and #6        ;so get index to correct one
        asl a
        asl a
        asl a
        asl a
        clc
        adc TMPVAR+$A
        sta TMPVAR+$A
        lda TMPVAR+$B
        adc #0
        sta TMPVAR+$B

	lda CHKCOL   ;Do we need to check collisions?
	bne spryl2   ;Yes, then jump to collision detecting loop

spryl:
        cpx #200     ;Don't draw row if it's off the screen 
        bcs skipdraw1

        lda ytab1,x  ;Otherwise normal loop
        clc          ;Get address through Y tables
        adc TMPVAR+6
        sta TMPVAR+8
        lda ytab2,x
        adc TMPVAR+7
        sta TMPVAR+9

        ldy #0
        lda (TMPVAR+$a),y   ;Get sprite data
        eor (TMPVAR+$8),y   ;EOR with what's on screen
        sta (TMPVAR+$8),y   ;And story
        iny
        lda (TMPVAR+$a),y   ;Get second byte of sprite data for this row
        eor (TMPVAR+$8),y   ;and EOR and store
        sta (TMPVAR+$8),y

skipdraw1:
        lda TMPVAR+$a       ;Update sprite address
        clc
        adc #2
        sta TMPVAR+$a
        lda TMPVAR+$b
        adc #0
        sta TMPVAR+$b
        inx
        dec TMPVAR+5       ;Decrease row count
        bpl spryl   ;And loop

        rts

spryl2: cpx #200     ;Don't draw row if it's off the screen 
        bcs skipdraw2
        lda ytab1,x ;Get address through Y table
        clc
        adc TMPVAR+6
        sta TMPVAR+8
        lda ytab2,x
        adc TMPVAR+7
        sta TMPVAR+9

        ldy #0

	lda (TMPVAR+$a),y  ;Get sprite data
	and (TMPVAR+$8),y  ;AND with screen data
	bne coll    ;Jump if not 0 (if not 0 then collision!)
donecol:
        lda (TMPVAR+$a),y ;Plot as normal
        eor (TMPVAR+$8),y
        sta (TMPVAR+$8),y
        iny

        lda (TMPVAR+$a),y  ;Collision again
	and (TMPVAR+$8),y
	bne coll2

doncol: lda (TMPVAR+$a),y  ;And plot again
	eor (TMPVAR+$8),y
        sta (TMPVAR+$8),y
skipdraw2:
        lda TMPVAR+$a      ;Update address
        clc
        adc #2
        sta TMPVAR+$a
        lda TMPVAR+$b
        adc #0
        sta TMPVAR+$b
        inx
        dec TMPVAR+5
        bpl spryl2

        rts

coll:   lda #1      ;Set collision flag
	sta COLLIDE
	jmp donecol

coll2:  lda #1      ;Two of these loops, so it can continue at the right point
	sta COLLIDE
	jmp doncol



dolh:   stx TMPVAR+6          ;Copy level header pointed to by X/Y
	sty TMPVAR+7          ;to levhead
	lda #<levhead
	sta TMPVAR+4
	lda #>levhead
	sta TMPVAR+5
	ldy #0
headl:  lda (TMPVAR+6),y
	sta (TMPVAR+4),y
	iny
	cpy #30        ;It's 30 bytes
	bne headl
	rts
	
;Level decompression
decomp: lda #<lev1     ;Get normal level address
        sta TMPVAR+4
        lda #>lev1
        sta TMPVAR+5
        stx TMPVAR+6          ;Store compressed level address (X/Y)
        sty TMPVAR+7
        ldy #0

deloop: lda (TMPVAR+6),y      ;Get next byte
        cmp #$ff       ;If $FF then end of level data
        beq deend
	sta TMPVAR+8        ;Save
	lsr a
	lsr a
	tax            ;Get run length number (top 6 bits)
	lda TMPVAR+8
	and #3         ;Get tile number
	
        inc TMPVAR+6          ;Increase source address
        bne nodinc2
        inc TMPVAR+7
nodinc2: nop
dewloop: sta (TMPVAR+4),y     ;Store tile
        inc TMPVAR+4          ;Increase dest address
        bne nodeinc
        inc TMPVAR+5
nodeinc: dex           ;Decrease run length count
        bne dewloop    ;and loop
        jmp deloop     ;Byte over - jump to start of loop

deend:  rts

;Make status display (why did I call it 'display bottom'?)
mkdispb:
        lda #<$9600
        sta tmp
        lda #>$9600
        sta tmp+1
        ldy #0
        ldx #0
mkdbl:
        lda dispbot,x ;Copy 'LIVES' text
        sta (tmp),y
        tya
        clc
        adc #$20
        tay
        bne noinc2
        inc tmp
noinc2:
	inx
	cpx #48
	bne mkdbl

        lda #<$960d
        sta tmp
        lda #>$960d
        sta tmp+1
        ldy #0
	ldx #0
mkdbl3:
        lda timedb,x  ;Copy 'TIME' text
	sta (tmp),y
        tya
        clc
        adc #$20
        tay
        bne noinc3
        inc tmp
noinc3:
	inx
	cpx #40
	bne mkdbl3

	lda #$10
	ldx #0
	rts

;enddat:  .BYTE $A,$D,$C,$D,$A,$E,$F,$FF



;Draw a number on the status bar
dodbnum:
        asl a ;Multiply number by 8
	asl a
	asl a
	clc   ;for character address lookup
	adc #<livenum
	sta TMPVAR+4
	lda #>livenum
	adc #0
	sta TMPVAR+5 ;Character source now in 4/5

        ldx #0
        ldy #0
updbl1:
        lda (TMPVAR+4),y  ;Copy character across
	sta (TMPVAR+8),y
        inc TMPVAR+4
        lda TMPVAR+4
        bne goon1
        inc TMPVAR+5
goon1:
        lda TMPVAR+8
        clc
        adc #$20
        sta TMPVAR+8
        lda TMPVAR+9
        adc #0
        sta TMPVAR+9
	inx
	cpx #8      ;8 bytes
	bne updbl1
	rts

;Update status bar
updb:	lda LIVES  ;Lives first (1 digit only)
	ldx #$96   ;Store screen address ($7A90)
	stx TMPVAR+$9
	ldx #$06
	stx TMPVAR+$8
	jsr dodbnum ;And display number

	lda TIME   ;First (last on the display) digit of time
	and #$f
	ldx #$96   ;Address is $7B20
	stx TMPVAR+$9
	ldx #$14
	stx TMPVAR+$8
	jsr dodbnum ;And display

	lda TIME   ;Middle digit now
	lsr  a      ;Shift to get it
	lsr a
	lsr a
	lsr a
	ldx #$96   ;Address is $7B18 
	stx TMPVAR+$9
	ldx #$13
	stx TMPVAR+$8
	jsr dodbnum ;And display

	lda TIME+1 ;And finally last digit (first on display)
	and #$f
	ldx #$96   ;Address is $7B10
	stx TMPVAR+$9
	ldx #$12
	stx TMPVAR+$8
	jsr dodbnum ;And display

	lda LCOMP  ;Now level number
	sed        ;Add 1, so level numbers aren't 0 based anymore
	clc
        adc #1
	cld
	and #$f
	ldx #$96   ;Address $7B58
	stx TMPVAR+$9
	ldx #$1f
	stx TMPVAR+$8
	jsr dodbnum ;And display

	lda LCOMP  ;Other digit (you decide if it's first or last)
	sed        ;Add 1, so level numbers aren't 0 based anymore
	clc
	adc #1
	cld
	lsr a
	lsr a
	lsr a
	lsr a
	ldx #$96  ;Address $7B50   
	stx TMPVAR+$9
	ldx #$1e
	stx TMPVAR+$8
	jsr dodbnum ;And display

	rts 

uptime: sed         ;Update time - use BCD decimal mode
	lda TIME
	sec
	sbc #1      ;Subtract 1 from time
	sta TIME
	lda TIME+1  ;Then update second byte
	sbc #0
	sta TIME+1
	cld
	lda TIME    ;Is time = 000?
	ora TIME+1
	beq timeup  ;Then time's up so kill the player
	rts

timeup: jmp die

clear:  
        lda #$D0  ; Clear4 color
        sta $B000

        lda #$80
        sta tmp+1
        lda #0       ;Clear the screen - this is _not_ small
        sta tmp
        tay
clrloop:
        sta (tmp),y
        iny
        bne clrloop
        inc tmp+1
        ldx tmp+1
        cpx #$98
        bne clrloop

        rts

savespc:
        sta TMPVAR+4
        asl TMPVAR+3    ;x2 - max 40
        asl TMPVAR+3    ;x4 - max 80
        asl TMPVAR+3    ;x8 - max 160
        asl TMPVAR+3    ;x16 - max 320
        rol TMPVAR+4
        asl TMPVAR+3    ;x32 - max 640
        rol TMPVAR+4
        lda TMPVAR+3
        clc
        adc #<lev1
        sta TMPVAR+3
        lda TMPVAR+4
        adc #>lev1
        sta TMPVAR+4
        rts

;Our event handler. This looks for vsync and increments the VSYNCCNT
eventh: cmp #4 ;is this vsync?
        bne l9  ;jump if isn't
        inc VSYNCCNT ;increment VSYNCCNT
l9
        rts ; end of game

ENMY:   .db 0,0,0,0,0
        .db 0,0,0,0,0
        .db 0,0,0,0,0
        .db 0,0,0,0,0
        .db 0,0,0,0,0

lev1:
        .blkb 640

