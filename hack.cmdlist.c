/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include	"config.h"
#include	"def.objclass.h"
#include	"def.func_tab.h"

int doredraw(),doredotopl(),dodrop(),dodrink(),doread(),dosearch(),
doversion(),doweararm(),dowearring(),doremarm(),doremring(),dopay(),doapply(),
dosave(),dowield(),ddoinv(),dozap(),ddocall(),dowhatis(),doengrave(),
dohelp(),doeat(),doddrop(),do_mname(),doidtrap(),doprwep(),doprarm(),doprring();
#ifdef SHELL
int dosh();
#endif SHELL
#ifdef OPTIONS
int doset();
#endif OPTIONS
int doup(), dodown(), done1(), donull();
int dothrow();
struct func_tab list[]={
	'\022', doredraw,
	'\020', doredotopl,
	'a', doapply,
/*	'A' : UNUSED */
/*	'b', 'B' : go sw */
	'c', ddocall,
	'C', do_mname,
	'd', dodrop,
	'D', doddrop,
	'e', doeat,
	'E', doengrave,
/*	'f', 'F' : multiple go (might become 'fight') */
/*	'g', 'G' : UNUSED */
/*	'h', 'H' : go west */
	'i', ddoinv,
/*	'I' : UNUSED */
/*	'j', 'J', 'k', 'K', 'l', 'L', 'm', 'M', 'n', 'N' : move commands */
#ifdef OPTIONS
	'o', doset,
#endif OPTIONS
/*	'O' : UNUSED */
	'p', dopay,
	'P', dowearring,
	'q', dodrink,
	'Q', done1,
	'r', doread,
	'R', doremring,
	's', dosearch,
	'S', dosave,
	't', dothrow,
	'T', doremarm,
/*	'u', 'U' : go ne */
	'v', doversion,
/*	'V' : UNUSED */
	'w', dowield,
	'W', doweararm,
/*	'x', 'X' : UNUSED */
/*	'y', 'Y' : go nw */
	'z', dozap,
/*	'Z' : UNUSED */
	'<', doup,
	'>', dodown,
	'/', dowhatis,
	'?', dohelp,
#ifdef SHELL
	'!', dosh,
#endif SHELL
	'.', donull,
	' ', donull,
	'^', doidtrap,
	 WEAPON_SYM,  doprwep,
	 ARMOR_SYM,  doprarm,
	 RING_SYM,  doprring,
	0,0,0
};
