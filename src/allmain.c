/*	SCCS Id: @(#)allmain.c	3.2	96/03/28	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* various code that was replicated in *main.c */

#include "hack.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif

#ifdef POSITIONBAR
STATIC_DCL void NDECL(do_postionbar);
#endif

#ifdef OVL0

void
moveloop()
{
#ifdef MICRO
	char ch;
	int abort_lev;
#endif
	int moverate = 0;
	boolean didmove = 0;

	flags.moonphase = phase_of_the_moon();
	if(flags.moonphase == FULL_MOON) {
		You("are lucky!  Full moon tonight.");
		change_luck(1);
	} else if(flags.moonphase == NEW_MOON) {
		pline("Be careful!  New moon tonight.");
	}
	flags.friday13 = friday_13th();
	if (flags.friday13) {
		pline("Watch out!  Bad things can happen on Friday the 13th.");
		change_luck(-1);
	}

	initrack();


	/* Note:  these initializers don't do anything except guarantee that
		we're linked properly.
	*/
	decl_init();
	monst_init();
	monstr_init();	/* monster strengths */
	objects_init();

#ifdef WIZARD
	if (wizard) add_debug_extended_commands();
#endif

	(void) encumber_msg(); /* in case they auto-picked up something */

	u.uz0.dlevel = u.uz.dlevel;

	for(;;) {
#ifdef CLIPPING
		cliparound(u.ux, u.uy);
#endif
#if defined(MAC68K) && defined(MAC_MPW32) && !defined(MODEL_FAR)
		UnloadAllSegments();  /* Marks non-resident segments as purgeable */
#endif
		get_nh_event();
#ifdef POSITIONBAR
		do_positionbar();
#endif

		didmove = flags.move;
		if(flags.move) {	/* actual time passed */
		    int oldmtimedone;
		    int wtcap;

		    if (u.utotype) deferred_goto();
		    wtcap = encumber_msg();
		    oldmtimedone = u.mtimedone;
		    dosounds();

		    if(moverate <= 0) {
			/* calculate how much time passed. */
			int moveamt = 0;
			if(Fast & ~INTRINSIC) moveamt = 6;
			else if(Fast) moveamt = 8;
			else moveamt = 12;

			switch(wtcap) {
			case UNENCUMBERED: break;
			case SLT_ENCUMBER: moveamt = (moveamt * 4) / 3; break;
			case MOD_ENCUMBER: moveamt *= 2; break;
			case HVY_ENCUMBER: moveamt *= 4; break;
			default: moveamt *= 12; break;
			}
			moverate += moveamt;
			settrack();
		    }

		    if(moverate > 0) {
			flags.mon_moving = TRUE;
			movemon();
			flags.mon_moving = FALSE;
			/* a monster may have levteleported player -dlc */
			if (u.utotype) deferred_goto();
			if(!rn2(u.uevent.udemigod ? 25 :
				(depth(&u.uz) >
				 depth(&stronghold_level))
				? 50 : 70))
			    (void) makemon((struct permonst *)0, 0, 0);
			++monstermoves;
			moverate -= 12;
		    }
		    if(Glib) glibr();
		    nh_timeout();
		    ++moves;
		    if (u.ublesscnt)  u.ublesscnt--;
		    if(flags.time && !flags.run)
			flags.botl = 1;

		    /* One possible result of prayer is healing.  Whether or
		     * not you get healed depends on your current hit points.
		     * If you are allowed to regenerate during the prayer, the
		     * end-of-prayer calculation messes up on this.
		     * Another possible result is rehumanization, which requires
		     * that encumbrance and movement rate be recalculated.
		     */
		    if (u.uinvulnerable) {
			/* for the moment at least, you're in tiptop shape */
			wtcap = UNENCUMBERED;
			moverate = 0;
		    } else if (u.mtimedone && u.mh < u.mhmax) {
			if (u.mh < 1) {
			    rehumanize();
			    moverate = 0;
			} else if (Regeneration ||
				 (wtcap < MOD_ENCUMBER && !(moves%20))) {
			    flags.botl = 1;
			    u.mh++;
			}
		    } else if (u.uhp < u.uhpmax) {
			if(u.ulevel > 9) {
			    int heal;

			    if(HRegeneration ||
			       (!(moves%3) &&
				(wtcap < MOD_ENCUMBER || !flags.mv))) {
				flags.botl = 1;
				if (ACURR(A_CON) <= 12) heal = 1;
				else heal = rnd((int) ACURR(A_CON)-12);
				if (heal > u.ulevel-9) heal = u.ulevel-9;
				u.uhp += heal;
				if(u.uhp > u.uhpmax)
				    u.uhp = u.uhpmax;
			    }
			} else if(HRegeneration ||
				  ((wtcap < MOD_ENCUMBER || !flags.mv) &&
				   (!(moves%((MAXULEV+12)/(u.ulevel+2)+1))))) {
			    flags.botl = 1;
			    u.uhp++;
			}
		    }

		    if (wtcap > MOD_ENCUMBER && flags.mv) {
			if(!(wtcap < EXT_ENCUMBER ? moves%30 : moves%10)) {
			    if(u.uhp > 1) {
				u.uhp--;
			    } else {
				You("pass out from exertion!");
				exercise(A_CON, FALSE);
				fall_asleep(-10, FALSE);
			    }
			}
		    }

		    if ((u.uen < u.uenmax) &&
			((wtcap < MOD_ENCUMBER &&
			  (!(moves%((MAXULEV + 8 - u.ulevel) *
				    (Role_is('W') ? 3 : 4) / 6))))
			 || Energy_regeneration)) {
			u.uen +=
			    rn1((int)(ACURR(A_WIS) + ACURR(A_INT)) / 10 + 1,1);
			if (u.uen > u.uenmax)  u.uen = u.uenmax;
			flags.botl = 1;
		    }

		    if(!u.uinvulnerable) {
			if(Teleportation && !rn2(85)) {
#ifdef REDO
			    xchar old_ux = u.ux, old_uy = u.uy;
#endif
			    tele();
#ifdef REDO
			    if (u.ux != old_ux || u.uy != old_uy) {
				/* clear doagain keystrokes */
				pushch(0);
				savech(0);
			    }
#endif
			}
			if(Polymorph && !rn2(100)) {
			    if (multi >= 0) {
				if (occupation)
				    stop_occupation();
				else
				    nomul(0);
			    }
			    polyself();
			    moverate = 0;
			} else if (u.ulycn >= LOW_PM &&
				   !rn2(80 - (20 * night()))) {
			    if (multi >= 0) {
				if (occupation)
				    stop_occupation();
				else
				    nomul(0);
			    }
			    you_were();
			    moverate = 0;
			}
		    }

		    if(Searching && multi >= 0) (void) dosearch0(1);
		    do_storms();
		    gethungry();
		    exerchk();
		    invault();
		    if (u.uhave.amulet) amulet();
		    if (!rn2(40+(int)(ACURR(A_DEX)*3)))
			u_wipe_engr(rnd(3));
		    if (u.uevent.udemigod && !u.uinvulnerable) {
			if (u.udg_cnt) u.udg_cnt--;
			if (!u.udg_cnt) {
			    intervene();
			    u.udg_cnt = rn1(200, 50);
			}
		    }
		    restore_attrib();
		    /* underwater and waterlevel vision are done here */
		    if (Is_waterlevel(&u.uz))
			movebubbles();
		    else if (Underwater) under_water(0);
		    /* vision while buried done here */
		    else if (u.uburied) under_ground(0);

		    if ((oldmtimedone && !u.mtimedone) ||
			(!oldmtimedone && u.mtimedone)) moverate = 0;
		}
		if(multi < 0) {
			if (++multi == 0)	/* finished yet? */
				unmul((char *)0);
		}

		find_ac();
		if(!flags.mv || Blind) {
		    /* redo monsters if hallu or wearing a helm of telepathy */
		    if (Hallucination ||
			(HTelepat & (WORN_HELMET|WORN_AMUL|W_ART)))
			see_monsters();

		    /* redo objects if hallucinating */
		    if (Hallucination) see_objects();

		    /* update swallowed display */
		    if (Hallucination && u.uswallow) swallowed(0);

		    if (vision_full_recalc) vision_recalc(0);	/* vision! */
		}
		if(flags.botl || flags.botlx) bot();

		flags.move = 1;

		if(multi >= 0 && occupation) {
#ifdef MICRO
			abort_lev = 0;
			if (kbhit()) {
				if ((ch = Getchar()) == ABORT)
					abort_lev++;
# ifdef REDO
				else
					pushch(ch);
# endif /* REDO */
			}
			if (!abort_lev && (*occupation)() == 0)
#else
			if ((*occupation)() == 0)
#endif
				occupation = 0;
			if(
#ifdef MICRO
			   abort_lev ||
#endif
			   monster_nearby()) {
				stop_occupation();
				reset_eat();
			}
#ifdef MICRO
			if (!(++occtime % 7))
				display_nhwindow(WIN_MAP, FALSE);
#endif
			continue;
		}

		if ((u.uhave.amulet || Clairvoyant) &&
		    !(In_endgame(&u.uz) || (HClairvoyant & I_BLOCKED)) &&
		    !(moves % 15) && !rn2(2))
			do_vicinity_map();

		if(u.utrap && u.utraptype == TT_LAVA) {
		    if(!is_lava(u.ux,u.uy))
			u.utrap = 0;
		    else {
			u.utrap -= 1<<8;
			if(u.utrap < 1<<8) {
			    killer_format = KILLED_BY;
			    killer = "molten lava";
			    You("sink below the surface and die.");
			    done(DISSOLVED);
			} else if(didmove && !u.umoved) {
			    Norep("You sink deeper into the lava.");
			    u.utrap += rnd(4);
			}
		    }
		}

#ifdef WIZARD
		if (flags.sanity_check)
		    sanity_check();
#endif

		u.umoved = FALSE;
		if(!didmove || moverate <= 0) {
		    if(multi > 0) {
			lookaround();
			if(!multi)	/* lookaround may clear multi */
				flags.move = 0;
			else if(flags.mv) {
				if(multi < COLNO && !--multi)
					flags.mv = flags.run = 0;
				domove();
			} else {
				--multi;
				rhack(save_cm);
			}
		    } else if(multi == 0) {
#ifdef MAIL
			ckmailstatus();
#endif
			rhack((char *)0);
		    }
		    /* !flags.move here: multiple movement command stopped */
		    if (flags.time && (!flags.move || !flags.mv)) flags.botl=1;
		}
		if (vision_full_recalc) vision_recalc(0);	/* vision! */
		if(multi && multi%7 == 0)
			display_nhwindow(WIN_MAP, FALSE);
	}
}

