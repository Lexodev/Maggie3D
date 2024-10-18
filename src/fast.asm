;--------------------------------------
; fast.asm
;
; Maggie3D shared library
; Fast ASM functions
; 
; @author Fabrice Labrador <fabrice.labrador@gmail.com>
; @version 1.2 March 2024 (updated: 10/03/2024)
;--------------------------------------

; Custom base address
CUSTOMBASE    = $DFF000

; Maggie registers
TEXTPT        = $0                ; Texture adr
TEXDPT        = $4                ; Dest adr
TEXZPT        = $8                ; Z buffer adr
TEXSTRT       = $E                ; Line len & start maggie
TEXMIP        = $10               ; Texture size
TEXMODE       = $12               ; Render mode
TEXDMOD       = $16               ; Dest modulo (texel size)
TEXUVAL       = $20               ; U value in fixed 16:16
TEXVVAL       = $24               ; V value in fixed 16:16
TEXDUVAL      = $28               ; dU value in fixed 16:16
TEXDVVAL      = $2C               ; dV value in fixed 16:16
TEXLIGHT      = $30               ; Ligth value in fixed 8:8
TEXDLIGHT     = $32               ; Delta light value in fixed 8:8
TEXLCOLOR     = $34               ; Light color in ARGB
TEXZVAL       = $38               ; Z value in fixed 16:16
TEXDZVAL      = $3C               ; dZ value in fixed 16:16

; Maggie mode
MODE_ZBUFFER  = 2
MODE_16BITS   = 4
MODE_24BITS   = 16

; Draw data structure
LEFT_CLIP     = 0
TOP_CLIP      = 4
RIGHT_CLIP    = 8
BOTTOM_CLIP   = 12
DELTA_DXDYL   = 16
DELTA_DZDYL   = 20
DELTA_DUDYL   = 24
DELTA_DVDYL   = 28
DELTA_DLDYL   = 32
DELTA_DXDYR   = 36
DELTA_DZDYR   = 40
DELTA_DUDYR   = 44
DELTA_DVDYR   = 48
DELTA_DLDYR   = 52
CRD_XL        = 56
CRD_XR        = 60
CRD_ZL        = 64
CRD_ZR        = 68
CRD_UL        = 72
CRD_UR        = 76
CRD_VL        = 80
CRD_VR        = 84
SCALE         = 88
INT_LL        = 92
INT_LR        = 96
DEST_ADR      = 100
DEST_BPR      = 104
DEST_BPP      = 108
ZBUF_ADR      = 112
ZBUF_BPR      = 116
ZBUF_BPP      = 120

; Use Maggie emulation
EMULATE_MAGGIE        = 1

; Emulation data
TEX_BPR       = 0
TEX_SCALE     = 4
TEX_CRED      = 8
TEX_CGREEN    = 10
TEX_CBLUE     = 12

; Vertex structure
VERTEX_X      = 0
VERTEX_Y      = VERTEX_X+4
VERTEX_Z      = VERTEX_Y+4
VERTEX_W      = VERTEX_Z+4
VERTEX_U      = VERTEX_W+4
VERTEX_V      = VERTEX_U+4
VERTEX_LIGHT  = VERTEX_V+4
VERTEX_SIZEOF = VERTEX_LIGHT+4

; Triangle structure
TRIANGLE_V1           = 0
TRIANGLE_V2           = TRIANGLE_V1+VERTEX_SIZEOF
TRIANGLE_V3           = TRIANGLE_V2+VERTEX_SIZEOF
TRIANGLE_TEXTURE      = TRIANGLE_V3+VERTEX_SIZEOF
TRIANGLE_COLOR        = TRIANGLE_TEXTURE+4
TRIANGLE_SIZEOF       = TRIANGLE_COLOR+4

