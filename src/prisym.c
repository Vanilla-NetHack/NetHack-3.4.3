/*	SCCS Id: @(#)prisym.c	3.0	89/11/15
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef WORM
#include "wseg.h"
#include "lev.h"

STATIC_DCL void FDECL(pwseg, (struct wseg *));
#endif

#ifdef OVL0

void
atl(x,y,ch)
register int x, y;
char ch;
{
	register struct rm *crm = &levl[x][y];

	if(x<0 || x>COLNO-1 || y<0 || y>ROWNO-1){
		impossible("atl(%d,%d,%c)",x,y,ch);
		return;
	}
	if(crm->seen && crm->scrsym == ch) return;
	/* crm->scrsym = (uchar) ch; */
	/* wrong if characters are signed but uchar is larger than char,
	 * and ch, when passed, was greater than 127.
	 * We probably should _really_ go around changing atl to take a
	 * uchar for its third argument...
	 */
	crm->scrsym = (uchar)((unsigned char) ch);
	crm->new = 1;
	on_scr(x,y);
}

void
on_scr(x,y)
register int x, y;
{
	if(x < scrlx) scrlx = x;
	if(x > scrhx) scrhx = x;
	if(y < scrly) scrly = y;
	if(y > scrhy) scrhy = y;
}

#endif /* OVL0 */
#ifdef OVL2

/* call: (x,y) - display
	(-1,0) - close (leave last symbol)
	(-1,-1)- close (undo last symbol)
	(-1,let)-open: initialize symbol
	(-2,let)-change let
	(-3,let)-set color
*/

void
tmp_at(x, y)
int x, y;
{
#ifdef LINT	/* static schar prevx, prevy; static char let; */
schar prevx=0, prevy=0;
uchar let;
uchar col;
#else
static schar NEARDATA prevx, NEARDATA prevy;
static uchar NEARDATA let;
static uchar NEARDATA col;
#endif

	switch ((int)x) {
	    case -2:		/* change let call */
		let = y;
		return;
	    case -1:		/* open or close call */
		if ((int)y >= 0) {
		    let = y;
		    prevx = -1;
		    col = AT_ZAP;
		    return;
		}
		break;
	    case -3:		/* set color call */
		col = y;
		return;
	}
	if(prevx >= 0 && cansee(prevx,prevy)) {
		delay_output();
		prl(prevx, prevy);	/* in case there was a monster */
		at(prevx, prevy, levl[prevx][prevy].scrsym, AT_APP);
	}
	if(x >= 0){	/* normal call */
		if(cansee(x,y)) at(x,y,let,col);
		prevx = x;
		prevy = y;
	} else {	/* close call */
		let = 0;
		prevx = -1;
	}
}

/* like the previous, but the symbols are first erased on completion */
void
Tmp_at2(x, y)
int x, y;
{
#ifdef LINT	/* static char let; static xchar cnt; static coord tc[COLNO]; */
uchar let;
xchar cnt;
coord tc[COLNO];	/* but watch reflecting beams! */
# ifdef TEXTCOLOR
uchar col;
# endif
#else
static uchar NEARDATA let;
static xchar NEARDATA cnt;
static coord NEARDATA tc[COLNO];	/* but watch reflecting beams! */
# ifdef TEXTCOLOR
static uchar NEARDATA col;
# endif
#endif
register int xx,yy;
	switch((int)x) {
	    case -1:
		if(y > 0) {	/* open call */
			let = y;
			cnt = 0;
#ifdef TEXTCOLOR
			col = AT_ZAP;
#endif
			return;
		}
		/* close call (do not distinguish y==0 and y==-1) */
		while(cnt--) {
			xx = tc[cnt].x;
			yy = tc[cnt].y;
			prl(xx, yy);
			at(xx, yy, levl[xx][yy].scrsym, AT_APP);
		}
		cnt = let = 0;	/* superfluous */
		return;
	    case -2:		/* change let call */
		let = y;
		return;
#ifdef TEXTCOLOR
	    case -3:		/* set color call */
		col = y;
		return;
#endif
	}
	/* normal call */
	if(cansee(x,y)) {
		if(cnt) delay_output();
#ifdef TEXTCOLOR
		at(x,y,let,col);
#else
		at(x,y,let,AT_ZAP);
#endif
		tc[cnt].x = x;
		tc[cnt].y = y;
		if(++cnt >= COLNO) panic("Tmp_at2 overflow?");
		levl[x][y].new = 0;	/* prevent pline-nscr erasing --- */
	}
}

#endif /* OVL2 */
#ifdef OVL1

