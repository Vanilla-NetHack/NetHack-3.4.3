/*	SCCS Id: @(#)zap.c	1.4	87/08/08
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* zap.c - version 1.0.3 */

#include "hack.h"

extern struct obj *mkobj_at();
extern struct monst *makemon(), *mkmon_at(), youmonst;
struct monst *bhit();
char *exclam();
#ifdef KAA
extern char *xname();
#endif

char *fl[]= {
	"magic missile",
	"bolt of fire",
	"sleep ray",
	"bolt of cold",
	"death ray",
	"magic missle",		/* Spell equivalents of above wands */
	"fireball",
	"sleep ray",
	"cone of cold",
	"finger of death"
};

/* Routines for IMMEDIATE wands and spells. */
/* bhitm: monster mtmp was hit by the effect of wand or spell otmp */
bhitm(mtmp, otmp)
register struct monst *mtmp;
register struct obj *otmp;
{
	wakeup(mtmp);
	switch(otmp->otyp) {
	case WAN_STRIKING:
#ifdef SPELLS
	case SPE_FORCE_BOLT:
#endif
		if(u.uswallow || rnd(20) < 10+mtmp->data->ac) {
			register int tmp = d(2,12);
			hit((otmp->otyp == WAN_STRIKING) ? "wand" : "spell", mtmp, exclam(tmp));
			resist(mtmp, otmp->olet, tmp, TELL);
		} else miss((otmp->otyp == WAN_STRIKING) ? "wand" : "spell", mtmp);
		break;
	case WAN_SLOW_MONSTER:
#ifdef SPELLS
	case SPE_SLOW_MONSTER:
#endif
		if(! resist(mtmp, otmp->olet, 0, NOTELL))
			mtmp->mspeed = MSLOW;
		break;
	case WAN_SPEED_MONSTER:
		if (!resist(mtmp, otmp->olet, 0, NOTELL))
			mtmp->mspeed = MFAST;
		break;
	case WAN_UNDEAD_TURNING:
#ifdef SPELLS
	case SPE_TURN_UNDEAD:
#endif
		if(index(UNDEAD,mtmp->data->mlet)) {

			if(!resist(mtmp, otmp->olet, rnd(8), NOTELL))
				mtmp->mflee = 1;
		}
		break;
	case WAN_POLYMORPH:
#ifdef SPELLS
	case SPE_POLYMORPH:
#endif
		if(!resist(mtmp, otmp->olet, 0, NOTELL))
		    if( newcham(mtmp,&mons[rn2(CMNUM)]) )
			if (!Hallucination)
			    objects[otmp->otyp].oc_name_known = 1;
		break;
	case WAN_CANCELLATION:
#ifdef SPELLS
	case SPE_CANCELLATION:
#endif
		if(!resist(mtmp, otmp->olet, 0, NOTELL))
			mtmp->mcan = 1;
		break;
	case WAN_TELEPORTATION:
#ifdef SPELLS
	case SPE_TELEPORT_AWAY:
#endif
		rloc(mtmp);
		break;
	case WAN_MAKE_INVISIBLE:
		mtmp->minvis = 1;
		break;
	case WAN_NOTHING:
		break;
	case WAN_PROBING:
#ifdef PROBING
		mstatusline(mtmp);
#else
		pline("Nothing Happens.");
#endif
		break;
	default:
		impossible("What an interesting effect (%u)", otmp->otyp);
	}
}

