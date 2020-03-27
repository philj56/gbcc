INCLUDE "hardware.inc"

SECTION "Header", ROM0[$100]

EntryPoint:
  di
  jp Start

REPT $134 - $104
  db 0
ENDR

  db "GBCCINTRO", 0, 0 ; ROM Title
  db "PHIL" ; Manufacturer Code
  db CART_COMPATIBLE_GBC ; GBC Flag
  db "PJ" ; Licensee Code
  db 0 ; SGB Flag
  db CART_ROM_MBC3_BAT_RTC ; Cartridge Type
  db CART_ROM_256K ; ROM Size
  db CART_RAM_NONE ; RAM Size
  db 1 ; Destination (Non-Japanese)
  db $33 ; Old Licensee Code
  db 0 ; Version Number

REPT $150 - $14D
  db 0
ENDR


SECTION "Game code", ROM0

Start:
  ; Turn off the LCD
.waitVBlank
  ld a, [rLY]
  cp 144 ; Check for VBlank
  jr c, .waitVBlank

  xor a
  ld [rLCDC], a ; Turn off the LCD

  ld hl, $8000
  ld de, GBCCLogo
  ld bc, GBCCLogoEnd - GBCCLogo

.copyLogoTiles
  ld a, [de]
  ld [hli], a
  inc de
  dec bc
  ld a, b ; Check if count is 0, as `dec bc` doesn't update flags
  or c
  jr nz, .copyLogoTiles

  ld hl, $9800 ; Top-left of the tilemap
  ld de, GBCCLogoMap
  ld bc, GBCCLogoMapEnd - GBCCLogoMap
.copyLogoMap
  ld a, [de]
  ld [hli], a
  inc de
  dec bc
  ld a, b ; Check if count is 0, as `dec bc` doesn't update flags
  or c
  jr nz, .copyLogoMap

  ; Set palettes
  ld a, %10000000
  ld [rBCPS], a

  ld a, %00111111
  ld [rBCPD], a
  ld a, %01000000
  ld [rBCPD], a

  ld a, %10011001
  ld [rBCPD], a
  ld a, %00110100
  ld [rBCPD], a

  ld a, %11010010
  ld [rBCPD], a
  ld a, %00101000
  ld [rBCPD], a

  ld a, %00101001
  ld [rBCPD], a
  ld a, %00100101
  ld [rBCPD], a

  ; Init display registers 
  xor a
  ld [rSCY], a
  ld [rSCX], a

  ld [rNR52], a ; Turn off sound

  ; Turn screen on and display background
  ld a, %10010001
  ld [rLCDC], a

  ; Lock up
.lockup
  halt
  jr .lockup


SECTION "GBCC Logo Tiles", ROM0