void
curs_on_u()
{
#ifdef CLIPPING
	cliparound(u.ux, u.uy);
	(void)win_curs(u.ux, u.uy);
#else
	curs(u.ux, u.uy+2);
#endif
}

void
pru()
{
	if(u.udispl && (Invisible || u.udisx != u.ux || u.udisy != u.uy))
		/* if(! levl[u.udisx][u.udisy].new) */
			if(!vism_at(u.udisx, u.udisy))
				newsym(u.udisx, u.udisy);
	if(Invisible
#ifdef POLYSELF
			|| u.uundetected
#endif
					) {
		u.udispl = 0;
		prl(u.ux,u.uy);
	} else
	if(!u.udispl || u.udisx != u.ux || u.udisy != u.uy) {
		atl(u.ux, u.uy, (char) u.usym);
		u.udispl = 1;
		u.udisx = u.ux;
		u.udisy = u.uy;
	}
	levl[u.ux][u.uy].seen = 1;
}

#endif /* OVL1 */
#ifdef OVL0

/* print a position that is visible for @ */
void
prl(x,y)
int x, y;
{
	register struct rm *room;
	register struct monst *mtmp = (struct monst *)0;
	register struct obj *otmp;
	register struct trap *ttmp;

	if(x == u.ux && y == u.uy && !Invisible
#ifdef POLYSELF
						&& !u.uundetected
#endif
								) {
		pru();
		return;
	}
	if(!isok(x,y)) return;
	room = &levl[x][y];
	if((!room->typ) ||
	   (IS_ROCK(room->typ) && levl[u.ux][u.uy].typ == CORR &&
				  !levl[u.ux][u.uy].lit))
	    /* the only lit corridor squares should be the entrances to
	     * outside castle areas */
		return;
	if(MON_AT(x, y)) mtmp = m_at(x,y);
	if(mtmp && !mtmp->mhide &&
		(!mtmp->minvis || See_invisible)) {
#ifdef WORM
		if(m_atseg)
			pwseg(m_atseg);
		else
#endif
		pmon(mtmp);
	}
	else if(OBJ_AT(x, y) && !is_pool(x,y)) {
		otmp = level.objects[x][y];
		atl(x,y,Hallucination ? rndobjsym() : otmp->olet);
	}
	else if(room->gmask && !is_pool(x,y))
		atl(x,y,Hallucination ? rndobjsym() : GOLD_SYM);
	else if((!mtmp || mtmp->data == &mons[PM_GIANT_SPIDER]) &&
		  (ttmp = t_at(x,y)) && ttmp->ttyp == WEB)
		atl(x,y,(char)WEB_SYM);
	else if(mtmp && (!mtmp->minvis || See_invisible)) {
		/* must be a hiding monster, but not hiding right now */
		/* assume for the moment that long worms do not hide */
		pmon(mtmp);
	}
	else if(!room->seen || room->scrsym == STONE_SYM) {
		room->new = room->seen = 1;
		newsym(x,y);
		on_scr(x,y);
	}
	room->seen = 1;
}

