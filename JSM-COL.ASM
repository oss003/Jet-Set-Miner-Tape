               org $2ac0-22

;****************************************
; Header for Atom emulator Wouter Ras
		 .db "JSM             "
		 .dw $2ac0
		 .dw $2ac0
		 .dw eind_asm-start_asm
;****************************************
start_asm    
               include macros.inc
               include jsm.inc
               include ctiles.inc
               include scrtab.inc
               include cfont.inc
               include csprites.inc
               include levels.inc
eind_asm