#endif /* OVL0 */
#ifdef OVL1

void
stop_occupation()
{
	if(occupation) {
		You("stop %s.", occtxt);
		occupation = 0;
/* fainting stops your occupation, there's no reason to sync.
		sync_hunger();
*/
#ifdef REDO
		nomul(0);
		pushch(0);
#endif
	}
}

#endif /* OVL1 */
#ifdef OVLB

void
display_gamewindows()
{
    WIN_MESSAGE = create_nhwindow(NHW_MESSAGE);
    WIN_STATUS = create_nhwindow(NHW_STATUS);
    WIN_MAP = create_nhwindow(NHW_MAP);
    WIN_INVEN = create_nhwindow(NHW_MENU);

#ifdef MAC
    /*
     * This _is_ the right place for this - maybe we will
     * have to split display_gamewindows into create_gamewindows
     * and show_gamewindows to get rid of this ifdef...
     */
	if ( ! strcmp ( windowprocs . name , "mac" ) ) {
	    SanePositions ( ) ;
	}
#endif

    /*
     * The mac port is not DEPENDENT on the order of these
     * displays, but it looks a lot better this way...
     */
    display_nhwindow(WIN_STATUS, FALSE);
    display_nhwindow(WIN_MESSAGE, FALSE);
    clear_glyph_buffer();
    display_nhwindow(WIN_MAP, FALSE);
}

