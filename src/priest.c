/*	SCCS Id: @(#)priest.c	3.0	89/06/26
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* Copyright (c) Izchak Miller, Steve Linhart, 1989. */
/* NetHack may be freely redistributed.  See license for details. */

/* block some unused #defines to avoid overloading some cpp's */
#define MONATTK_H
#include "hack.h"
#include "mfndpos.h"
#include "eshk.h"
#include "epri.h"

/* used for the insides of shk_move and pri_move */
int
move_special(mtmp,monroom,appr,uondoor,avoid,omx,omy,gx,gy)
register struct monst *mtmp;
schar monroom,appr;
boolean uondoor,avoid;
register xchar omx,omy,gx,gy;
{
	register xchar nx,ny,nix,niy;
	register schar i;
	schar chcnt,cnt;
	coord poss[9];
	long info[9];
	long allowflags;
	struct obj *ib = 0;

	if(omx == gx && omy == gy)
		return(0);
	if(mtmp->mconf) {
		avoid = FALSE;
		appr = 0;
	}

	nix = omx;
	niy = omy;
	if (mtmp->isshk) allowflags = ALLOW_SSM;
	else allowflags = ALLOW_SSM | ALLOW_SANCT;
	if (passes_walls(mtmp->data)) allowflags |= (ALLOW_ROCK|ALLOW_WALL);
	if (throws_rocks(mtmp->data)) allowflags |= ALLOW_ROCK;
	if (tunnels(mtmp->data) &&
		    (!needspick(mtmp->data) || m_carrying(mtmp, PICK_AXE)))
		allowflags |= ALLOW_DIG;
	cnt = mfndpos(mtmp, poss, info, allowflags);
	if (allowflags & ALLOW_DIG) if(!mdig_tunnel(mtmp)) return(-2);

	if(mtmp->isshk && avoid && uondoor) { /* perhaps we cannot avoid him */
		for(i=0; i<cnt; i++)
		    if(!(info[i] & NOTONL)) goto pick_move;
		avoid = FALSE;
	}

#define	GDIST(x,y)	(dist2(x,y,gx,gy))
pick_move:
	chcnt = 0;
	for(i=0; i<cnt; i++) {
		nx = poss[i].x;
		ny = poss[i].y;
		if(levl[nx][ny].typ == ROOM ||
#if defined(ALTARS) && defined(THEOLOGY)
			(mtmp->ispriest &&
			    levl[nx][ny].typ == ALTAR) ||
#endif
			(mtmp->isshk &&
			    (monroom != ESHK(mtmp)->shoproom
			    || ESHK(mtmp)->following))) {
		    if(avoid && (info[i] & NOTONL))
			continue;
		    if((!appr && !rn2(++chcnt)) ||
			(appr && GDIST(nx,ny) < GDIST(nix,niy))) {
			    nix = nx;
			    niy = ny;
		    }
		}
	}
#if defined(ALTARS) && defined(THEOLOGY)
	if(mtmp->ispriest && avoid &&
			nix == omx && niy == omy && online(omx,omy)) {
		/* might as well move closer as long it's going to stay
		 * lined up */
		avoid = FALSE;
		goto pick_move;
	}
#endif

	if(nix != omx || niy != omy) {
		remove_monster(omx, omy);
		place_monster(mtmp, nix, niy);
		pmon(mtmp);
		if(ib) {
			if (cansee(mtmp->mx,mtmp->my))
			    pline("%s picks up %s.", Monnam(mtmp),
				distant_name(ib,doname));
			freeobj(ib);
			mpickobj(mtmp, ib);
		}
		return(1);
	}
	return(0);
}

#if defined(ALTARS) && defined(THEOLOGY)

struct mkroom *
in_temple(x, y)
register int x, y;
{
	register int roomno = inroom(x, y);

	if (roomno < 0 || rooms[roomno].rtype != TEMPLE)
		return((struct mkroom *)0);
	return(&rooms[roomno]);
}

static boolean
histemple_at(priest, x, y)
register struct monst *priest;
register int x, y;
{
	return(EPRI(priest)->shroom == inroom(x, y) && 
	       EPRI(priest)->shrlevel == dlevel);
}

/*
 * pri_move: return 1: he moved  0: he didn't  -1: let m_move do it  -2: died
 */
