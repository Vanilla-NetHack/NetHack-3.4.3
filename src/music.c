/*	SCCS Id: @(#)music.c	3.0	88/10/22
/* 	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the different functions designed to manipulate the
 * musical instruments and their various effects.
 *
 * Actually the list of instruments / effects is :
 *
 * Flute		may calm snakes if player has enough dexterity
 * Magic flute		may put monsters to sleep:  area of effect depends
 *			on player level.
 * Horn			Will awaken monsters:  area of effect depends on player
 *			level.  May also scare monsters.
 * Fire horn		Acts like a wand of fire.
 * Frost horn		Acts like a wand of cold.
 * Bugle		Will awaken soldiers (if any):  area of effect depends
 *			on player level.
 * Harp			May calm nymph if player has enough dexterity.
 * Magic harp		Charm monsters:  area of effect depends on player
 *			level.
 * Drum			Will awaken monsters like the horn.
 * Drum of earthquake	Will initiate an earthquake whose intensity depends
 *			on player level.  That is, it creates ramdom pits
 *			called here chasms.
 */


#include "hack.h"

#ifdef MUSIC

/*
 * Wake every monster in range...
 */

static void
awaken_monsters(distance)
int distance;
{
	register struct monst *mtmp = fmon;

	while(mtmp) {
		if (dist(mtmp->mx, mtmp->my) < distance/3) {
			/* May scare some monsters */
			if (!resist(mtmp, SCROLL_SYM, 0, NOTELL))
			  mtmp->mflee = 1;
		} else if (dist(mtmp->mx, mtmp->my) < distance) {
			mtmp->msleep = 0;
			mtmp->mfroz = 0;
		}
		mtmp = mtmp->nmon;
	}
}

/*
 * Make monsters fall asleep.  Note that they may resist the spell.
 */

static void
put_monsters_to_sleep(distance)
int distance;
{
	register struct monst *mtmp = fmon;

	while(mtmp) {
		  if (dist(mtmp->mx, mtmp->my) < distance)
		    if(!mtmp->mfroz && !resist(mtmp, WAND_SYM, 0, NOTELL))
		      mtmp->mfroz = 1;
		mtmp = mtmp->nmon;
	}
}

/*
 * Charm snakes in range.  Note that the snakes are NOT tamed.
 */

static void
charm_snakes(distance)
int distance;
{
	register struct monst *mtmp = fmon;

	while (mtmp) {
		if (mtmp->data->mlet == S_SNAKE && dist(mtmp->mx, mtmp->my) < distance) {
			mtmp->mpeaceful = 1;
			if (cansee(mtmp->mx, mtmp->my))
			  pline("%s freezes and sways with the music, then seems quieter.",defmonnam(mtmp));
		}
		mtmp = mtmp->nmon;
	}
}

/*
 * Calm nymphs in range.
 */

static void
calm_nymphs(distance)
int distance;
{
	register struct monst *mtmp = fmon;

	while (mtmp) {
		if (mtmp->data->mlet == S_NYMPH && dist(mtmp->mx, mtmp->my) < distance) {
			mtmp->mpeaceful = 1;
			if (cansee(mtmp->mx, mtmp->my))
			  pline("%s listens cheerfully to the music, then seems quieter.",defmonnam(mtmp));
		}
		mtmp = mtmp->nmon;
	}
}

/* Awake only soldiers of the level. */

static void
awaken_soldiers() {
#ifdef ARMY
#define IS_SOLDIER(dat)	((int)((dat) - mons) >= PM_UNARMORED_SOLDIER && \
			 (int) ((dat) - mons) <= PM_CAPTAIN)
	register struct monst *mtmp = fmon;

	while(mtmp) {
		if (IS_SOLDIER(mtmp->data))
			mtmp->mpeaceful = mtmp->msleep = mtmp->mfroz = 0;
		mtmp = mtmp->nmon;
	}
#endif /* ARMY /**/
}

/* Charm monsters in range.  Note that they may resist the spell. */

static void
charm_monsters(distance)
int distance;
{
	register struct monst *mtmp = fmon;

	while(mtmp) {
		if(dist(mtmp->mx, mtmp->my) <= distance)
		    if(!resist(mtmp, SCROLL_SYM, 0, NOTELL))
			(void) tamedog(mtmp, (struct obj *) 0);
		mtmp = mtmp->nmon;
	}

}

/* Generate earthquake :-) of desired force.
 * That is:  create random chasms (pits).
 */