; Quad structure
QUAD_V1               = 0
QUAD_V2               = QUAD_V1+VERTEX_SIZEOF
QUAD_V3               = QUAD_V2+VERTEX_SIZEOF
QUAD_V4               = QUAD_V3+VERTEX_SIZEOF
QUAD_TEXTURE          = QUAD_V4+VERTEX_SIZEOF
QUAD_COLOR            = QUAD_TEXTURE+4
QUAD_SIZEOF           = QUAD_COLOR+4

  SECTION fast,code

;--------------------------------------
; Fast round triangle vertex X & Y
;
; @in a0.l triangle address
;--------------------------------------
  xdef _M3D_FastRoundTriangle

_M3D_FastRoundTriangle:
  move.l  a0,-(sp)
; Vertex v1
  fint.s  VERTEX_X(a0),fp0
  fint.s  VERTEX_Y(a0),fp1
  fmove.s fp0,VERTEX_X(a0)
  fmove.s fp1,VERTEX_Y(a0)
; Vertex v2
  lea     VERTEX_SIZEOF(a0),a0
  fint.s  VERTEX_X(a0),fp0
  fint.s  VERTEX_Y(a0),fp1
  fmove.s fp0,VERTEX_X(a0)
  fmove.s fp1,VERTEX_Y(a0)
; Vertex v3
  lea     VERTEX_SIZEOF(a0),a0
  fint.s  VERTEX_X(a0),fp0
  fint.s  VERTEX_Y(a0),fp1
  fmove.s fp0,VERTEX_X(a0)
  fmove.s fp1,VERTEX_Y(a0)
  move.l  (sp)+,a0
  rts

;--------------------------------------
; Fast round quad vertex X & Y
;
; @in a0.l quad address
;--------------------------------------
  xdef _M3D_FastRoundQuad

_M3D_FastRoundQuad:
  move.l  a0,-(sp)
; Vertex v1
  fint.s  VERTEX_X(a0),fp0
  fint.s  VERTEX_Y(a0),fp1
  fmove.s fp0,VERTEX_X(a0)
  fmove.s fp1,VERTEX_Y(a0)
; Vertex v2
  lea     VERTEX_SIZEOF(a0),a0
  fint.s  VERTEX_X(a0),fp0
  fint.s  VERTEX_Y(a0),fp1
  fmove.s fp0,VERTEX_X(a0)
  fmove.s fp1,VERTEX_Y(a0)
; Vertex v3
  lea     VERTEX_SIZEOF(a0),a0
  fint.s  VERTEX_X(a0),fp0
  fint.s  VERTEX_Y(a0),fp1
  fmove.s fp0,VERTEX_X(a0)
  fmove.s fp1,VERTEX_Y(a0)
; Vertex v4
  lea     VERTEX_SIZEOF(a0),a0
  fint.s  VERTEX_X(a0),fp0
  fint.s  VERTEX_Y(a0),fp1
  fmove.s fp0,VERTEX_X(a0)
  fmove.s fp1,VERTEX_Y(a0)
  move.l  (sp)+,a0
  rts

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
; Draw functions and data
;--------------------------------------

;--------------------------------------
; Clear the draw region
;
; @in a0.l region address
; @in d0.l region width in pixels
; @in d1.l region height in lines
; @in d2.l bytes per row
; @in d3.l color in ARGB to use
;--------------------------------------
  xdef _M3D_FastClearRegion16

_M3D_FastClearRegion16:
  movem.l d0-d4/a0-a2,-(sp)
.ConvertColor:
  moveq.l #0,d4
  move.b  d3,d4
  lsr.w   #3,d4               ; Blue component
  lsr.w   #5,d3
  andi.w  #$7E0,d3
  or.w    d3,d4               ; Green component
  lsr.l   #5,d3
  andi.w  #$F800,d3
  or.w    d3,d4               ; Red component
  move.w  d4,d3
  swap    d3
  move.w  d4,d3               ; d3 = RGBRGB
.CheckWidth:
  move.l  d0,d4
  andi.l  #$3,d4
  cmpi.w  #3,d4
  bne.s   .Not3
  lea     .Block3,a2
  subq.l  #3,d0
  bra.s   .GoClear