int
pri_move(priest)
register struct monst *priest;
{
	register xchar gx,gy,omx,omy;
	schar temple;
	boolean avoid = TRUE;

	omx = priest->mx;
	omy = priest->my;

	if(!histemple_at(priest, omx, omy)) return(-1);

	temple = EPRI(priest)->shroom;
	
	gx = EPRI(priest)->shrpos.x;
	gy = EPRI(priest)->shrpos.y;

	gx += rn1(3,-1);	/* mill around the altar */
	gy += rn1(3,-1);

	if(!priest->mpeaceful) {
		if(dist(omx,omy) < 3) {
			if(Displaced)
				Your("displaced image doesn't fool %s!",
					mon_nam(priest));
			(void) mattacku(priest);
			return(0);
		} else if(temple == inroom(u.ux,u.uy)) {
			/* don't chase player outside temple */
			long saveBlind = Blinded;
			struct obj *saveUblindf = ublindf;
			Blinded = 0;
			ublindf = (struct obj *)0;
			if(priest->mcansee && !Invis && cansee(omx,omy)) {
				gx = u.ux;
				gy = u.uy;
			}
			Blinded = saveBlind;
			ublindf = saveUblindf;
			avoid = FALSE;
		}
	} else if(Invis) avoid = FALSE;
	
	return(move_special(priest,temple,TRUE,FALSE,avoid,omx,omy,gx,gy));
}

/* exclusively for mktemple() */
void
priestini(lvl, sx, sy, align)
register int lvl, sx, sy, align;
{
	register struct monst *priest;
	register struct obj *otmp = (struct obj *)0;
#ifdef SPELLS
	register int cnt;
#endif
	if(MON_AT(sx+1, sy)) rloc(m_at(sx+1, sy)); /* insurance */

	if(priest = makemon(&mons[!rn2(2) ? PM_TEMPLE_PRIEST : 
			PM_TEMPLE_PRIESTESS], sx+1, sy)) {
		EPRI(priest)->shroom = inroom(sx, sy);
		EPRI(priest)->shralign = align;
		EPRI(priest)->shrpos.x = sx;
		EPRI(priest)->shrpos.y = sy;
		EPRI(priest)->shrlevel = lvl;
		EPRI(priest)->ismale = 
				(priest->data == &mons[PM_TEMPLE_PRIEST]);
		Strcpy(EPRI(priest)->deitynam, a_gname_at(sx, sy));
		priest->mtrapseen = ~0;	/* traps are known */
		priest->mpeaceful = 1;
		priest->ispriest = 1;
		priest->msleep = 0;

		/* now his/her goodies... */
		(void) mongets(priest, CHAIN_MAIL);
		(void) mongets(priest, SMALL_SHIELD);

		/* Do NOT put the rest in m_initinv.    */
		/* Priests created elsewhere than in a  */
		/* temple should not carry these items, */
		/* except for the mace.			*/
#ifdef SPELLS
		cnt = rn1(2,3);
		while(cnt) {
		    otmp = mkobj(SPBOOK_SYM, FALSE);
		    if(otmp) mpickobj(priest, otmp);
		    cnt--;
		}
#endif
		if(p_coaligned(priest)) {
		    (void) mongets(priest, rn2(2) ? CLOAK_OF_PROTECTION
						  : CLOAK_OF_MAGIC_RESISTANCE);
#ifdef NAMED_ITEMS
		    otmp = mk_aligned_artifact((unsigned)EPRI(priest)->shralign + 1);
		    if(otmp) {
			otmp->spe = rnd(4);
			mpickobj(priest, otmp);
		    }
#endif
		} else {
		    if(!rn2(5)) 
			otmp = mksobj(CLOAK_OF_MAGIC_RESISTANCE, FALSE); 
		    else otmp = mksobj(CLOAK_OF_PROTECTION, FALSE); 
		    if(otmp) {
			if(!rn2(2)) curse(otmp);
			mpickobj(priest, otmp);
		    }
		    otmp = mksobj(MACE, FALSE);
		    if(otmp) {
			otmp->spe = rnd(3);
			if(!rn2(2)) curse(otmp);
			mpickobj(priest, otmp);
		    }
		}
	}
}

char *
priestname(priest)
register struct monst *priest;
{
	static char pname[PL_NSIZ];

	Strcpy(pname, "the ");
	if(priest->minvis) Strcat(pname, "invisible ");
	if(priest->data != &mons[PM_TEMPLE_PRIEST] &&
			priest->data != &mons[PM_TEMPLE_PRIESTESS]) {
		Strcat(pname, priest->data->mname);
		Strcat(pname, " ");
	}
	if(EPRI(priest)->ismale)
		Strcat(pname, "priest of ");
	else 	Strcat(pname, "priestess of ");
	Strcat(pname, EPRI(priest)->deitynam);
	return(pname);
}