static void
do_earthquake(force)
int force;
{
	register int x,y;
	struct monst *mtmp;
	struct trap *chasm;
	int start_x, start_y, end_x, end_y;

	start_x = u.ux - (force * 2);
	start_y = u.uy - (force * 2);
	end_x = u.ux + (force * 2);
	end_y = u.uy + (force * 2);
	if (start_x < 1) start_x = 1;
	if (start_y < 1) start_y = 1;
	if (end_x >= COLNO) end_x = COLNO - 1;
	if (end_y >= ROWNO) end_y = ROWNO - 1;
	for (x=start_x; x<=end_x; x++)
	  for (y=start_y; y<=end_y; y++)
	    if (!rn2(14 - force)) {
		    switch (levl[x][y].typ) {
#ifdef FOUNTAINS
			  case FOUNTAIN : /* Make the fountain disappear */
			    if (cansee(x,y))
			      pline("The fountain falls into a chasm.");
			    goto do_pit;
#endif
#ifdef SINKS
			  case SINK :
			    if (cansee(x,y))
			      pline("The kitchen sink falls into a chasm.");
			    goto do_pit;
#endif
#ifdef ALTARS
			  case ALTAR :
			    if (cansee(x,y))
			      pline("The altar falls into a chasm.");
			    goto do_pit;
#endif
#ifdef THRONES
			  case THRONE :
			    if (cansee(x,y))
			      pline("The throne falls into a chasm.");
				/* Falls into next case */
#endif
			  case ROOM :
			  case CORR : /* Make a pit */
do_pit:			    chasm = maketrap(x,y,PIT);
			    chasm->tseen = 1;

			    levl[x][y].doormask = 0;

			    /* We have to check whether monsters or player
			       fall in a chasm... */

			    if (levl[x][y].mmask) {
				mtmp = m_at(x,y);
				if(!is_flyer(mtmp->data)) {
				    mtmp->mtrapped = 1;
				    if(cansee(x,y))
					pline("%s falls into a chasm!",
						Monnam(mtmp));
				    else if (flags.soundok && humanoid(mtmp->data))
					You("hear a scream!");
				    if ((mtmp->mhp -= rnd(6)) <= 0) {
					int saved_conf = u.umconf;

					if(!cansee(x,y))
					    pline("It has died!");
					else {
					    You("destroy %s!",
					    mtmp->mtame ?
						a_monnam(mtmp, "poor") :
						mon_nam(mtmp));
					}
					xkilled(mtmp,0);
					u.umconf = saved_conf;
				    }
				}
			    } else if (x == u.ux && y == u.uy) {
				    if (Levitation
#ifdef POLYSELF
					|| is_flyer(uasmon)
#endif
					) {
					    pline("A chasm opens up under you!");
					    You("don't fall in!");
				    } else {
					    You("fall into a chasm!");
					    u.utrap = rn1(6,2);
					    u.utraptype = TT_PIT;
					    losehp(rnd(6),"fall into a chasm");
					    selftouch("Falling, you");
				    }
			    } else
				newsym(x,y);
			    break;
			  case DOOR : /* Make the door collapse */
			    if (levl[x][y].doormask == D_NODOOR) break;
			    if (cansee(x,y))
				pline("The door collapses.");
			    levl[x][y].doormask = D_NODOOR;
			    if (!levl[x][y].mmask && !(x == u.ux && y == u.uy))
				newsym(x,y);
			    break;
		    }
	    }
}

/*
 * The player is trying to extract something from his/her instrument.
 */

static int
do_improvisation(instr)
struct obj *instr;
{
	int damage;

