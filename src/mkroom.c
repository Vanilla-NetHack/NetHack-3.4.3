/*	SCCS Id: @(#)mkroom.c	3.0	88/11/24
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Entry points:
 *	mkroom() -- make and stock a room of a given type
 *	nexttodoor() -- return TRUE if adjacent to a door
 *	has_dnstairs() -- return TRUE if given room has a down staircase
 *	has_upstairs() -- return TRUE if given room has an up staircase
 *	dist2() -- Euclidean square-of-distance function
 *	courtmon() -- generate a court monster
 */

#define MONATTK_H	/* comment line for pre-compiled headers */
/* block some unused #defines to avoid overloading some cpp's */
#include "hack.h"

#ifdef OVLB
static boolean FDECL(isbig, (struct mkroom *));
static struct mkroom * FDECL(pick_room,(BOOLEAN_P));
static void NDECL(mkshop), FDECL(mkzoo,(int)), NDECL(mkswamp);
#ifdef ORACLE
static void NDECL(mkdelphi);
#endif
#if defined(ALTARS) && defined(THEOLOGY)
static void NDECL(mktemple);
#endif

static struct permonst * NDECL(morguemon);
#ifdef ARMY
static struct permonst * NDECL(squadmon);
#endif
#endif /* OVLB */

#define sq(x) ((x)*(x))

#ifdef OVLB

static boolean
isbig(sroom)
register struct mkroom *sroom;
{
	register int area = (sroom->hx - sroom->lx) * (sroom->hy - sroom->ly);
	return( area > 20 );
}

void
mkroom(roomtype)
/* make and stock a room of a given type */
int	roomtype;
{

    if (roomtype >= SHOPBASE)
	mkshop();	/* someday, we should be able to specify shop type */
    else switch(roomtype) {
#ifdef THRONES
	case COURT:	mkzoo(COURT); break;
#endif
	case ZOO:	mkzoo(ZOO); break;
	case BEEHIVE:	mkzoo(BEEHIVE); break;
	case MORGUE:	mkzoo(MORGUE); break;
	case BARRACKS:	mkzoo(BARRACKS); break;
	case SWAMP:	mkswamp(); break;
#ifdef ORACLE
	case DELPHI:	mkdelphi(); break;
#endif
#if defined(ALTARS) && defined(THEOLOGY)
	case TEMPLE:	mktemple(); break;
#endif
	default:	impossible("Tried to make a room of type %d.", roomtype);
    }
}

static void
mkshop()
{
	register struct mkroom *sroom;
	int i = -1;
#ifdef WIZARD
# ifdef __GNULINT__
	register char *ep = (char *)0;
# else
	register char *ep;
# endif

	/* first determine shoptype */
	if(wizard){
		ep = getenv("SHOPTYPE");
		if(ep){
			if(*ep == 'z' || *ep == 'Z'){
				mkzoo(ZOO);
				return;
			}
			if(*ep == 'm' || *ep == 'M'){
				mkzoo(MORGUE);
				return;
			}
			if(*ep == 'b' || *ep == 'B'){
				mkzoo(BEEHIVE);
				return;
			}
#ifdef THRONES
			if(*ep == 't' || *ep == 'T' || *ep == '\\'){
				mkzoo(COURT);
				return;
			}
#endif
#ifdef ARMY
			if(*ep == 's' || *ep == 'S'){
				mkzoo(BARRACKS);
				return;
			}
#endif /* ARMY */
#if defined(ALTARS) && defined(THEOLOGY)
			if(*ep == '_'){
				mktemple();
				return;
			}
#endif
			if(*ep == '}'){
				mkswamp();
				return;
			}
			for(i=0; shtypes[i].name; i++)
				if(*ep == shtypes[i].symb) goto gottype;
			if(*ep == 'g' || *ep == 'G')
				i = 0;
			else
				i = -1;
		}
	}
gottype:
#endif
	for(sroom = &rooms[0]; ; sroom++){
		if(sroom->hx < 0) return;
		if(sroom - rooms >= nroom) {
			pline("rooms not closed by -1?");
			return;
		}
		if(sroom->rtype != OROOM) continue;
		if(!sroom->rlit || has_dnstairs(sroom) || has_upstairs(sroom))
			continue;
		if(
#ifdef WIZARD
		   (wizard && ep && sroom->doorct != 0) ||
#endif
			sroom->doorct == 1) break;
	}

	if(i < 0) {			/* shoptype not yet determined */
	    register int j;

	    /* pick a shop type at random */
	    for(j = rn2(100), i = 0; j -= shtypes[i].prob; i++)
		if (j < 0)	break;

	    /* big rooms cannot be wand or book shops,
	     * - so make them general stores
	     */
	    if(isbig(sroom) && (shtypes[i].symb == WAND_SYM
#ifdef SPELLS
				|| shtypes[i].symb == SPBOOK_SYM
#endif
								)) i = 0;
	}
	sroom->rtype = SHOPBASE + i;

	/* stock the room with a shopkeeper and artifacts */
	stock_room(&(shtypes[i]), sroom);
}