boolean
p_coaligned(priest)
struct monst *priest;
{
	return(!strcmp(u_gname(), EPRI(priest)->deitynam));
}

static int
t_alignment(troom)
struct mkroom *troom;
{
	int x, y;

	shrine_pos(&x,&y,troom);

	if(IS_ALTAR(levl[x][y].typ) && (levl[x][y].altarmask & A_SHRINE) != 0)
		return(levl[x][y].altarmask & ~A_SHRINE); 
	return(-2); /* arbitrary non-alignment type value */
}

static boolean
is_shrined(troom)
struct mkroom *troom;
{
	int x, y;

	shrine_pos(&x,&y,troom);

	if(IS_ALTAR(levl[x][y].typ) && (levl[x][y].altarmask & A_SHRINE) != 0)
		return(TRUE);
	return(FALSE);
}

static boolean
t_coaligned(troom)
struct mkroom *troom;
{
	return(t_alignment(troom) == u.ualigntyp + 1);
}

struct monst *
findpriest(troom)
struct mkroom *troom;
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if(mtmp->ispriest && histemple_at(mtmp,mtmp->mx,mtmp->my)
			&& &rooms[EPRI(mtmp)->shroom] == troom)
		return(mtmp);
	return (struct monst *)0;
}

static boolean
p_inhistemple(troom)
struct mkroom *troom;
{
	register struct monst *priest;

	priest = findpriest(troom);
	if(priest) return(TRUE);
	return(FALSE);
}

void
intemple() {
	register struct mkroom *troom;

	if(troom = in_temple(u.ux, u.uy)) {
	    boolean shrined = is_shrined(troom);
	    boolean tended = p_inhistemple(troom);

	    if(!in_temple(u.ux0, u.uy0)) {
		pline("Pilgrim, you enter a%s place!",
			(!(shrined || tended) ? " desecrated and deserted" :
			 !shrined ? " desecrated" :
			 !tended ? "n untended sacred" :
			  " sacred"));
		if(!t_coaligned(troom) || u.ualign < -5 || !shrined || !tended)
		    You("have a%s forbidding feeling...",
				(!shrined || !tended) ? "" :
				 " strange");
		else You("experience a strange sense of peace.");
	    } else if(!(shrined || tended) && !rn2(5)) {
		switch(rn2(3)) {
		    case 0: You("have an eerie feeling..."); break;
		    case 1: You("feel like you are being watched."); break;
		    default: pline("A shiver runs down your spine."); break;
		}
		if(!rn2(5)) {
		    struct monst *mtmp;

		    if(!(mtmp = makemon(&mons[PM_GHOST],u.ux,u.uy))) return;
		    pline("An enormous ghost appears next to you!");
		    mtmp->mpeaceful = 0;
		    if(flags.verbose)
		        You("are frightened to death, and unable to move.");
		    nomul(-3);
		    nomovemsg = "You regain your composure.";
		}
	    }
	}
}

