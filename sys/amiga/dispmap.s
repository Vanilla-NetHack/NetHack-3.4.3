;    SCCS Id: @(#)dispmap.s    3.2    94/04/19
;Copyright (c) Andrew Church, Olney, Maryland,  1994.
;NetHack may be freely redistributed.  See license for details.

;Display the game map (using tiles).
;
;Call from C: display_map(struct Window *win)
;where win is the window to draw the map in.
;
;At the moment, this routine is limited to tile display sizes of 8, 16,
;24, 32, and 48 pixels square (determined by mxsize/mysize).
;
;This code depends on structs amiv_glyph_node and PDAT.  If either of
;these are changed, the corresponding offsets below will also need to be
;changed.

;struct BitMap
bm_BytesPerRow	equ 0
bm_Rows		equ 2
bm_Flags	equ 4
bm_Depth	equ 5
bm_Planes	equ 8
bm_sizeof	equ 40

;struct Window (incomplete)
wd_LeftEdge	equ 4
wd_TopEdge	equ 6
wd_RPort	equ 50
wd_BorderLeft	equ 54
wd_BorderTop	equ 55

;struct RastPort (incomplete)
rp_BitMap	equ 4

;struct amiv_glyph_node
gn_odstx	equ 0
gn_odsty	equ 2
gn_srcx		equ 4
gn_srcy		equ 6
gn_dstx		equ 8
gn_dsty		equ 10
gn_bitmap	equ 12
gn_sizeof	equ 16

;struct PDAT
pdat_nplanes	equ 0
pdat_pbytes	equ 4
pdat_across	equ 8
pdat_down	equ 12
pdat_npics	equ 16
pdat_xsize	equ 20
pdat_ysize	equ 24
pdat_sizeof	equ 28

	section text,code

	xdef	_display_map
	xdef	loop		;for debugging
	xdef	put8		; |
	xdef	put16		; |
	xdef	put24		; |
	xdef	put32		; |
	xdef	put48		; V
	xref	_glyph_node_index
	xref	_amiv_g_nodes
	xref	_pictdata
	xref	_mxsize
	xref	_mysize
	xref	_clipping
	xref	_clipx
	xref	_clipy
	xref	_clipxmax
	xref	_clipymax
	xref	_amii_extraplanes
	xref	_reclip

;display_map(struct Window *vw)
;
;Register usage:
;	D0 - temp
;	D1 - temp
;	D2 - glyph_node loop counter
;	D3 - temp
;	D4 - Number of planes to use
;	D5 - index (bytes) into tile bitmap
;	D6 - temp
;	D7 - temp
;	A0 - tile bitmap
;	A1 - overview window bitmap
;	A2 - amiv_g_nodes[]
;	A3 - tile display routine (depends on requested tile size)
;	A4 - data base (from main program)
;	A5 - source bitplane \ used by
;	A6 - dest bitplane   /  putNN
;
;Passed to putNN on stack:
;     [ 0(a7)	- return address ]
;	4(a7)	- BytesPerRow of tile bitmap
;       6(a7)	- BytesPerRow of overview bitmap
;	8(a7)	- Bitmap offset to beginning of top row of window
;	12(a7)	- X coordinate of left edge of window

_display_map:
	cmp.l	#2,_reclip
	bne	dispmap
	rts
dispmap	move.l	4(a7),a1
	movem.l d2-d7/a2-a3/a5-a6,-(a7)
	move.l	a1,a6
	move.l	wd_RPort(a6),a1
	move.l	rp_BitMap(a1),a1
	move.w	bm_BytesPerRow(a1),d0
	pea	bm_Planes(a1)
	lea	_pictdata,a0
	move.l	pdat_nplanes(a0),d4
	add.l	_amii_extraplanes,d4
	move.w	d4,-(a7)
	move.w	wd_TopEdge(a6),d1
	mulu	d0,d1
	move.w	wd_LeftEdge(a6),-(a7)
	move.l	d1,-(a7)
	move.w	d0,-(a7)
	subq.l	#2,a7
	lea	_amiv_g_nodes,a2
	move.l	_mxsize,d5
	move.w	_glyph_node_index,d2
	subq.w	#1,d2
	cmp.w	#8,d5
	beq	set8
	cmp.w	#16,d5
	beq	set16
	cmp.w	#24,d5
	beq	set24
	cmp.w	#32,d5
	beq	set32
	lea	put48(pc),a3
	bra	loop
set8	lea	put8(pc),a3
	bra	loop
set16	lea	put16(pc),a3
	bra	loop
set24	lea	put24(pc),a3
	bra	loop
set32	lea	put32(pc),a3
loop	move.l	gn_bitmap(a2),a0
	tst.l	_clipping
	beq	noclip
	moveq	#0,d0
	move.w	gn_odstx(a2),d0
	cmp.l	_clipx,d0
	blt	endlp
	cmp.l	_clipxmax,d0
	bge	endlp
	move.w	gn_odsty(a2),d0
	cmp.l	_clipy,d0
	blt	endlp
	cmp.l	_clipymax,d0
	bge	endlp
noclip	moveq	#0,d5
	move.w	gn_srcx(a2),d5
	lsr.w	#3,d5
	move.w	gn_srcy(a2),d0
	move.w	bm_BytesPerRow(a0),d7
	mulu	d7,d0
	add.l	d0,d5
	lea	bm_Planes(a0),a0
	move.w	d7,(a7)
	move.w	10(a7),d4
	move.l	12(a7),a1
	jsr	(a3)
endlp	lea	gn_sizeof(a2),a2
	dbf	d2,loop
	lea	16(a7),a7
	movem.l	(a7)+,d2-d7/a2-a3/a5-a6
	rts


put8:
	subq.w	#1,d4
	move.w	4(a7),d7
	add.w	d7,d7
	moveq	#0,d3
	move.w	gn_dstx(a2),d3
	add.w	12(a7),d3
	lsr.w	#3,d3
	move.w	gn_dsty(a2),d0
	mulu	6(a7),d0
	add.l	d0,d3
	add.l	8(a7),d3
p8Plp	moveq	#7,d6
	move.l	(a0)+,a5
	add.l	d5,a5
	move.l	(a1)+,a6
	add.l	d3,a6
p8Ylp	move.w	(a5),d0		;No loops here - they'd slow this down
	lsl.w	#1,d0
	roxl.b	#1,d1
	lsl.w	#2,d0
	roxl.b	#1,d1
	lsl.w	#2,d0
	roxl.b	#1,d1
	lsl.w	#2,d0
	roxl.b	#1,d1
	lsl.w	#2,d0
	roxl.b	#1,d1
	lsl.w	#2,d0
	roxl.b	#1,d1
	lsl.w	#2,d0
	roxl.b	#1,d1
	lsl.w	#2,d0
	roxl.b	#1,d1
	move.b	d1,(a6)
	add.w	d7,a5
	add.w	6(a7),a6
	dbf	d6,p8Ylp
	dbf	d4,p8Plp
	rts

put16:
	subq.w	#1,d4
	move.w	6(a7),d7
	moveq	#0,d3
	move.w	gn_dstx(a2),d3
	add.w	12(a7),d3
	lsr.w	#3,d3
	move.w	gn_dsty(a2),d0
	mulu	d7,d0
	add.l	d0,d3
	add.l	8(a7),d3
p16Plp	moveq	#15,d6
	move.l	(a0)+,a5
	add.l	d5,a5
	move.l	(a1)+,a6
	add.l	d3,a6
p16Ylp	move.w	(a5),d0
	move.b	d0,1(a6)
	lsr.w	#8,d0
	move.b	d0,(a6)
	add.w	4(a7),a5
	add.w	d7,a6
	dbf	d6,p16Ylp
	dbf	d4,p16Plp
	rts

put24:
	move.w	d2,-(a7)
	subq.w	#1,d4
	move.w	8(a7),d7
	moveq	#0,d3
	move.w	gn_dstx(a2),d3
	add.w	14(a7),d3
	lsr.w	#3,d3
	move.w	gn_dsty(a2),d0
	mulu	d7,d0
	add.l	d0,d3
	add.l	10(a7),d3
p24Plp	moveq	#7,d6
	move.l	(a0)+,a5
	add.l	d5,a5
	move.l	(a1)+,a6
	add.l	d3,a6
p24Ylp	move.w	(a5),d0
	moveq	#0,d1
	add.w	d0,d0
	scs	d2
	and.b	#3,d2
	or.b	d2,d1
	add.w	d0,d0
	roxl.l	#1,d1
	add.w	d0,d0
	scs	d2
	and.b	#3,d2
	lsl.l	#2,d1
	or.b	d2,d1
	add.w	d0,d0
	roxl.l	#1,d1
	add.w	d0,d0
	scs	d2
	and.b	#3,d2
	lsl.l	#2,d1
	or.b	d2,d1
	add.w	d0,d0
	roxl.l	#1,d1
	add.w	d0,d0
	scs	d2
	and.b	#3,d2
	lsl.l	#2,d1
	or.b	d2,d1
	add.w	d0,d0
	roxl.l	#1,d1
	add.w	d0,d0
	scs	d2
	and.b	#3,d2
	lsl.l	#2,d1
	or.b	d2,d1
	add.w	d0,d0
	roxl.l	#1,d1
	add.w	d0,d0
	scs	d2
	and.b	#3,d2
	lsl.l	#2,d1
	or.b	d2,d1
	add.w	d0,d0
	roxl.l	#1,d1
	add.w	d0,d0
	scs	d2
	and.b	#3,d2
	lsl.l	#2,d1
	or.b	d2,d1
	add.w	d0,d0
	roxl.l	#1,d1
	add.w	d0,d0
	scs	d2
	and.b	#3,d2
	lsl.l	#2,d1
	or.b	d2,d1
	add.w	d0,d0
	roxl.l	#1,d1
	swap	d1
	move.b	d1,(a6)
	swap	d1
	move.b	d1,2(a6)
	lsr.w	#8,d1
	move.b	d1,1(a6)
	add.w	6(a7),a5
	add.w	d7,a6
	move.w	(a5),d0
	moveq	#0,d1
	add.w	d0,d0
	scs	d2
	and.b	#3,d2
	or.b	d2,d1
	add.w	d0,d0
	roxl.l	#1,d1
	add.w	d0,d0
	scs	d2
	and.b	#3,d2
	lsl.l	#2,d1
	or.b	d2,d1
	add.w	d0,d0
	roxl.l	#1,d1
	add.w	d0,d0
	scs	d2
	and.b	#3,d2
	lsl.l	#2,d1
	or.b	d2,d1
	add.w	d0,d0
	roxl.l	#1,d1
	add.w	d0,d0
	scs	d2
	and.b	#3,d2
	lsl.l	#2,d1
	or.b	d2,d1
	add.w	d0,d0
	roxl.l	#1,d1
	add.w	d0,d0
	scs	d2
	and.b	#3,d2
	lsl.l	#2,d1
	or.b	d2,d1
	add.w	d0,d0
	roxl.l	#1,d1
	add.w	d0,d0
	scs	d2
	and.b	#3,d2
	lsl.l	#2,d1
	or.b	d2,d1
	add.w	d0,d0
	roxl.l	#1,d1
	add.w	d0,d0
	scs	d2
	and.b	#3,d2
	lsl.l	#2,d1
	or.b	d2,d1
	add.w	d0,d0
	roxl.l	#1,d1
	add.w	d0,d0
	scs	d2
	and.b	#3,d2
	lsl.l	#2,d1
	or.b	d2,d1
	add.w	d0,d0
	roxl.l	#1,d1
	swap	d1
	move.b	d1,0(a6,d7.w)
	move.b	d1,(a6)
	swap	d1
	move.b	d1,2(a6,d7.w)
	move.b	d1,2(a6)
	lsr.w	#8,d1
	move.b	d1,1(a6,d7.w)
	move.b	d1,1(a6)
	add.w	6(a7),a5
	add.w	d7,a6
	add.w	d7,a6
	dbf	d6,p24Ylp
	dbf	d4,p24Plp
	move.w	(a7)+,d2
	rts

put32:
	move.w	d2,-(a7)
	subq.w	#1,d4
	move.w	8(a7),d7
	moveq	#0,d3
	move.w	gn_dstx(a2),d3
	add.w	14(a7),d3
	lsr.w	#3,d3
	move.w	gn_dsty(a2),d0
	mulu	d7,d0
	add.l	d0,d3
	add.l	10(a7),d3
p32Plp	moveq	#15,d6
	move.l	(a0)+,a5
	add.l	d5,a5
	move.l	(a1)+,a6
	add.l	d3,a6
p32Ylp	move.w	(a5),d0
	moveq	#0,d1
	add.w	d0,d0
	scs	d2
	and.b	#$C0,d2
	or.b	d2,d1
	add.w	d0,d0
	scs	d2
	and.b	#$30,d2
	or.b	d2,d1
	add.w	d0,d0
	scs	d2
	and.b	#$0C,d2
	or.b	d2,d1
	add.w	d0,d0
	scs	d2
	and.b	#$03,d2
	or.b	d2,d1
	move.b	d1,0(a6,d7.w)
	move.b	d1,(a6)+
	moveq	#0,d1
	add.w	d0,d0
	scs	d2
	and.b	#$C0,d2
	or.b	d2,d1
	add.w	d0,d0
	scs	d2
	and.b	#$30,d2
	or.b	d2,d1
	add.w	d0,d0
	scs	d2
	and.b	#$0C,d2
	or.b	d2,d1
	add.w	d0,d0
	scs	d2
	and.b	#$03,d2
	or.b	d2,d1
	move.b	d1,0(a6,d7.w)
	move.b	d1,(a6)+
	moveq	#0,d1
	add.w	d0,d0
	scs	d2
	and.b	#$C0,d2
	or.b	d2,d1
	add.w	d0,d0
	scs	d2
	and.b	#$30,d2
	or.b	d2,d1
	add.w	d0,d0
	scs	d2
	and.b	#$0C,d2
	or.b	d2,d1
	add.w	d0,d0
	scs	d2
	and.b	#$03,d2
	or.b	d2,d1
	move.b	d1,0(a6,d7.w)
	move.b	d1,(a6)+
	moveq	#0,d1
	add.w	d0,d0
	scs	d2
	and.b	#$C0,d2
	or.b	d2,d1
	add.w	d0,d0
	scs	d2
	and.b	#$30,d2
	or.b	d2,d1
	add.w	d0,d0
	scs	d2
	and.b	#$0C,d2
	or.b	d2,d1
	add.w	d0,d0
	scs	d2
	and.b	#$03,d2
	or.b	d2,d1
	move.b	d1,0(a6,d7.w)
	move.b	d1,(a6)
	add.w	6(a7),a5
	add.w	d7,a6
	lea	-3(a6,d7.w),a6
	dbf	d6,p32Ylp
	dbf	d4,p32Plp
	move.w	(a7)+,d2
	rts

put48:
	move.w	d2,-(a7)
	subq.w	#1,d4
	move.w	d4,-(a7)
	move.l	d5,-(a7)
	move.w	14(a7),d7
	moveq	#0,d3
	move.w	gn_dstx(a2),d3
	add.w	20(a7),d3
	lsr.w	#3,d3
	move.w	gn_dsty(a2),d0
	add.l	16(a7),d3
	mulu	d7,d0
	add.l	d0,d3
	move.w	d7,d5
	add.w	d5,d5
p48Plp	moveq	#15,d6
	move.l	(a0)+,a5
	add.l	(a7),a5
	move.l	(a1)+,a6
	add.l	d3,a6
p48Ylp	move.w	(a5),d0
	moveq	#0,d1
	moveq	#21,d4
	moveq	#0,d2
	add.w	d0,d0
	scs	d2
	and.b	#7,d2
	lsl.l	d4,d2
	or.l	d2,d1
	subq.w	#3,d4
	moveq	#0,d2
	add.w	d0,d0
	scs	d2
	and.b	#7,d2
	lsl.l	d4,d2
	or.l	d2,d1
	subq.w	#3,d4
	moveq	#0,d2
	add.w	d0,d0
	scs	d2
	and.b	#7,d2
	lsl.l	d4,d2
	or.l	d2,d1
	add.w	d0,d0
	scs	d2
	ext.w	d2
	and.w	#$7000,d2
	or.w	d2,d1
	add.w	d0,d0
	scs	d2
	ext.w	d2
	and.w	#$E00,d2
	or.w	d2,d1
	add.w	d0,d0
	scs	d2
	ext.w	d2
	and.w	#$1C0,d2
	or.w	d2,d1
	add.w	d0,d0
	scs	d2
	and.b	#$38,d2
	or.b	d2,d1
	add.w	d0,d0
	scs	d2
	ext.w	d2
	and.b	#7,d2
	or.b	d2,d1
	swap	d1
	move.b	d1,0(a6,d7.w)
	move.b	d1,0(a6,d5.w)
	move.b	d1,(a6)+
	rol.l	#8,d1
	move.b	d1,0(a6,d7.w)
	move.b	d1,0(a6,d5.w)
	move.b	d1,(a6)+
	rol.l	#8,d1
	move.b	d1,0(a6,d7.w)
	move.b	d1,0(a6,d5.w)
	move.b	d1,(a6)+
	moveq	#0,d1
	moveq	#21,d4
	moveq	#0,d2
	add.w	d0,d0
	scs	d2
	and.b	#7,d2
	lsl.l	d4,d2
	or.l	d2,d1
	subq.w	#3,d4
	moveq	#0,d2
	add.w	d0,d0
	scs	d2
	and.b	#7,d2
	lsl.l	d4,d2
	or.l	d2,d1
	subq.w	#3,d4
	moveq	#0,d2
	add.w	d0,d0
	scs	d2
	and.b	#7,d2
	lsl.l	d4,d2
	or.l	d2,d1
	add.w	d0,d0
	scs	d2
	ext.w	d2
	and.w	#$7000,d2
	or.w	d2,d1
	add.w	d0,d0
	scs	d2
	ext.w	d2
	and.w	#$E00,d2
	or.w	d2,d1
	add.w	d0,d0
	scs	d2
	ext.w	d2
	and.w	#$1C0,d2
	or.w	d2,d1
	add.w	d0,d0
	scs	d2
	and.b	#$38,d2
	or.b	d2,d1
	add.w	d0,d0
	scs	d2
	ext.w	d2
	and.b	#7,d2
	or.b	d2,d1
	swap	d1
	move.b	d1,0(a6,d7.w)
	move.b	d1,0(a6,d5.w)
	move.b	d1,(a6)+
	rol.l	#8,d1
	move.b	d1,0(a6,d7.w)
	move.b	d1,0(a6,d5.w)
	move.b	d1,(a6)+
	rol.l	#8,d1
	move.b	d1,0(a6,d7.w)
	move.b	d1,0(a6,d5.w)
	move.b	d1,(a6)
	add.w	12(a7),a5
	add.w	d7,a6
	lea	-5(a6,d5.w),a6
	dbf	d6,p48Ylp
	subq.w	#1,4(a7)
	bpl	p48Plp
	move.l	(a7)+,d5
	addq.l	#2,a7
	move.w	(a7)+,d2
	rts

	end