.Not3:
  cmpi.w  #2,d4
  bne.s   .Not2
  lea     .Block2,a2
  subq.l  #2,d0
  bra.s   .GoClear
.Not2:
  cmpi.w  #1,d4
  bne.s   .Not1
  lea     .Block1,a2
  subq.l  #1,d0
  bra.s   .GoClear
.Not1:
  lea     .NextBlock,a2
.GoClear:
  lsr.l   #2,d0
  subq.l  #1,d0
  subq.l  #1,d1
.NextLine:
  move.l  a0,a1
  move.l  d0,d7
  jmp     (a2)
  nop
.Block3:
  move.w  d3,(a1)+
.Block2:
  move.w  d3,(a1)+
.Block1:
  move.w  d3,(a1)+
.NextBlock:
  move.l  d3,(a1)+
  move.l  d3,(a1)+
  dbf     d7,.NextBlock
  adda.l  d2,a0
  dbf     d1,.NextLine
  movem.l (sp)+,d0-d4/a0-a2
  rts

  xdef _M3D_FastClearRegion24

_M3D_FastClearRegion24:
  movem.l d0-d7/a0-a2,-(sp)
.ConvertColor:
  move.l  d3,d5           ; ARVB
  lsl.l   #8,d5           ; RVB0
  move.b  d3,d5           ; RVBB
  ror.l   #8,d5           ; BRVB in d5
  lsl.l   #8,d3           ; RVB0
  move.l  d3,d4           ; RVB0
  rol.l   #8,d4           ; VB0R
  or.w    d4,d3           ; RVBR in d4
  swap    d3              ; BRRV
  move.w  d3,d4           ; VBRV
  swap    d3              ; RVBR in d3
.CheckWidth:
  move.l  d0,d6
  andi.l  #$3,d6
  cmpi.w  #3,d6
  bne.s   .Not3
  lea     .Block3,a2
  subq.l  #3,d0
  bra.s   .GoClear
.Not3:
  cmpi.w  #2,d6
  bne.s   .Not2
  lea     .Block2,a2
  subq.l  #2,d0
  bra.s   .GoClear
.Not2:
  cmpi.w  #1,d6
  bne.s   .Not1
  lea     .Block1,a2
  subq.l  #1,d0
  bra.s   .GoClear
.Not1:
  lea     .NextBlock,a2
.GoClear:
  lsr.l   #2,d0
  subq.l  #1,d0
  subq.l  #1,d1
.NextLine:
  move.l  a0,a1
  move.l  d0,d7
  jmp     (a2)
  nop
.Block3:
  move.b  d4,(a1)+
  move.w  d5,(a1)+
.Block2:
  move.b  d4,(a1)+
  move.w  d5,(a1)+
.Block1:
  move.b  d4,(a1)+
  move.w  d5,(a1)+
.NextBlock:
  move.l  d3,(a1)+
  move.l  d4,(a1)+
  move.l  d5,(a1)+
  dbf     d7,.NextBlock
  adda.l  d2,a0
  dbf     d1,.NextLine
  movem.l (sp)+,d0-d7/a0-a2
  rts

  xdef _M3D_FastClearRegion32

_M3D_FastClearRegion32:
  movem.l d0-d4/a0-a2,-(sp)
.CheckWidth:
  move.l  d0,d4
  andi.l  #$1,d4
  cmpi.w  #1,d4
  bne.s   .Not1
  lea     .Block1,a2
  subq.l  #1,d0
  bra.s   .GoClear
.Not1:
  lea     .NextBlock,a2
.GoClear:
  lsr.l   #1,d0
  subq.l  #1,d0
  subq.l  #1,d1
.NextLine:
  move.l  a0,a1
  move.l  d0,d7
  jmp     (a2)
  nop
.Block1:
  move.l  d3,(a1)+
.NextBlock:
  move.l  d3,(a1)+
  move.l  d3,(a1)+
  dbf     d7,.NextBlock
  adda.l  d2,a0
  dbf     d1,.NextLine
  movem.l (sp)+,d0-d4/a0-a2
  rts

