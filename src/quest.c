/*	SCCS Id: @(#)quest.c	3.2	96/03/15	*/
/*	Copyright 1991, M. Stephenson		  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/*  quest dungeon branch routines. */

#include "quest.h"
#include "qtext.h"

#define Not_firsttime	(on_level(&u.uz0, &u.uz))
#define Qstat(x)	(quest_status.x)

static void NDECL(on_start);
static void NDECL(on_locate);
static void NDECL(on_goal);
static boolean NDECL(not_capable);
static boolean FDECL(is_pure, (BOOLEAN_P));
static void FDECL(expulsion, (BOOLEAN_P));
static void NDECL(chat_with_leader);
static void NDECL(chat_with_nemesis);
static void NDECL(chat_with_guardian);

static void
on_start()
{
  if(!Qstat(first_start)) {
    qt_pager(QT_FIRSTTIME);
    Qstat(first_start) = TRUE;
  } else if((u.uz0.dnum != u.uz.dnum) || (u.uz0.dlevel < u.uz.dlevel)) {
    if(Qstat(not_ready) <= 2) qt_pager(QT_NEXTTIME);
    else	qt_pager(QT_OTHERTIME);
  }
}

static void
on_locate()
{
  if(!Qstat(first_locate)) {
    qt_pager(QT_FIRSTLOCATE);
    Qstat(first_locate) = TRUE;
  } else if(u.uz0.dlevel < u.uz.dlevel)
	qt_pager(QT_NEXTLOCATE);
}

static void
on_goal()
{
  if (Qstat(killed_nemesis)) {
    return;
  } else if (!Qstat(made_goal)) {
    qt_pager(QT_FIRSTGOAL);
    Qstat(made_goal) = 1;
  } else {
    qt_pager(QT_NEXTGOAL);
    if(Qstat(made_goal) < 7) Qstat(made_goal)++;
  }
}

void
quest_init()
{
/*
 *	Special setup modifications here:
 *
 *	Unfortunately, this is going to have to be done
 *	on each newgame or restore, because you lose the permonst mods
 *	across a save/restore :-)
 *
 *	1 - The Rogue Leader is the Tourist Nemesis.
 *	2 - Priests start with a random alignment - convert the leader and
 *	    guardians here.
 *	3 - Elves can have one of two different leaders, but can't work it
 *	    out here because it requires hacking the level file data (see
 *	    sp_lev.c).
 */
#ifdef TOURIST
    if (Role_is('T')) {
	mons[PM_MASTER_OF_THIEVES].msound = MS_NEMESIS;
	mons[PM_MASTER_OF_THIEVES].mflags2 &= ~(M2_PEACEFUL);
	mons[PM_MASTER_OF_THIEVES].mflags2 |= (M2_NASTY|M2_STALK|M2_HOSTILE);
	mons[PM_MASTER_OF_THIEVES].mflags3 = M3_WANTSARTI | M3_WAITFORU;
    } else
#endif
    if (Role_is('P')) {
	mons[PM_ARCH_PRIEST].maligntyp = u.ualignbase[1]*3;
	mons[PM_ACOLYTE].maligntyp = u.ualignbase[1]*3;
    }
}

void
onquest()
{
	if(u.uevent.qcompleted || Not_firsttime) return;
	if(!Is_special(&u.uz)) return;

	if(Is_qstart(&u.uz)) on_start();
	else if(Is_qlocate(&u.uz) && u.uz.dlevel > u.uz0.dlevel) on_locate();
	else if(Is_nemesis(&u.uz)) on_goal();
	return;
}

void
nemdead()
{
	if(!Qstat(killed_nemesis)) {
	    Qstat(killed_nemesis) = TRUE;
	    qt_pager(QT_KILLEDNEM);
	}
}

void
artitouch()
{
	if(!Qstat(touched_artifact)) {
	    Qstat(touched_artifact) = TRUE;
	    qt_pager(QT_GOTIT);
	    exercise(A_WIS, TRUE);
	}
}

/* external hook for do.c (level change check) */
boolean
ok_to_quest()
{
	return((boolean)((Qstat(got_quest) || Qstat(got_thanks)))
			&& is_pure(FALSE));
}

static boolean
not_capable()
{
	return((boolean)(u.ulevel < MIN_QUEST_LEVEL));
}

static boolean
is_pure(talk)
boolean talk;
{
    aligntyp original_alignment = u.ualignbase[1];

#ifdef WIZARD
    if (wizard && talk) {
	if (u.ualign.type != original_alignment) {
	    You("are currently %s instead of %s.",
		align_str(u.ualign.type), align_str(original_alignment));
	} else if (u.ualignbase[0] != original_alignment) {
	    You("have converted.");
	} else if (u.ualign.record < MIN_QUEST_ALIGN) {
	    You("are currently %d and require %d.",
		u.ualign.record, MIN_QUEST_ALIGN);
	    if (yn_function("adjust?", (char *)0, 'y') == 'y')
		u.ualign.record = MIN_QUEST_ALIGN;
	}
    }
#endif
    return (boolean)(u.ualign.record >= MIN_QUEST_ALIGN &&
		     u.ualign.type == original_alignment &&
		     u.ualignbase[0] == original_alignment);
}

/*
 * Expell the player to the stairs on the parent of the quest dungeon.
 *
 * This assumes that the hero is currently _in_ the quest dungeon and that
 * there is a single branch to and from it.
 */