uchar
news0(x,y)
register xchar x,y;
{
	register struct obj *otmp;
	register struct trap *ttmp;
	struct rm *room;
	register uchar tmp;	/* don't compare char with uchar -- OIS */
	register int croom;

	room = &levl[x][y];
	/* note: a zero scrsym means to ignore the presence of objects */
	if(!room->seen) tmp = STONE_SYM;
	else if(room->typ == POOL || room->typ == MOAT) tmp = POOL_SYM;
	else if(OBJ_AT(x, y) && !Blind && room->scrsym) {
		otmp = level.objects[x][y];
		tmp = Hallucination ? rndobjsym() : otmp->olet;
	}
	else if(room->gmask && !Blind && room->scrsym) 
		tmp = Hallucination ? rndobjsym() : GOLD_SYM;
	else if(x == xupstair && y == yupstair) tmp = UP_SYM;
	else if(x == xdnstair && y == ydnstair) tmp = DN_SYM;
#ifdef STRONGHOLD
	else if(x == xupladder && y == yupladder) tmp = UPLADDER_SYM;
	else if(x == xdnladder && y == ydnladder) tmp = DNLADDER_SYM;
#endif
	else if((ttmp = t_at(x,y)) && ttmp->ttyp == WEB) tmp = WEB_SYM;
	else if(ttmp && ttmp->tseen) tmp = TRAP_SYM;
	else switch(room->typ) {
	case SCORR:
		tmp = ' ';	/* _not_ STONE_SYM! */
		break;
	case SDOOR:
		croom = inroom(x,y);
		if(croom == -1) {
#ifdef STRONGHOLD
			if(IS_WALL(levl[x-1][y].typ)) tmp = HWALL_SYM;
			else tmp = VWALL_SYM;
			break;
#else
			impossible("door %d %d not in room",x,y);
#endif
		}
		if(rooms[croom].lx-1 == x || rooms[croom].hx+1 == x)
			tmp = VWALL_SYM;
		else	/* SDOORs aren't created on corners */
			tmp = HWALL_SYM;
  		break;
	case HWALL:
#ifdef STRONGHOLD
		if (is_maze_lev && is_drawbridge_wall(x,y) >= 0) tmp = DB_HWALL_SYM;
		else
#endif
		tmp = HWALL_SYM;
		break;
	case VWALL:
#ifdef STRONGHOLD
		if (is_maze_lev && is_drawbridge_wall(x,y) >= 0) tmp = DB_VWALL_SYM;
		else
#endif
		tmp = VWALL_SYM;
		break;
	case TLCORNER:
		tmp = TLCORN_SYM;
		break;
	case TRCORNER:
		tmp = TRCORN_SYM;
		break;
	case BLCORNER:
		tmp = BLCORN_SYM;
		break;
	case BRCORNER:
		tmp = BRCORN_SYM;
		break;
	case DOOR:
		if (room->doormask == D_NODOOR || room->doormask & D_BROKEN)
		    tmp = NO_DOOR_SYM;
		else if (room->doormask & (D_CLOSED|D_LOCKED))
		    tmp = CLOSED_DOOR_SYM;
		/* We know door is open. */
		else {
		    croom=inroom(x,y);
		    if(croom == -1) {
#ifdef STRONGHOLD
			if(IS_WALL(levl[x-1][y].typ)||IS_WALL(levl[x+1][y].typ))
			    tmp = H_OPEN_DOOR_SYM;
			else
			    tmp = V_OPEN_DOOR_SYM;
#else
			impossible("door %d %d not in room",x,y);
#endif
		    } else if(rooms[croom].ly<=y && y<=rooms[croom].hy)
			tmp = V_OPEN_DOOR_SYM;
		    else
			tmp = H_OPEN_DOOR_SYM;
		}
		break;
	case CORR:
		tmp = CORR_SYM;
		break;
#ifdef STRONGHOLD
	case DRAWBRIDGE_UP:
		if((room->drawbridgemask & DB_UNDER) == DB_MOAT) tmp = POOL_SYM;
		else tmp = ROOM_SYM;
		break;
	case DRAWBRIDGE_DOWN:
#endif /* STRONGHOLD /**/
	case ROOM:
		if(room->lit || cansee(x,y) || Blind) tmp = ROOM_SYM;
		else tmp = STONE_SYM;
		break;
#ifdef POLYSELF
	case STONE:
		tmp = STONE_SYM;
		break;
#endif
#ifdef FOUNTAINS
	case FOUNTAIN:
		tmp = FOUNTAIN_SYM;
		break;
#endif
#ifdef THRONES
	case THRONE:
		tmp = THRONE_SYM;
		break;
#endif
#ifdef SINKS
	case SINK:
		tmp = SINK_SYM;
		break;
#endif
#ifdef ALTARS
	case ALTAR:
		tmp = ALTAR_SYM;
		break;
#endif
	case CROSSWALL:
		tmp = CRWALL_SYM;
		break;
	case TUWALL:
		tmp = TUWALL_SYM;
		break;
	case TDWALL:
		tmp = TDWALL_SYM;
		break;
	case TLWALL:
		tmp = TLWALL_SYM;
		break;
	case TRWALL:
		tmp = TRWALL_SYM;
		break;
/*
	case POOL:
		tmp = POOL_SYM;
		break;
*/
	default:
		tmp = ERRCHAR;
	}
	return(tmp);
}

void
newsym(x,y)
register int x, y;
{
	atl(x,y,(char)news0(x,y));
}

#endif /* OVL0 */
#ifdef OVLB

/* used with wand of digging (or pick-axe): fill scrsym and force display */
/* also when a POOL evaporates */
void
mnewsym(x, y)
register int x, y;
{
	register struct rm *room;
	uchar newscrsym;	/* OIS */

	if(!vism_at(x,y)) {
		room = &levl[x][y];
		newscrsym = news0(x,y);
		if(room->scrsym != newscrsym) {
			room->scrsym = newscrsym;
			room->seen = 0;
		}
	}
}

#endif /* OVLB */
#ifdef OVL1

void
nosee(x,y)
register int x, y;
{
	register struct rm *room;

	if(!isok(x,y)) return;
	room = &levl[x][y];
	if(levl[x][y].scrsym == ROOM_SYM
	   && !room->lit && !Blind) {
		room->scrsym = STONE_SYM;	/* was ' ' -- OIS */
		room->new = 1;
		on_scr(x,y);
	}
}