	if (Confusion)
	  pline("What you produce is quite far from music...");
	else
	  You("start playing the %s.", xname(instr));
	switch (instr->otyp) {
	      case FLUTE:	/* May charm snakes */
		if (rn2(ACURR(A_DEX)) + u.ulevel > 25)
		  charm_snakes((int)u.ulevel*3);
		break;
	      case MAGIC_FLUTE: /* Make monster fall asleep */
		You("produce soft music.");
		put_monsters_to_sleep((int)u.ulevel*5);
		break;
	      case HORN:	/* Awaken monsters or scare monsters */
		You("produce a frightful, grave sound.");
		awaken_monsters((int)u.ulevel*30);
		break;
	      case FROST_HORN:	/* Idem wand of cold */
	      case FIRE_HORN:	/* Idem wand of fire */
		if (instr->spe > 0) {
			instr->spe--;
			if (!getdir(1)) {
				if (!Blind)
				    pline("The %s glows then fades.", xname(instr));
			} else {
				if (!u.dx && !u.dy && !u.dz) {
					if((damage = zapyourself(instr)))
					  losehp(damage,"self-inflicted injury");
					makeknown(instr->otyp);
					return(2);
				}
				buzz((instr->otyp == FROST_HORN) ? 3 : 1, rn1(6,6), u.ux, u.uy, u.dx, u.dy);
				makeknown(instr->otyp);
				return(2);
			}
		}
		break;
	      case BUGLE:	/* Awaken & attract soldiers */
		You("extract a loud noise from the %s.",xname(instr));
		awaken_soldiers();
		break;
	      case HARP:	/* May calm Nymph */
		if (rn2(ACURR(A_DEX)) + u.ulevel > 25)
		  calm_nymphs((int)u.ulevel*3);
		break;
	      case MAGIC_HARP:	/* Charm monsters */
		if (instr->spe > 0) {
			pline("The %s produces very attractive music.",xname(instr));
			instr->spe--;
			charm_monsters(((int)u.ulevel - 1) / 3 + 1);
		}
		break;
	      case DRUM:	/* Awaken monsters */
		You("beat a deafening row!");
		awaken_monsters((int)u.ulevel * 40);
		break;
	      case DRUM_OF_EARTHQUAKE:	/* create several pits */
		if (instr->spe > 0) {
			You("produce a heavy, thunderous rolling!");
			pline("The entire dungeon is shaking around you!");
			do_earthquake(((int)u.ulevel - 1) / 3 + 1);
			instr->spe--;
			makeknown(DRUM_OF_EARTHQUAKE);
		}
		break;
	      default:
		impossible("What a weird instrument (%d)!",instr->otyp);
		break;
	}
	return (2);		/* That takes time */
}

/*
 * So you want music...
 */

int
do_play_instrument(instr)
struct obj *instr;
{
#ifdef STRONGHOLD
    char buf[BUFSZ], *s, c = 'y';
    int x,y;
    boolean ok;

    if (instr->otyp != DRUM && instr->otyp != DRUM_OF_EARTHQUAKE) {
	pline("Improvise? ");
	c = yn();
    }
    if (c == 'n') {
	pline("What tune are you playing? [what 5 notes] ");
	getlin(buf);
	for(s=buf;*s;s++)
	    *s = (*s >='a' && *s<='z') ? 'A' + *s - 'a' : *s;
	You("extract a strange sound from the %s!",xname(instr));
	/* Check if there was the Stronghold drawbridge near
	 * and if the tune conforms to what we're waiting for.
	 */
	if (dlevel == stronghold_level)
	    if (!strcmp(buf,tune)) {
		/* Search for the drawbridge */
		for(y=u.uy-1; y<=u.uy+1; y++)
		    for(x=u.ux-1;x<=u.ux+1;x++)
			if (find_drawbridge(&x,&y)) {
			    if (levl[x][y].typ == DRAWBRIDGE_DOWN)
				close_drawbridge(x,y);
			    else
				open_drawbridge(x,y);
			    return 0;
			}
	    } else if (flags.soundok) {
		/* Okay, it wasn't the right tune, but perhaps
		 * we can give the player some hints like in the
		 * Mastermind game */
		ok = FALSE;
		for(y = u.uy-1; y <= u.uy+1 && !ok; y++)
		    for(x = u.ux-1; x <= u.ux+1 && !ok; x++)
			if(IS_DRAWBRIDGE(levl[x][y].typ) ||
			   is_drawbridge_wall(x,y) >= 0)
				ok = TRUE;
		if (ok) { /* There is a drawbridge near */
		    int tumblers, gears;
		    boolean matched[5];

		    tumblers = gears = 0;
		    for(x=0; x < 5; x++)
			matched[x] = FALSE;

		    for(x=0; x < strlen(buf); x++)
			if(x < 5) {
			    if(buf[x] == tune[x]) {
				gears++;
				matched[x] = TRUE;
			    } else
				for(y=0; y < 5; y++)
				    if(!matched[y] &&
				       buf[x] == tune[y] &&
				       buf[y] != tune[y]) {
					tumblers++;
					matched[y] = TRUE;
					break;
				    }
			}
		    if(tumblers)
			if(gears)
			You("hear %d tumbler%s click and %d gear%s turn.",
			    tumblers, (tumblers > 1 ? "s" : ""),
			    gears, (gears > 1 ? "s" : ""));
			else
			    You("hear %d tumbler%s click.",
			    tumblers, (tumblers > 1 ? "s" : ""));
		    else if(gears)
			You("hear %d gear%s turn.",
			gears, (gears > 1 ? "s" : ""));
		}
	    }
	return 1;
    } else
#endif /* STRONGHOLD /**/
	    return do_improvisation(instr);
}

#endif /* MUSIC /**/