GBCCLogo:
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FC
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$01,$00
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$7F,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$F4,$F8,$C0,$E0,$80,$C0
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$01,$00,$00,$00,$00,$00
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$BF,$7F,$0F,$1F,$0F,$07
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FE,$FE,$FC,$F8,$F8,$F8,$F0,$F0,$E0
  db $E8,$F0,$A0,$C0,$80,$00,$00,$00,$00,$00,$00,$00,$00,$00,$01,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$18,$00,$7E,$FF
  db $2F,$1F,$0F,$07,$01,$03,$01,$01,$00,$01,$01,$00,$01,$00,$81,$01
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $C0,$80,$C0,$80,$C0,$80,$C0,$80,$C0,$80,$C0,$80,$80,$C0,$C0,$C0
  db $00,$00,$00,$00,$00,$00,$00,$00,$1C,$00,$1F,$3E,$1F,$3F,$1F,$3F
  db $03,$03,$01,$01,$01,$00,$00,$00,$00,$00,$00,$00,$00,$80,$C0,$80
  db $FF,$FF,$FF,$FF,$FF,$FF,$7F,$FF,$FF,$7F,$7F,$7F,$3F,$7F,$7F,$3F
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FE,$F8,$FC,$F8,$F0,$F0,$E0,$E0,$C0
  db $FF,$FF,$FC,$F0,$C0,$80,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $FF,$FF,$07,$01,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $FF,$FF,$FF,$FF,$7F,$3F,$1F,$0F,$0F,$07,$03,$07,$07,$03,$07,$03
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FE,$FE,$FC
  db $FF,$FF,$FE,$FF,$FC,$F8,$E0,$F0,$E0,$C0,$00,$80,$00,$00,$00,$00
  db $EF,$F0,$80,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $FF,$3F,$0F,$07,$02,$01,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$7F,$1F,$3F,$1F,$1F,$1F,$0F,$0F,$0F
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FE,$FE,$FE,$FC,$FE
  db $E0,$C0,$80,$C0,$C0,$80,$80,$00,$00,$00,$00,$00,$01,$00,$01,$01
  db $05,$03,$0F,$07,$0F,$1F,$1F,$3F,$3F,$7F,$FF,$7F,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $A3,$C1,$FB,$F7,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $C0,$C0,$C0,$C0,$C0,$C0,$E0,$C0,$E0,$C0,$E0,$C0,$E0,$C0,$E0,$C0
  db $3F,$1F,$3F,$1F,$3F,$1F,$3F,$1F,$1F,$1F,$1E,$1F,$1E,$1C,$14,$18
  db $C0,$80,$80,$C0,$80,$C0,$C0,$80,$00,$80,$00,$00,$00,$00,$00,$00
  db $7F,$3F,$7F,$3F,$7F,$3F,$7F,$3F,$7F,$3F,$3F,$7F,$7F,$7F,$FF,$7F
  db $FF,$FF,$FF,$FF,$FF,$FE,$FE,$FE,$FE,$FC,$F8,$FC,$F8,$F8,$F0,$F8
  db $C0,$80,$80,$00,$00,$00,$00,$00,$00,$00,$01,$00,$01,$01,$03,$03
  db $00,$00,$03,$00,$0F,$07,$3F,$1F,$3F,$7F,$FF,$FF,$FF,$FF,$FF,$FF
  db $00,$00,$6C,$F0,$FF,$FE,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $03,$07,$07,$07,$07,$0F,$9F,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FC,$F8,$F8,$F0,$E0,$F0,$E0,$E0,$E0,$C0,$80,$C0,$C0,$80,$00,$80
  db $00,$00,$00,$00,$01,$00,$05,$03,$0F,$07,$1F,$0F,$3F,$1F,$3F,$3F
  db $0F,$00,$7F,$3F,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $80,$00,$D0,$E0,$F0,$F8,$FE,$FC,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $0F,$0F,$0F,$0F,$1F,$0F,$3F,$1F,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FE,$FC,$FC,$FC,$FC,$F8,$F8,$F8,$F8,$F8,$F0,$F8,$F8,$F0,$F8,$F0
  db $03,$01,$03,$03,$07,$03,$03,$07,$07,$07,$0F,$07,$07,$0F,$0F,$0F
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FB,$FC,$C0,$E0,$80,$C0,$C0,$80
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$DF,$3F,$05,$03,$01,$00,$00,$00
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$7F,$FF,$7F,$3F
  db $C0,$E0,$C0,$E0,$E0,$E0,$E0,$E0,$E0,$E0,$E0,$E0,$E0,$E0,$F0,$E0
  db $10,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$01,$03,$01,$07,$03,$0F,$07,$01,$03,$00,$01,$00,$00
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$7F,$FF
  db $F8,$F0,$E0,$F0,$F0,$E0,$E0,$E0,$E0,$E0,$C0,$E0,$E0,$C0,$E0,$C0
  db $03,$07,$0F,$07,$0F,$0F,$1F,$0F,$0F,$1F,$1F,$1F,$3F,$1F,$3F,$1F
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FE,$FF,$FF,$FE,$FE,$FE,$FC,$FE,$FE,$FC,$FE,$FC,$FC,$FC
  db $80,$00,$00,$00,$00,$00,$01,$00,$01,$01,$03,$01,$03,$01,$03,$03
  db $3F,$7F,$FF,$7F,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $F0,$F0,$F0,$F0,$F0,$F0,$F0,$F0,$F0,$F0,$F0,$F0,$F0,$F0,$F0,$F0
  db $0F,$0F,$0F,$0F,$1F,$0F,$1F,$0F,$1F,$0F,$0F,$0F,$0F,$0F,$0F,$0F
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $80,$80,$80,$80,$80,$80,$C0,$80,$DF,$E0,$FF,$FF,$FF,$FF,$FF,$FF
  db $00,$00,$00,$00,$00,$00,$00,$00,$E0,$00,$E0,$F0,$F8,$F0,$F0,$F0
  db $3F,$1F,$0F,$1F,$1F,$0F,$0F,$0F,$07,$0F,$0F,$07,$07,$0F,$07,$0F
  db $F0,$E0,$F0,$E0,$F0,$E0,$F0,$E0,$F0,$E0,$F0,$E0,$F0,$E0,$F0,$E0
  db $00,$00,$08,$00,$0D,$0E,$0F,$0F,$1F,$0F,$1F,$0F,$1F,$0F,$1F,$0F
  db $00,$00,$00,$00,$00,$00,$00,$80,$80,$C0,$C0,$E0,$E0,$F0,$F8,$F0
  db $3F,$7F,$3F,$3F,$3F,$1F,$0F,$1F,$1F,$0F,$0F,$0F,$0F,$07,$0F,$07
  db $C0,$C0,$C0,$C0,$C0,$C0,$C0,$C0,$C0,$C0,$E0,$C0,$E0,$C0,$C0,$E0
  db $1F,$3F,$1F,$3F,$1F,$3F,$1F,$3F,$3F,$1F,$3F,$1F,$1F,$1F,$0F,$1F
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FC,$FC,$FC,$FC,$F8,$FC,$F8,$FC,$F8,$FC,$F8,$FC,$FC,$FC,$FC,$FC
  db $03,$03,$03,$03,$07,$03,$07,$03,$07,$03,$07,$03,$03,$03,$03,$03
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $F8,$F0,$F8,$F0,$F8,$F8,$FC,$F8,$F8,$FC,$FC,$FC,$FC,$FE,$FF,$FE
  db $07,$0F,$0F,$07,$07,$07,$07,$03,$03,$01,$01,$00,$00,$00,$00,$00
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$3F,$7F,$17,$0F
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FE,$FA,$FC,$D0,$E0
  db $E0,$F0,$F0,$E0,$C0,$E0,$80,$C0,$00,$80,$00,$00,$00,$00,$00,$00
  db $0F,$0F,$0F,$0F,$1F,$0F,$0F,$1F,$1F,$1F,$3F,$1F,$3F,$3F,$3F,$7F
  db $F0,$E0,$F0,$E0,$F0,$E0,$F0,$E0,$F0,$E0,$F0,$E0,$E0,$E0,$E0,$E0
  db $1F,$0F,$1F,$0F,$1F,$0F,$1F,$0F,$1F,$0F,$1F,$0F,$0F,$1F,$1F,$1F
  db $F8,$F0,$F8,$F8,$F8,$F8,$F8,$F8,$F8,$F0,$F0,$F0,$F0,$E0,$E0,$C0
  db $07,$07,$07,$07,$07,$07,$07,$07,$07,$07,$0F,$07,$07,$0F,$0F,$0F
  db $E0,$E0,$F0,$E0,$E0,$F0,$F0,$F0,$F8,$F0,$F8,$F8,$F8,$FC,$FE,$FC
  db $1F,$0F,$07,$0F,$07,$07,$07,$03,$03,$01,$01,$00,$00,$00,$00,$00
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$7F,$3F,$1F
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FB,$FC
  db $FC,$FC,$FE,$FC,$FE,$FC,$FC,$FE,$FE,$FE,$FF,$FE,$FF,$FF,$FF,$FF
  db $01,$03,$03,$01,$01,$01,$00,$01,$00,$00,$00,$00,$00,$00,$80,$00
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$7F,$7F,$3F,$0F,$1F
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $00,$00,$00,$80,$80,$C0,$C0,$E0,$E0,$F0,$FC,$F8,$FF,$FE,$FF,$FF
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$B0,$C0
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$01,$00,$1F,$07
  db $00,$00,$01,$00,$03,$01,$03,$07,$1F,$0F,$5F,$3F,$FF,$FF,$FF,$FF
  db $FF,$7F,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $E0,$E0,$E0,$E0,$E0,$E0,$C0,$E0,$E0,$E0,$E0,$E0,$F0,$E0,$F0,$F0
  db $1F,$1F,$1A,$1C,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $80,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$01,$05,$03,$07,$0F
  db $1F,$0F,$1F,$1F,$1F,$3F,$7F,$3F,$FF,$7F,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FE,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $00,$00,$80,$00,$C0,$80,$E0,$C0,$E0,$F0,$FC,$F8,$FF,$FE,$FF,$FF
  db $0D,$03,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$A0,$C0
  db $60,$80,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$01,$00
  db $3F,$1F,$0F,$1F,$1F,$0F,$0F,$0F,$0F,$0F,$1F,$0F,$3F,$1F,$7F,$FF
  db $80,$80,$80,$C0,$E0,$C0,$F0,$E0,$F8,$F0,$F8,$FC,$FF,$FE,$FF,$FF
  db $0F,$07,$01,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$40,$80
  db $FF,$FF,$7F,$FF,$1F,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $FF,$FF,$F9,$FF,$80,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $FF,$FF,$FF,$FF,$7F,$3F,$1F,$3F,$3F,$1F,$3F,$1F,$3F,$1F,$7F,$3F
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FC,$F8,$FF,$FE,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $00,$00,$C6,$01,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $5F,$3F,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FB,$FC,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $EF,$1F,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $D0,$E0,$FB,$FC,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $00,$00,$00,$00,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $00,$00,$1B,$07,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
  db $7F,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF
GBCCLogoEnd:


SECTION "GBCC Logo Map", ROM0

GBCCLogoMap:
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$02,$03,$04,$00,$06,$07,$08,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$15,$16,$17,$18,$00,$1A,$1B,$1C,$1D,$00,$1F,$20,$21,$22,$23,$24,$25,$26,$27,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $28,$29,$2A,$00,$2C,$00,$2E,$2F,$30,$31,$32,$33,$34,$35,$36,$37,$38,$39,$3A,$3B,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $3C,$3D,$00,$3F,$40,$41,$42,$43,$44,$04,$46,$47,$00,$00,$4A,$4B,$4C,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $50,$51,$00,$53,$54,$55,$56,$57,$58,$59,$5A,$5B,$00,$00,$5E,$5F,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $64,$65,$66,$67,$68,$69,$56,$6B,$6C,$6D,$6E,$6F,$70,$71,$72,$73,$74,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$79,$7A,$7B,$7C,$7D,$7E,$7F,$80,$81,$82,$83,$84,$85,$86,$87,$88,$89,$8A,$8B,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$92,$93,$94,$00,$00,$00,$98,$99,$00,$00,$9C,$9D,$9E,$9F,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
  db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
GBCCLogoMapEnd:
