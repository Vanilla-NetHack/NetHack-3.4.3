;	SCCS Id: @(#)ovlmgr.asm 	3.0.624	90/02/18
;  Copyright (c) Pierre Martineau and Stephen Spackman, 1989, 1990.
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
;		      - re-entrancy is not rEQUired

DOSALLOC	EQU	48h			; memory allocation
DOSFREE 	EQU	49h			; free allocated memory
DOSREALLOC	EQU	4ah			; modify memory block
DOSREAD 	EQU	3fh			; read bytes from handle
DOSSEEK 	EQU	42h			; logical handle seek
DOSOPEN 	EQU	3dh			; open handle
DOSCLOSE	EQU	3eh			; close handle
DOSGETVEC	EQU	35h			; get interrupt vector
DOSSETVEC	EQU	25h			; set interrupt vector
DOSEXEC 	EQU	4bh			; exec child process
DOS		EQU	21h			; Dos interrupt #
PRINT		EQU	09h			; print string
TERMINATE	EQU	4ch			; terminate process
CR		EQU	0dh
LF		EQU	0ah
BELL		EQU	07h
FAERIE		EQU	0h			; Used for dummy segment allocation
PARSIZ		EQU	10h			; this is the size of a paragraph - this better not change!

; The following EXTRNs are supplied by the linker

EXTRN		$$OVLBASE:BYTE			; segment of OVERLAY_AREA
EXTRN		$$MPGSNOVL:BYTE 		; ^ to module table
EXTRN		$$MPGSNBASE:WORD		; ^ to module segment fixups
EXTRN		$$INTNO:BYTE			; interrupt number to be used
EXTRN		$$COVL:WORD			; number of physical overlays
EXTRN		$$CGSN:WORD			; number of modules
EXTRN		$$MAIN:FAR			; ^ to function main()

PUBLIC		$$OVLINIT			; Our entry point
						; called by the c startup code

ovlflgrec	RECORD	running:1=0,locked:1=0,loaded:1=0 ; overlay flags

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

pspseg		SEGMENT PARA AT FAERIE		; dummy segment for psp
		ORG	2ch			; ^ to segment of environmemt in psp
pspenv		LABEL	WORD
pspseg		ENDS

ovltbl		SEGMENT PARA AT FAERIE		; Dummy segment definition for overlay table

; NOTE: This segment definition MUST be exactly 16 bytes long

ovlflg		ovlflgrec	<0,0,0> 	; overlay flags
ovltblpad1	DB	?			; go ahead, delete me!
ovlmemblk	DW	?			; ^ to allocated memory block
ovllrudat	DD	?			; misc lru data (pseudo time stamp)
ovlseg		DW	?			; ovl segment physical add.
ovlfiloff	DW	?			; ovl file offset in pages (512 bytes)
ovlsiz		DW	?			; ovl size in paragraphs
ovltblpad	DB	PARSIZ - ($ - ovlflg) MOD PARSIZ DUP (?) ; pad to 16 bytes

IF1
IF		($ - ovlflg) GT PARSIZ
		.ERR
		%OUT This segment MUST be no more than 16 bytes, REALLY!!!
ENDIF
ENDIF

OVLSEGSIZ	EQU	PARSIZ			; this had better be true!!! (16 bytes)

ovltbl		ENDS

EXEHDR		STRUC				; structure of an EXE header
exesign 	DW	5a4dh			; signature
exelstpgesiz	DW	?			; last page size (512 byte pages)
exesiz		DW	?			; total pages (including partial last page)
relocitems	DW	?			; number of relocation entries
hdrparas	DW	?			; number of paragraphs in the header
minalloc	DW	?			; minimum paragraph allocation
maxalloc	DW	?			; maximum patagraph allocation
exess		DW	?			; initial stack segment
exesp		DW	?			; initial stack pointer
exechksum	DW	?			; checksum
exeip		DW	?			; initial instruction pointer
execs		DW	?			; initial code segment
reloctbloff	DW	?			; offset from beginning of header to relocation table
exeovlnum	DW	?			; overlay number
EXEHDR		ENDS

MASK_used	EQU	1			; memory block flag

MEMCTLBLK	STRUC				; memory block structure
memblkflg	DB	?			; flags
memblkpad1	DB	?			; go ahead, delete me!
memblknxt	DW	?			; ^ to next block
memblkprv	DW	?			; ^ to previous block
memblkovl	DW	?			; ^ to overlay occupying this block
memblksiz	DW	?			; size in paragraphs
memblkpad	DB	PARSIZ - memblkpad MOD PARSIZ DUP (?) ; pad to 16 bytes
MEMCTLBLK	ENDS

MEMCTLBLKSIZ	EQU	TYPE MEMCTLBLK / PARSIZ ; should equal 1 paragraph

;-------------------------------------------------------------------------------

code		SEGMENT PUBLIC

; NOTE: the following order is optimum for alignement purposes accross the
;	entire INTEL 80x86 family of processors.

ovltim		DD	?			; pseudo-lru time variable
farcall 	DD	?			; internal trampoline.
oldvec		DD	-1			; saved interrupt vector
oldint21	DD	-1			; saved int 21 vector
ovlexefilhdl	DW	-1			; always-open file handle of our .EXE
ovltblbse	DW	-1			; segment of first overlay descriptor
ovlcnt		DW	?			; # overlays
modcnt		DW	?			; # of modules
ovlrootcode	DW	?			; logical segment of OVERLAY_AREA
ovldata 	DW	?			; logical segment of OVERLAY_END
memblk1st	DW	?			; first memory block
pspadd		DW	?			; our psp address + 10h (for relocations)
bxreg		DW	?			; temp save area
esreg		DW	?			; temp save area
moduletbl	DD	256 DUP (?)		; module lookup table (256 modules)
curovl		DW	OFFSET stkframe 	; ^ into stack frame
stkframe	DW	64*3 DUP (?)		; internal stack (64 ovls deep)
hdr		EXEHDR	<>			; EXE header work area
intnum		DB	?			; overlay interrupt number

noroom		DB	CR,LF,'Not enough memory to run this program. Time to go to the store.',CR,LF,BELL,'$'
nocore		DB	CR,LF,'Your dog eats all your remaining memory! You die.',CR,LF,BELL,'$'
nofile		DB	CR,LF,'The Nymph stole your .EXE file! You die.',CR,LF,BELL,'$'
exitmsg 	DB	CR,LF,'$'

;-------------------------------------------------------------------------------

$$OVLINIT	PROC	FAR			; Init entry point

		ASSUME	CS:code,DS:pspseg,ES:NOTHING

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
		cmp	WORD PTR [si],0
		jnz	envloop
		add	si,4			; point to EXE filename
		mov	al,0			; access code
		mov	ah,DOSOPEN
		mov	dx,si
		int	DOS			; open EXE
		jnc	dontdie
		mov	al,5
		mov	dx,OFFSET nofile
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

		ASSUME	ES:ovltbl

		xor	bp,bp
		xor	di,di
		xor	si,si
filsegtbllpp:					; initialise ovl table
		call	gethdr			; get an EXE header
		mov	ax,ovltblbse
		add	ax,hdr.exeovlnum
		mov	es,ax			; ^ to ovl table entry
		xor	ax,ax
		mov	WORD PTR ovllrudat,ax	; initialise ovl lru
		mov	WORD PTR ovllrudat+2,ax
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

		ASSUME	ES:nothing		; prepare first two memory blocks
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
		mov	WORD PTR ovltim,0	; initialise global lru time stamp
		mov	WORD PTR ovltim+2,0
		mov	ax,OFFSET stkframe
		mov	curovl,ax		; initialise stack frame pointer
		mov	di,ax
		mov	WORD PTR cs:[di],-1	; initialise stack frame
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
		mov	intnum,al
		int	DOS			; get original vector
		mov	WORD PTR oldvec,bx	; save original vector
		mov	WORD PTR oldvec+2,es

		mov	ah,DOSGETVEC
		mov	al,21h
		int	DOS			; get original vector
		mov	WORD PTR oldint21,bx	; save original vector
		mov	WORD PTR oldint21+2,es

		mov	ah,DOSSETVEC
		mov	al,intnum
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

		ASSUME	DS:code

		mov	bx,OFFSET $$MPGSNBASE	; ^ to linker provided overlay segment fixups
		mov	si,OFFSET $$MPGSNOVL	; ^ to linker provided module table
		mov	di,OFFSET moduletbl	; ^ to our module table
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

$$OVLINIT	ENDP

;-------------------------------------------------------------------------------

ovlmgr		PROC	FAR			; This the it!

		ASSUME CS:code,DS:NOTHING,ES:NOTHING

		mov	bxreg,bx		; preserve bx
		mov	esreg,es		; and es
		pop	bx			; retrieve caller ip
		pop	es			;     "      "    cs
		push	ax
		push	si
		mov	ax,es:[bx+1]		; offset in ovl to call
		mov	WORD PTR farcall,ax	; into trampoline
		xor	ah,ah
		mov	al,es:[bx]		; module # to call
		add	bx,3			; fix return address
		mov	si,curovl		; get stack frame pointer
		mov	cs:[si+2],es		; save return seg
		mov	cs:[si+4],bx		; and return offset

		mov	bx,ax
		shl	bx,1
		shl	bx,1			; * 4 (2 words/entry in module tbl)
		add	bx,OFFSET moduletbl
		mov	es,cs:[bx]		; ovl tbl entry
		mov	ax,cs:[bx+2]		; segment fixup
		mov	cs:[si+6],es		; ovl entry into stack frame
		add	curovl,6		; update stack

		ASSUME	ES:ovltbl

		mov	si,WORD PTR ovltim	; lru time stamp
		inc	si			; time passes!
		mov	WORD PTR ovltim,si	; update global clock
		mov	WORD PTR ovllrudat,si	; as well as ovl clock
		mov	si,WORD PTR ovltim+2	; high order word
		jz	ininc			; dword increment
cryupcdon:	mov	WORD PTR ovllrudat+2,si
		test	ovlflg,mask loaded	; ovl loaded?
		jz	inload			; load it then.
ovlloadedupc:
		add	ax,ovlseg		; add fixup and segment address
		mov	WORD PTR farcall+2,ax	; into trampoline
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
		mov	cx,WORD PTR ovltim+2
		jz	outinc
crydncdon:	mov	WORD PTR ovllrudat+2,cx
		test	ovlflg,mask loaded	; ovl loaded?
		jz	outload 		; better get it before someone notices
jmpback:
		sub	curovl,6		; adjust stack
		mov	bx,bxreg		; get registers back
		mov	es,esreg
		pop	cx
		iret				; and GO back

ininc:
		inc	si
		mov	WORD PTR ovltim+2,si	; update global clock
		jmp	cryupcdon

inload:
		call	loadoverlay		; self explanatory
		jmp	ovlloadedupc

outinc:
		inc	cx
		mov	WORD PTR ovltim+2,cx
		jmp	crydncdon

outload:
		call	loadoverlay
		jmp	jmpback

ovlmgr		ENDP

;-------------------------------------------------------------------------------

loadoverlay	PROC	NEAR			; load overlay pointed to by es

		ASSUME	CS:code,DS:NOTHING,ES:ovltbl

		push	ax
		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	bp
		push	ds
		push	es			; just in case
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
		add	ax,MEMCTLBLKSIZ 	; skip mem ctl blk
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
		mov	dx,OFFSET nofile
		jmp	putserr

loadoverlay	ENDP

;-------------------------------------------------------------------------------

ovlrlc		PROC	NEAR			; ds:0 -> the overlay to relocate

		ASSUME	CS:code,DS:NOTHING,ES:ovltbl

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

ovlrlc		ENDP

;-------------------------------------------------------------------------------

getvictim	PROC	NEAR			; select a victim to discard (and free up some memory)

		ASSUME	CS:code,DS:ovltbl,ES:NOTHING

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
skip1:		add	bx,OVLSEGSIZ
		loop	foon1
		or	bp,bp			; were we more successful this time?
		jnz	gotvictim		; now we got one.
nomoremem:
		mov	al,5			; were really %$# now!
		mov	dx,OFFSET nocore
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
getvictim	ENDP

;-------------------------------------------------------------------------------

setrunning	PROC	NEAR			; set running flag on overlays still running

		ASSUME CS:code,DS:NOTHING,ES:ovltbl

		push	es
		mov	es,ovltblbse
		mov	cx,ovlcnt
		xor	bx,bx
jim:		and	ovlflg[bx],NOT MASK running ; start by clearing them all
		add	bx,OVLSEGSIZ
		loop	jim

		; Now chain down the stack links, setting running flags

		mov	bx,curovl
		sub	bx,6
		jmp	jam
jamloop:
		mov	ds,cs:[bx]

		ASSUME	DS:ovltbl

		or	ovlflg,MASK running
		sub	bx,6
jam:
		cmp	WORD PTR cs:[bx],-1	; end of stack ?
		jnz	jamloop
		pop	es
		ret

setrunning	ENDP

;-------------------------------------------------------------------------------

int21		PROC	FAR

; free almost all overlay memory if app. tries to call the DOS exec function.

		cmp	ah,DOSEXEC
		jz	freeall
		cmp	ah,TERMINATE
		jz	saybyebye
notours:
		jmp	cs:oldint21
saybyebye:
		pop	ax			; clean up stack
		pop	ax
		pop	ax
		mov	al,0			; return code 0
		mov	dx,OFFSET exitmsg
		jmp	putserr
freeall:
		or	al,al			; is it load and exec?
		jnz	notours
		push	ax
		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	bp
		push	es
		push	ds			; preserve calling env.

		ASSUME CS:code,DS:NOTHING,ES:ovltbl

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

		mov	dx,WORD PTR cs:oldint21
		mov	ds,WORD PTR cs:oldint21+2
		mov	ah,DOSSETVEC		; put back DOS vector to avoid calling ourselves again!
		mov	al,21h
		int	DOS

		mov	dx,WORD PTR cs:oldvec
		mov	ds,WORD PTR cs:oldvec+2
		mov	ah,DOSSETVEC
		mov	al,intnum
		int	DOS

		mov	es,cs:ovltblbse
		mov	bx,cs:ovlcnt
		add	bx,2			; re-adjust alloced size
		mov	ah,DOSREALLOC
		int	DOS
		mov	bp,sp
		push	[bp+22]			; ensure returned flags are based on user's!
		popf
		pop	ds
		pop	es
		pop	bp
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax

		int	DOS			; allow DOS to continue!

		push	ax
		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	bp
		push	es
		push	ds			; preserve calling env.
		mov	bp,sp
		pushf
		pop	[bp+22]			; fix return flags

; re-allocate our memory after a DOS exec function

		call	reallocmem

		mov	ah,DOSGETVEC
		mov	al,21h
		int	DOS			; get original vector
		mov	WORD PTR cs:oldint21,bx ; save original vector
		mov	WORD PTR cs:oldint21+2,es

		mov	ah,DOSSETVEC
		mov	al,21h
		mov	bx,cs
		mov	ds,bx
		mov	dx,OFFSET int21 	; point to int21
		int	DOS			; set vector

		mov	ah,DOSGETVEC
		mov	al,intnum
		int	DOS			; get original vector
		mov	WORD PTR cs:oldvec,bx	; save original vector
		mov	WORD PTR cs:oldvec+2,es

		mov	ah,DOSSETVEC
		mov	al,intnum
		mov	bx,cs
		mov	ds,bx
		mov	dx,OFFSET ovlmgr	; point to ovlmgr
		int	DOS			; set vector

		pop	ds
		pop	es
		pop	bp
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		iret

int21		ENDP

;-------------------------------------------------------------------------------

reallocmem	PROC	NEAR

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
		mov	ax,cs:ovltblbse
		add	ax,cs:ovlcnt
		mov	es,ax			; ^ to first mem blk in alloced mem
		pop	ax
		sub	ax,cs:ovlcnt		; remove ovl tbl size
		mov	es:memblksiz,ax 	; fix mem blk size

		pop	es
		ret

reallocmem	ENDP

;-------------------------------------------------------------------------------

releasepages	PROC	NEAR			; Arg in es, result in ax

; release any memory (and overlays) where this overlay should reside

		ASSUME	ES:ovltbl

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
		add	dx,MEMCTLBLKSIZ 	; end of memory to release
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
		add	dx,MEMCTLBLKSIZ 	; end of memory to be released
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
		add	cx,MEMCTLBLKSIZ 	; paragraphs needed to load ovl
		jmp	splitblklow		; split remaining block
dvartsilly:
		mov	ax,ds:memblknxt
dvart:
		or	ax,ax			; enf of mem list?
		jz	dvartnocore
		jmp	dvartloop		; play it again Sam.
dvartnocore:
		mov	al,5			; super OOPS!
		mov	dx,OFFSET nocore
		jmp	putserr

releasepages	ENDP

;-------------------------------------------------------------------------------

getpages	PROC	NEAR			; get enough memory to load ovl

		mov	cx,ax
		add	cx,MEMCTLBLKSIZ 	; total paragraphs needed
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

getpages	ENDP

;-------------------------------------------------------------------------------

splitblklow	PROC	NEAR

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

splitblklow	ENDP

;-------------------------------------------------------------------------------

splitblkhigh	PROC	NEAR

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

splitblkhigh	ENDP

;-------------------------------------------------------------------------------

largestmem	PROC	NEAR	; returns seg in ax, size in dx; clobbers bx,ds,es
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

largestmem	ENDP

;-------------------------------------------------------------------------------

killmem 	PROC	NEAR

		test	ds:memblkflg,MASK_used	; is it used?
		jz	memnotused		; don't kill ovl
		push	es
		mov	es,ds:memblkovl
		and	es:ovlflg,NOT MASK loaded ; zap ovl associated with this blk
		pop	es
memnotused:
		jmp	mergemem		; merge free memory

killmem 	ENDP

;-------------------------------------------------------------------------------

killovl 	PROC	NEAR		; preserves bx

		mov	ds,ax

		ASSUME	DS:ovltbl

		and	ovlflg,NOT MASK loaded	; ovl no longer loaded
		mov	ax,ovlmemblk		; get mem blk
		mov	ds,ax
		jmp	mergemem		; merge free memory

killovl 	ENDP

;-------------------------------------------------------------------------------

mergemem	PROC	NEAR

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

mergemem	ENDP

;-------------------------------------------------------------------------------

gethdr		PROC	NEAR			; read EXE header from handle

		push	cx
		mov	ax,cs
		mov	ds,ax
		mov	dx,OFFSET hdr		; a place to put it
		mov	bx,ovlexefilhdl 	; the file handle
		mov	cx,TYPE EXEHDR		; header size in bytes
		mov	ah,DOSREAD
		int	DOS			; read from file
		jc	exegone 		; oops
		cmp	ax,cx			; got correct number of bytes?
		jnz	exegone 		; nope
		pop	cx
		ret				; Wow, it worked!
exegone:
		mov	al,5			; You lose!
		mov	dx,OFFSET nofile
		jmp	putserr

gethdr		ENDP

;-------------------------------------------------------------------------------

putserr 	PROC	NEAR

; display error msg, close file, restore int vectors, free mem and return to DOS.

		push	ax			; keep return code for later
		mov	ax,cs
		mov	ds,ax
		mov	ah,PRINT
		int	DOS			; display error msg
		mov	dx,WORD PTR oldvec	; get old vector
		cmp	dx,-1			; was it ever replaced?
		jz	free21			; nope
		push	ds
		mov	ds,WORD PTR oldvec+2
		mov	ah,DOSSETVEC		; put it back then.
		mov	al,intnum
		int	DOS
		pop	ds
free21:
		mov	dx,WORD PTR oldint21
		cmp	dx,-1
		jz	freemem
		push	ds
		mov	ds,WORD PTR oldint21+2
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

putserr 	ENDP

code		ENDS

		END