bhito(obj, otmp)	/* object obj was hit by the effect of wand otmp */
register struct obj *obj, *otmp;	/* returns TRUE if sth was done */
{
	register int res = TRUE;
#ifdef DGKMOD
	struct obj *otmp2;
#endif

	if(obj == uball || obj == uchain)
		res = FALSE;
	else
	switch(otmp->otyp) {
	case WAN_POLYMORPH:
#ifdef SPELLS
	case SPE_POLYMORPH:
#endif
		/* preserve symbol and quantity, but turn rocks into gems */
#ifdef DGKMOD
		otmp2 = mkobj_at((obj->otyp == ROCK
			|| obj->otyp == ENORMOUS_ROCK) ? GEM_SYM : obj->olet,
			obj->ox, obj->oy);
		otmp2->quan = obj->quan;
		/* keep special fields (including charges on wands) */
		/* The DGK modification doesn't allow polymorphing a weapon
		   with enchantments into another one, and doesn't allow 
		   polymorphed rings to have plusses.  KAA*/
		if (index("/)[", otmp2->olet)) otmp2->spe = obj->spe;
	        /* Amulets gets cheap   stewr 870807 */
		if (obj->otyp == AMULET_OF_YENDOR) otmp2->spe = obj->spe;
		/* Wands of wishing max 3 stewr 870808 */
		if ((otmp2->otyp == WAN_WISHING) 
		    && (obj->spe > 3)) otmp2->spe = 3;
		otmp2->cursed = otmp->cursed;
		/* update the weight */
		otmp2->owt = weight(otmp2);
#else
		mkobj_at((obj->otyp == ROCK || obj->otyp == ENORMOUS_ROCK)
			? GEM_SYM : obj->olet,
			obj->ox, obj->oy) -> quan = obj->quan;
#endif
		delobj(obj);
		break;
	case WAN_STRIKING:
#ifdef SPELLS
	case SPE_FORCE_BOLT:
#endif
		if(obj->otyp == ENORMOUS_ROCK)
			fracture_rock(obj);
		else
			res = FALSE;
		break;
	case WAN_CANCELLATION:
#ifdef SPELLS
	case SPE_CANCELLATION:
#endif
		if(obj->spe && obj->olet != AMULET_SYM) {
			obj->known = 0;
			obj->spe = 0;
		}
		break;
	case WAN_TELEPORTATION:
#ifdef SPELLS
	case SPE_TELEPORT_AWAY:
#endif
		rloco(obj);
		break;
	case WAN_MAKE_INVISIBLE:
		obj->oinvis = 1;
		break;
	case WAN_UNDEAD_TURNING:
#ifdef SPELLS
	case SPE_TURN_UNDEAD:
#endif
		res = revive(obj);
		break;
	case WAN_SLOW_MONSTER:		/* no effect on objects */
#ifdef SPELLS
	case SPE_SLOW_MONSTER:
#endif
	case WAN_SPEED_MONSTER:
	case WAN_NOTHING:
	case WAN_PROBING:
		res = FALSE;
		break;
	default:
		impossible("What an interesting effect (%u)", otmp->otyp);
	}
	return(res);
}

/*
 * zappable - returns 1 if zap is available, 0 otherwise.
 *	      it removes a charge from the wand if zappable.
 * added by GAN 11/03/86
 */
int
zappable(wand)
register struct obj *wand;
{
	if(wand->spe < 0 || (wand->spe ==0 && rn2(121)))
		return(0);
	else  {
		if(wand->spe == 0)
			pline("You wrest one more spell from the worn-out wand.");
		wand->spe--;
		return(1);
	}
}

/*
 * zapnodir - zaps an NODIR wand.
 * added by GAN 11/03/86
 */
zapnodir(wand)
register struct obj *wand;
{
	switch(wand->otyp){
		case WAN_LIGHT:
			litroom(TRUE);
			break;
		case WAN_SECRET_DOOR_DETECTION:
			if(!findit()) return(1);
			break;
		case WAN_CREATE_MONSTER:
			{ register int cnt = 1;
			if(!rn2(23)) cnt += rn2(7) + 1;
			while(cnt--)
			    (void) makemon((struct permonst *) 0, u.ux, u.uy);
			}
			break;
		case WAN_WISHING:
			  
			if(u.uluck + rn2(5) < 0) {
				pline("Unfortunately, nothing happens.");
				break;
			}
			makewish();
			break;
	}
	if(!objects[wand->otyp].oc_name_known) {
			objects[wand->otyp].oc_name_known = 1;
			more_experienced(0,10);
	}
}

dozap()
{
	register struct obj *obj;
	int	damage;

	obj = getobj("/", "zap");
	if(!obj) return(0);
	
	/* zappable addition done by GAN 11/03/86 */
	if(!zappable(obj))  {
		pline("Nothing Happens.");
		return(1);
	}
	if(!(objects[obj->otyp].bits & NODIR) && !getdir(1)) {
		pline("The %s glows and fades.",xname(obj));
		return(1);	/* make him pay for knowing !NODIR */
	}
#ifdef KAA
     if(!u.dx && !u.dy && !u.dz && !(objects[obj->otyp].bits & NODIR)) {

		if((damage = zapyourself(obj)))
			losehp(damage,"self-inflicted injury");
		return(1);
     }
#endif
	weffects(obj);
	return(1);
}