void
priest_talk(priest)
register struct monst *priest;
{
	boolean coaligned = p_coaligned(priest);
	boolean strayed = (u.ualign < 0);
 
	if(priest->mflee) {
	    kludge("%s doesn't want anything to do with you!", 
				Monnam(priest));
	    priest->mtame = priest->mpeaceful = 0;
	    return;
	}

	/* priests don't chat unless peaceful and in their own temple */
	if(!histemple_at(priest,priest->mx,priest->my) || priest->mtame ||
		 !priest->mpeaceful || priest->mfroz || priest->msleep) {
            if(priest->mfroz || priest->msleep) {
	        kludge("%s breaks out of his reverie!", Monnam(priest));
                priest->mfroz = priest->msleep = 0;
	    }
	    /* The following is now impossible according to monst.c, */
	    /* but it should stay just in case we change the latter. */
	    if(priest->mtame)
		kludge("%s breaks out of your taming spell!", Monnam(priest));
	    priest->mtame = priest->mpeaceful = 0;
	    switch(rn2(3)) {
	        case 0: 
		   verbalize("Thou wouldst have words, eh?  I'll give thee a word or two!"); 
		   break;
	        case 1: 
		   verbalize("Talk?  Here is what I have to say!"); 
		   break;
	        default: 
		   verbalize("Pilgrim, I have lost mine desire to talk.");
		   break;
	    }
	    return;
	}

	/* he desecrated the temple and now he wants to chat? */
	if(!is_shrined(&rooms[inroom(priest->mx, priest->my)])
		&& priest->mpeaceful) {
	    verbalize("Begone!  Thou desecratest this holy place with thy presence.");
	    priest->mpeaceful = 0;
	    return;
	} 

	if(!u.ugold) {
	    if(coaligned && !strayed) {
	        kludge("%s gives you two bits for an ale.", Monnam(priest));
	        u.ugold = 2L;
		if (priest->mgold) priest->mgold -= 2L;
	    } else
		kludge("%s is not interested.", Monnam(priest));
	    return;
	} else {
	    long offer;

	    kludge("%s asks you for a contribution for the temple.",
			Monnam(priest));
	    if((offer = bribe(priest)) == 0) {
		verbalize("Thou shalt regret thine action!");
		if(coaligned) u.ualign--;
	    } else if(offer < (u.ulevel * 200)) {
		if(u.ugold > (offer * 2L)) verbalize("Cheapskate.");
		else {
		    verbalize("I thank thee for thy contribution.");
		    /*  give player some token  */
		}
	    } else if(offer < (u.ulevel * 400)) {
		verbalize("Thou art indeed a pious individual.");
		if(u.ugold < (offer * 2L)) { 
		    if(coaligned && u.ualign < -5) u.ualign++;
		    verbalize("I bestow upon thee a blessing.");
		    Clairvoyant += rn1(500,500);
		}
	    } else if(offer < (u.ulevel * 600)) {
		verbalize("Thy devotion has been rewarded.");
		if (!(Protection & INTRINSIC))  {
			Protection |= INTRINSIC;
			if (!u.ublessed)  u.ublessed = rnd(3) + 1;
		} else u.ublessed++;
	    } else {
		verbalize("Thy selfless generosity is deeply appreciated.");
		if(u.ugold < (offer * 2L) && coaligned) {
		    if(strayed && (moves - u.ucleansed) > 5000L) { 
			u.ualign = 0; /* cleanse him */
			u.ucleansed = moves;
		    } else { 
		        u.ualign += 2;
		    }
		}
	    }
	}
}

boolean
u_in_sanctuary(troom) 
register struct mkroom *troom;
{
	register struct mkroom *troom2;

	troom2 = in_temple(u.ux, u.uy);

	return(troom && troom2 && troom == troom2 && is_shrined(troom2) && 
			t_coaligned(troom2) && u.ualign > -5);
}

void
ghod_hitsu() 	/* when attacking a priest in his temple */
{
	int x, y, ax, ay;
	struct monst *priest;
	struct mkroom *troom = in_temple(u.ux, u.uy);

	if(!troom || !is_shrined(troom)) return;

	/* shrine converted by human sacrifice */
	if((priest = findpriest(troom)) && 
	    strcmp(EPRI(priest)->deitynam,
		a_gname_at(EPRI(priest)->shrpos.x, EPRI(priest)->shrpos.y))) 
		    return;

	shrine_pos(&x,&y,troom);
	ax = x;
	ay = y;

	if((u.ux == x && u.uy == y) || !linedup(u.ux, u.uy, x, y)) {
	    if(IS_DOOR(levl[u.ux][u.uy].typ)) {
		if(u.ux == troom->lx - 1) {
		    x = troom->hx;
		    y = u.uy;
		} else if(u.ux == troom->hx + 1) {
		    x = troom->lx;
		    y = u.uy;
		} else if(u.uy == troom->ly - 1) {
		    x = u.ux;
		    y = troom->hy;
		} else if(u.uy == troom->hy + 1) {
		    x = u.ux;
		    y = troom->ly;
		}
	    } else {
		switch(rn2(4)) {
		case 0:  x = u.ux; y = troom->ly; break;
		case 1:  x = u.ux; y = troom->hy; break;
		case 2:  x = troom->lx; y = u.uy; break;
		default: x = troom->hx; y = u.uy; break;
		}
	    }
	    if(!linedup(u.ux, u.uy, x, y)) return;
	}

	switch(rn2(3)) {
	case 0: 
	    pline("%s roars in anger:  \"Thou shalt suffer!\"", 
			a_gname_at(ax, ay));
	    break;
	case 1: 
	    pline("%s's voice booms:  \"How darest thou harm my servant!\"",
			a_gname_at(ax, ay));
	    break;
	default: 
	    pline("%s roars:  \"Thou dost profane my shrine!\"",
			a_gname_at(ax, ay));
	    break;
	}

	buzz(-15, 6, x, y, sgn(tbx), sgn(tby)); /* -15: bolt of lightning */
}

void
angry_priest()
{
	register struct monst *priest;

	if(!(priest = findpriest(in_temple(u.ux, u.uy)))) return;
	wakeup(priest);
}
#endif /* ALTARS && THEOLOGY */