;--------------------------------------
; Fast flat shading
;
; @in d0.w number of lines to draw
; @in a0.l data buffer
; @in a6.l Maggie register
;--------------------------------------
  xdef _M3D_FastFlatShading

_M3D_FastFlatShading:
  movem.l d0-a6,-(sp)
  moveq.l #0,d1
  move.l  d1,TEXUVAL(a6)
  move.l  d1,TEXVVAL(a6)
  move.l  d1,TEXDUVAL(a6)
  move.l  d1,TEXDVVAL(a6)
  move.w  d1,TEXDLIGHT(a6)
  fmove.s INT_LL(a0),fp0
  fmul.s  #65535.0,fp0
  fmove.w fp0,TEXLIGHT(a6)
  subq.w  #1,d0
.NextLine:

; Calcul edge coords
;    xs = floor(draw_data->crd_xl);
  fmove.s CRD_XL(a0),fp1
  fmove.l fp1,d1
;    xe = floor(draw_data->crd_xr);
  fmove.s CRD_XR(a0),fp2
  fmove.l fp2,d2

; Draw if line is not outside of clipping region
;    if (xs < draw_data->right_clip && xe >= draw_data->left_clip) {
  fcmp.s  RIGHT_CLIP(a0),fp1
  fbge.w  .Outside
  fcmp.s  LEFT_CLIP(a0),fp2
  fblt.w  .Outside

; Calcul interpolations
;      dz = draw_data->crd_zr - draw_data->crd_zl;
  fmove.s CRD_ZR(a0),fp4
  fsub.s  CRD_ZL(a0),fp4
;      dx = xe - xs + 1.0;
  move.l  d2,d3
  sub.l   d1,d3
  addq.l  #1,d3
  fmove.l d3,fp3

; DX could be 0 in some situations
;      if (dx > 0.0) {
  tst.l   d3
  ble.b   .ZeroDX
;        dz /= dx;
  fdiv    fp3,fp4
;      }
.ZeroDX:

; Calcul Z value
;      zi = draw_data->crd_zl;
  fmove.s CRD_ZL,fp5

; Horizontal clipping
;      if (xs < draw_data->left_clip) {
  fcmp.s  LEFT_CLIP(a0),fp1
  fbge.w  .NoLeftClip
;        cdx = draw_data->left_clip - xs;
  fmove.s LEFT_CLIP(a0),fp0
  fsub    fp1,fp0
;        zi += cdx * dz;
  fmul    fp4,fp0
  fadd    fp0,fp5
;        xs = draw_data->left_clip;
  fmove.s LEFT_CLIP(a0),fp1
  fmove.l fp1,d1
;        dx = xe - xs + 1.0;
  move.l  d2,d3
  sub.l   d1,d3
  addq.l  #1,d3
;      }
.NoLeftClip:

;      if (xe >= draw_data->right_clip) {
  fcmp.s  RIGHT_CLIP(a0),fp2
  fblt.w  .NoRightClip
;        dx = draw_data->right_clip - xs;
  fmove.s RIGHT_CLIP(a0),fp3
  fsub    fp1,fp3
  fmove.l fp3,d3
;      }
.NoRightClip:

; Destination address
;      dest = draw_data->dest_adr + ((LONG)xs * draw_data->dest_bpp);
  move.l  d1,d6
  mulu.w  DEST_BPP(a0),d6
  add.l   DEST_ADR(a0),d3
; Z buffer address
;      zbuf = draw_data->zbuf_adr + ((LONG)xs * draw_data->zbuf_bpp);
  move.l  d1,d7
  mulu.w  ZBUF_BPP(a0),d6
  add.l   ZBUF_ADR(a0),d3

; Start drawing
;      maggie->destination = (APTR) dest;
  move.l  d6,TEXDPT(a6)
;      maggie->zbuffer = (APTR) zbuf;
  move.l  d7,TEXZPT(a6)
;      maggie->z_start = (LFIXED) (zi * 65536.0);
  fmul.s  #65536.0,fp5
  fmove.w fp5,TEXZVAL(a6)
;      maggie->z_delta = (LFIXED) (dz * 65536.0);
  fmul.s  #65536.0,fp4
  fmove.w fp4,TEXDZVAL(a6)
;      WaitBlit();
.WaitBlitter:
  btst    #14,CUSTOMBASE+$2
  bne.s   .WaitBlitter
;      maggie->start_length = (UWORD) dx;
  move.w  d3,TEXSTRT(a6)
;      maggie->texture = NULL;
;      M3D_EmulateMaggie();
  IFNE    EMULATE_MAGGIE
  move.l  #0,TEXTPT(a6)
  jsr     _M3D_FastEmulateMaggie
  ENDC
;    }