#ifdef KAA
#define	makeknown(x)	objects[x].oc_name_known = 1

zapyourself(obj)
	register struct obj	*obj;
{
struct obj	*otmp;
int	damage = 0;

	switch(obj->otyp) {
		case WAN_STRIKING:
#ifdef SPELLS
		case SPE_FORCE_BOLT:
#endif
		    pline("You magically bash yourself!");
		    damage=d(8,6);
		    break;
		case WAN_FIRE:
		    makeknown(WAN_FIRE);
#ifdef SPELLS
		case SPE_FIREBALL:
#endif
		    pline("You've set light to yourself!");
		    if (!Fire_resistance) damage=d(12,6);
		    burn_scrolls();
		    boil_potions();
		    break;
		case WAN_COLD:
		    makeknown(WAN_COLD);
#ifdef SPELLS
		case SPE_CONE_OF_COLD:
#endif
		    pline("You imitate a popsicle!");
		    if (!Cold_resistance) damage=d(12,6);
		    break;
		case WAN_MAGIC_MISSILE:
		    makeknown(WAN_MAGIC_MISSILE);
#ifdef SPELLS
		case SPE_MAGIC_MISSILE:
#endif
		    damage = d(4,6);
		    pline("Idiot!  You've shot yourself!"); 
		    break;
		case WAN_POLYMORPH:
		    makeknown(WAN_POLYMORPH);
#ifdef SPELLS
		case SPE_POLYMORPH:
#endif
		    polyself();
		    break;
		case WAN_CANCELLATION:
#ifdef SPELLS
		case SPE_CANCELLATION:
#endif
		    for(otmp = invent; otmp; otmp = otmp->nobj)
		       if(otmp != uball && otmp->otyp != AMULET_OF_YENDOR)
			      otmp->spe = 0;
		    if(u.mtimedone) rehumanize();
		    flags.botl = 1;  /* because of potential AC change */
		    find_ac();
		    break;
	       case WAN_MAKE_INVISIBLE:
		    HInvis |= INTRINSIC;
		    /* Tough luck if you cannot see invisible! */
		    if (!See_invisible) newsym(u.ux, u.uy);
		    break;
	       case WAN_SPEED_MONSTER:
		    Fast |= INTRINSIC;
		    break;
	       case WAN_SLEEP:
		    makeknown(WAN_SLEEP);
#ifdef SPELLS
		case SPE_SLEEP:
#endif
		    pline("The sleep ray hits you!");
		    nomul(-rn2(50));
		    break;
		case WAN_SLOW_MONSTER:
#ifdef SPELLS
		case SPE_SLOW_MONSTER:
#endif
		    Fast = 0;
		    break;
		case WAN_TELEPORTATION:
#ifdef SPELLS
		case SPE_TELEPORT_AWAY:
#endif
		    tele();
		    break;
		case WAN_DEATH:
#ifdef SPELLS
		case SPE_FINGER_OF_DEATH:
#endif
		    killer = "death ray";
		    pline("You irradiate yourself with pure energy!");
		    pline("You die.");
		    done("died");
		    break;
#ifdef SPELLS
		case SPE_LIGHT:
		    pline("You've blinded yourself!");
		    Blind += rnd(100);
		    break;		
		case SPE_DIG:
		case SPE_TURN_UNDEAD:
		case SPE_DETECT_UNSEEN:
#endif
		case WAN_DIGGING:
		case WAN_UNDEAD_TURNING:
		case WAN_NOTHING:
		    break;
		default: impossible("object %d used?",obj->otyp);
	}
	return(damage);
}
#endif /* KAA /**/

