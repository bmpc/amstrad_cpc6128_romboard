; select background ROM


PrintChar       equ &BB5A

KL_ROM_SELECT   equ &B90F
KL_ROM_ENABLE   equ &B900
KL_ROM_DISABLE  equ &B903
ROM_BANK        equ &8000

ROM_START       equ &C000

  org &8010

  ld hl, ROM_BANK
  ld c, (hl)                    ;ROM position
  call KL_ROM_SELECT            ;KL ROM SELECT

  ld c, 255
  ld hl, ROM_START
PrintByte:
  ld a, (hl)
  push bc
  push hl
     ld h,0         
     ld l,a        
     ld de,DecNumber 
     call Num2Dec
     ld hl,DecNumber 
     call PrintString
  pop hl
  pop bc
  ld a, 44                      ;space
  call PrintChar
  inc hl
  dec c
  ld a, c
  cp 0
  jr nz, PrintByte             ; if A != 0 -> keep receiving bytes

Wait:
  nop
  nop
  nop
  jp Wait
  
  ;ret                          ; returns to basic

; 16-bit Integer to ASCII (decimal) - adapted from http://map.grauw.nl/sources/external/z80bits.html
Num2Dec:
  ld bc,-10000
  call Num1
  ld bc,-1000
  call Num1
  ld bc,-100
  call Num1
  ld c,-10
  call Num1
  ld c,b
Num1:
  ld a,'0'-1
Num2:
  inc a
  add hl,bc
  jr c,Num2
  sbc hl,bc
  cp a,'0'                       ; replace leading zeros with spaces
  jr nz, Num3
  ld a,'0'
Num3:
  ld (de),a
  inc de
  ret

; Print a '255' terminated string
PrintString:
  ld a, (hl)                     ; load memory referenced by HL into register A
  cp 255                         ; Compare byte with 255
  ret z                          ; return if A == 255
  inc hl                         ; increment HL
  call PrintChar
  jr PrintString

DecNumber:
  db '00000',255
