/*	SCCS Id: @(#)allmain.c	3.1	93/05/23	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* various code that was replicated in *main.c */

#include "hack.h"

#ifndef NO_SIGNAL
#include <signal.h>
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

	/* Note:  these initializers don't do anything except guarantee that
		we're linked properly.
	*/
	decl_init();
	monst_init();
	monstr_init();	/* monster strengths */
	objects_init();

	(void) encumber_msg(); /* in case they auto-picked up something */

	for(;;) {
#ifdef CLIPPING
		cliparound(u.ux, u.uy);
#endif
#if defined(MAC_MPW32) && !defined(MODEL_FAR)
		UnloadAllSegments();  /* Marks non-resident segments as purgeable */
#endif
		get_nh_event();

		didmove = flags.move;
		if(flags.move) {	/* actual time passed */
#ifdef POLYSELF
		    int oldmtimedone;
#endif
		    int wtcap;

		    if (u.utotype) deferred_goto();
		    wtcap = encumber_msg();
#ifdef POLYSELF
		    oldmtimedone = u.mtimedone;
#endif

#ifdef SOUNDS
		    dosounds();
#endif

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
			movemon();
			/* a monster may have levteleported player -dlc */
			if (u.utotype) deferred_goto();
			if(!rn2(u.uevent.udemigod ? 25 :
				(depth(&u.uz) >
				 depth(&stronghold_level))
				? 50 : 70))
			    (void) makemon((struct permonst *)0, 0, 0);
			++monstermoves;
			remove_cadavers(&fobj);
			remove_cadavers(&invent);
			moverate -= 12;
		    }
		    if(Glib) glibr();
		    nh_timeout();
		    ++moves;
		    if (u.ublesscnt)  u.ublesscnt--;
		    if(flags.time) flags.botl = 1;
		    /* One possible result of prayer is healing.  Whether or
		     * not you get healed depends on your current hit points.
		     * If you are allowed to regenerate during the prayer, the
		     * end-of-prayer calculation messes up on this.
		     */
		    if (u.uinvulnerable)
			;
		    else
#ifdef POLYSELF
		    if (u.mtimedone && u.mh < u.mhmax) {
			if (u.mh < 1) {
			    rehumanize();
			    moverate = 0;
			} else if (Regeneration ||
				 (wtcap < MOD_ENCUMBER && !(moves%20))) {
			    flags.botl = 1;
			    u.mh++;
			}
		    } else
#endif
		    if(u.uhp < u.uhpmax) {
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
				pline("You pass out from exertion!");
				exercise(A_CON, FALSE);
				nomul(-10);
				u.usleep = 1;
			    }
			}
		    }

		    if ((u.uen < u.uenmax) &&
			((wtcap < MOD_ENCUMBER &&
			  (!(moves%((MAXULEV + 1 - u.ulevel) *
				    (pl_character[0] == 'W' ? 3 : 4) / 2))))
			 || Energy_regeneration)) {
			u.uen +=
			    rn1((int)(ACURR(A_WIS) + ACURR(A_INT)) / 10 + 1,1);
			if (u.uen > u.uenmax)  u.uen = u.uenmax;
			flags.botl = 1;
		    }

		    if(!u.uinvulnerable) {
			if(Teleportation && !rn2(85)) tele();
#ifdef POLYSELF
			if(Polymorph && !rn2(100)) {
			    if (multi >= 0) {
				if (occupation)
				    stop_occupation();
				else
				    nomul(0);
			    }
			    polyself();
			    moverate = 0;
			} else if (u.ulycn >= 0 && !rn2(80 - (20 * night()))) {
			    if (multi >= 0) {
				if (occupation)
				    stop_occupation();
				else
				    nomul(0);
			    }
			    you_were();
			    moverate = 0;
			}
#endif
		    }

		    if(Searching && multi >= 0) (void) dosearch0(1);
		    do_storms();
		    hatch_eggs();
		    burn_lamps();
		    gethungry();
		    exerchk();
		    invault();
		    amulet();
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
		    else if (Underwater)
			under_water(0);

#ifdef POLYSELF
		    if ((oldmtimedone && !u.mtimedone) ||
			(!oldmtimedone && u.mtimedone)) moverate = 0;
#endif
		}
		if(multi < 0) {
			if(!++multi){
				pline(nomovemsg ? nomovemsg :
					(const char *)"You can move again.");
				nomovemsg = 0;
				u.usleep = 0;
				if(afternmv) (*afternmv)();
				afternmv = 0;
			}
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

		if((u.uhave.amulet || Clairvoyant) && !In_endgame(&u.uz) &&
						!(moves%15) && !rn2(2))
			do_vicinity_map();

		if(u.utrap && u.utraptype == TT_LAVA) {
		    if(!is_lava(u.ux,u.uy))
			u.utrap = 0;
		    else {
			u.utrap -= 1<<8;
			if(u.utrap < 1<<8) {
			    killer_format = KILLED_BY;
			    killer = "molten lava";
			    You("sink below the surface and suffocate.");
			    done(DROWNING); /*whatever*/
			} else if(didmove && !u.umoved) {
			    Norep("You sink deeper into the lava.");
			    u.utrap += rnd(4);
			}
		    }
		}

		u.umoved = FALSE;
		if(!didmove || moverate <= 0) {
		    if(multi > 0) {
			lookaround();
			if(!multi) {	/* lookaround may clear multi */
				flags.move = 0;
				continue;
			}
			if(flags.mv) {
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
			rhack(NULL);
		    }
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
#ifdef MFLOPPY
	gameDiskPrompt();
#endif

	fobj = invent = level.buriedobjlist = migrating_objs = (struct obj *)0;
	fmon = migrating_mons = (struct monst *)0;
	ftrap = 0;
	flags.ident = 1;

	if(wiz1_level.dlevel == 0) init_dungeons();
	init_objects();		/* must be before u_init() */
	u_init();
	init_artifacts();	/* must be after u_init() */

#ifndef NO_SIGNAL
	(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif
#ifdef NEWS
	if(flags.news) display_file(NEWS, FALSE);
#endif
#ifdef MULDGN
	load_qtlist();	/* load up the quest text info */
	quest_init();
	if(flags.legacy && moves == 1) com_pager(1);
#endif
	mklev();
	u_on_upstairs();
	check_special_room(FALSE);
	vision_reset();		/* set up internals for level (after mklev) */

	flags.botlx = 1;

	/* Move the monster from under you or else
	 * makedog() will fail when it calls makemon().
	 * 			- ucsfcgl!kneller
	 */
	if(MON_AT(u.ux, u.uy)) mnexto(m_at(u.ux, u.uy));

#ifdef CLIPPING
	cliparound(u.ux, u.uy);
#endif
	(void) makedog();
	docrt();

#ifdef INSURANCE
	save_currentstate();
#endif
	return;
}

#endif /* OVLB */

/*allmain.c*/