/* called for various wand and spell effects - M. Stephenson */
weffects(obj)
	register struct	obj	*obj;
{
	xchar zx,zy;

	if(objects[obj->otyp].bits & IMMEDIATE) {
		if(u.uswallow)
			bhitm(u.ustuck, obj);
		else if(u.dz) {
			if(u.dz > 0 && o_at(u.ux,u.uy)) {
				register struct obj *otmp;
				
				/* changed by GAN to hit all objects there */
				for(otmp = fobj; otmp ; otmp = otmp->nobj)
					if(otmp->ox == u.ux &&
					   otmp->oy == u.uy)
						(void) bhito(otmp, obj);
			}
		} else
			(void) bhit(u.dx,u.dy,rn1(8,6),0,bhitm,bhito,obj);
	} else {
	    switch(obj->otyp){
		case WAN_LIGHT:
#ifdef SPELLS
		case SPE_LIGHT:
#endif
			litroom(TRUE);
			break;
		case WAN_SECRET_DOOR_DETECTION:
#ifdef SPELLS
		case SPE_DETECT_UNSEEN:
#endif
			if(!findit()) return(1);
			break;
		case WAN_CREATE_MONSTER:
			{ register int cnt = 1;
			if(!rn2(23)) cnt += rn2(7) + 1;
			while(cnt--)
			    (void) makemon((struct permonst *) 0, u.ux, u.uy);
			}
			break;
		case WAN_WISHING:
			if(u.uluck + rn2(5) < 0) {
			    pline("Unfortunately, nothing happens.");
			    break;
			}
			makewish();
			break;
		case WAN_DIGGING:
#ifdef SPELLS
		case SPE_DIG:
#endif
			/* Original effect (approximately):
			 * from CORR: dig until we pierce a wall
			 * from ROOM: piece wall and dig until we reach
			 * an ACCESSIBLE place.
			 * Currently: dig for digdepth positions;
			 * also down on request of Lennart Augustsson.
			 */
			{ register struct rm *room;
			  register int digdepth;
			if(u.uswallow) {
				register struct monst *mtmp = u.ustuck;

				pline("You pierce %s's stomach wall!",
					monnam(mtmp));
				mtmp->mhp = 1;	/* almost dead */
				unstuck(mtmp);
				mnexto(mtmp);
				break;
			}
			if(u.dz) {
			    if(u.dz < 0) {
				pline("You loosen a rock from the ceiling.");
				pline("It falls on your head!");
				losehp(1, "falling rock");
				mksobj_at(ROCK, u.ux, u.uy);
				fobj->quan = 1;
				stackobj(fobj);
				if(Invisible) newsym(u.ux, u.uy);
			    } else {
				dighole();
			    }
			    break;
			}
			zx = u.ux+u.dx;
			zy = u.uy+u.dy;
			digdepth = 8 + rn2(18);
			Tmp_at(-1, '*');	/* open call */
			while(--digdepth >= 0) {
				if(!isok(zx,zy)) break;
				room = &levl[zx][zy];
				Tmp_at(zx,zy);
				if(!xdnstair){
					if(zx < 3 || zx > COLNO-3 ||
					    zy < 3 || zy > ROWNO-3)
						break;
					if(room->typ == HWALL ||
					    room->typ == VWALL){
						room->typ = ROOM;
						break;
					}
				} else
				if(room->typ == HWALL || room->typ == VWALL ||
				   room->typ == SDOOR || room->typ == LDOOR){
					room->typ = DOOR;
					digdepth -= 2;
				} else
				if(room->typ == SCORR || !room->typ) {
					room->typ = CORR;
					digdepth--;
				}
				mnewsym(zx,zy);
				zx += u.dx;
				zy += u.dy;
			}
			mnewsym(zx,zy);	/* not always necessary */
			Tmp_at(-1,-1);	/* closing call */
			break;
			}
		default:
#ifdef SPELLS
			if((int) obj->otyp >= SPE_MAGIC_MISSILE) {

				buzz((int) obj->otyp - SPE_MAGIC_MISSILE + 5,
					u.ux, u.uy, u.dx, u.dy);
			} else
#endif

				buzz((int) obj->otyp - WAN_MAGIC_MISSILE,
					u.ux, u.uy, u.dx, u.dy);
			break;
		}
		if(!objects[obj->otyp].oc_name_known) {
			objects[obj->otyp].oc_name_known = 1;
			more_experienced(0,10);
		}
	}
	return;
}

char *
exclam(force)
register int force;
{
	/* force == 0 occurs e.g. with sleep ray */
	/* note that large force is usual with wands so that !! would
		require information about hand/weapon/wand */
	return( (force < 0) ? "?" : (force <= 4) ? "." : "!" );
}