static struct mkroom *
pick_room(strict)
register boolean strict;
/* pick an unused room, preferably with only one door */
{
	register struct mkroom *sroom;
	register int i = nroom;

	for(sroom = &rooms[rn2(nroom)]; i--; sroom++) {
		if(sroom == &rooms[nroom])
			sroom = &rooms[0];
		if(sroom->hx < 0)
			return (struct mkroom *)0;
		if(sroom->rtype != OROOM)	continue;
		if(!strict) {
		    if(has_upstairs(sroom) || (has_dnstairs(sroom) && rn2(3)))
			continue;
		} else if(has_upstairs(sroom) || has_dnstairs(sroom))
			continue;
		if(sroom->doorct == 1 || !rn2(5))
			return sroom;
	}
	return (struct mkroom *)0;
}

static void
mkzoo(type)
int type;
{
	register struct mkroom *sroom;
	struct monst *mon;
	register int sx,sy,i;
	int sh, tx, ty, goldlim = 500 * dlevel;

	if(!(sroom = pick_room(FALSE))) return;

	sroom->rtype = type;
	sh = sroom->fdoor;
	switch(type) {
#ifdef __GNULINT__
	    default:
		/* make sure tx and ty are initialized */
#endif
	    case COURT:
		tx = somex(sroom); ty = somey(sroom); break;
		/* TODO: try to ensure the enthroned monster is an M2_PRINCE */
	    case BEEHIVE:
		tx = sroom->lx + (sroom->hx - sroom->lx + 1)/2;
		ty = sroom->ly + (sroom->hy - sroom->ly + 1)/2;
		break;
	}
	for(sx = sroom->lx; sx <= sroom->hx; sx++)
	    for(sy = sroom->ly; sy <= sroom->hy; sy++){
		if((sx == sroom->lx && doors[sh].x == sx-1) ||
		   (sx == sroom->hx && doors[sh].x == sx+1) ||
		   (sy == sroom->ly && doors[sh].y == sy-1) ||
		   (sy == sroom->hy && doors[sh].y == sy+1)) continue;
		mon = makemon(
#ifdef THRONES
		    (type == COURT) ? courtmon() :
#endif
#ifdef ARMY
		    (type == BARRACKS) ? squadmon() :
#endif
		    (type == MORGUE) ? morguemon() :
		    (type == BEEHIVE) ?
			(sx == tx && sy == ty ? &mons[PM_QUEEN_BEE] : 
			 &mons[PM_KILLER_BEE]) :
		    (struct permonst *) 0,
		   sx, sy);
		if(mon) {
			mon->msleep = 1;
#ifdef THRONES
			if (type==COURT && mon->mpeaceful) {
				mon->mpeaceful = 0;
				mon->malign = max(3,abs(mon->data->maligntyp));
			}
#endif
		}
		switch(type) {
		    case ZOO:
			i = sq(dist2(sx,sy,doors[sh].x,doors[sh].y));
			if(i >= goldlim) i = 5*dlevel;
			goldlim -= i;
			mkgold((long)(10 + rn2(i)), sx, sy);
			break;
		    case MORGUE:
			if(!rn2(5))
			    (void) mk_tt_object(CORPSE, sx, sy);
			if(!rn2(10))	/* lots of treasure buried with dead */
			    (void) mksobj_at((rn2(3)) ? LARGE_BOX : CHEST, sx, sy);
			break;
		    case BEEHIVE:
			if(!rn2(3))
			    (void) mksobj_at(LUMP_OF_ROYAL_JELLY, sx, sy);
			break;
		    case BARRACKS:
			if(!rn2(20))	/* the payroll and some loot */
			    (void) mksobj_at((rn2(3)) ? LARGE_BOX : CHEST, sx, sy);
			break;
		}
	}
#ifdef THRONES
	if(type == COURT)  {
		levl[tx][ty].typ = THRONE;
		levl[tx][ty].scrsym = THRONE_SYM;

		sx = somex(sroom);
		sy = somey(sroom);
		mkgold((long) rn1(50 * dlevel,10), sx, sy);
		(void) mksobj_at(CHEST, sx, sy);    /* the royal coffers */
	}
#endif

}