.Outside:
; Interpolate next points
;    draw_data->crd_xl += draw_data->delta_dxdyl;
  fmove.s CRD_XL(a0),fp0
  fadd.s  DELTA_DXDYL(a0),fp0
  fmove.s fp0,CRD_XL(a0)
;    draw_data->crd_zl += draw_data->delta_dzdyl;
  fmove.s CRD_ZL(a0),fp0
  fadd.s  DELTA_DZDYL(a0),fp0
  fmove.s fp0,CRD_ZL(a0)
;    draw_data->crd_xr += draw_data->delta_dxdyr;
  fmove.s CRD_XR(a0),fp0
  fadd.s  DELTA_DXDYR(a0),fp0
  fmove.s fp0,CRD_XR(a0)
;    draw_data->crd_zr += draw_data->delta_dzdyr;
  fmove.s CRD_ZR(a0),fp0
  fadd.s  DELTA_DZDYR(a0),fp0
  fmove.s fp0,CRD_ZR(a0)

; Next line address
;    draw_data->dest_adr += draw_data->dest_bpr;
  move.l  DEST_ADR(a0),d7
  add.l   DEST_BPR(a0),d7
  move.l  d7,DEST_ADR(a0)
;    draw_data->zbuf_adr += draw_data->zbuf_bpr;
  move.l  ZBUF_ADR(a0),d7
  add.l   ZBUF_BPR(a0),d7
  move.l  d7,ZBUF_ADR(a0)

  dbf     d0,.NextLine
  movem.l (sp)+,d0-a6
  rts

;--------------------------------------
;  MAGGIE EMULATION
;--------------------------------------


; Blend texel with color (d4 texel)
M3D_FastMaggieBlend:
  movem.l d0-d2,-(sp)
;  tr = (texel >> 16) & 0xff;
  move.l  d4,d0
  swap    d0
  andi.l  #$ff,d0
;  tg = (texel >> 8) & 0xff;
  move.l  d4,d1
  lsr.w   #8,d1
  andi.l  #$ff,d1
;  tb = texel & 0xff;
  move.l  d4,d2
  andi.l  #$ff,d2
;  tr = tr * red / 0xff;
  mulu.w  TEX_CRED(a4),d0
  divu.w  #255,d0
  andi.l  #$ff,d0
;  tg = tg * green / 0xff;
  mulu.w  TEX_CGREEN(a4),d1
  divu.w  #255,d1
  andi.l  #$ff,d1
;  tb = tb * blue / 0xff;
  mulu.w  TEX_CBLUE(a4),d2
  divu.w  #255,d2
  andi.l  #$ff,d2
;  return ((tr << 16) | (tg << 8) | tb);
  move.l  d2,d4
  rol.l   #8,d1
  or.l    d1,d4
  swap    d0
  or.l    d0,d4
  movem.l (sp)+,d0-d2
  rts

; Shade texel with light (d4 texel)
M3D_FastMaggieShade:
  movem.l d0-d3,-(sp)
  lsr.w   #8,d3
;  tr = (texel >> 16) & 0xff;
  move.l  d4,d0
  swap    d0
  andi.l  #$ff,d0