hit(str,mtmp,force)
register char *str;
register struct monst *mtmp;
register char *force;		/* usually either "." or "!" */
{
	if(!cansee(mtmp->mx,mtmp->my)) pline("The %s hits it.", str);
	else pline("The %s hits %s%s", str, monnam(mtmp), force);
}

miss(str,mtmp)
register char *str;
register struct monst *mtmp;
{
	if(!cansee(mtmp->mx,mtmp->my)) pline("The %s misses it.",str);
	else pline("The %s misses %s.",str,monnam(mtmp));
}

/* bhit: called when a weapon is thrown (sym = obj->olet) or when an
   IMMEDIATE wand is zapped (sym = 0); the weapon falls down at end of
   range or when a monster is hit; the monster is returned, and bhitpos
   is set to the final position of the weapon thrown; the ray of a wand
   may affect several objects and monsters on its path - for each of
   these an argument function is called. */
/* check !u.uswallow before calling bhit() */

struct monst *
bhit(ddx,ddy,range,sym,fhitm,fhito,obj)
register int ddx,ddy,range;		/* direction and range */
char sym;				/* symbol displayed on path */
int (*fhitm)(), (*fhito)();		/* fns called when mon/obj hit */
struct obj *obj;			/* 2nd arg to fhitm/fhito */
{
	register struct monst *mtmp;
	register struct obj *otmp;
	register int typ;

	bhitpos.x = u.ux;
	bhitpos.y = u.uy;

	if(sym) tmp_at(-1, sym);	/* open call */
	while(range-- > 0) {
		bhitpos.x += ddx;
		bhitpos.y += ddy;
		typ = levl[bhitpos.x][bhitpos.y].typ;
		if(mtmp = m_at(bhitpos.x,bhitpos.y)){
			if(sym) {
				tmp_at(-1, -1);	/* close call */
				return(mtmp);
			}
			(*fhitm)(mtmp, obj);
			range -= 3;
		}
		/* modified by GAN to hit all objects */
		if(fhito && o_at(bhitpos.x,bhitpos.y)){
			int hitanything = 0;
			for(otmp = fobj; otmp; otmp = otmp->nobj)
				if(otmp->ox == bhitpos.x &&
				   otmp->oy == bhitpos.y)
					hitanything += (*fhito)(otmp, obj);
			if(hitanything)	range--;
		}
		if(!ZAP_POS(typ)) {
			bhitpos.x -= ddx;
			bhitpos.y -= ddy;
			break;
		}
		if(sym) tmp_at(bhitpos.x, bhitpos.y);
	}

	/* leave last symbol unless in a pool */
	if(sym)
	   tmp_at(-1, (levl[bhitpos.x][bhitpos.y].typ == POOL) ? -1 : 0);
	return(0);
}

struct monst *
boomhit(dx,dy) {
	register int i, ct;
	register struct monst *mtmp;
	char sym = ')';
	extern schar xdir[], ydir[];

	bhitpos.x = u.ux;
	bhitpos.y = u.uy;

	for(i=0; i<8; i++) if(xdir[i] == dx && ydir[i] == dy) break;
	tmp_at(-1, sym);	/* open call */
	for(ct=0; ct<10; ct++) {
		if(i == 8) i = 0;
		sym = ')' + '(' - sym;
		tmp_at(-2, sym);	/* change let call */
		dx = xdir[i];
		dy = ydir[i];
		bhitpos.x += dx;
		bhitpos.y += dy;
		if(mtmp = m_at(bhitpos.x, bhitpos.y)){
			tmp_at(-1,-1);
			return(mtmp);
		}
		if(!ZAP_POS(levl[bhitpos.x][bhitpos.y].typ)) {
			bhitpos.x -= dx;
			bhitpos.y -= dy;
			break;
		}
		if(bhitpos.x == u.ux && bhitpos.y == u.uy) { /* ct == 9 */
			if(rn2(20) >= 10+u.ulevel){	/* we hit ourselves */
				(void) thitu(10, rnd(10), "boomerang");
				break;
			} else {	/* we catch it */
				tmp_at(-1,-1);
				pline("Skillfully, you catch the boomerang.");
				return(&youmonst);
			}
		}
		tmp_at(bhitpos.x, bhitpos.y);
		if(ct % 5 != 0) i++;
	}
	tmp_at(-1, -1);	/* do not leave last symbol */
	return(0);
}