static struct permonst *
morguemon()
{
	register int i = rn2(100), hd = rn2(dlevel);

	if(hd > 10 && i < 10)
		return((Inhell) ? mkclass(S_DEMON) : &mons[ndemon()]);
	if(hd > 8 && i > 85)
		return(mkclass(S_VAMPIRE));

	return((i < 20) ? &mons[PM_GHOST]
			: (i < 40) ? &mons[PM_WRAITH] : mkclass(S_ZOMBIE));
}

static void
mkswamp()	/* Michiel Huisjes & Fred de Wilde */
{
	register struct mkroom *sroom;
	register int sx,sy,i,eelct = 0;

	for(i=0; i<5; i++) {		/* 5 tries */
		sroom = &rooms[rn2(nroom)];
		if(sroom->hx < 0 || sroom->rtype != OROOM ||
		   has_upstairs(sroom) || has_dnstairs(sroom))
			continue;

		/* satisfied; make a swamp */
		sroom->rtype = SWAMP;
		for(sx = sroom->lx; sx <= sroom->hx; sx++)
		for(sy = sroom->ly; sy <= sroom->hy; sy++)
		if(!OBJ_AT(sx, sy) && levl[sx][sy].gmask == 0 &&
		   !MON_AT(sx, sy) && !t_at(sx,sy) && !nexttodoor(sx,sy)) {
		    if((sx+sy)%2) {
			levl[sx][sy].typ = POOL;
			levl[sx][sy].scrsym = POOL_SYM;
			if(!eelct || !rn2(4)) {
				(void) makemon(mkclass(S_EEL), sx, sy);
				eelct++;
			}
		    } else if(!rn2(4))	/* swamps tend to be moldy */
			(void) makemon(mkclass(S_FUNGUS), sx, sy);
		}
	}
}

#ifdef ORACLE
static void
mkdelphi()
{
	register struct mkroom *sroom;
	register struct monst *oracl;
	int dy,xx,yy;

	if(doorindex >= DOORMAX) return;
	if(!(sroom = pick_room(FALSE))) return;

	if(!place_oracle(sroom,&dy,&xx,&yy)) return;

	if(MON_AT(xx, yy))
	    rloc(m_at(xx, yy)); /* insurance */

	/* set up Oracle and environment */
	if(!(oracl = makemon(&mons[PM_ORACLE],xx,yy))) return;
	sroom->rtype = DELPHI;
	oracl->mpeaceful = 1;

	yy -= dy;
	if(accessible(xx-1, yy))
	    (void) mkcorpstat(STATUE, &mons[PM_FOREST_CENTAUR], xx-1, yy);
	if(accessible(xx, yy))
	    (void) mkcorpstat(STATUE, &mons[PM_MOUNTAIN_CENTAUR], xx, yy);
	if(accessible(xx+1,yy))
	    (void) mkcorpstat(STATUE, &mons[PM_PLAINS_CENTAUR], xx+1, yy);
# ifdef FOUNTAINS
	mkfount(0,sroom);
# endif
}
#endif

