/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"

extern struct monst *makemon();
struct monst *bhit();
char *exclam();

char *fl[]= {
	"magic missile",
	"bolt of fire",
	"sleep ray",
	"bolt of cold",
	"death ray"
};

dozap()
{
	register struct obj *obj;
	register struct monst *mtmp;
	xchar zx,zy;
	register num;

	obj = getobj("/", "zap");
	if(!obj) return(0);
	if(obj->spe < 0 || (obj->spe == 0 && rn2(121))) {
		pline("Nothing Happens");
		return(1);
	}
	if(obj->spe == 0)
		pline("You wrest one more spell from the worn-out wand.");
	if(!(objects[obj->otyp].bits & NODIR) && !getdir())
		return(1); /* make him pay for knowing !NODIR */
	obj->spe--;
	if(objects[obj->otyp].bits & IMMEDIATE) {
		if((u.uswallow && (mtmp = u.ustuck)) ||
		   (mtmp = bhit(u.dx,u.dy,rn1(8,6),0))) {
			wakeup(mtmp);
			switch(obj->otyp) {
			case WAN_STRIKING:
				if(rnd(20) < 10+mtmp->data->ac) {
					register int tmp = d(2,12);
					hit("wand", mtmp, exclam(tmp));
					mtmp->mhp -= tmp;
					if(mtmp->mhp < 1) killed(mtmp);
				} else miss("wand", mtmp);
				break;
			case WAN_SLOW_MONSTER:
				mtmp->mspeed = MSLOW;
				break;
			case WAN_SPEED_MONSTER:
				mtmp->mspeed = MFAST;
				break;
			case WAN_UNDEAD_TURNING:
				if(index("WVZ&",mtmp->data->mlet)) {
					mtmp->mhp -= rnd(8);
					if(mtmp->mhp<1) killed(mtmp);
					else mtmp->mflee = 1;
				}
				break;
			case WAN_POLYMORPH:
				if( newcham(mtmp,&mons[rn2(CMNUM)]) )
					objects[obj->otyp].oc_name_known = 1;
				break;
			case WAN_CANCELLATION:
				mtmp->mcan = 1;
				break;
			case WAN_TELEPORT_MONSTER:
				rloc(mtmp);
				break;
			case WAN_MAKE_INVISIBLE:
				mtmp->minvis = 1;
				break;
#ifdef WAN_PROBING
			case WAN_PROBING:
				mstatusline(mtmp);
				break;
#endif WAN_PROBING
			default:
				pline("What an interesting wand (%d)",
					obj->otyp);
				impossible();
			}
		}
	} else {
	switch(obj->otyp){
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
			{ char buf[BUFSZ];
			  register struct obj *otmp;
			  extern struct obj *readobjnam(), *addinv();
		      if(u.uluck + rn2(5) < 0) {
			pline("Unfortunately, nothing happens.");
			break;
		      }
		      pline("You may wish for an object. What do you want? ");
		      getlin(buf);
		      otmp = readobjnam(buf);
		      otmp = addinv(otmp);
		      prinv(otmp);
		      break;
			}
		case WAN_DIGGING:
			{ register struct rm *room;
			  register int digdepth;
			if(u.uswallow) {
				pline("You pierce %s's stomach wall!",
					monnam(u.ustuck));
				u.uswallow = 0;
				mnexto(u.ustuck);
				u.ustuck->mhp = 1;	/* almost dead */
				u.ustuck = 0;
				setsee();
				docrt();
				break;
			}
			zx = u.ux+u.dx;
			zy = u.uy+u.dy;
			if(!isok(zx,zy)) break;
			digdepth = 4 + rn2(10);
			if(levl[zx][zy].typ == CORR) num = CORR;
			else num = ROOM;
			Tmp_at(-1, '*');	/* open call */
			while(digdepth--) {
				if(zx == 0 || zx == COLNO-1 ||
					 zy == 0 || zy == ROWNO-1)
					break;
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
				} else if(num == ROOM || num == 10){
					if(room->typ != ROOM && room->typ) {
						if(room->typ != CORR)
							room->typ = DOOR;
						if(num == 10) break;
						num = 10;
					} else if(!room->typ)
						room->typ = CORR;
				} else {
					if(room->typ != CORR && room->typ) {
						room->typ = DOOR;
						break;
					} else room->typ = CORR;
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
			buzz((int) obj->otyp - WAN_MAGIC_MISSILE,
				u.ux, u.uy, u.dx, u.dy);
			break;
		}
		if(!objects[obj->otyp].oc_name_known) {
			u.urexp += 10;
			objects[obj->otyp].oc_name_known = 1;
		}
	}
 return(1);
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

/* sets bhitpos to the final position of the weapon thrown */
/* coord bhitpos; */

/* check !u.uswallow before calling bhit() */
struct monst *
bhit(ddx,ddy,range,sym)
register ddx,ddy,range;
char sym;
{
	register struct monst *mtmp;

	bhitpos.x = u.ux;
	bhitpos.y = u.uy;

	if(sym) tmp_at(-1, sym);	/* open call */
	while(range--) {
		bhitpos.x += ddx;
		bhitpos.y += ddy;
		if(mtmp = m_at(bhitpos.x,bhitpos.y)){
			if(sym) tmp_at(-1, -1);	/* close call */
			return(mtmp);
		}
		if(levl[bhitpos.x][bhitpos.y].typ<CORR) {
			bhitpos.x -= ddx;
			bhitpos.y -= ddy;
			break;
		}
 if(sym) tmp_at(bhitpos.x, bhitpos.y);
	}
	if(sym) tmp_at(-1, 0);	/* leave last symbol */
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
		if(levl[bhitpos.x][bhitpos.y].typ<CORR) {
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
				return((struct monst *) -1);
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
		(dx == dy) ? '\\' : (dx && dy) ? '/' : dx ? '-' : '|';
}

/* type < 0: monster spitting fire at you */
buzz(type,sx,sy,dx,dy)
register int type;
register xchar sx,sy;
register int dx,dy;
{
	register char *fltxt = (type < 0) ? "blaze of fire" : fl[type];
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
			if(cansee(sx-dx,sy-dy)) pline("The %s bounces!",fltxt);
			if(levl[sx][sy-dy].typ > DOOR) bounce = 1;
			if(levl[sx-dx][sy].typ > DOOR) {
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
		if((mon = m_at(sx,sy)) &&
		   (type >= 0 || mon->data->mlet != 'D')) {
			wakeup(mon);
			if(rnd(20) < 18 + mon->data->ac) {
				register int tmp = zhit(mon,type);
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
			if(rnd(20) < 18+u.uac) {
				register int dam = 0;
				range -= 2;
				pline("The %s hits you!",fltxt);
				switch(type) {
				case 0:
					dam = d(2,6);
					break;
				case -1:	/* dragon fire */
				case 1:
					if(Fire_resistance)
						pline("You don't feel hot!");
					else dam = d(6,6);
					break;
				case 2:
					nomul(-rnd(25)); /* sleep ray */
					break;
				case 3:
					if(Cold_resistance)
						pline("You don't feel cold!");
					else dam = d(6,6);
					break;
				case 4:
					u.uhp = -1;
				}
 losehp(dam,fltxt);
			} else pline("The %s whizzes by you!",fltxt);
		}
		if(lev->typ <= DOOR) {
			int bounce = 0, rmn;
			if(cansee(sx,sy)) pline("The %s bounces!",fltxt);
			range--;
			if(!dx || !dy || !rn2(20)){
				dx = -dx;
				dy = -dy;
			} else {
			  if((rmn = levl[sx][sy-dy].typ) > DOOR &&
			    (
			     rmn >= ROOM ||
				levl[sx+dx][sy-dy].typ > DOOR)){
				bounce = 1;
			  }
			  if((rmn = levl[sx-dx][sy].typ) > DOOR &&
			    (
			     rmn >= ROOM ||
				levl[sx-dx][sy+dy].typ > DOOR)){
				if(!bounce || rn2(2)){
					bounce = 2;
				}
			  }
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
		tmp = d(2,6);
		break;
	case -1:		/* Dragon blazing fire */
	case 1:			/* fire */
		if(index("Dg", mon->data->mlet)) break;
		tmp = d(6,6);
		if(mon->data->mlet == 'Y') tmp += 7;
		break;
	case 2:			/* sleep*/
		mon->mfroz = 1;
		break;
	case 3:			/* cold */
		if(index("YFgf", mon->data->mlet)) break;
		tmp = d(6,6);
		if(mon->data->mlet == 'D') tmp += 7;
		break;
	case 4:			/* death*/
		if(index("WVZ",mon->data->mlet)) break;
		tmp = mon->mhp+1;
		break;
	}
	mon->mhp -= tmp;
	return(tmp);
}
