;	SCCS Id: @(#)ovlmgr.asm 	3.0	89/11/16
;  Copyright (c) Pierre Martineau and Stephen Spackman, 1989.
;  This product may be freely redistributed.  See NetHack license for details.

		PAGE	60,132
		TITLE	'Overlay manager for use with Microsoft overlay linker'
		SUBTTL	'Brought to you by Pierre Martineau and Stephen Spackman'

; acknowledgements:   - No thanks to Microsoft
;		      - alltrsidsctysti!!!
;		      - izchak and friends for impetus
;		      - us for brilliance
;		      - coffee for speed
;		      - others as necessary

; assumptions:	      - all registers are preserved including flags
;		      - the stack is preserved
;		      - re-entrancy is not required

DOSALLOC	equ	48h			; memory allocation
DOSFREE 	equ	49h			; free allocated memory
DOSREALLOC	equ	4ah			; modify memory block
DOSREAD 	equ	3fh			; read bytes from handle
DOSSEEK 	equ	42h			; logical handle seek
DOSOPEN 	equ	3dh			; open handle
DOSCLOSE	equ	3eh			; close handle
DOSGETVEC	equ	35h			; get interrupt vector
DOSSETVEC	equ	25h			; set interrupt vector
DOSEXEC 	equ	4bh			; exec child process
DOS		equ	21h			; Dos interrupt #
PRINT		equ	09h			; print string
TERMINATE	equ	4ch			; terminate process
CR		equ	0dh
LF		equ	0ah
BELL		equ	07h
FAERIE		equ	0h			; Used for dummy segment allocation
PARSIZ		equ	10h			; this is the size of a paragraph - this better not change!

; The following extrns are supplied by the linker

extrn		$$OVLBASE:byte			; segment of OVERLAY_AREA
extrn		$$MPGSNOVL:byte 		; ^ to module table
extrn		$$MPGSNBASE:word		; ^ to module segment fixups
extrn		$$INTNO:byte			; interrupt number to be used
extrn		$$COVL:word			; number of physical overlays
extrn		$$CGSN:word			; number of modules
extrn		$$MAIN:far			; ^ to function main()

public		$$OVLINIT			; Our entry point
						; called by the c startup code

ovlflgrec	record	running:1=0,locked:1=0,loaded:1=0 ; overlay flags

; This is a dirty hack. What we need is a virtual segment that will be built
; by the (our) loader in multiple copies, one per overlay. Unfortunately, this
; doesn't seem to be a sensible idea in the minds of the folks at Microsoft.
; Declaring this segment AT will ensure that it never appears in the exefile,
; and ASSUME is dumb enough to be fooled.
;
; The reason we want to do this is also not-to-be-tried-at-home: it turns out
; that we can code a faster interrupt handler if we map overlay numbers to
; segment values. Normally I would consider this unacceptable programming
; practise because it is 86-mode specific, but the *need* for this entire
; programme is 86-mode specific, anyway.

pspseg		segment para at FAERIE		; dummy segment for psp
		org	2ch			; ^ to segment of environmemt in psp
pspenv		LABEL	WORD
pspseg		ends

ovltbl		segment para at FAERIE		; Dummy segment definition for overlay table

; NOTE: This segment definition MUST be exactly 16 bytes long

ovlflg		ovlflgrec	<0,0,0> 	; overlay flags
ovltblpad1	db	?			; go ahead, delete me!
ovlmemblk	dw	?			; ^ to allocated memory block
ovlseg		dw	0			; ovl segment physical add.
ovlfiloff	dw	?			; ovl file offset in pages (512 bytes)
ovlsiz		dw	?			; ovl size in paragraphs
ovllrudat	dd	0			; misc lru data (pseudo time stamp)
ovltblpad2	dw	?			; go ahead, delete me!

if1
if		$ gt PARSIZ
		.err
		%out This segment MUST be no more than 16 bytes, REALLY!!!
endif
endif

ovlsegsiz	equ	PARSIZ			; this had better be true!!! (16 bytes)

ovltbl		ends

EXEHDR		struc				; structure of an EXE header
exesign 	dw	5a4dh			; signature
exelstpgesiz	dw	?			; last page size (512 byte pages)
exesiz		dw	?			; total pages (including partial last page)
relocitems	dw	?			; number of relocation entries
hdrparas	dw	?			; number of paragraphs in the header
minalloc	dw	?			; minimum paragraph allocation
maxalloc	dw	?			; maximum patagraph allocation
exess		dw	?			; initial stack segment
exesp		dw	?			; initial stack pointer
exechksum	dw	?			; checksum
exeip		dw	?			; initial instruction pointer
execs		dw	?			; initial code segment
reloctbloff	dw	?			; offset from beginning of header to relocation table
exeovlnum	dw	?			; overlay number
EXEHDR		ends

MASK_used	equ	1			; memory block flag

memctlblk	struc				; memory block structure
memblkflg	db	0			; flags
memblkpad1	db	0			; go ahead, delete me!
memblknxt	dw	0			; ^ to next block
memblkprv	dw	0			; ^ to previous block
memblkovl	dw	0			; ^ to overlay occupying this block
memblksiz	dw	0			; size in paragraphs
memblkpad	db	PARSIZ - ($ - memblkflg) mod parsiz dup (?) ; pad to 16 bytes
memctlblk	ends

memctlblksiz	equ	memblkpad + SIZE memblkpad ; should equal 1 paragraph (16 bytes)

;-------------------------------------------------------------------------------

code		segment public

ovlexefilhdl	dw	-1			; always-open file handle of our .EXE
ovltim		dd	0			; pseudo-lru time variable
curovl		dw	offset framestk 	; ^ into stack frame
ovlcnt		dw	0			; # overlays
modcnt		dw	0			; # of modules
ovltblbse	dw	-1			; segment of first overlay descriptor
ovlrootcode	dw	0			; logical segment of OVERLAY_AREA
ovldata 	dw	0			; logical segment of OVERLAY_END
memblk1st	dw	0			; first memory block
pspadd		dw	0			; our psp address + 10h (for relocations)
oldvec		dd	-1			; saved interrupt vector
oldint21	dd	-1			; saved int 21 vector
memstat 	db	0ffh			; must we re-allocate some memory
bxreg		dw	0			; temp save area
esreg		dw	0			; temp save area
farcall 	dd	0			; internal trampoline.
hdr		EXEHDR	<>			; EXE header work area
hdrsize 	equ	$ - hdr

framestk	dw	100h dup (0)		; internal stack

moduletbl	dw	256*2 dup (0)		; module lookup table

noroom		db	CR,LF,'Not enough memory to run this program.  Time to go to the store.',CR,LF,BELL,'$'
nocore		db	CR,LF,'Your dog eats all your remaining memory!  You die.',CR,LF,BELL,'$'
nofile		db	CR,LF,'The Nymph stole your .EXE file!  You die.',CR,LF,BELL,'$'
exitmsg 	db	CR,LF,'$'

;-------------------------------------------------------------------------------

$$OVLINIT	proc	far			; Init entry point

		assume	cs:code,ds:pspseg,es:nothing

		push	ax
		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	bp
		push	ds
		push	es			; save the world
		mov	ax,ds			; get our psp
		add	ax,10h
		mov	pspadd,ax		; save it
		mov	ds,pspenv		; get environment segment
		mov	si,-1
envloop:					; search for end of environment
		inc	si
		cmp	word ptr [si],0
		jnz	envloop
		add	si,4			; point to EXE filename
		mov	al,0			; access code
		mov	ah,DOSOPEN
		mov	dx,si
		int	DOS			; open EXE
		jnc	dontdie
		mov	al,5
		mov	dx,offset nofile
		jmp	putserr 		; cry to the world!
dontdie:
		mov	ovlexefilhdl,ax 	; save handle
		mov	ax,SEG $$OVLBASE	; OVERLAY_AREA segment
		mov	ovlrootcode,ax

; Now allocate memory
		mov	bx,0900h		; allocate memory for malloc()
		mov	ah,DOSALLOC
		int	DOS
		jnc	getmore
		jmp	buyram
getmore:
		mov	es,ax			; find largest free memory
		mov	ah,DOSALLOC
		mov	bx,0ffffh		; Everything
		int	DOS
		mov	ah,DOSALLOC		; allocate our own memory
		int	DOS
		jnc	gotitall
		jmp	buyram
gotitall:
		mov	memstat,0		; indicate that we have memory
		mov	ovltblbse,ax		; overlay descriptor table begins at start of memory block
		mov	ax,SEG $$COVL		; segment of DGROUP
		mov	ds,ax
		mov	cx,$$CGSN		; number of modules
		mov	modcnt,cx		; save for later use
		mov	cx,$$COVL		; number of physical overlays
		mov	ovlcnt,cx		; save for later use
		sub	bx,cx			; enough mem for ovl tbl?
		jnc	memloop
		jmp	buyram
memloop:
		push	bx
		mov	ah,DOSFREE		; free first block for malloc()
		int	DOS
		jnc	cockadoodledoo
		jmp	buyram
cockadoodledoo:

		assume	es:ovltbl

		xor	bp,bp
		xor	di,di
		xor	si,si
filsegtbllpp:					; initialise ovl table
		call	gethdr			; get an EXE header
		mov	ax,ovltblbse
		add	ax,hdr.exeovlnum
		mov	es,ax			; ^ to ovl table entry
		xor	ax,ax
		mov	word ptr ovllrudat,ax	; initialise ovl lru
		mov	word ptr ovllrudat+2,ax
		mov	ovlseg,ax		; initialise ovl segment
		mov	ovlflg,al		; initialise ovl flags
		mov	ax,hdr.exesiz
		shl	ax,1
		shl	ax,1
		shl	ax,1
		shl	ax,1
		shl	ax,1			; * 32
		mov	dx,hdr.exelstpgesiz
		or	dx,dx
		jz	emptypage
		shr	dx,1
		shr	dx,1
		shr	dx,1
		shr	dx,1			; / 16
		inc	dx
		sub	ax,20h
		add	ax,dx
emptypage:
		mov	ovlsiz,ax		; overlay size in paragraphs
		sub	ax,hdr.hdrparas 	; actual size of code and relocation table
		cmp	hdr.exeovlnum,0 	; skip if ovl 0 (root code)
		jz	notlargest
		cmp	ax,di			; find largest ovl
		jc	notlargest
		mov	di,ax
		mov	si,ovlsiz
notlargest:
		mov	ovlfiloff,bp		; initialise ovl file offset
		add	bp,hdr.exesiz		; ^ to next overlay
		mov	dx,bp
		mov	cl,dh
		mov	dh,dl
		xor	ch,ch
		xor	dl,dl
		shl	dx,1
		rcl	cx,1			; cx:dx = bp * 512
		mov	al,0
		mov	ah,DOSSEEK		; seek to next ovl
		int	DOS
		mov	ax,ovlcnt
		dec	ax
		cmp	ax,hdr.exeovlnum	; all overlays done?
		jz	makmemblk
		jmp	filsegtbllpp		; Nope, go for more.
makmemblk:
		push	si			; contains largest ovl size in paragraphs

		assume	es:nothing		; prepare first two memory blocks
						; OVERLAY_AREA and allocated memory block
		mov	ax,ovlrootcode		; OVERLAY_AREA segment
		mov	es,ax
		mov	si,ovltblbse
		add	si,ovlcnt		; end of ovl table
		mov	es:memblkflg,0		; clear mem flags
		mov	es:memblknxt,si 	; point to next
		mov	es:memblkprv,0		; set previous to nothing
		mov	es:memblksiz,di 	; di contains OVERLAY_AREA size in paragraphs
		add	di,ax
		mov	ovldata,di		; end of OVERLAY_END
		mov	es,si			; end of ovl tbl (first memory block in allocated memory)
		mov	es:memblkflg,0		; clear mem flags
		mov	es:memblknxt,0		; set next to nothing
		mov	es:memblkprv,ax 	; point to previous
		pop	si
		pop	bx
		mov	es:memblksiz,bx 	; allocated memory block size less ovl table
		mov	memblk1st,ax		; save pointer to first mem block
		mov	word ptr ovltim,0	; initialise global lru time stamp
		mov	word ptr ovltim+2,0
		mov	ax,offset framestk
		mov	curovl,ax		; initialise stack frame pointer
		mov	di,ax
		mov	word ptr cs:[di],-1	; initialise stack frame
		add	di,6
		mov	ax,ovltblbse
		mov	cs:[di],ax
		mov	curovl,di
		mov	es,ax
		mov	es:ovlflg,MASK running OR MASK locked OR MASK loaded ; set flags on ovl 0
		inc	si			; largest ovl size + 1 paragraph
		cmp	bx,si			; enough memory to alloc largest?
		jnc	chgintvec
buyram:
		mov	al,5
		mov	dx,OFFSET noroom	; free up some TSRs or something
		jmp	putserr
chgintvec:
		mov	ax,SEG $$INTNO
		mov	ds,ax
		mov	ah,DOSGETVEC
		mov	al,$$INTNO		; get int number to use
		int	DOS			; get original vector
		mov	word ptr oldvec,bx	; save original vector
		mov	word ptr oldvec+2,es

		mov	ah,DOSGETVEC
		mov	al,21h
		int	DOS			; get original vector
		mov	word ptr oldint21,bx	; save original vector
		mov	word ptr oldint21+2,es

		mov	ax,SEG $$INTNO
		mov	ds,ax
		mov	ah,DOSSETVEC
		mov	al,$$INTNO
		mov	bx,cs
		mov	ds,bx
		mov	dx,OFFSET ovlmgr	; point to ovlmgr
		int	DOS			; set vector

		mov	ah,DOSSETVEC
		mov	al,21h
		mov	bx,cs
		mov	ds,bx
		mov	dx,OFFSET int21 	; point to int21
		int	DOS			; set vector

		mov	cx,modcnt		; module count
		mov	ax,SEG $$MPGSNBASE
		mov	es,ax
		mov	ax,cs
		mov	ds,ax

		assume	ds:code

		mov	bx,offset $$MPGSNBASE	; ^ to linker provided overlay segment fixups
		mov	si,offset $$MPGSNOVL	; ^ to linker provided module table
		mov	di,offset moduletbl	; ^ to our module table
modloop:
		mov	al,es:[si]		; real physical ovl number
		xor	ah,ah
		add	ax,ovltblbse		; ovlctlseg address
		mov	[di],ax 		; save in module table
		mov	ax,es:[bx]		; get seg fixup
		sub	ax,ovlrootcode		; adjust for relative reference
		mov	[di+2],ax		; save in module table
		add	di,4
		add	bx,2
		inc	si
		loop	modloop

		pop	es
		pop	ds
		pop	bp
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax			; restore the world
		jmp	$$MAIN			; And away we go!

$$OVLINIT	endp

;-------------------------------------------------------------------------------

ovlmgr		proc	far			; This the it!

		assume cs:code,ds:nothing,es:nothing

		mov	bxreg,bx		; preserve bx
		mov	esreg,es		; and es
		pop	bx			; retrieve caller ip
		pop	es			;     "      "    cs
		push	ax
		push	si
		mov	ax,es:[bx+1]		; offset in ovl to call
		mov	word ptr farcall,ax	; into trampoline
		xor	ah,ah
		mov	al,es:[bx]		; module # to call
		add	bx,3			; fix return address
		mov	si,curovl		; get stack frame pointer
		mov	cs:[si+2],es		; save return seg
		mov	cs:[si+4],bx		; and return offset

		mov	bx,ax
		shl	bx,1
		shl	bx,1			; * 4 (2 words/entry in module tbl)
		add	bx,offset moduletbl
		mov	es,cs:[bx]		; ovl tbl entry
		mov	ax,cs:[bx+2]		; segment fixup
		mov	cs:[si+6],es		; ovl entry into stack frame
		add	curovl,6		; update stack

		assume	es:ovltbl

		mov	si,WORD PTR ovltim	; lru time stamp
		inc	si			; time passes!
		mov	WORD PTR ovltim,si	; update global clock
		mov	WORD PTR ovllrudat,si	; as well as ovl clock
		jz	ininc			; dword increment
cryupcdon:	test	ovlflg,mask loaded	; ovl loaded?
		jz	inload			; load it then.
ovlloadedupc:
		add	ax,ovlseg		; add fixup and segment address
		mov	word ptr farcall+2,ax	; into trampoline
		mov	bx,bxreg		; retore all registers
		mov	es,esreg
		pop	si
		pop	ax
		popf				; don't forget these!
		call	DWORD PTR farcall	; and GO
		pushf				; preserve registers again!
		mov	esreg,es
		mov	bxreg,bx
		mov	bx,curovl		; stack frame pointer
		mov	es,cs:[bx-6]		; retrieve ovl tbl entry
		push	cs:[bx-4]		; set return address
		push	cs:[bx-2]
		push	cx
		mov	cx,WORD PTR ovltim	; do the lru thing again
		inc	cx
		mov	WORD PTR ovltim,cx
		mov	WORD PTR ovllrudat,cx
		jz	outinc
crydncdon:	test	ovlflg,mask loaded	; ovl loaded?
		jz	outload 		; better get it before someone notices
jmpback:
		sub	curovl,6		; adjust stack
		mov	bx,bxreg		; get registers back
		mov	es,esreg
		pop	cx
		iret				; and GO back

ininc:
		mov	si,WORD PTR ovltim+2	; high word of lru
		inc	si
		mov	WORD PTR ovltim+2,si	; update global and
		mov	WORD PTR ovllrudat+2,si ; ovl clocks
		jmp	cryupcdon

inload:
		call	loadoverlay		; self explanatory
		jmp	ovlloadedupc

outinc:
		mov	cx,WORD PTR ovltim+2
		inc	cx
		mov	WORD PTR ovltim+2,cx
		mov	WORD PTR ovllrudat+2,cx
		jmp	crydncdon

outload:
		call	loadoverlay
		jmp	jmpback

ovlmgr		endp

;-------------------------------------------------------------------------------

loadoverlay	proc	near			; load overlay pointed to by es

		assume	cs:code,ds:nothing,es:ovltbl

		push	ax
		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	bp
		push	ds
		push	es			; just in case
		cmp	memstat,0
		jz	dontrealloc
		call	reallocmem
dontrealloc:
		call	setrunning		; set the running flags
		test	ovlflg,MASK running	; was it already running?
		jnz	fxdadr			; Yup, it's a toughie
		mov	ax,ovlsiz		; How much?
		call	getpages		; never fail mem alloc, you bet.
		jmp	gleaner
fxdadr:
		call	releasepages		; free memory where this ovl should be loaded
gleaner:
		mov	ovlmemblk,ax		; memory block to use
		add	ax,memctlblksiz / PARSIZ; skip mem ctl blk
		mov	ds,ax
		mov	dx,ovlfiloff		; where in the file is it?
		mov	cl,dh
		mov	dh,dl
		xor	ch,ch
		xor	dl,dl
		shl	dx,1
		rcl	cx,1			; cx:dx = dx * 512
		mov	ah,DOSSEEK		; lseek to position
		mov	al,0
		mov	bx,ovlexefilhdl 	; never closing handle
		int	DOS
		jc	burnhead		; oops!
		xor	dx,dx
		mov	cx,ovlsiz		; number of paragraphs to load
		shl	cx,1
		shl	cx,1
		shl	cx,1
		shl	cx,1			; * 16 = number of bytes
		mov	ah,DOSREAD		; prevent random DOS behaviour
		int	DOS
		jc	burnhead		; double oops!
		call	ovlrlc			; perform relocation normally done by DOS EXE loader
		pop	es			; retrieve ovl tbl entry
		or	ovlflg,MASK loaded	; because it is now
		pop	ds
		pop	bp
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret

burnhead:
		mov	al,5
		mov	dx,offset nofile
		jmp	putserr

loadoverlay	endp

;-------------------------------------------------------------------------------

ovlrlc		proc	near			; ds:0 -> the overlay to relocate

		assume	cs:code,ds:nothing,es:ovltbl

		mov	cx,ds:relocitems	; roto-count
		mov	ax,ds
		add	ax,ds:hdrparas		; skip header
		mov	ovlseg,ax		; actual code starts here
		mov	di,ax
		sub	di,ovlrootcode		; segment fixup value
		mov	si,ds:reloctbloff	; ^ relocation tbl in header
		jcxz	relocdone		; not such a good idea, after all
dorelocs:					; labels don't GET comments
		lodsw				; offset into load module
		mov	bx,ax
		lodsw				; segment in load module (zero reference)
		add	ax,pspadd		; now it is psp relative
		add	ax,di			; and now it is relative to the actual load address
		mov	es,ax
		mov	ax,es:[bx]		; pickup item to relocate
		add	ax,pspadd		; make it psp relative
		cmp	ax,ovlrootcode		; is it below the OVERLAY_AREA?
		jc	reloccomputed		; yup. it's relocated
		cmp	ax,ovldata		; is it above OVERLAY_AREA
		jnc	reloccomputed		; yup. it's relocated
		add	ax,di			; it's in OVERLAY_AREA, this one's ours.
reloccomputed:
		mov	es:[bx],ax		; RAM it home!?!
		loop	dorelocs		; what goes around, comes around.
relocdone:	ret

ovlrlc		endp

;-------------------------------------------------------------------------------

getvictim	proc	near			; select a victim to discard (and free up some memory)

		assume	cs:code,ds:ovltbl,es:nothing

		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	bp
		push	ds
		mov	ds,ovltblbse		; ^ ovl tbl
		xor	ax,ax			; will contain the low word of lru
		mov	dx,ax			; will contain the high word of lru
		mov	bp,ax			; will contain ovl tbl entry
		mov	bx,ax			; ovl tbl ptr
;		 mov	 cx,ovlcnt		 ; number of ovl's to scan
;foon:		 test	 ovlflg[bx],MASK loaded  ; is this one loaded?
;		 jz	 skip			 ; nope, try next one
;		 test	 ovlflg[bx],MASK locked  ; is this one loacked?
;		 jnz	 skip			 ; yup, try next one
;		 test	 ovlflg[bx],MASK running ; is this one running?
;		 jnz	 skip			 ; yup, try next one
;		 mov	 si,WORD PTR ovltim	 ; get global lru
;		 mov	 di,WORD PTR ovltim+2
;		 sub	 si,WORD PTR ovllrudat[bx] ; subtract from ovl lru
;		 sbb	 di,WORD PTR ovllrudat[bx+2]
;		 cmp	 dx,di			 ; is this one older?
;		 jc	 better 		 ; it sure is
;		 jnz	 skip			 ; it definitely isn't
;		 cmp	 ax,si
;		 jnc	 skip			 ; it really isn't
;better:	 mov	 ax,si			 ; save the lru stuff and ovl ptr
;		 mov	 dx,di
;		 mov	 bp,bx
;skip:		 add	 bx,ovlsegsiz		 ; do next ovl
;		 loop	 foon
;		 or	 bp,bp			 ; did we find anyone to kill?
;		 jnz	 gotvictim		 ; yes we did, partner.
;		 xor	 bx,bx			 ; Oh well, do it again disregarding the running flag
		mov	cx,ovlcnt
foon1:		test	ovlflg[bx],MASK loaded
		jz	skip1
		test	ovlflg[bx],MASK locked
		jnz	skip1
		mov	si,WORD PTR ovltim
		mov	di,WORD PTR ovltim+2
		sub	si,WORD PTR ovllrudat[bx]
		sbb	di,WORD PTR ovllrudat[bx+2]
		cmp	dx,di
		jc	better1
		jnz	skip1
		cmp	ax,si
		jnc	skip1
better1:	mov	ax,si
		mov	dx,di
		mov	bp,bx
skip1:		add	bx,ovlsegsiz
		loop	foon1
		or	bp,bp			; were we more successful this time?
		jnz	gotvictim		; now we got one.
nomoremem:
		mov	al,5			; were really %$# now!
		mov	dx,offset nocore
		jmp	putserr
gotvictim:
		shr	bp,1			; convert offset to segment
		shr	bp,1
		shr	bp,1
		shr	bp,1
		mov	ax,ds
		add	ax,bp
		pop	ds
		pop	bp
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		ret
getvictim	endp

;-------------------------------------------------------------------------------

setrunning	proc	near			; set running flag on overlays still running

		assume cs:code,ds:nothing,es:ovltbl

		push	es
		mov	es,ovltblbse
		mov	cx,ovlcnt
		xor	bx,bx
jim:		and	ovlflg[bx],NOT MASK running ; start by clearing them all
		add	bx,ovlsegsiz
		loop	jim

		; Now chain down the stack links, setting running flags

		mov	bx,curovl
		sub	bx,6
		jmp	jam
jamloop:
		mov	ds,cs:[bx]
		assume	ds:ovltbl
		or	ovlflg,MASK running
		sub	bx,6
jam:
		cmp	word ptr cs:[bx],-1	; end of stack ?
		jnz	jamloop
		pop	es
		ret

setrunning	    endp

;-------------------------------------------------------------------------------

int21		proc	near

; free almost all overlay memory if app. tries to call the DOS exec function.

		cmp	ah,DOSEXEC
		jz	freeall
		cmp	ah,TERMINATE
		jz	saybyebye
		jmp	cs:oldint21
saybyebye:
		pop	ax			; clean up stack
		pop	ax
		pop	ax
		mov	al,0			; return code 0
		mov	dx,offset exitmsg
		jmp	putserr
freeall:
		push	ax
		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	bp
		push	es
		push	ds			; preserve calling env.

		assume cs:code,ds:nothing,es:ovltbl

		mov	ax,cs:memblk1st 	; start de-allocating from first blk
		jmp	short lastblk
unloadlp:
		mov	ds,ax
		cmp	ax,cs:ovltblbse 	; in alloced area ?
		jc	nextmemblk
		test	ds:memblkflg,MASK_used	; mem blk used ?
		jz	nextmemblk
		mov	es,ds:memblkovl
		and	ovlflg,NOT MASK loaded	; flag overlay as unloaded
nextmemblk:
		mov	ax,ds:memblknxt
lastblk:
		or	ax,ax			; keep going till no more
		jnz	unloadlp

		mov	ax,cs:ovltblbse
		add	ax,cs:ovlcnt
		mov	es,ax			; ^ to first mem blk in alloced mem
		mov	es:memblksiz,2		; adjust size
		mov	es:memblknxt,0		; no other blocks after this one
		mov	es:memblkflg,0		; not used
		mov	cs:memstat,0ffh 	; memory needs to be re-alloced some day

		mov	dx,word ptr cs:oldint21
		mov	ds,word ptr cs:oldint21+2
		mov	ah,DOSSETVEC		; put back DOS vector to avoid calling ourselves again!
		mov	al,21h
		int	DOS

		mov	es,cs:ovltblbse
		mov	bx,cs:ovlcnt
		add	bx,2			; re-adjust alloced size
		mov	ah,DOSREALLOC
		int	DOS
		pop	ds
		pop	es
		pop	bp
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		jmp	cs:oldint21		; allow DOS to continue!

int21		endp

;-------------------------------------------------------------------------------

reallocmem	proc	near

; re-allocate our memory after a DOS exec function

		push	es
		mov	ah,DOSREALLOC
		mov	es,cs:ovltblbse 	; mem blk handle
		mov	bx,0ffffh		; find out how much there is
		int	DOS
		mov	ah,DOSREALLOC		; re-allocate our own memory
		mov	es,cs:ovltblbse
		push	bx			; contains largest available blk
		int	DOS
		mov	cs:memstat,0		; flag it re-alloced
		mov	ax,cs:ovltblbse
		add	ax,cs:ovlcnt
		mov	es,ax			; ^ to first mem blk in alloced mem
		pop	ax
		sub	ax,cs:ovlcnt		; remove ovl rbl size
		mov	es:memblksiz,ax 	; fix mem blk size

		mov	ah,DOSGETVEC
		mov	al,21h
		int	DOS			; get original vector
		mov	word ptr cs:oldint21,bx ; save original vector
		mov	word ptr cs:oldint21+2,es

		mov	ah,DOSSETVEC
		mov	al,21h
		mov	bx,cs
		mov	ds,bx
		mov	dx,OFFSET int21 	; point to int21
		int	DOS			; set vector

		pop	es
		ret

reallocmem	endp

;-------------------------------------------------------------------------------

releasepages	proc	near			; Arg in es, result in ax

; release any memory (and overlays) where this overlay should reside

		assume	es:ovltbl

		mov	bx,es:ovlmemblk 	; start of memory to release
doitagain:
		mov	ax,memblk1st		; first memory blk
		jmp	dvart
dvartloop:
		mov	ds,ax			; memory blk to check
		cmp	bx,ax			; does it start below the memory to release?
		jnc	dvartsmaller		; yup
		mov	dx,bx
		add	dx,es:ovlsiz
		add	dx,memctlblksiz / PARSIZ; end of memory to release
		cmp	ax,dx			; does it start above?
		jnc	dvartsilly		; yup
		call	killmem 		; it's in the way. Zap it.
		jmp	chkmemblk
dvartsmaller:
		add	ax,ds:memblksiz 	; end of this memory blk
		cmp	bx,ax			; does it end below the memory to release?
		jnc	dvartsilly		; yup
		call	killmem 		; Oh well, zap it too.
chkmemblk:					; was that enough?
		mov	ax,ds			; recently freed memory blk
		cmp	bx,ax			; does it start in the memory to be released?
		jc	dvartsilly		; yup, wasn't enough
		mov	dx,bx
		add	dx,es:ovlsiz
		add	dx,memctlblksiz / PARSIZ; end of memory to be released
		add	ax,ds:memblksiz 	; end of freed memory
		cmp	ax,dx			; does it end in the memory to be released?
		jc	dvartsilly		; yup, release more
dvartgotblk:
		mov	ax,ds			; this is it!
		mov	cx,bx
		sub	cx,ax			; # of paragraphs between start of memory to release and mem blk
		jz	nosplit
		call	splitblkhigh		; split the block
nosplit:
		mov	cx,es:ovlsiz
		add	cx,memctlblksiz / PARSIZ; paragraphs needed to load ovl
		jmp	splitblklow		; split remaining block
dvartsilly:
		mov	ax,ds:memblknxt
dvart:
		or	ax,ax			; enf of mem list?
		jz	dvartnocore
		jmp	dvartloop		; play it again Sam.
dvartnocore:
		mov	al,5			; super OOPS!
		mov	dx,offset nocore
		jmp	putserr

releasepages	endp

;-------------------------------------------------------------------------------

getpages	proc	near			; get enough memory to load ovl

		mov	cx,ax
		add	cx,memctlblksiz / PARSIZ; total paragraphs needed
		call	largestmem		; find largest free blk
		cmp	dx,cx			; large enough?
		jnc	gotdork 		; yup.
dorkkill:
		call	getvictim		; select a victim to release
		call	killovl 		; kill the selected victim
		cmp	ds:memblksiz,cx 	; was it enough?
		jc	dorkkill		; nope, select another one
gotdork:
		jmp	splitblklow		; split the free blk

getpages	endp

;-------------------------------------------------------------------------------

splitblklow	proc	near

; split a block of memory returning the lower one to be used.

		push	es
		or	ds:memblkflg,MASK_used	; set low block used
		mov	ax,ds
		add	ax,cx
		mov	es,ax			; ^ to upper blk to be created
		mov	ax,ds:memblksiz
		sub	ax,cx
		cmp	ax,1			; must be at least 1 para remaining to split
		jc	noodorksplit		; don't split
		mov	ds:memblksiz,cx 	; fix blk sizes
		mov	es:memblksiz,ax
		mov	ax,ds:memblknxt 	; fix pointers
		mov	es:memblknxt,ax
		mov	ds:memblknxt,es
		mov	es:memblkprv,ds
		mov	es:memblkflg,0		; set upper to not used
		push	ds
		mov	ax,es:memblknxt
		or	ax,ax
		jz	domergelow
		mov	ds,ax			; fix blk after upper to point to upper
		mov	ds:memblkprv,es
domergelow:
		mov	ax,es
		mov	ds,ax
		call	mergemem		; merge remaining free memory
		pop	ds
noodorksplit:
		pop	es
		mov	ds:memblkovl,es 	; fix ptr to ovl
		mov	ax,ds			; return lower blk segment
		ret

splitblklow	endp

;-------------------------------------------------------------------------------

splitblkhigh	proc	near

; split a block of memory returning the upper one to be used.

		push	es
		mov	ax,ds
		add	ax,cx
		mov	es,ax			; ^ to upper blk to be created
		mov	ax,ds:memblksiz
		sub	ax,cx			; # of para remaining in upper blk
		mov	ds:memblksiz,cx 	; fix blk sizes
		mov	es:memblksiz,ax
		mov	ax,ds:memblknxt 	; fix blk pointers
		mov	es:memblknxt,ax
		mov	ds:memblknxt,es
		mov	es:memblkprv,ds
		mov	ds:memblkflg,0		; set lower to not used
		or	es:memblkflg,MASK_used	; set upper to used
		mov	ax,es:memblknxt
		or	ax,ax
		jz	domergehigh
		push	ds			; fix blk after upper to point to upper
		mov	ds,ax
		mov	ds:memblkprv,es
		pop	ds
domergehigh:
		call	mergemem		; merge remaining free memory
nodorksplit:
		mov	ax,es
		mov	ds,ax
		pop	es
		mov	ds:memblkovl,es 	; fix ovl ptr
		mov	ax,ds			; return upper blk segment
		ret

splitblkhigh	endp

;-------------------------------------------------------------------------------

largestmem	proc	near	; returns seg in ax, size in dx; clobbers bx,ds,es
				; retruns first block that's large enough if possible

		mov	ax,memblk1st		; first mem blk
		xor	dx,dx			; largest size found
		jmp	gook
gookloop:	mov	ds,ax
		test	ds:memblkflg,MASK_used	; is this blk used?
		jnz	gookme			; yup
		cmp	ds:memblksiz,cx 	; is it large enough?
		jc	gookme			; nope
		mov	dx,ds:memblksiz 	; got one!
		ret
gookme:
		mov	ax,ds:memblknxt
gook:		or	ax,ax			; end of list?
		jnz	gookloop		; around and around
		ret

largestmem	endp

;-------------------------------------------------------------------------------

killmem 	proc	near

		test	ds:memblkflg,MASK_used	; is it used?
		jz	memnotused		; don't kill ovl
		push	es
		mov	es,ds:memblkovl
		and	es:ovlflg,NOT MASK loaded ; zap ovl associated with this blk
		pop	es
memnotused:
		jmp	mergemem		; merge free memory

killmem 	endp

;-------------------------------------------------------------------------------

killovl 	proc	near		; preserves bx

		mov	ds,ax
		assume	ds:ovltbl
		and	ovlflg,NOT MASK loaded	; ovl no longer loaded
		mov	ax,ovlmemblk		; get mem blk
		mov	ds,ax
		jmp	mergemem		; merge free memory

killovl 	endp

;-------------------------------------------------------------------------------

mergemem	proc	near

; merge physically adjacent free memory blocks. Preserves es. ds -> a free block.

		push	es
		and	ds:memblkflg,NOT MASK_used ; set current free
		mov	ax,ds:memblkprv 	; get previous blk
		or	ax,ax			; was there a previous blk?
		jz	gibber			; nope
		mov	es,ax
		test	es:memblkflg,MASK_used	; is the previous blk used?
		jnz	gibber			; yup
		add	ax,es:memblksiz 	; end of previous blk
		mov	dx,ds
		cmp	dx,ax			; physically adjacent?
		jnz	gibber			; nope
		mov	ax,ds:memblksiz
		add	es:memblksiz,ax 	; adjust size of new larger blk
		mov	ax,ds:memblknxt 	; fix pointers
		mov	es:memblknxt,ax
		or	ax,ax
		jz	almostgibber
		mov	ds,ax			; fix pointer of next blk
		mov	ds:memblkprv,es
almostgibber:
		mov	ax,es
		mov	ds,ax			; new blk segment
gibber:
		mov	ax,ds:memblknxt 	; get next blk
		or	ax,ax			; was there a next blk?
		jz	killdone		; nope
		mov	es,ax
		test	es:memblkflg,MASK_used	; is the nxt blk used?
		jnz	killdone		; yup
		mov	ax,ds
		add	ax,ds:memblksiz 	; end of this blk
		mov	dx,es
		cmp	ax,dx			; physically adjacent?
		jnz	killdone		; nope
		mov	ax,es:memblksiz
		add	ds:memblksiz,ax 	; adjust size of new larger blk
		mov	ax,es:memblknxt 	; fix pointers
		mov	ds:memblknxt,ax
		or	ax,ax
		jz	killdone
		mov	es,ax			; fix pointer of blk after nxt
		mov	es:memblkprv,ds
killdone:
		and	ds:memblkflg,NOT MASK_used ; make sure it's free
		pop	es
		ret

mergemem	endp

;-------------------------------------------------------------------------------

gethdr		proc	near			; read EXE header from handle

		push	cx
		mov	ax,cs
		mov	ds,ax
		mov	dx,offset hdr		; a place to put it
		mov	bx,ovlexefilhdl 	; the file handle
		mov	cx,hdrsize		; header size in bytes
		mov	ah,DOSREAD
		int	DOS			; read from file
		jc	exegone 		; oops
		cmp	ax,cx			; got correct number of bytes?
		jnz	exegone 		; nope
		pop	cx
		ret				; Wow, it worked!
exegone:
		mov	al,5			; You lose!
		mov	dx,offset nofile
		jmp	putserr

gethdr		endp

;-------------------------------------------------------------------------------

putserr 	proc	near

; display error msg, close file, restore int vectors, free mem and return to DOS.

		push	ax			; keep return code for later
		mov	ax,cs
		mov	ds,ax
		mov	ah,PRINT
		int	DOS			; display error msg
		mov	dx,word ptr oldvec	; get old vector
		cmp	dx,-1			; was it ever replaced?
		jz	free21			; nope
		push	ds
		mov	ds,word ptr oldvec+2
		mov	ah,DOSSETVEC		; put it back then.
		mov	al,$$INTNO
		int	DOS
		pop	ds
free21:
		mov	dx,word ptr oldint21
		cmp	dx,-1
		jz	freemem
		push	ds
		mov	ds,word ptr oldint21+2
		mov	ah,DOSSETVEC		; put it back then.
		mov	al,21h
		int	DOS
		pop	ds
freemem:
		mov	ax,ovltblbse		; get memory blk segment
		cmp	ax,-1			; was one ever allocated?
		jz	closefile		; nope
		mov	es,ax
		mov	ah,DOSFREE		; must free it.
		int	DOS
closefile:
		mov	bx,ovlexefilhdl 	; get file handle
		cmp	bx,-1			; was the file ever opened?
		jz	byebye			; nope
		mov	ah,DOSCLOSE		; close it
		int	DOS
byebye:
		pop	ax			; return code in al
		mov	ah,TERMINATE
		int	DOS			; terminate this process

putserr 	endp

code		ends

		end
