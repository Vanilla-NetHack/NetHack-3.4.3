;	SCCS Id: @(#)ovlmgr.asm 		90/05/27
;  Copyright (c) 1989, 1990 Pierre Martineau and Stephen Spackman. All Rights Reserved.
;  This product may be freely redistributed.  See NetHack license for details.

VERSION 	EQU	3081h

		PAGE	57,132
		TITLE	'DOS Overlay Manager for MSC 5.1+'
		SUBTTL	'Copyright (c) 1989, 1990 Pierre Martineau and Stephen Spackman. All Rights Reserved.'

; acknowledgements:   - Many thanks to Norm Meluch for his invaluable help
;		      - No thanks to Microsoft
;		      - alltrsidsctysti!!!
;		      - izchak and friends for impetus
;		      - us for brilliance
;		      - coffee for speed
;		      - others as necessary

; assumptions:	      - all registers are preserved including flags
;		      - the stack is preserved
;		      - re-entrancy is not required

DOSALLOC	EQU	48h			; memory allocation
DOSFREE 	EQU	49h			; free allocated memory
DOSREALLOC	EQU	4ah			; modify memory block
DOSREAD 	EQU	3fh			; read bytes from handle
DOSSEEK 	EQU	42h			; logical handle seek
DOSOPEN 	EQU	3dh			; open handle
DOSCLOSE	EQU	3eh			; close handle
DOSEXEC 	EQU	4bh			; exec child process
DOSPUTC 	EQU	02h			; print a char
DOSVERSION	EQU	30h			; get version number
DOSGETVEC	EQU	35h			; get interrupt vector
DOS		EQU	21h			; Dos interrupt #
PRINT		EQU	09h			; print string
TERMINATE	EQU	4ch			; terminate process
EMM		EQU	67h			; EMM handler int vector
EMMSTATUS	EQU	40h			; get EMM status
EMMFRAME	EQU	41h			; get EMM page frame
EMMTOTALS	EQU	42h			; get EMM pages available
EMMALLOC	EQU	43h			; allocate EMM pages
EMMMAP		EQU	44h			; map EMM pages
EMMFREE 	EQU	45h			; free EMM pages
CR		EQU	0dh
LF		EQU	0ah
ESCAPE		EQU	1bh
BELL		EQU	07h
PARSIZ		EQU	10h			; this is the size of a paragraph - this better not change!
FAERIE		EQU	00h			; Used for dummy segment allocation

NOERR		EQU	0
DOSERR		EQU	1
FILEERR 	EQU	2
NOMEMERR	EQU	3
FILEIOERR	EQU	4
VICTIMERR	EQU	5
RELERR		EQU	6
EMSERR		EQU	7
HDRERR		EQU	8

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
IFDEF i386
OP32		MACRO				; 32 bit operand override
		DB	066h
		ENDM

pusha		MACRO				; push all registers
		DB	060h
		ENDM

popa		MACRO				; pop all registers
		DB	061h
		ENDM
ENDIF

ovlflgrec	RECORD	locked:1=0,ems:1=0,loaded:1=0 ; overlay flags

; This is a dirty hack. What we need is a virtual segment that will be built
; by the (our) loader in multiple copies, one per overlay. Unfortunately, this
; doesn't seem to be a sensible idea in the minds of the folks at Microsoft.
; Declaring this segment AT will ensure that it never appears in the exefile,
; and ASSUME is dumb enough to be fooled.
;
; The reason we want to do this is also not-to-be-tried-at-home: it turns out
; that we can code a faster interrupt handler if we map overlay numbers to
; segment values. Normally we would consider this unacceptable programming
; practise because it is 86-mode specific, but the *need* for this entire
; programme is 86-mode specific, anyway.

pspseg		SEGMENT PARA AT FAERIE		; dummy segment for psp
		ORG	2ch			; ^ to segment of environmemt in psp
pspenv		LABEL	WORD
pspseg		ENDS

ovltbl		SEGMENT PARA AT FAERIE		; Dummy segment definition for overlay table

; NOTE: This segment definition MUST be exactly 16 bytes long

ovlflg		ovlflgrec	<0,0,0> 	; overlay flags
ovlinvcnt	DB	?			; invocation count
ovlmemblk	DW	?			; ^ to allocated memory block
ovllrudat	DD	?			; misc lru data (pseudo time stamp)
ovlemshdl	DW	?			; ovl ems memory handle
ovlfiloff	DW	?			; ovl file offset in pages (512 bytes)
ovlsiz		DW	?			; ovl size in paragraphs
ovlhdrsiz	DW	?			; hdr size in paragraphs

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

; NOTE: the following order is optimum for alignment purposes across the
;	entire INTEL 80x86 family of processors.

ovltim		DD	?			; pseudo-lru time variable
farcall 	DD	?			; internal trampoline.
oldvec		DD	-1			; saved interrupt vector
oldint21	DD	-1			; saved int 21 vector
sireg		DW	?			; temp save area
IFDEF i386
		DW	?			; for esi
ENDIF
dsreg		DW	?			; temp save area
ssreg		DW	?
spreg		DW	?
ovlexefilhdl	DW	-1			; always-open file handle of our .EXE
ovltblbse	DW	-1			; segment of first overlay descriptor
memblks 	DW	16 DUP (-1)		; allocated memory blocks
memblk1st	DW	?			; first memory block
emsmemblks	DW	16 DUP (-1)		; ems allocated memory blocks (64K each)
curemshandle	DW	-1			; currently mapped handle
ovlcnt		DW	?			; # overlays
modcnt		DW	?			; # of modules
ovlrootcode	DW	?			; logical segment of OVERLAY_AREA
ovldata 	DW	?			; logical segment of OVERLAY_END
pspadd		DW	?			; our psp address + 10h (for relocations)
emsframe	DW	?			; EMM page frame segment
moduletbl	DD	256 DUP (?)		; module lookup table (256 modules)
curovl		DW	OFFSET stkframe 	; ^ into stack frame
stkframe	DW	256*3 DUP (?)		; internal stack (256 ovls deep)
tempmem 	DW	16 DUP (-1)		; temp mem block storage
intnum		DW	?			; ovlmgr int number
hdr		EXEHDR	<>			; EXE header work area
		DB	512-TYPE EXEHDR DUP (?) ; exe hdr buffer for relocations
EXEHDRTMPSIZ	EQU	$ - hdr 		; size of temp reloc buffer
errortbl	DW	-1			; error message pointers
		DW	OFFSET baddos
		DW	OFFSET nofile
		DW	OFFSET noroom
		DW	OFFSET nofile
		DW	OFFSET nocore
		DW	OFFSET nocore
		DW	OFFSET badems
		DW	OFFSET nofile
		DW	OFFSET unknown
		DW	OFFSET unknown
		DW	OFFSET unknown
		DW	OFFSET unknown
		DW	OFFSET unknown
		DW	OFFSET unknown
		DW	OFFSET unknown
emmname 	DB	"EMMXXXX0"              ; EMM device driver name
emmtot		DW	0			; total emm blocks free
emmframesiz	DW	4			; frame size in blocks
emmflg		DB	0			; EMM present flag

i386code	DB	'386 specific code enabled.',CR,LF,'$'
memavl		DB	'Conventional memory available: $'
paragraphs	DB	'H paragraphs.',CR,LF,'$'
emsavl		DB	'EMS memory available: $'
pages		DB	'H 16K-pages.',CR,LF,'$'
noroom		DB	'Not enough free memory left to run this program.$'
nocore		DB	'Internal memory allocation failure.$'
nofile		DB	'Inaccessible EXE file. Can',27,'t load overlays.$'
baddos		DB	'Incorrect DOS version. Must be 3.00 or later.$'
badems		DB	'EMS memory manager error.$'
unknown 	DB	'Unknown error!$'
msghead 	DB	ESCAPE,'[0m',ESCAPE,'[K',CR,LF,ESCAPE,'[K',ESCAPE,'[1mOVLMGR:',ESCAPE,'[0m $'
diag		DB	ESCAPE,'[K',CR,LF,ESCAPE,'[K','        ($'
msgtail 	DB	ESCAPE,'[K',CR,LF,ESCAPE,'[K',BELL,'$'

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
		cld
		mov	ax,ds			; get our psp
		add	ax,10h
		mov	pspadd,ax		; save it
		mov	ah,DOSVERSION
		int	DOS
		cmp	al,3			; DOS 3.0 or later
		jnc	doenvthing
		mov	al,DOSERR		; incorrect version of dos
		jmp	putserr
doenvthing:
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
		mov	al,FILEERR		; can't open file!
		jmp	putserr
dontdie:
		mov	ovlexefilhdl,ax 	; save handle
IFNDEF NOEMS
chkems:
		mov	ah,DOSGETVEC
		mov	al,EMM
		int	DOS
		mov	ax,cs
		mov	ds,ax
		mov	di,0ah
		mov	si,OFFSET emmname
		mov	cx,8
		repe	cmpsb
		mov	al,0
		jnz	setemmflg
		mov	al,-1
setemmflg:
		mov	emmflg,al
		jnz	noemshere
		mov	ah,EMMFRAME
		int	EMM
		mov	emsframe,bx
		mov	ah,EMMTOTALS
		int	EMM
		mov	emmtot,bx
noemshere:
ENDIF
		mov	ax,SEG $$OVLBASE	; OVERLAY_AREA segment
		mov	ovlrootcode,ax
		mov	ax,SEG $$COVL		; segment of DGROUP
		mov	ds,ax
		mov	bx,$$CGSN		; number of modules
		mov	modcnt,bx		; save for later use
		mov	bx,$$COVL		; number of physical overlays
		mov	ovlcnt,bx		; save for later use

; Now allocate memory
		mov	ah,DOSALLOC		; bx contains # paras needed for ovltbl
		int	DOS
		jnc	gotovlram
		jmp	buyram
gotovlram:
		mov	ovltblbse,ax		; overlay descriptor table begins at start of memory block

		push	cs
		pop	ds
IFDEF DEBUG
IFDEF i386
		mov	ah,print
		mov	dx,OFFSET msghead
		int	DOS
		mov	ah,print
		mov	dx,OFFSET i386code
		int	DOS
ENDIF
		mov	ah,print
		mov	dx,OFFSET msghead
		int	DOS
		mov	ah,print
		mov	dx,OFFSET memavl
		int	DOS
		mov	ax,0a000h
		sub	ax,ovltblbse
		call	itoa
		mov	ah,print
		mov	dx,OFFSET paragraphs
		int	DOS
IFNDEF NOEMS
		mov	ah,print
		mov	dx,OFFSET msghead
		int	DOS
		mov	ah,print
		mov	dx,OFFSET emsavl
		int	DOS
		mov	ax,emmtot
		call	itoa
		mov	ah,print
		mov	dx,OFFSET pages
		int	DOS
ENDIF
ENDIF
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
		mov	ovlflg,al		; initialise ovl flags
		mov	ovlinvcnt,al		; initialise invocation count
		mov	ovlemshdl,-1
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
		sub	ax,hdr.hdrparas 	; actual size of code
		mov	ovlsiz,ax		; overlay size in paragraphs
		cmp	hdr.exeovlnum,0 	; skip if ovl 0 (root code)
		jz	notlargest
		cmp	ax,di			; find largest ovl
		jc	notlargest
		mov	di,ax
notlargest:
		mov	ax,hdr.hdrparas
		shl	ax,1
		shl	ax,1
		shl	ax,1
		shl	ax,1
		mov	ovlhdrsiz,ax		; hdr size in bytes
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
		ASSUME	ES:nothing		; prepare first memory block

		mov	ax,ovlrootcode		; OVERLAY_AREA segment
		mov	memblk1st,ax		; save pointer to first mem block
		mov	es,ax
		mov	es:memblkflg,0		; clear mem flags
		mov	es:memblknxt,0		; set next to nothing
		mov	es:memblkprv,0		; set previous to nothing
		mov	es:memblkovl,0		; no overlay loaded
		mov	es:memblksiz,di 	; di contains OVERLAY_AREA size in paragraphs
		add	ax,di
		mov	ovldata,ax		; end of OVERLAY_END
		push	di
		mov	es,ovltblbse		; temporary
		call	getemsmem		; see if any ems available
		mov	es:ovlemshdl,-1 	; fix these!
		and	es:ovlflg,NOT MASK ems
		push	dx
		call	getmoreram		; see if there are any other pieces lying around
		pop	ax
		pop	di
		or	ax,ax			; any ems?
		jnz	noramcheck
		inc	di
		cmp	dx,di
		jc	buyram
noramcheck:
		mov	WORD PTR ovltim,0	; initialise global lru time stamp
		mov	WORD PTR ovltim+2,0
		mov	di,OFFSET stkframe
		mov	WORD PTR cs:[di],-1	; initialise stack frame
		add	di,6
		mov	ax,ovltblbse
		mov	cs:[di],ax
		mov	curovl,di		; initialise stack frame pointer
		mov	es,ax
		mov	es:ovlflg,MASK locked OR MASK loaded ; set flags on ovl 0
		jmp	short chgintvec
buyram:
		mov	al,NOMEMERR		; free up some TSRs or something
		jmp	putserr
chgintvec:
		mov	ax,SEG $$INTNO
		mov	ds,ax
		mov	al,$$INTNO		; get int number to use
		xor	ah,ah
		shl	ax,1
		shl	ax,1
		mov	intnum,ax
		call	setvectors		; set up interrupt vectors
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

ovlmgr		PROC	FAR			; This is the it!

		ASSUME	DS:NOTHING,ES:NOTHING

IFDEF i386
		OP32
ENDIF
		mov	sireg,si		; preserve si
		mov	dsreg,ds		; and ds
		pop	si			; retrieve caller ip
		pop	ds			;     "      "    cs
		push	ax
		push	bx
		cld
		lodsb				; module # to call
		xor	ah,ah
		mov	bx,ax
		lodsw				; offset in ovl to call
		mov	WORD PTR farcall,ax	; into trampoline
		mov	ax,si
		mov	si,curovl		; get stack frame pointer
		add	si,6			; update stack
		mov	cs:[si-4],ds		; save return seg
		mov	cs:[si-2],ax		; and return offset

		shl	bx,1
		shl	bx,1			; * 4 (2 words/entry in module tbl)
		add	bx,OFFSET moduletbl
		mov	ds,cs:[bx]		; ovl tbl entry
		mov	ax,cs:[bx+2]		; segment fixup
		mov	cs:[si],ds		; ovl entry into stack frame
		mov	curovl,si

		ASSUME	DS:ovltbl

IFDEF i386
		OP32
ENDIF
		mov	si,WORD PTR ovltim	; lru time stamp
IFDEF i386
		OP32
ENDIF
		inc	si			; time passes!
IFDEF i386
		OP32
ENDIF
		mov	WORD PTR ovltim,si	; update global clock
IFDEF i386
		OP32
ENDIF
		mov	WORD PTR ovllrudat,si	; as well as ovl clock
IFNDEF i386
		mov	si,WORD PTR ovltim+2
		jz	ininc			; dword increment
cryupcdon:
		mov	WORD PTR ovllrudat+2,si ; as well as ovl clock
ENDIF
		test	ovlflg,MASK loaded	; ovl loaded?
		jz	inload			; load it or map it then.
ovlloadedupc:
		inc	ovlinvcnt
		add	ax,ovlmemblk		; add fixup and segment address
		mov	WORD PTR farcall+2,ax	; into trampoline
IFDEF i386
		OP32
ENDIF
		mov	si,sireg		; retore all registers
		mov	ds,dsreg
		pop	bx
		pop	ax
		popf				; don't forget these!
		call	DWORD PTR farcall	; and GO
		pushf				; preserve registers again!
		mov	dsreg,ds
IFDEF i386
		OP32
ENDIF
		mov	sireg,si
		mov	si,curovl		; stack frame pointer
		mov	ds,cs:[si]
		dec	ovlinvcnt
		sub	si,6			; adjust stack
		mov	ds,cs:[si]		; retrieve ovl tbl entry
		push	cs:[si+2]		; set return address
		push	cs:[si+4]
		mov	curovl,si
IFDEF i386
		OP32
ENDIF
		mov	si,WORD PTR ovltim	; do the lru thing again
IFDEF i386
		OP32
ENDIF
		inc	si
IFDEF i386
		OP32
ENDIF
		mov	WORD PTR ovltim,si
IFDEF i386
		OP32
ENDIF
		mov	WORD PTR ovllrudat,si
IFNDEF i386
		mov	si,WORD PTR ovltim+2
		jz	outinc
crydncdon:
		mov	WORD PTR ovllrudat+2,si
ENDIF
		test	ovlflg,MASK loaded	; ovl loaded?
		jz	outload 		; better get it before someone notices
jmpback:
IFDEF i386
		OP32
ENDIF
		mov	si,sireg		; get registers back
		mov	ds,dsreg
		iret				; and GO back

IFNDEF i386
ininc:
		inc	si
		mov	WORD PTR ovltim+2,si	; update global and
		jmp	cryupcdon
ENDIF

inload:
		test	ovlflg,MASK ems
		jz	infile
		push	ax
		mov	ax,ovlemshdl
		call	mappage
		pop	ax
		jmp	ovlloadedupc
infile:
		call	loadoverlay		; self explanatory
		jmp	ovlloadedupc

IFNDEF i386
outinc:
		inc	si
		mov	WORD PTR ovltim+2,si
		jmp	crydncdon
ENDIF

outload:
		test	ovlflg,MASK ems
		jz	outfile
		push	ax
		mov	ax,ovlemshdl
		call	mappage
		pop	ax
		jmp	jmpback
outfile:
		call	loadoverlay
		jmp	jmpback

ovlmgr		ENDP

;-------------------------------------------------------------------------------

loadoverlay	PROC	NEAR			; load overlay pointed to by es

		ASSUME	DS:NOTHING,ES:ovltbl

IFDEF i386
		OP32
		pusha			       ; eax,ecx,edx,ebx,esp,ebp,esi,edi
ELSE
		push	ax
		push	cx
		push	dx
		push	bx
		push	bp
		push	si
		push	di
ENDIF
		push	ds
		push	es			; just in case
		mov	ax,ds
		mov	es,ax
		cmp	ovlinvcnt,0
		jnz	fxdadr			; Yup, it's a toughie
		mov	ax,ovlsiz		; How much?
		call	getpages		; never fail mem alloc, you bet.
		jmp	gleaner
fxdadr:
		call	releasepages		; free memory where this ovl should be loaded
gleaner:
		add	ax,MEMCTLBLKSIZ 	; skip mem ctl blk
		mov	ovlmemblk,ax		; memory block to use
		mov	ds,ax
		mov	dx,ovlfiloff		; where in the file is it?
		mov	cl,dh
		mov	dh,dl
		xor	ch,ch
		xor	dl,dl
		shl	dx,1
		rcl	cx,1			; cx:dx = dx * 512
		mov	ax,ovlhdrsiz
		push	cx
		push	dx
		add	dx,ax
		adc	cx,0			; position to code
		mov	ah,DOSSEEK		; lseek to code
		mov	al,0			; from beginning of file
		mov	bx,ovlexefilhdl 	; never closing handle
		int	DOS
		jc	burnhead		; oops!
		xor	dx,dx			; buf = ds:0
		mov	cx,ovlsiz		; number of paragraphs to load
		shl	cx,1
		shl	cx,1
		shl	cx,1
		shl	cx,1			; * 16 = number of bytes
		mov	ah,DOSREAD		; prevent random DOS behaviour
		int	DOS			; read in code
		jc	burnhead		; double oops!
		pop	dx
		pop	cx			; position of hdr
		mov	ah,DOSSEEK		; lseek to hdr
		mov	al,0			; from beginning of file
		mov	bx,ovlexefilhdl 	; never closing handle
		int	DOS
		jc	burnhead		; oops!
		mov	cx,EXEHDRTMPSIZ 	; reloc buffer size
		mov	dx,OFFSET hdr
		push	ds
		mov	ax,cs
		mov	ds,ax
		mov	ah,DOSREAD		; prevent random DOS behaviour
		int	DOS			; read in header
		pop	ds
		jc	burnhead		; double oops!

		call	ovlrlc			; perform relocation normally done by DOS EXE loader
		pop	es			; retrieve ovl tbl entry
		pop	ds

		ASSUME	DS:ovltbl,ES:NOTHING

		or	ovlflg,MASK loaded	; because it is now
IFDEF i386
		OP32
		popa
ELSE
		pop	di
		pop	si
		pop	bp
		pop	bx
		pop	dx
		pop	cx
		pop	ax
ENDIF
		ret

burnhead:
		mov	al,FILEIOERR		; some kind of I/O error
		jmp	putserr

loadoverlay	ENDP

;-------------------------------------------------------------------------------

ovlrlc		PROC	NEAR			; ds:0 -> the overlay to relocate

		ASSUME	DS:NOTHING,ES:NOTHING

		mov	si,OFFSET hdr
		mov	bp,si
		add	bp,EXEHDRTMPSIZ 	; ^ to end of buf+1
		mov	cx,cs:[si.relocitems]	; roto-count
		jcxz	relocdone		; not such a good idea, after all
		mov	di,ds
		sub	di,ovlrootcode		; segment fixup value
		add	si,cs:[si.reloctbloff]	; ^ relocation table
dorelocs:					; labels don't GET comments
		cmp	si,bp			; past the end ?
		jc	getoffsetl
		call	getnxtreloc		; get another hunk
getoffsetl:
		mov	bl,cs:[si]		; offset into load module
		inc	si
		cmp	si,bp			; past the end ?
		jc	getoffseth
		call	getnxtreloc		; get another hunk
getoffseth:
		mov	bh,cs:[si]		; offset into load module
		inc	si
		cmp	si,bp			; past the end ?
		jc	getsegmentl
		call	getnxtreloc		; get another hunk
getsegmentl:
		mov	al,cs:[si]		; segment in load module (zero reference)
		inc	si
		cmp	si,bp			; past the end ?
		jc	getsegmenth
		call	getnxtreloc		; get another hunk
getsegmenth:
		mov	ah,cs:[si]		; segment in load module (zero reference)
		inc	si
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

getnxtreloc	PROC	NEAR

		ASSUME	DS:NOTHING,ES:NOTHING

		push	bx
		push	cx
		push	di
		push	bp
		push	ds
		push	es
		mov	cx,EXEHDRTMPSIZ 	; reloc buffer size
		mov	dx,OFFSET hdr
		mov	ax,cs
		mov	ds,ax
		mov	bx,ovlexefilhdl 	; never closing handle
		mov	ah,DOSREAD		; prevent random DOS behaviour
		int	DOS			; read in header
		jnc	nxtrelocok
		jmp	burnhead		; double oops!
nxtrelocok:
		mov	si,OFFSET hdr
		pop	es
		pop	ds
		pop	bp
		pop	di
		pop	cx
		pop	bx
		ret

getnxtreloc	ENDP

;-------------------------------------------------------------------------------

getvictim	PROC	NEAR			; select a victim to discard (and free up some memory)

		ASSUME	DS:ovltbl,ES:NOTHING

		push	bx
		push	cx
		push	dx
		push	si
		push	di
		push	bp
		push	ds
		mov	ds,ovltblbse		; ^ ovl tbl
IFDEF i386
		OP32
ENDIF
		xor	ax,ax			; will contain the low word of lru
IFDEF i386
		OP32
ENDIF
		mov	dx,ax			; will contain the high word of lru
		mov	bp,ax			; will contain ovl tbl entry
		mov	bx,ax			; ovl tbl ptr
		mov	cx,ovlcnt
foon1:
		test	ovlflg[bx],MASK locked
		jnz	skip1
		test	ovlflg[bx],MASK ems
		jnz	foon2
		test	ovlflg[bx],MASK loaded
		jz	skip1
foon2:
IFDEF i386
		OP32
ENDIF
		mov	si,WORD PTR ovltim
IFNDEF i386
		mov	di,WORD PTR ovltim+2
ENDIF
IFDEF i386
		OP32
ENDIF
		sub	si,WORD PTR ovllrudat[bx]
IFNDEF i386
		sbb	di,WORD PTR ovllrudat[bx+2]
ENDIF
IFDEF i386
		OP32
		cmp	dx,si
ELSE
		cmp	dx,di
ENDIF
IFDEF i386
		jnc	skip1
ELSE
		jc	better1
		jnz	skip1
		cmp	ax,si
		jnc	skip1
ENDIF
better1:
IFDEF i386
		OP32
		mov	dx,si
ELSE
		mov	ax,si
		mov	dx,di
ENDIF
		mov	bp,bx
skip1:
		add	bx,OVLSEGSIZ
		loop	foon1
		or	bp,bp			; were we more successful this time?
		jnz	gotvictim		; now we got one.
nomoremem:
		mov	al,VICTIMERR		; were really %$# now!
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

int21		PROC	FAR

; free almost all overlay memory if app. tries to call the DOS exec function.

		cmp	ah,DOSEXEC
		jz	freeall
		cmp	ah,TERMINATE
		jz	saybyebye
notours:
		jmp	cs:oldint21
saybyebye:
		mov	al,NOERR		; return code 0
		jmp	putserr
freeall:
		or	al,al			; is it load and exec?
		jnz	notours
		push	ax
		push	cx
		push	dx
		push	bx
		push	bp
		push	si
		push	di
		push	es
		push	ds			; preserve calling env.

		ASSUME	DS:NOTHING,ES:ovltbl

		mov	es,ovltblbse
		mov	cx,ovlcnt		; unload all overlays that are
		mov	bx,OVLSEGSIZ		; in EMS or are in alloced mem.
		dec	cx
memunloadlp:
		test	[bx.ovlflg],MASK ems
		jnz	memunload
		test	[bx.ovlflg],MASK loaded
		jz	nxtmemunload
		mov	ax,[bx.ovlmemblk]
		sub	ax,MEMCTLBLKSIZ
		cmp	ax,memblks		; allocated memory ?
		jc	nxtmemunload
memunload:
		and	[bx.ovlflg],NOT MASK loaded ; you're outta there!
nxtmemunload:
		add	bx,OVLSEGSIZ
		loop	memunloadlp

		mov	curemshandle,-1 	; no current handle anymore

		mov	ax,memblks
		cmp	ax,-1
		jz	nosecondblk
		mov	es,ax			; ^ to second mem blk
		mov	es,es:memblkprv 	; get previous pointer
		mov	es:memblknxt,0		; no other blocks after this one
nosecondblk:
		mov	cx,16			; do all allocated mem blocks
		mov	si,OFFSET memblks
freememblklp:
		mov	ax,cs:[si]		; get memory blk segment
		cmp	ax,-1			; was one ever allocated?
		jz	nxtmemblklp		; nope
		mov	es,ax
		mov	ah,DOSFREE		; must free it.
		int	DOS
		mov	WORD PTR cs:[si],-1
nxtmemblklp:
		add	si,2
		loop	freememblklp

		call	rstvectors		; restore all int vectors

		mov	bp,sp
		push	[bp+22] 		; ensure returned flags are based on user's!
		popf
		pop	ds
		pop	es
		pop	di
		pop	si
		pop	bp
		pop	bx
		pop	dx
		pop	cx
		pop	ax

		mov	ssreg,ss		; preserve these due to a
		mov	spreg,sp		; DOS bug.

		int	DOS			; allow DOS to continue!

		mov	ss,ssreg
		mov	sp,spreg

		push	ax
		push	cx
		push	dx
		push	bx
		push	bp
		push	si
		push	di
		push	es
		push	ds			; preserve calling env.
		mov	bp,sp
		pushf
		pop	[bp+22] 		; fix return flags

		call	getmoreram		; re-allocate our memory
		call	setvectors		; patch vectors again

		pop	ds
		pop	es
		pop	di
		pop	si
		pop	bp
		pop	bx
		pop	dx
		pop	cx
		pop	ax
		iret

int21		ENDP

;-------------------------------------------------------------------------------

releasepages	PROC	NEAR			; Arg in es, result in ax

; release any memory (and overlays) where this overlay should reside

		ASSUME	DS:NOTHING,ES:ovltbl

		mov	bx,ovlmemblk		; start of memory to release
		sub	bx,MEMCTLBLKSIZ
		mov	dx,bx
		add	dx,es:ovlsiz
		add	dx,MEMCTLBLKSIZ 	; end of memory to release
		mov	ax,ovlemshdl
		cmp	ax,-1
		jz	doitagain
		call	mappage
		or	ovlflg,MASK ems
		mov	ax,emsframe
		jmp	dvart
doitagain:
		mov	ax,memblk1st		; first memory blk
		jmp	dvart
dvartloop:
		mov	ds,ax			; memory blk to check
		cmp	bx,ax			; does it start below the memory to release?
		jnc	dvartsmaller		; yup
		cmp	ax,dx			; does it start above?
		jnc	dvartnocore		; yup
		call	killmem 		; it's in the way. Zap it.
		jmp	dvartloop
dvartsmaller:
		add	ax,ds:memblksiz 	; end of this memory blk
		cmp	bx,ax			; does it end below the memory to release?
		jnc	dvartsilly		; yup
		test	ds:memblkflg,MASK_used
		jz	dvartfree
		call	killmem 		; Oh well, zap it too.
		add	ax,ds:memblksiz 	; end of this memory blk
dvartfree:
		cmp	ax,dx			; does it end in the memory to be released?
		jc	dvartsilly
dvartgotblk:
		mov	ax,ds			; this is it!
		mov	cx,bx
		sub	cx,ax			; # of paragraphs between start of memory to release and mem blk
		jz	nosplit
		push	es
		call	splitblk
		or	es:memblkflg,MASK_used	; set high block used
		call	mergemem		; merge remaining free memory
		mov	ax,es
		mov	ds,ax
		pop	es
nosplit:
		mov	cx,es:ovlsiz
		add	cx,MEMCTLBLKSIZ 	; paragraphs needed to load ovl
		jmp	splitblklow		; split remaining block
dvartsilly:
		mov	ax,ds:memblknxt
dvart:
		or	ax,ax			; end of mem list?
		jz	dvartnocore
		jmp	dvartloop		; play it again Sam.
dvartnocore:
		mov	al,RELERR		; super OOPS!
		jmp	putserr

releasepages	ENDP

;-------------------------------------------------------------------------------

getpages	PROC	NEAR			; get enough memory to load ovl

		ASSUME	DS:NOTHING,ES:ovltbl

		mov	ovlemshdl,-1		; clear any EMS stuff
		and	ovlflg,NOT MASK ems
		mov	cx,ax
		add	cx,MEMCTLBLKSIZ 	; total paragraphs needed
dorkagain:
		call	largestmem		; find largest free blk
		cmp	dx,cx			; large enough?
		jnc	gotdork 		; yup.
		call	getemsmem		; try to allocate ems
		cmp	dx,cx			; any available ?
		jnc	gotdork
dorkkill:
		call	getvictim		; select a victim to release
		call	killovl 		; kill the selected victim
		jmp	dorkagain
gotdork:
		jmp	splitblklow		; split the free blk

getpages	ENDP

;-------------------------------------------------------------------------------

splitblklow	PROC	NEAR

; split a block of memory returning the lower one to be used.

		ASSUME	DS:NOTHING,ES:NOTHING

		push	es
		or	ds:memblkflg,MASK_used	; set low block used
		call	splitblk
		jc	splitlowdone
		push	ds
		mov	ax,es
		mov	ds,ax
		call	mergemem		; merge remaining free memory
		pop	ds
splitlowdone:
		pop	es
		mov	ds:memblkovl,es 	; fix ptr to ovl
		mov	ax,ds			; return lower blk segment
		ret

splitblklow	ENDP

;-------------------------------------------------------------------------------

splitblk	PROC	NEAR

		ASSUME	DS:NOTHING,ES:NOTHING

		mov	ax,ds
		add	ax,cx
		mov	es,ax			; ^ to upper blk to be created
		mov	ax,ds:memblksiz
		sub	ax,cx
		jbe	nofix			; must be at least 1 para remaining to split
		mov	ds:memblksiz,cx 	; fix blk sizes
		mov	es:memblksiz,ax
		mov	ax,ds:memblknxt 	; fix pointers
		mov	es:memblknxt,ax
		mov	ds:memblknxt,es
		mov	es:memblkprv,ds
		mov	es:memblkflg,0		; set upper to not used
		mov	ax,es:memblknxt
		or	ax,ax
		jz	nofix
		push	ds
		mov	ds,ax			; fix blk after upper to point to upper
		mov	ds:memblkprv,es
		pop	ds
		clc
		ret
nofix:
		stc
		ret

splitblk	ENDP

;-------------------------------------------------------------------------------

largestmem	PROC	NEAR	; returns seg in ax, size in dx
				; retruns first block that's large enough if possible

		ASSUME	DS:NOTHING,ES:ovltbl

		mov	ax,memblk1st		; first mem blk
		xor	dx,dx			; largest size found
		jmp	gook
gookloop:
		mov	ds,ax
		test	ds:memblkflg,MASK_used	; is this blk used?
		jnz	gookme			; yup
		cmp	ds:memblksiz,cx 	; is it large enough?
		jc	gookme			; nope
		mov	dx,ds:memblksiz 	; got one!
		ret
gookme:
		mov	ax,ds:memblknxt
gook:
		or	ax,ax			; end of list?
		jnz	gookloop		; around and around
		ret

largestmem	ENDP

;-------------------------------------------------------------------------------

killmem 	PROC	NEAR

		ASSUME	DS:NOTHING,ES:ovltbl

		test	ds:memblkflg,MASK_used	; is it used?
		jz	memnotused		; don't kill ovl
		push	es
		mov	es,ds:memblkovl
		and	ovlflg,NOT MASK loaded	; zap ovl associated with this blk
		and	ovlflg,NOT MASK ems
		pop	es
memnotused:
		jmp	mergemem		; merge free memory

killmem 	ENDP

;-------------------------------------------------------------------------------

killovl 	PROC	NEAR		; preserves bx

		ASSUME	DS:ovltbl,ES:NOTHING

		mov	ds,ax
		and	ovlflg,NOT MASK loaded	; ovl no longer loaded
		test	ovlflg,MASK ems 	; was it in ems ?
		jz	noemskill
		and	ovlflg,NOT MASK ems	; no longer in ems
		mov	ax,ovlemshdl
		call	mappage
noemskill:
		mov	ax,ovlmemblk		; get mem blk
		sub	ax,MEMCTLBLKSIZ
		mov	ds,ax
		jmp	mergemem		; merge free memory

killovl 	ENDP

;-------------------------------------------------------------------------------

mergemem	PROC	NEAR

; merge physically adjacent free memory blocks. Preserves es. ds -> a free block.

		ASSUME	DS:NOTHING,ES:NOTHING

		push	dx
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
		pop	dx
		mov	ax,ds
		ret

mergemem	ENDP

;-------------------------------------------------------------------------------

getmoreram	PROC	NEAR			; try to alloc remaining pieces
						; of memory if any
		ASSUME	DS:NOTHING,ES:NOTHING	; return dx = biggest block

		push	cx
		push	bx
		push	si
		push	di
		push	ds
		push	es
		xor	dx,dx
		mov	ax,memblk1st
nxtlowblk:
		mov	ds,ax
		mov	ax,ds:memblknxt
		or	ax,ax
		jnz	nxtlowblk

		mov	si,OFFSET memblks	; a place to store the handles
		mov	di,OFFSET tempmem	; a place to store the rejects
		mov	cx,16			; 16 more max
getramlp:
		mov	ah,DOSALLOC
		mov	bx,0ffffh		; Everything
		int	DOS
		cmp	bx,10h			; nothing smaller than .25k please
		jc	gotallram
		mov	ah,DOSALLOC		; allocate our own memory
		int	DOS
		jc	gotallram		; oops!
		cmp	ax,ovltblbse		; is it after our first mem blk?
		jc	releaseblk
		cmp	dx,bx
		jnc	notbigger
		mov	dx,bx
notbigger:
		mov	cs:[si],ax		; save it
		mov	es,ax
		mov	es:memblkflg,0		; clear mem flags
		mov	es:memblknxt,0		; set next to nothing
		mov	es:memblkovl,0		; no overlays loaded
		mov	es:memblkprv,ds 	; point to previous
		mov	es:memblksiz,bx 	; allocated memory block size
		mov	ds:memblknxt,es 	; point to next
		add	si,2
		mov	ds,ax
		jmp	short getnxtram
releaseblk:
		mov	cs:[di],ax
		add	di,2
getnxtram:
		loop	getramlp
gotallram:
		mov	si,OFFSET tempmem
		mov	cx,16
releaselp:
		mov	ax,cs:[si]
		cmp	ax,-1
		jz	relnext
		mov	es,ax
		mov	ah,DOSFREE
		int	DOS
		mov	WORD PTR cs:[si],-1
relnext:
		add	si,2
		loop	releaselp
		pop	es
		pop	ds
		pop	di
		pop	si
		pop	bx
		pop	cx
		ret

getmoreram	ENDP

;-------------------------------------------------------------------------------

getemsmem	PROC	NEAR

		ASSUME	DS:NOTHING,ES:ovltbl

		xor	dx,dx			; no ems memory
		cmp	emmflg,-1
		jz	testemsslots
		ret
testemsslots:
		mov	curemshandle,-1
		mov	di,OFFSET emsmemblks
		mov	bx,cx
		mov	cx,16
emsfreeslot:
		mov	ax,cs:[di]
		cmp	ax, -1
		jz	gotemsslot
		call	mappage
		cmp	ax,bx
		jnc	foundpage
		add	di,2
		loop	emsfreeslot
		mov	cx,bx
		xor	dx,dx
		ret
gotemsslot:
		mov	cx,bx
		mov	bx,4
		mov	ah,EMMALLOC
		push	cx			; paranoia ! shouldn't be necessary.
		push	di
		push	es
		int	EMM
		pop	es
		pop	di
		pop	cx
		or	ah,ah
		jz	gotsomeems
		xor	dx,dx
		ret
gotsomeems:
		mov	cs:[di],dx
		mov	ovlemshdl,dx
		or	ovlflg,MASK ems
		mov	ax,dx
		call	mapemspages
		mov	ax,emsframe
		mov	ds,ax
		mov	ds:memblkflg,0		; clear mem flags
		mov	ds:memblknxt,0		; set next to nothing
		mov	ds:memblkprv,0		; set previous to nothing
		mov	ds:memblkovl,0		; no overlay loaded
		mov	dx,1000h
		mov	ds:memblksiz,dx
		ret

foundpage:
		mov	cx,bx
		mov	ds,si
		mov	dx,ax
		mov	ax,cs:[di]
		mov	ovlemshdl,ax
		or	ovlflg,MASK ems
		ret

getemsmem	ENDP

;-------------------------------------------------------------------------------

mappage 	PROC	NEAR			; map a 64K block of EMS mem.

		ASSUME	DS:NOTHING,ES:ovltbl

		cmp	ax,curemshandle
		jnz	doems
		ret
doems:
		push	bx
		push	dx
		push	ds
		push	es
		call	mapemspages
		mov	ax,emsframe
		xor	dx,dx
		xor	si,si
emsset:
		mov	ds,ax
		test	ds:memblkflg,MASK_used	; mem blk used ?
		jz	emsfreeblk
		mov	es,ds:memblkovl
		or	ovlflg,MASK ems OR MASK loaded
		jmp	emsnext
emsfreeblk:
		mov	ax,ds:memblksiz
		cmp	dx,ax
		jnc	emsnext
		mov	dx,ax
		mov	si,ds
emsnext:
		mov	ax,ds:memblknxt
		or	ax,ax
		jnz	emsset

		mov	ax,dx
		pop	es
		pop	ds
		pop	dx
		pop	bx
		ret

mappage 	ENDP

;-------------------------------------------------------------------------------

mapemspages	PROC	NEAR

		ASSUME	DS:NOTHING,ES:ovltbl

		push	es
		push	bx
		push	cx
		push	dx
		mov	curemshandle,ax
		mov	dx,ax
		mov	ah,EMMMAP
		xor	al,al			; physical page 0
		xor	bx,bx			; logical page 0
		push	dx
		int	EMM
		pop	dx
		or	ah,ah
		jnz	emmerror
		mov	ah,EMMMAP
		mov	al,1			; physical page 1
		mov	bx,1			; logical page 1
		push	dx
		int	EMM
		pop	dx
		or	ah,ah
		jnz	emmerror
		mov	ah,EMMMAP
		mov	al,2			; physical page 2
		mov	bx,2			; logical page 2
		push	dx
		int	EMM
		pop	dx
		or	ah,ah
		jnz	emmerror
		mov	ah,EMMMAP
		mov	al,3			; physical page 3
		mov	bx,3			; logical page 3
		int	EMM
		or	ah,ah
		jnz	emmerror
		mov	es,ovltblbse
		mov	cx,ovlcnt
		xor	bx,bx
testems:
		test	ovlflg[bx],MASK ems
		jz	nxttestems
		and	ovlflg[bx],NOT MASK loaded
nxttestems:
		add	bx,OVLSEGSIZ
		loop	testems
		pop	dx
		pop	cx
		pop	bx
		pop	es
		ret

emmerror:
		mov	al,EMSERR		; ems manager error
		jmp	putserr

mapemspages	ENDP

;-------------------------------------------------------------------------------

gethdr		PROC	NEAR			; read EXE header from handle

		ASSUME	DS:NOTHING,ES:NOTHING

		push	cx
		push	ds
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
		pop	ds
		pop	cx
		ret				; Wow, it worked!
exegone:
		mov	al,HDRERR		; You lose!
		jmp	putserr

gethdr		ENDP

;-------------------------------------------------------------------------------

putserr 	PROC	NEAR

; display error msg, close file, restore int vectors, free mem and return to DOS.

		ASSUME	DS:NOTHING,ES:NOTHING

		xor	ah,ah
		push	ax			; keep return code for later
		push	cs
		pop	ds
		mov	bx,ax
		shl	bx,1
		add	bx,OFFSET errortbl
		mov	dx,[bx]
		cmp	dx,-1
		jz	freeints
		push	dx
		mov	dx,OFFSET msghead
		mov	ah,PRINT
		int	DOS
		pop	dx
		mov	ah,PRINT
		int	DOS			; display error msg

		mov	ah,PRINT
		mov	dx,OFFSET diag
		int	DOS
		pop	ax
		push	ax
		call	itoa			; error number
		mov	ah,DOSPUTC
		mov	dl,':'
		int	DOS
		mov	ax,VERSION
		call	itoa			; version number
		mov	ah,DOSPUTC
		mov	dl,':'
		int	DOS
		mov	ax,0a000h
		sub	ax,ovltblbse		; conventional memory
		call	itoa
		mov	ah,DOSPUTC
		mov	dl,':'
		int	DOS
		mov	si,OFFSET emsmemblks
		mov	cx,16
		xor	ax,ax
emstotlp:
		cmp	WORD PTR cs:[si],-1
		jz	gotemstot
		add	ax,emmframesiz
		add	si,2
		loop	emstotlp
gotemstot:
		call	itoa			; ems usage in blocks
		mov	ah,DOSPUTC
		mov	dl,')'
		int	DOS

		mov	dx,OFFSET msgtail
		mov	ah,PRINT
		int	DOS
freeints:
		call	rstvectors		; restore all int vectors

		mov	ax,ovltblbse
		cmp	ax,-1
		jz	freememblks
		mov	es,ax
		mov	ah,DOSFREE
		int	DOS
freememblks:
		mov	cx,16			; do all allocated mem blocks
		mov	si,OFFSET memblks
freememlp:
		mov	ax,cs:[si]		; get memory blk segment
		cmp	ax,-1			; was one ever allocated?
		jz	nxtmemlp		; nope
		mov	es,ax
		mov	ah,DOSFREE		; must free it.
		int	DOS
nxtmemlp:
		add	si,2
		loop	freememlp
		mov	cx,16			; do all allocated ems blocks
		mov	si,OFFSET emsmemblks
freeemsmemlp:
		mov	dx,cs:[si]		; get memory blk segment
		cmp	dx,-1			; was one ever allocated?
		jz	nxtemsmemlp		; nope
		mov	ah,EMMFREE		; must free it.
		int	EMM
nxtemsmemlp:
		add	si,2
		loop	freeemsmemlp
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

;-------------------------------------------------------------------------------

itoa		PROC	NEAR

		push	ax
		xchg	ah,al
		call	putbyte
		pop	ax
		jmp	putbyte

itoa		ENDP

;-------------------------------------------------------------------------------

putbyte 	PROC	NEAR

		push	ax
		shr	al,1
		shr	al,1
		shr	al,1
		shr	al,1
		call	nibble
		pop	ax
		jmp	nibble

putbyte 	ENDP

;-------------------------------------------------------------------------------

nibble		PROC	NEAR

		and	al,0fh
		add	al,30h
		cmp	al,3ah
		jc	nibok
		add	al,7
nibok:
		mov	dl,al
		mov	ah,DOSPUTC
		int	DOS
		ret

nibble		ENDP

;-------------------------------------------------------------------------------

setvectors	PROC	NEAR

		push	ds
		xor	ax,ax
		mov	ds,ax
		mov	si,cs:intnum
		cli
		mov	ax,[si]
		mov	WORD PTR cs:oldvec,ax	; save original vector
		mov	ax,[si+2]
		mov	WORD PTR cs:oldvec+2,ax
		mov	ax,OFFSET ovlmgr	; point to ovlmgr
		mov	[si],ax 		; set int vector
		mov	[si+2],cs

		mov	si,DOS*4
		mov	ax,[si]
		mov	WORD PTR cs:oldint21,ax ; save original vector
		mov	ax,[si+2]
		mov	WORD PTR cs:oldint21+2,ax
		mov	ax,OFFSET int21 	; point to new int21
		mov	[si],ax 		; set int vector
		mov	[si+2],cs
		sti
		pop	ds
		ret

setvectors	ENDP

;-------------------------------------------------------------------------------

rstvectors	PROC	NEAR

		push	ds
		xor	ax,ax
		mov	ds,ax
		mov	si,DOS*4
		cli
		mov	ax,WORD PTR cs:oldint21 ; put back dos vector
		cmp	ax,-1
		jz	rstvec
		mov	[si],ax
		mov	ax,WORD PTR cs:oldint21+2
		mov	[si+2],ax
rstvec:
		mov	si,cs:intnum
		mov	ax,WORD PTR cs:oldvec	; put back ovlmgr vector
		cmp	ax,-1
		jz	rstdone
		mov	[si],ax
		mov	ax,WORD PTR cs:oldvec+2
		mov	[si+2],ax
		sti
rstdone:
		pop	ds
		ret

rstvectors	ENDP

code		ENDS

		END