static void
expulsion(seal)
boolean seal;
{
    branch *br;
    d_level *dest;
    int portal_flag;

    br = dungeon_branch("The Quest");
    dest = (br->end1.dnum == u.uz.dnum) ? &br->end2 : &br->end1;
    portal_flag = u.uevent.qexpelled ? 0 :	/* returned via artifact? */
		  !seal ? 1 : -1;
    schedule_goto(dest, FALSE, FALSE, portal_flag, (char *)0, (char *)0);
    if (seal)	/* remove the portal to the quest - sealing it off */
	u.uevent.qexpelled = 1;
}

static void
chat_with_leader()
{
/*	Rule 0:	Cheater checks.					*/
	if(u.uhave.questart && !Qstat(met_nemesis))
	    Qstat(cheater) = TRUE;

/*	It is possible for you to get the amulet without completing
 *	the quest.  If so, try to induce the player to quest.
 */
	if(Qstat(got_thanks)) {
/*	Rule 1:	You've gone back with/without the amulet.	*/
	    if(u.uhave.amulet)	qt_pager(QT_HASAMULET);

/*	Rule 2:	You've gone back before going for the amulet.	*/
	    else		qt_pager(QT_POSTHANKS);
	}

/*	Rule 3: You've got the artifact and are back to return it. */
	  else if(u.uhave.questart) {
	    if(u.uhave.amulet)	qt_pager(QT_HASAMULET);
	    else		qt_pager(QT_OFFEREDIT);
	    Qstat(got_thanks) = TRUE;
	    u.uevent.qcompleted = 1;	/* you did it! */

/*	Rule 4: You haven't got the artifact yet.	*/
	} else if(Qstat(got_quest)) qt_pager(rn1(10, QT_ENCOURAGE));

/*	Rule 5: You aren't yet acceptable - or are you? */
	else {
	  if(!Qstat(met_leader)) {
	    qt_pager(QT_FIRSTLEADER);
	    Qstat(met_leader) = TRUE;
	    Qstat(not_ready) = 0;
	  } else qt_pager(QT_NEXTLEADER);
	  /* the quest leader might have passed through the portal into
	     the regular dungeon; none of the remaining make sense there */
	  if (!on_level(&u.uz, &qstart_level)) return;

	  if(not_capable()) {
	    qt_pager(QT_BADLEVEL);
	    exercise(A_WIS, TRUE);
	    expulsion(FALSE);
	  } else if(!is_pure(TRUE)) {
	    qt_pager(QT_BADALIGN);
	    if(Qstat(not_ready) == MAX_QUEST_TRIES) {
	      qt_pager(QT_LASTLEADER);
	      expulsion(TRUE);
	    } else {
	      Qstat(not_ready)++;
	      exercise(A_WIS, TRUE);
	      expulsion(FALSE);
	    }
	  } else {	/* You are worthy! */
	    qt_pager(QT_ASSIGNQUEST);
	    exercise(A_WIS, TRUE);
	    Qstat(got_quest) = TRUE;
	  }
	}
}

void
leader_speaks(mtmp)
	register struct monst *mtmp;
{
	/* maybe you attacked leader? */
	if(!mtmp->mpeaceful) {
		Qstat(pissed_off) = TRUE;
		mtmp->mstrategy &= ~STRAT_WAITMASK;	/* end the inaction */
	}
	/* the quest leader might have passed through the portal into the
	   regular dungeon; if so, mustn't perform "backwards expulsion" */
	if (!on_level(&u.uz, &qstart_level)) return;

	if(Qstat(pissed_off)) {
	  qt_pager(QT_LASTLEADER);
	  expulsion(TRUE);
	} else chat_with_leader();
}

static void
chat_with_nemesis()
{
/*	The nemesis will do most of the talking, but... */
	qt_pager(rn1(10, QT_DISCOURAGE));
	if(!Qstat(met_nemesis)) Qstat(met_nemesis++);
}

void
nemesis_speaks()
{
	if(!Qstat(in_battle)) {
	  if(u.uhave.questart) qt_pager(QT_NEMWANTSIT);
	  else if(!Qstat(made_goal)) qt_pager(QT_FIRSTNEMESIS);
	  else if(Qstat(made_goal) < 3) qt_pager(QT_NEXTNEMESIS);
	  else if(Qstat(made_goal) < 7) qt_pager(QT_OTHERNEMESIS);
	  else if(!rn2(5))	qt_pager(rn1(10, QT_DISCOURAGE));
	  if(Qstat(made_goal) < 7) Qstat(made_goal)++;
	  Qstat(met_nemesis) = TRUE;
	} else /* he will spit out random maledictions */
	  if(!rn2(5))	qt_pager(rn1(10, QT_DISCOURAGE));
}

static void
chat_with_guardian()
{
/*	These guys/gals really don't have much to say... */
	qt_pager(rn1(5, QT_GUARDTALK));
}

void
quest_chat(mtmp)
	register struct monst *mtmp;
{
    switch(mtmp->data->msound) {
	    case MS_LEADER:	chat_with_leader(); break;
	    case MS_NEMESIS:	chat_with_nemesis(); break;
	    case MS_GUARDIAN:	chat_with_guardian(); break;
	    default:	impossible("quest_chat: Unknown quest character %s.",
				   mon_nam(mtmp));
	}
}

void
quest_talk(mtmp)
	register struct monst *mtmp;
{
    switch(mtmp->data->msound) {
	    case MS_LEADER:	leader_speaks(mtmp); break;
	    case MS_NEMESIS:	nemesis_speaks(); break;
	    default:		break;
	}
}

void
quest_stat_check(mtmp)
	struct monst *mtmp;
{
    if(mtmp->data->msound == MS_NEMESIS)
	Qstat(in_battle) =
	    (mtmp->mcanmove && !mtmp->msleep && monnear(mtmp, u.ux, u.uy));
}

/*quest.c*/
