/*	SCCS Id: @(#)allmain.c	3.0	89/09/26
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* various code that was replicated in *main.c */

#include "hack.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif

int (*afternmv)();
int (*occupation)();

void
moveloop()
{
#ifdef MSDOS
	char ch;
	int abort;
#endif

	for(;;) {
		if(flags.move) {	/* actual time passed */

#ifdef SOUNDS
			dosounds();
#endif
			settrack();

			if(moves%2 == 0 ||
			  (!(Fast & ~INTRINSIC) && (!Fast || rn2(3)))) {
				movemon();
#ifdef HARD
				if(!rn2(u.udemigod?25:(dlevel>30)?50:70))
#else
				if(!rn2(70))
#endif
				    (void) makemon((struct permonst *)0, 0, 0);
				++monstermoves;
			}
			if(Glib) glibr();
			timeout();
			++moves;
#ifdef THEOLOGY
			if (u.ublesscnt)  u.ublesscnt--;
#endif
			if(flags.time) flags.botl = 1;
#ifdef POLYSELF
			if(u.mtimedone)
			    if(u.mh < 1) rehumanize();
			else
#endif
			    if(u.uhp < 1) {
				You("die...");
				done(DIED);
			    }
#ifdef POLYSELF
			if (u.mtimedone) {
			    if (u.mh < u.mhmax) {
				if (Regeneration || !(moves%20)) {
					flags.botl = 1;
					u.mh++;
				}
			    }
			}
#endif
			if(u.uhp < u.uhpmax) {
				if(u.ulevel > 9) {
				    int heal;

				    if(HRegeneration || !(moves%3)) {
					flags.botl = 1;
					if (ACURR(A_CON) <= 12) heal = 1;
					else heal = rnd((int) ACURR(A_CON)-12);
					if (heal > u.ulevel-9) heal = u.ulevel-9;
					u.uhp += heal;
					if(u.uhp > u.uhpmax)
					    u.uhp = u.uhpmax;
				    }
				} else if(HRegeneration ||
					(!(moves%((MAXULEV+12)/(u.ulevel+2)+1)))) {
					flags.botl = 1;
					u.uhp++;
				}
			}
#ifdef SPELLS
			if ((u.uen<u.uenmax) && (!(moves%(19-ACURR(A_INT)/2)))) {
				u.uen += rn2((int)ACURR(A_WIS)/5 + 1) + 1;
				if (u.uen > u.uenmax)  u.uen = u.uenmax;
				flags.botl = 1;
			}
#endif
			if(Teleportation && !rn2(85)) tele();
#ifdef POLYSELF
			if(Polymorph && !rn2(100))
				polyself();
			if(u.ulycn >= 0 && !rn2(80 - (20 * night())))
				you_were();
#endif
			if(Searching && multi >= 0) (void) dosearch0(1);
			hatch_eggs();
			gethungry();
			invault();
			amulet();
#ifdef HARD
			if (!rn2(40+(int)(ACURR(A_DEX)*3))) u_wipe_engr(rnd(3));
			if (u.udemigod) {
				if (u.udg_cnt) u.udg_cnt--;
				if (!u.udg_cnt) {
					intervene();
					u.udg_cnt = rn1(200, 50);
				}
			}
#endif
			restore_attrib();
		}
		if(multi < 0) {
			if(!++multi){
				pline(nomovemsg ? nomovemsg :
					"You can move again.");
				nomovemsg = 0;
				if(afternmv) (*afternmv)();
				afternmv = 0;
			}
		}

		find_ac();
		if(!flags.mv || Blind)
		{
			seeobjs();
			seemons();
			seeglds();
			nscr();
		}
		if(flags.botl || flags.botlx) bot();

		flags.move = 1;

		if(multi >= 0 && occupation) {
#ifdef MSDOS
			abort = 0;
			if (kbhit()) {
				if ((ch = Getchar()) == ABORT)
					abort++;
# ifdef REDO
				else
					pushch(ch);
# endif /* REDO */
			}
			if(abort || monster_nearby())
#else
			if(monster_nearby())
#endif
				stop_occupation();
			else if ((*occupation)() == 0)
				occupation = 0;
#ifdef MSDOS
			if (!(++occtime % 7))
				(void) fflush(stdout);
#endif
			continue;
		}

		if((u.uhave_amulet || Clairvoyant) && 
#ifdef ENDGAME
			dlevel != ENDLEVEL &&
#endif
			!(moves%15) && !rn2(2)) do_vicinity_map();

		u.umoved = FALSE;
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
		if(multi && multi%7 == 0)
			(void) fflush(stdout);
	}
}

void
stop_occupation()
{
	if(occupation) {
		You("stop %s.", occtxt);
		occupation = 0;
#ifdef REDO
		multi = 0;
		pushch(0);
#endif
	}
}

void
newgame() {
#ifdef DGK
	gameDiskPrompt();
#endif

	fobj = fcobj = invent = 0;
	fmon = fallen_down = 0;
	ftrap = 0;
	fgold = 0;
	flags.ident = 1;

	init_objects();
	u_init();

#ifndef NO_SIGNAL
	(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif

	mklev();
	u.ux = xupstair;
	u.uy = yupstair;
	(void) inshop();

	setsee();
	flags.botlx = 1;

	/* Move the monster from under you or else
	 * makedog() will fail when it calls makemon().
	 * 			- ucsfcgl!kneller
	 */
	if(MON_AT(u.ux, u.uy)) mnexto(m_at(u.ux, u.uy));

	(void) makedog();
	seemons();
#ifdef NEWS
	if(flags.nonews || !readnews())
		/* after reading news we did docrt() already */
#endif
		docrt();

	return;
}