char
dirlet(dx,dy) register dx,dy; {
	return
		(dx == dy) ? '\\' : (dx && dy) ? '/' : dx ? HWALL_SYM : VWALL_SYM;
}

/* type == -1: monster spitting fire at you */
/* type == -1,-2,-3: bolts sent out by wizard */
/* called with dx = dy = 0 with vertical bolts */
buzz(type,sx,sy,dx,dy)
register int type;
register xchar sx,sy;
register int dx,dy;
{
	int abstype = (type == 10) ? 1 : abs(type);
	register char *fltxt = (type == -1 || type == 10) ? "blaze of fire" : fl[abstype];
	struct rm *lev;
	xchar range;
	struct monst *mon;

	if(u.uswallow) {
		register int tmp;

		if(type < 0) return;
		tmp = zhit(u.ustuck, type);
		pline("The %s rips into %s%s",
			fltxt, monnam(u.ustuck), exclam(tmp));
		return;
	}
	if(type < 0) pru();
	range = rn1(7,7);
	Tmp_at(-1, dirlet(dx,dy));	/* open call */
	while(range-- > 0) {
		sx += dx;
		sy += dy;
		if((lev = &levl[sx][sy])->typ) Tmp_at(sx,sy);
		else {
			int bounce = 0;
			if(cansee(sx-dx,sy-dy))
				pline("The %s bounces!", fltxt);
			if(ZAP_POS(levl[sx][sy-dy].typ))
				bounce = 1;
			if(ZAP_POS(levl[sx-dx][sy].typ)) {
				if(!bounce || rn2(2)) bounce = 2;
			}
			switch(bounce){
			case 0:
				dx = -dx;
				dy = -dy;
				continue;
			case 1:
				dy = -dy;
				sx -= dx;
				break;
			case 2:
				dx = -dx;
				sy -= dy;
				break;
			}
			Tmp_at(-2,dirlet(dx,dy));
			continue;
		}
		if(lev->typ == POOL && abstype == 1 /* fire */) {
			range -= 3;
			lev->typ = ROOM;
			if(cansee(sx,sy)) {
				mnewsym(sx,sy);
				pline("The water evaporates.");
			} else
				pline("You hear a hissing sound.");
		}
		if(o_at(sx,sy) && abstype == 1)
			if(burn_floor_scrolls(sx,sy) && cansee(sx,sy))  {
				mnewsym(sx,sy);
				pline("You see a puff of smoke.");
			}
		if((mon = m_at(sx,sy)) &&
		   (type != -1 || mon->data->mlet != 'D')) {
			wakeup(mon);
			if(rnd(20) < 18 + mon->data->ac) {
				register int tmp = zhit(mon,abstype);
				if(mon->mhp < 1) {
					if(type < 0) {
					    if(cansee(mon->mx,mon->my))
					      pline("%s is killed by the %s!",
						Monnam(mon), fltxt);
					    mondied(mon);
					} else
					    killed(mon);
				} else
					hit(fltxt, mon, exclam(tmp));
				range -= 2;
			} else
				miss(fltxt,mon);
		} else if(sx == u.ux && sy == u.uy) {
			nomul(0);
			if(rnd(20) < 18+u.uac) {
				register int dam = 0;
				range -= 2;
				pline("The %s hits you!",fltxt);
				switch(abstype) {
				case 0:
				case 5:	dam = d(2,6);
					break;
				case 1:
				case 6:	if(Fire_resistance)
						pline("You don't feel hot!");
					else dam = d(6,6);
					if(!rn2(3)) {
						boil_potions();
						burn_scrolls();
					}
					break;
				case 2:
				case 7:	nomul(-rnd(25)); /* sleep ray */
					break;
				case 3:
				case 8:	if(Cold_resistance)
						pline("You don't feel cold!");
					else dam = d(6,6);
					break;
				case 4:
				case 9:	u.uhp = -1;
					break;
				}
				losehp(dam,fltxt);
			} else pline("The %s whizzes by you!",fltxt);
			stop_occupation();
		}
		if(!ZAP_POS(lev->typ)) {
			int bounce = 0, rmn;
			if(cansee(sx,sy)) pline("The %s bounces!",fltxt);
			range--;
			if(!dx || !dy || !rn2(20)){
				dx = -dx;
				dy = -dy;
			} else {
			  if(ZAP_POS(rmn = levl[sx][sy-dy].typ) &&
			    (IS_ROOM(rmn) || ZAP_POS(levl[sx+dx][sy-dy].typ)))
				bounce = 1;
			  if(ZAP_POS(rmn = levl[sx-dx][sy].typ) &&
			    (IS_ROOM(rmn) || ZAP_POS(levl[sx-dx][sy+dy].typ)))
				if(!bounce || rn2(2))
					bounce = 2;

			  switch(bounce){
			  case 0:
				dy = -dy;
				dx = -dx;
				break;
			  case 1:
				dy = -dy;
				break;
			  case 2:
				dx = -dx;
				break;
			  }
			  Tmp_at(-2, dirlet(dx,dy));
			}
		}
	}
	Tmp_at(-1,-1);
}