;  tg = (texel >> 8) & 0xff;
  move.l  d4,d1
  lsr.w   #8,d1
  andi.l  #$ff,d1
;  tb = texel & 0xff;
  move.l  d4,d2
  andi.l  #$ff,d2
;  tr = tr * shade / 0xff;
  mulu.w  d3,d0
  divu.w  #255,d0
  andi.l  #$ff,d0
;  tg = tg * shade / 0xff;
  mulu.w  d3,d1
  divu.w  #255,d1
  andi.l  #$ff,d1
;  tb = tb * shade / 0xff;
  mulu.w  d3,d2
  divu.w  #255,d2
  andi.l  #$ff,d2
;  return ((tr << 16) | (tg << 8) | tb);
  move.l  d2,d4
  rol.l   #8,d1
  or.l    d1,d4
  swap    d0
  or.l    d0,d4
  movem.l (sp)+,d0-d3
  rts

; Output 16bits texel (d4 texel, a1 dest)
M3D_FastMaggie16Bits:
  movem.l d0-d2,-(sp)
  move.l  d4,d0
  lsr.l   #8,d0
  andi.w  #$f800,d0
  move.l  d4,d1
  andi.w  #$fc00,d1
  lsr.w   #5,d1
  move.l  d4,d2
  andi.w  #$f8,d2
  lsr.w   #3,d2
  or.w    d0,d1
  or.w    d1,d2
  move.w  d2,(a1)
  movem.l (sp)+,d0-d2
  rts

; Output 24bits texel (d4 texel, a1 dest)
M3D_FastMaggie24Bits:
  move.b  d4,2(a1)
  lsr.l   #8,d4
  move.b  d4,1(a1)
  lsr.l   #8,d4
  move.b  d4,(a1)
  rts

; Output 32bits texel (d4 texel, a1 dest)
M3D_FastMaggie32Bits:
  move.l  d4,(a1)
  rts

;--------------------------------------
; Fast maggie emulation
;
; @in a6.l maggie register base
;--------------------------------------
  xdef _M3D_FastEmulateMaggie

_M3D_FastEmulateMaggie:
  movem.l d0-a6,-(sp)

  lea     MaggieEmulation,a4

;  ui = maggie->u_start;
;  vi = maggie->v_start;
;  zi = maggie->z_start;
;  li = maggie->light_start;
  move.l  TEXUVAL(a6),d0            ; ui
  move.l  TEXVVAL(a6),d1            ; vi
  move.l  TEXZVAL(a6),d2            ; zi
  move.w  TEXLIGHT(a6),d3           ; li

;  red = (UBYTE) ((maggie->color >> 16) & 0xff);
  move.l  TEXLCOLOR(a6),d4
  swap    d4
  andi.l  #$ff,d4
  move.w  d4,TEX_CRED(a4)
;  green = (UBYTE) ((maggie->color >> 8) & 0xff);
  move.l  TEXLCOLOR(a6),d4
  lsr.l   #8,d4
  andi.l  #$ff,d4
  move.w  d4,TEX_CGREEN(a4)
;  blue = (UBYTE) (maggie->color & 0xff);
  move.l  TEXLCOLOR(a6),d4
  andi.l  #$ff,d4
  move.w  d4,TEX_CBLUE(a4)

;  texture = (UBYTE *) maggie->texture;
;  destination = (UBYTE *) maggie->destination;
;  zbuffer = (UWORD *) maggie->zbuffer;
  move.l  TEXTPT(a6),a0             ; texture
  move.l  TEXDPT(a6),a1             ; destination
  move.l  TEXZPT(a6),a2             ; zbuffer

;  tex_bpr = 4 << maggie->tex_size;
  moveq.l #4,d4
  moveq.l #0,d5
  move.w  TEXMIP(a6),d5
  lsl.l   d5,d4
  move.l  d4,TEX_BPR(a4)

;  tex_scale = 16 + (8 - maggie->tex_size);
  moveq.l #8,d5
  sub.w   TEXMIP(a6),d5
  addi.w  #16,d5
  move.l  d5,TEX_SCALE(a4)