void
newgame()
{
	int i;

#ifdef MFLOPPY
	gameDiskPrompt();
#endif

	fobj = invent = level.buriedobjlist = migrating_objs = (struct obj *)0;
	fmon = migrating_mons = (struct monst *)0;
	ftrap = 0;
	flags.ident = 1;

	if(wiz1_level.dlevel == 0) init_dungeons();

	for (i = 0; i < NUMMONS; i++)
		mvitals[i].mvflags = mons[i].geno & G_NOCORPSE;

	init_objects();		/* must be before u_init() */
	u_init();
	init_artifacts();	/* must be after u_init() */

#ifndef NO_SIGNAL
	(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif
#ifdef NEWS
	if(flags.news) display_file(NEWS, FALSE);
#endif
	load_qtlist();	/* load up the quest text info */
	quest_init();

	mklev();
	u_on_upstairs();
#ifdef CLIPPING
	/* pline() (hence You()) will call flush_screen() if u.ux is set,
	 * which will be confused if clipping is not set up.
	 * this is the equivalent of the restgamestate() call for new games.
	 */
	cliparound(u.ux, u.uy);
#endif
	check_special_room(FALSE);
	vision_reset();		/* set up internals for level (after mklev) */

	flags.botlx = 1;

	/* Move the monster from under you or else
	 * makedog() will fail when it calls makemon().
	 *			- ucsfcgl!kneller
	 */
	if(MON_AT(u.ux, u.uy)) mnexto(m_at(u.ux, u.uy));
	(void) makedog();
	docrt();

	if(flags.legacy && moves == 1) {
		flush_screen(1);
		com_pager(1);
	}

#ifdef INSURANCE
	save_currentstate();
#endif
	program_state.something_worth_saving++;	/* useful data now exists */
	return;
}

#ifdef POSITIONBAR
do_positionbar()
{
	static char pbar[COLNO];
	char *p;
	
	p = pbar;
	/* up stairway */
	if (upstair.sx &&
	   (glyph_to_cmap(level.locations[upstair.sx][upstair.sy].glyph) ==
	    S_upstair ||
 	    glyph_to_cmap(level.locations[upstair.sx][upstair.sy].glyph) == 
	    S_upladder)) {
		*p++ = '<';
		*p++ = upstair.sx;
	}
	if (sstairs.sx &&
	   (glyph_to_cmap(level.locations[sstairs.sx][sstairs.sy].glyph) ==
	    S_upstair ||
 	    glyph_to_cmap(level.locations[sstairs.sx][sstairs.sy].glyph) == 
	    S_upladder)) {
		*p++ = '<';
		*p++ = sstairs.sx;
	}

	/* down stairway */
	if (dnstair.sx &&
	   (glyph_to_cmap(level.locations[dnstair.sx][dnstair.sy].glyph) ==
	    S_dnstair ||
 	    glyph_to_cmap(level.locations[dnstair.sx][dnstair.sy].glyph) ==
	    S_dnladder)) {
		*p++ = '>';
		*p++ = dnstair.sx;
	}
	if (sstairs.sx &&
	   (glyph_to_cmap(level.locations[sstairs.sx][sstairs.sy].glyph) ==
	    S_dnstair ||
 	    glyph_to_cmap(level.locations[sstairs.sx][sstairs.sy].glyph) == 
	    S_dnladder)) {
		*p++ = '>';
		*p++ = sstairs.sx;
	}

	/* hero location */
	if (u.ux) {
		*p++ = '@';
		*p++ = u.ux;
	}
	/* fence post */
	*p = 0;

	update_positionbar(pbar);
}
#endif

#endif /* OVLB */

/*allmain.c*/
