/*	SCCS Id: @(#)explode.c 3.1	93/05/15	*/
/*	Copyright (C) 1990 by Ken Arromdee */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* Note: Arrays are column first, while the screen is row first */
static int expl[3][3] = {
	{ S_explode1, S_explode4, S_explode7 },
	{ S_explode2, S_explode5, S_explode8 },
	{ S_explode3, S_explode6, S_explode9 }
};

/* Note: I had to choose one of three possible kinds of "type" when writing
 * this function: a wand type (like in zap.c), an adtyp, or an object type.
 * Wand types get complex because they must be converted to adtyps for
 * determining such things as fire resistance.  Adtyps get complex in that
 * they don't supply enough information--was it a player or a monster that
 * did it, and with a wand, spell, or breath weapon?  Object types share both
 * these disadvantages....
 */
void
explode(x, y, type, dam, olet)
int x, y;
int type; /* the same as in zap.c */
int dam;
char olet;
{
	int i, j, k;
	boolean starting = 1;
	boolean visible, any_shield;
	int uhurt = 0; /* 0=unhurt, 1=items damaged, 2=you and items damaged */
	const char *str;
	int idamres, idamnonres;
	struct monst *mtmp;
	uchar adtyp;
	int explmask[3][3];
		/* 0=normal explosion, 1=do shieldeff, 2=do nothing */
	boolean shopdamage = FALSE;

	switch(abs(type) % 10) {
		default: impossible("explosion base type %d?", type); return;

		case 1: str = (olet == SCROLL_CLASS ? "tower of flame" : "fireball"); adtyp = AD_FIRE; break;
		/* case 3: str = "ball of cold"; adtyp = AD_COLD; break; */
		/* case 5: str = "ball lightning"; adtyp = AD_ELEC; break; */
		/* case 7: str = "acid ball"; adtyp = AD_ACID; break; */
	}

	any_shield = visible = FALSE;
	for(i=0; i<3; i++) for(j=0; j<3; j++) {
		if (!isok(i+x-1, j+y-1)) {
			explmask[i][j] = 2;
			continue;
		}
		if (i+x-1 == u.ux && j+y-1 == u.uy) {
		    switch(adtyp) {
			case AD_FIRE:
				explmask[i][j] = Fire_resistance ? 1 : 0;
				break;
			/* case AD_COLD: */
			/* case AD_ELEC: */
			/* case AD_DISN: */
			/* case AD_ACID: */
			default:
				impossible("explosion type %d?", adtyp);
				explmask[i][j] = 0;
				break;
		    }
		}
		/* can be both you and mtmp if you're swallowed */
		if ((mtmp = m_at(i+x-1, j+y-1)) != 0) {
		    switch(adtyp) {
			case AD_FIRE:
				explmask[i][j] = resists_fire(mtmp->data)
					? 1 : 0;
				break;
			/* case AD_COLD: */
			/* case AD_ELEC: */
			/* case AD_DISN: */
			/* case AD_ACID: */
			default:
				impossible("explosion type %d?", adtyp);
				explmask[i][j] = 0;
				break;
		    }
		} else if (i+x-1 != u.ux || j+y-1 != u.uy)
		    explmask[i][j] = 0;

		if (cansee(i+x-1, j+y-1)) visible = TRUE;
		if (explmask[i][j] == 1) any_shield = TRUE;
	}

	if (visible) {
		/* Start the explosion */
		for(i=0; i<3; i++) for(j=0; j<3; j++) {
			if (explmask[i][j] == 2) continue;
			tmp_at(starting ? DISP_BEAM : DISP_CHANGE,
					    cmap_to_glyph(expl[i][j]));
			tmp_at(i+x-1, j+y-1);
			starting = 0;
		}
		curs_on_u();	/* will flush screen and output */

		if (any_shield) {	/* simulate a shield effect */
		    for (k = 0; k < SHIELD_COUNT; k++) {
			for(i=0; i<3; i++) for(j=0; j<3; j++) {
			    if (explmask[i][j] == 1)
				/*
				 * Bypass tmp_at() and send the shield glyphs
				 * directly to the buffered screen.  tmp_at()
				 * will clean up the location for us later.
				 */
				show_glyph(i+x-1, j+y-1,
					cmap_to_glyph(shield_static[k]));
			}
			curs_on_u();	/* will flush screen and output */
			delay_output();
		    }

		    /* Cover last shield glyph with blast symbol. */
		    for(i=0; i<3; i++) for(j=0; j<3; j++) {
			if (explmask[i][j] == 1)
			    show_glyph(i+x-1,j+y-1,cmap_to_glyph(expl[i][j]));
		    }

		} else {		/* delay a little bit. */
		    delay_output();
		    delay_output();
		}

	} else You("hear a blast.");


	for(i=0; i<3; i++) for(j=0; j<3; j++) {
		if (explmask[i][j] == 2) continue;
		if (i+x-1 == u.ux && j+y-1 == u.uy)
			uhurt = (explmask[i][j] == 1) ? 1 : 2;
		idamres = idamnonres = 0;
		(void)zap_over_floor((xchar)(i+x-1), (xchar)(j+y-1),
				     type, &shopdamage);

		mtmp = m_at(i+x-1, j+y-1);
		if (!mtmp) continue;
		if (u.uswallow && mtmp == u.ustuck) {
			if (is_animal(u.ustuck->data))
				pline("%s gets heartburn!",
				      Monnam(u.ustuck));
			else
				pline("%s gets slightly toasted!",
				      Monnam(u.ustuck));
		} else
		pline("%s is caught in the %s!",
			cansee(i+x-1, j+y-1) ? Monnam(mtmp) : "It", str);

		idamres += destroy_mitem(mtmp, SCROLL_CLASS, (int) adtyp);
		idamres += destroy_mitem(mtmp, SPBOOK_CLASS, (int) adtyp);
		/* Fire resistance protects monsters from burning scrolls, */
		/* but not from exploding potions. */
		idamnonres += destroy_mitem(mtmp, POTION_CLASS, (int) adtyp);
/*
		idamnonres += destroy_mitem(mtmp, WAND_CLASS, (int) adtyp);
		idamnonres += destroy_mitem(mtmp, RING_CLASS, (int) adtyp);
*/
		if (explmask[i][j] == 1) {
			golemeffects(mtmp, (int) adtyp, dam + idamres);
			mtmp->mhp -= idamnonres;
		} else {
		/* call resist with 0 and do damage manually so 1) we can
		 * get out the message before doing the damage, and 2) we can
		 * call mondied, not killed, if it's not your blast
		 */
			int mdam = dam;

			if (resist(mtmp, olet, 0, FALSE)) {
				pline("%s resists the magical blast!",
					cansee(i+x-1,j+y-1) ? Monnam(mtmp)
					: "It");
				mdam = dam/2;
			}
			if (mtmp == u.ustuck)
				mdam *= 2;
			if (resists_cold(mtmp->data) /* && adtyp == AD_FIRE */)
				mdam *= 2;
			mtmp->mhp -= mdam;
			mtmp->mhp -= (idamres + idamnonres);
		}
		if (mtmp->mhp <= 0) {
			if (type >= 0) killed(mtmp);
			else mondied(mtmp);
		}
	}

	if (visible) tmp_at(DISP_END, 0); /* clear the explosion */

	/* Do your injury last */
	if (uhurt) {
	        if (type >= 0 && flags.verbose && olet != SCROLL_CLASS)
			You("are caught in the %s!", str);
		if (uhurt == 2) u.uhp -= dam;
		if (u.uhp <= 0) {
			char buf[BUFSZ];

			if (type >= 0 && olet != SCROLL_CLASS) {
			    killer_format = NO_KILLER_PREFIX;
			    Sprintf(buf, "caught %sself in %s own %s.",
				    him[flags.female], his[flags.female], str);
			} else {
			    killer_format = KILLED_BY;
			    Strcpy(buf, str);
			}
			killer = buf;
			/* done(adtyp == AD_FIRE ? BURNING : DIED); */
			done(BURNING);
		}
		exercise(A_STR, FALSE);
#if defined(POLYSELF)
		ugolemeffects((int) adtyp, dam);
#endif
		destroy_item(SCROLL_CLASS, (int) adtyp);
		destroy_item(SPBOOK_CLASS, (int) adtyp);
		destroy_item(POTION_CLASS, (int) adtyp);
/*
		destroy_item(RING_CLASS, (int) adtyp);
		destroy_item(WAND_CLASS, (int) adtyp);
*/
	}
	if (shopdamage) {
		pay_for_damage("burn away");
/* (only if we ever add non-fire balls)
		pay_for_damage(adtyp == AD_FIRE ? "burn away" :
			       adtyp == AD_COLD ? "shatter" :
			       adtyp == AD_DISN ? "disintegrate" : "destroy");
*/
	}
}

/*explode.c*/