zhit(mon,type)			/* returns damage to mon */
register struct monst *mon;
register type;
{
	register int tmp = 0;

	switch(type) {
	case 0:			/* magic missile */
	case 5: tmp = d(2,6);
		break;
	case -1:		/* Dragon blazing fire */
	case 1:			/* fire wand*/
	case 6:			/* fire spell */
	case 10:		/* Polymorphed human blazing fire */
		if(index("Dg", mon->data->mlet)) break;
		tmp = d(6,6);
		if(index("YF", mon->data->mlet)) tmp += 7;
		break;
	case 2:			/* sleep*/
	case 7: tmp = 0;
		if(!resist(mon, (type == 2) ? '/' : '+', 0, NOTELL))
			mon->mfroz = 1;
		break;
	case 3:			/* cold */
	case 8:
		if(index("YFgf", mon->data->mlet)) break;
		tmp = d(6,6);
		if(mon->data->mlet == 'D') tmp += 7;
		break;
	case 4:			/* death*/
	case 9:
		if(index(UNDEAD, mon->data->mlet)) break;
		tmp = mon->mhp+1;
		break;
	}
	if (resist(mon, (type < 5) ? '/' : '+', 0, NOTELL)) tmp /= 2;
	mon->mhp -= tmp;
	return(tmp);
}

#define	CORPSE_I_TO_C(otyp)	(char) ((otyp >= DEAD_ACID_BLOB)\
		     ?  'a' + (otyp - DEAD_ACID_BLOB)\
		     :	'@' + (otyp - DEAD_HUMAN))
revive(obj)
register struct obj *obj;
{
	register struct monst *mtmp;
	register int let;

	if(obj->olet == FOOD_SYM && obj->otyp > CORPSE) {
#ifdef KAA
		switch (obj->otyp) {
			case DEAD_HUMAN: { let = 'Z'; break; }
			case DEAD_GIANT: { let = '9'; break; }
			case DEAD_DEMON: { let = '&'; break; }
			default: let = CORPSE_I_TO_C(obj->otyp);
		}
		delobj(obj);
/* Originally there was a bug which caused the object not to be erased
   from the screen.  This happened because first the monster got created,
   then the corpse removed.  Although delobj() called unpobj(), the object
   didn't get erased from the screen because the monster was sitting on top
   of it.  Solution: place the delobj() call before the mkmon() call. */
		mtmp = mkmon_at(let, obj->ox, obj->oy);
		if (mtmp && obj->otyp == DEAD_HUMAN) {
			mtmp->mhp = mtmp->mhpmax = 100;
			mtmp->mspeed = MFAST;
		}
#endif
		/* do not (yet) revive shopkeepers */
		/* Note: this might conceivably produce two monsters
			at the same position - strange, but harmless */
#ifndef KAA
		delobj(obj);
		mtmp = mkmon_at(CORPSE_I_TO_C(obj->otyp),obj->ox,obj->oy);
#endif
	}
	return(!!mtmp);		/* TRUE if some monster created */
}

rloco(obj)
register struct obj *obj;
{
	register tx,ty,otx,oty;

	otx = obj->ox;
	oty = obj->oy;
	do {
		tx = rn1(COLNO-3,2);
		ty = rn2(ROWNO);
	} while(!goodpos(tx,ty));
	obj->ox = tx;
	obj->oy = ty;
	if(cansee(otx,oty))
		newsym(otx,oty);
}

