;--------------------------------------
; fast.asm
;
; Maggie3D library
; Fast ASM functions
; 
; @author Fabrice Labrador <fabrice.labrador@gmail.com>
; @version 1.0 November 2023
;--------------------------------------

; Base address
CUSTOM    = $DFF000

; Maggie registers
TEXTPT    = $250              ; Texture adr
TEXDPT    = $254              ; Dest adr
TEXZPT    = $258              ; Z buffer adr
TEXSTRT   = $25E              ; Line len & start maggie
TEXMIP    = $260              ; Texture size
TEXMODE   = $262              ; Render mode
TEXDMODE  = $266              ; Dest modulo (texel size)
TEXUVAL   = $270              ; U value in fixed 16:16
TEXVVAL   = $274              ; V value in fixed 16:16
TEXDUVAL  = $278              ; dU value in fixed 16:16
TEXDVVAL  = $27C              ; dV value in fixed 16:16
TEXLIGHT  = $280              ; Ligth value in fixed 8:8
TEXDLIGHT = $282              ; Delta light value in fixed 8:8
TEXLCOLOR = $284              ; Light color in ARGB
TEXZVAL   = $288              ; Z value in fixed 16:16
TEXDZVAL  = $28C              ; dZ value in fixed 16:16

  SECTION fast,code

;--------------------------------------
; Clear the Z buffer
;
; @in a0.l Z buffer address
; @in d0.w number of lines to clear
; @in d1.w number of bytes per line
;
; @out d0.l Operation success
;--------------------------------------
  xdef _M3D_FastClearZBuffer

_M3D_FastClearZBuffer:
  movem.l d1/d2/d6/a0,-(sp)
  move.l  #$FFFFFFFF,d2
  lsr.w   #3,d1
  subq.w  #1,d1
  subq.w  #1,d0
.NextLine:
  move.w  d1,d6
.NextBlock:
  move.l  d2,(a0)+
  move.l  d2,(a0)+
  dbf     d6,.NextBlock
  dbf     d0,.NextLine
  movem.l (sp)+,d1/d2/d6/a0
  move.l  #0,d0
  rts

;--------------------------------------
; Fast flat shading
;
; @in d7.w number of lines to draw
; @in a0.l maggie registers base
; @in a1.l data buffer
;--------------------------------------
  xdef _M3D_FastFlatShading

_M3D_FastFlatShading:
  movem.l d1-a6,-(sp)
  moveq.l #0,d0
  move.l  d0,TEXUVAL(a0)
  move.l  d0,TEXVVAL(a0)
  move.l  d0,TEXDUVAL(a0)
  move.l  d0,TEXDVVAL(a0)
  move.w  d0,TEXDLIGHT(a0)
  subq.w  #1,d7
.NextLine:

  
  dbf     d7,.NextLine
  movem.l (sp)+,d1-a6
  rts

;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------

; Maggie fake registers offsets
FTEXTPT    = $0               ; Texture adr
FTEXDPT    = $4               ; Dest adr
FTEXZPT    = $8               ; Z buffer adr
FTEXSTRT   = $E               ; Line len & start maggie
FTEXMIP    = $10              ; Texture size
FTEXMODE   = $12              ; Render mode
FTEXDMODE  = $16              ; Dest modulo (texel size)
FTEXUVAL   = $20              ; U value in fixed 16:16
FTEXVVAL   = $24              ; V value in fixed 16:16
FTEXDUVAL  = $28              ; dU value in fixed 16:16
FTEXDVVAL  = $2C              ; dV value in fixed 16:16
FTEXLIGHT  = $30              ; Ligth value in fixed 8:8
FTEXDLIGHT = $32              ; Delta light value in fixed 8:8
FTEXLCOLOR = $34              ; Light color in ARGB
FTEXZVAL   = $38              ; Z value in fixed 16:16
FTEXDZVAL  = $3C              ; dZ value in fixed 16:16

;--------------------------------------
; Simple Maggie emulation
;
; @in a0.l Maggie fake registers
;--------------------------------------
  xdef _M3D_EmulateMaggie16Bits

_M3D_EmulateMaggie16Bits:
  movem.l d1-a6,-(sp)
  move.l  FTEXUVAL(a0),d1           ; ui
  move.l  FTEXVVAL(a0),d2           ; vi
  move.w  FTEXSTRT(a0),d7           ; count
  move.l  FTEXTPT(a0),a1            ; texture
  move.l  FTEXDPT(a0),a2            ; destination
  tst.l   a1
  bne.s   .Texmap
.Color:
  move.l  FTEXLCOLOR(a0),d3         ; color
  move.l  d3,d4
  lsr.l   #8,d3
  andi.w  #$f800,d3
  move.l  d4,d5
  andi.w  #$fc,d4
  lsr.w   #5,d4
  andi.w  #$f8,d5
  lsr.w   #3,d5
  or.w    d4,d3
  or.w    d5,d3
  subq.l  #1,d7
.Shade:
  move.w  d3,(a2)+
  dbf     d7,.Shade
  bra     .Return
.Texmap:
  move.w  #$f80f,d3
  subq.l  #1,d7
.Texture:
  move.w  d3,(a2)+
  add.l   FTEXDUVAL(a0),d1
  add.l   FTEXDVVAL(a0),d2
  dbf     d7,.Texture
.Return:
  movem.l (sp)+,d1-a6
  rts

  END