#if defined(ALTARS) && defined(THEOLOGY)
void
shrine_pos(sx,sy,troom)
int *sx,*sy;
struct mkroom *troom;
{
	*sx = troom->lx + ((troom->hx - troom->lx) / 2);
	*sy = troom->ly + ((troom->hy - troom->ly) / 2);
}

static void
mktemple()
{
	register struct mkroom *sroom;
	int sx,sy;

	if(!(sroom = pick_room(TRUE))) return;

	/* set up Priest and shrine */
	sroom->rtype = TEMPLE;
	shrine_pos(&sx,&sy,sroom);
	/*
	 * In temples, shrines are blessed altars
	 * located in the center of the room
	 */
	levl[sx][sy].typ = ALTAR;
	levl[sx][sy].scrsym = ALTAR_SYM;
	levl[sx][sy].altarmask = rn2((int)A_LAW+1);
	priestini(dlevel, sx, sy, (int) levl[sx][sy].altarmask);
 	levl[sx][sy].altarmask |= A_SHRINE;
}
#endif

boolean
nexttodoor(sx,sy)
register int sx, sy;
{
	register int dx, dy;
	register struct rm *lev;
	for(dx = -1; dx <= 1; dx++) for(dy = -1; dy <= 1; dy++) {
		if(!isok(sx+dx, sy+dy)) continue;
		if(IS_DOOR((lev = &levl[sx+dx][sy+dy])->typ) ||
		    lev->typ == SDOOR)
			return(TRUE);
	}
	return(FALSE);
}

boolean
has_dnstairs(sroom)
register struct mkroom *sroom;
{
	return(sroom->lx <= xdnstair && xdnstair <= sroom->hx &&
		   sroom->ly <= ydnstair && ydnstair <= sroom->hy);
}

boolean
has_upstairs(sroom)
register struct mkroom *sroom;
{
	return(sroom->lx <= xupstair && xupstair <= sroom->hx &&
		   sroom->ly <= yupstair && yupstair <= sroom->hy);
}

#endif /* OVLB */
#ifdef OVL0

int
dist2(x0,y0,x1,y1)
int x0, y0, x1, y1;
{
	register int dx = x0 - x1, dy = y0 - y1;
	return sq(dx) + sq(dy);
}

#endif /* OVL0 */
#ifdef OVLB

#ifdef THRONES
struct permonst *
courtmon()
{
	int     i = rn2(60) + rn2(3*dlevel);
	if (i > 100)		return(mkclass(S_DRAGON));
	else if (i > 95)	return(mkclass(S_GIANT));
	else if (i > 85)	return(mkclass(S_TROLL));
	else if (i > 75)	return(mkclass(S_CENTAUR));
	else if (i > 60)	return(mkclass(S_ORC));
	else if (i > 45)	return(&mons[PM_BUGBEAR]);
	else if (i > 30)	return(&mons[PM_HOBGOBLIN]);
	else if (i > 15)	return(mkclass(S_GNOME));
	else			return(mkclass(S_KOBOLD));
}
#endif /* THRONES /**/

#ifdef ARMY
#define	    NSTYPES	(PM_CAPTAIN-PM_SOLDIER+1)

struct {
    unsigned	pm;
    unsigned	prob;
}   squadprob[NSTYPES] = {
    PM_SOLDIER, 80, PM_SERGEANT, 15, PM_LIEUTENANT, 4, PM_CAPTAIN, 1
};

static struct permonst *
squadmon() {	    /* return soldier types. */

	register struct permonst *ptr;
	register int	i, cpro, sel_prob = rnd(80+dlevel);

	for(cpro = i = 0; i < NSTYPES; i++)
	    if((cpro += squadprob[i].prob) > sel_prob) {

		ptr = &mons[squadprob[i].pm];
		goto gotone;
	    }
	ptr = &mons[squadprob[rn2(NSTYPES)].pm];
gotone:
	if(!(ptr->geno & G_GENOD))  return(ptr);
	else			    return((struct permonst *) 0);
}
#endif /* ARMY /* */

#endif /* OVLB */