;  count = maggie->start_length;
  move.w  TEXSTRT(a6),d7            ; count
  subq.l  #1,d7

.RenderingLoop:
;    if (texture == NULL) {
  cmpa.l  #0,a0
  bne.s   .TexMapping
.FlatMapping:
;      texel = 0xffffffff;
  move.l  #$ffffffff,d4
;    } else {
  bra.s   .CheckTransparency
.TexMapping:
  move.l  TEX_BPR(a4),d4
  move.l  TEX_SCALE(a4),d5

;      index = ((vi >> tex_scale) * tex_bpr) + ((ui >> tex_scale) * 4);
  move.l  d1,d6
  lsr.l   d5,d6
  mulu.w  d6,d4
  move.l  d0,d6
  lsr.l   d5,d6
  mulu.w  #4,d6
  add.l   d6,d4
;      texel = (texture[index] << 24) | (texture[index+1] << 16) | (texture[index+2] << 8) | texture[index+3];
  move.l  a0,a3
  add.l   d4,a3
  move.l  (a3),d4

;    }

.CheckTransparency:
;    if ((texel & 0xff) >= 0x80) {
  cmpi.b  #$80,d4
  blo.s   .NothingToDraw

.CheckZBuffer:
;      draw_texel = TRUE;
;      if (maggie->mode & M3D_ZBUFFER) {
  move.w  TEXMODE(a6),d5
  andi.w  #MODE_ZBUFFER,d5
  beq.s   .DrawTexel
;        if ((zi >> 16) > *zbuffer) {
  move.l  d2,d5
  swap    d5
  cmp.w   (a2),d5
;          draw_texel = FALSE;
  bhi.s   .NothingToDraw
;        } else {
;          *zbuffer = (zi >> 16);
  move.w  d5,(a2)
;        }
;      }
;      if (draw_texel) {
.DrawTexel:
;        texel >>= 8;
  lsr.l   #8,d4
;        texel = M3D_MaggieBlend(texel, red, green ,blue);
  bsr     M3D_FastMaggieBlend
;        texel = M3D_MaggieShade(texel, (UBYTE) (li >> 8));
  bsr     M3D_FastMaggieShade
;        if (maggie->mode & M3D_16BITS) {
.Rendering16Bits:
  move.w  TEXMODE(a6),d5
  andi.w  #MODE_16BITS,d5
  beq.s   .Rendering24Bits
;          M3D_Maggie16Bits((UWORD *) destination, texel);
  bsr     M3D_FastMaggie16Bits
  bra.s   .NothingToDraw
;        } else if (maggie->mode & M3D_24BITS) {
.Rendering24Bits:
  move.w  TEXMODE(a6),d5
  andi.w  #MODE_24BITS,d5
  beq.s   .Rendering32Bits
;          M3D_Maggie24Bits(destination, texel);
  bsr     M3D_FastMaggie24Bits
  bra.s   .NothingToDraw
;        } else {
.Rendering32Bits:
;          M3D_Maggie32Bits((ULONG *) destination, texel);
  bsr     M3D_FastMaggie32Bits
;        }
;      }

.NothingToDraw:
;    }
;    destination += maggie->modulo;
  adda.w  TEXDMOD(a6),a1
;    zbuffer++;
  adda.w  #2,a2
;    ui += maggie->u_delta;
  add.l   TEXDUVAL(a6),d0
;    vi += maggie->v_delta;
  add.l   TEXDVVAL(a6),d1
;    zi += maggie->z_delta;
  add.l   TEXDZVAL(a6),d2
;    li += maggie->light_delta;
  add.w   TEXDLIGHT(a6),d3
;  }
  dbf     d7,.RenderingLoop

.Return:
  movem.l (sp)+,d0-a6
  rts

MaggieEmulation:
  dc.l 0,0                  ; tex_bpr, tex_scale
  dc.w 0,0,0                ; tex_crd, tex_cgreen, tex_cblue

  END