void
prl1(x,y)
register int x, y;
{
	if(u.dx) {
		if(u.dy) {
			prl(x-(2*u.dx),y);
			prl(x-u.dx,y);
			prl(x,y);
			prl(x,y-u.dy);
			prl(x,y-(2*u.dy));
		} else {
			prl(x,y-1);
			prl(x,y);
			prl(x,y+1);
		}
	} else {
		prl(x-1,y);
		prl(x,y);
		prl(x+1,y);
	}
}

void
nose1(x,y)
register int x, y;
{
	if(u.dx) {
		if(u.dy) {
			nosee(x,u.uy);
			nosee(x,u.uy-u.dy);
			nosee(x,y);
			nosee(u.ux-u.dx,y);
			nosee(u.ux,y);
		} else {
			nosee(x,y-1);
			nosee(x,y);
			nosee(x,y+1);
		}
	} else {
		nosee(x-1,y);
		nosee(x,y);
		nosee(x+1,y);
	}
}

int
vism_at(x,y)
register int x, y;
{
	if(x == u.ux && y == u.uy && !Invisible) return(1);

	if(MON_AT(x, y))
		return(showmon(m_at(x,y)));
	return(0);
}

#endif /* OVL1 */
#ifdef OVLB

#ifdef NEWSCR
void
pobj(obj)
register struct obj *obj;
{
	register int show = (!obj->oinvis || See_invisible) &&
		cansee(obj->ox,obj->oy);
	if(obj->odispl){
		if(obj->odx != obj->ox || obj->ody != obj->oy || !show)
		if(!vism_at(obj->odx,obj->ody)){
			newsym(obj->odx, obj->ody);
			obj->odispl = 0;
		}
	}
	if(show && !vism_at(obj->ox,obj->oy)){
		atl(obj->ox,obj->oy,obj->olet);
		obj->odispl = 1;
		obj->odx = obj->ox;
		obj->ody = obj->oy;
	}
}
#endif /* NEWSCR /**/

void
unpobj(obj)
register struct obj *obj;
{
/* 	if(obj->odispl){
		if(!vism_at(obj->odx, obj->ody))
			newsym(obj->odx, obj->ody);
		obj->odispl = 0;
	}
*/
	if(!vism_at(obj->ox,obj->oy))
		newsym(obj->ox,obj->oy);
}

#ifdef WORM
STATIC_OVL void
pwseg(wtmp)
register struct wseg *wtmp;
{
	if(!wtmp->wdispl){
		atl(wtmp->wx, wtmp->wy, S_WORM_TAIL);
		wtmp->wdispl = 1;
	}
}
#endif


#ifdef STUPID_CPP	/* otherwise these functions are macros in rm.h */
boolean IS_WALL(typ)
unsigned typ;
{
	return(typ && typ <= TRWALL);
}

boolean IS_STWALL(typ)
unsigned typ;
{
	return(typ <= TRWALL);			/* STONE <= (typ) <= TRWALL */
}

boolean IS_ROCK(typ)
unsigned typ;
{
	return(typ < POOL);			/* absolutely nonaccessible */
}

boolean IS_DOOR(typ)
unsigned typ;
{
	return(typ == DOOR);
}

boolean ACCESSIBLE(typ)
unsigned typ;
{
	return(typ >= DOOR);			/* good position */
}

boolean IS_ROOM(typ)
unsigned typ;
{
	return(typ >= ROOM);			/* ROOM, STAIRS, furniture.. */
}

boolean ZAP_POS(typ)
unsigned typ;
{
	return(typ >= POOL);
}

boolean SPACE_POS(typ)
unsigned typ;
{
	return(typ > DOOR);
}

boolean IS_POOL(typ)
unsigned typ;
{
	return(typ >= POOL && typ <= DRAWBRIDGE_UP);
}

boolean IS_THRONE(typ)
unsigned typ;
{
	return(typ == THRONE);
}

boolean IS_FOUNTAIN(typ)
unsigned typ;
{
	return(typ == FOUNTAIN);
}

boolean IS_SINK(typ)
unsigned typ;
{
	return(typ == SINK);
}

boolean IS_ALTAR(typ)
unsigned typ;
{
	return(typ == ALTAR);
}

boolean IS_DRAWBRIDGE(typ)
unsigned typ;
{
	return(typ == DRAWBRIDGE_UP || typ == DRAWBRIDGE_DOWN);
}

boolean IS_FURNITURE(typ)
unsigned typ;
{
	return(typ >= STAIRS && typ <= ALTAR);
}
#endif /* STUPID_CPP */

#endif /* OVLB */