fracture_rock(obj)	/* fractured by pick-axe or wand of striking */
register struct obj *obj;			   /* no texts here! */
{
	/* unpobj(obj); */
	obj->otyp = ROCK;
	obj->quan = 7 + rn2(60);
	obj->owt = weight(obj);
	obj->olet = WEAPON_SYM;
	if(cansee(obj->ox,obj->oy))
		prl(obj->ox,obj->oy);
}

boil_potions()
{
	register struct obj *obj, *obj2;
	register int scrquan, i;
	
	for(obj = invent; obj; obj = obj2) {
		obj2 = obj->nobj;
		if(obj->olet == POTION_SYM) {
			scrquan = obj->quan;
			for(i = 1; i <= scrquan; i++) 
				if(!rn2(3)) {
					pline("%s %s boils and explodes!",
					(obj->quan != 1) ? "One of your" : "Your",
					xname(obj));
					potionbreathe(obj);
					useup(obj);
					losehp(rn2(4),"boiling potion");
				}
		}
	}
}
				
burn_scrolls()
{
	register struct obj *obj, *obj2;
	register int cnt = 0;
	register int scrquan, i;

	for(obj = invent; obj; obj = obj2) {
		obj2 = obj->nobj;
		if(obj->olet == SCROLL_SYM) {
			scrquan = obj->quan;
			for(i = 1; i <= scrquan ; i++)
				if(!rn2(3))  {
					cnt++;
					useup(obj);
				}
		}
	}

	/* "Killed by a burning scrolls" doesn't make too much sense.  KAA*/
	if (cnt) {
		pline("%s of your scrolls catch%s fire!",
		cnt==1 ? "One" : "Some", cnt==1 ? "es" : "");
		if(Fire_resistance)
			pline("You aren't hurt!");
		else
			losehp(cnt,"burning scroll");
	}
}

resist(mtmp, olet, damage, tell)
register struct monst	*mtmp;
register char	olet;
register int	damage, tell;
{
register int	resisted = 0;
#ifdef HARD
register int	level;

	switch(olet)  {

	    case '/':	level = 8;
			break;

	    case '?':	level = 6;
			break;

	    case '!':	level = 5;
			break;

	    default:	level = u.ulevel;
			break;
	}

	resisted = (rn2(100) - mtmp->data->mlevel + level) < mtmp->data->mr;
	if(resisted) {

		if(tell) pline("The %s resists!", mtmp->data->mname);
		mtmp->mhp -= damage/2;
	} else
#endif
		mtmp->mhp -= damage;

	if(mtmp->mhp < 1) killed(mtmp);
	return(resisted);
}

/*
 * burn scrolls on floor at position x,y
 * return the number of scrolls burned
 */
int
burn_floor_scrolls(x,y)
{
	register struct obj *obj, *obj2;
	register int scrquan, i;
	register int cnt = 0;

	for(obj = fobj; obj; obj = obj2) {
		obj2 = obj->nobj;
		/* Bug fix - KAA */
		if(obj->ox == x && obj->oy == y && obj->olet == SCROLL_SYM) {
			scrquan = obj->quan;
			for(i = 1; i <= scrquan ; i++)
				if(!rn2(3))  {
					cnt++;
					useupf(obj);
				}
		}
	}
	return(cnt);
}

makewish()	/* Separated as there are now 3 places you can wish at. */
{
	char buf[BUFSZ];
	register struct obj *otmp;
	extern struct obj *readobjnam(), *addinv();
	int wishquan, mergquan;

	pline("You may wish for an object. What do you want? ");
	getlin(buf);
	if(buf[0] == '\033') buf[0] = 0;
	otmp = readobjnam(buf);
#ifdef KAA
/* Wishing for gold has been implemented in readobjnam() and returns 0
   if successful. */
	if (otmp) { 
#endif
		wishquan = otmp->quan;
		otmp = addinv(otmp);
		/* indented lines added below so quantity shows
		 *  right.     GAN - 11/13/86
		 */
		  mergquan = otmp->quan;
		  otmp->quan = wishquan; /* to fool prinv() */
		prinv(otmp);
		  otmp->quan = mergquan;
#ifdef KAA
	}
#endif
}
