# 1 "hack.u_init.c"




# 1 "./hack.h"




# 1 "./config.h"
























	

	
	









































































typedef	char	schar;








typedef	unsigned char	uchar;






typedef schar	xchar;
typedef	xchar	boolean;		















# 5 "./hack.h"



# 1 "/usr/include/strings.h"






char	*strcat();
char	*strncat();
int	strcmp();
int	strncmp();
char	*strcpy();
char	*strncpy();
int	strlen();
char	*index();
char	*rindex();
# 8 "./hack.h"
# 12 "./hack.h"





# 1 "./def.objclass.h"





struct objclass {
	char *oc_name;		
	char *oc_descr;		
	char *oc_uname;		
		unsigned oc_name_known:1;
		unsigned oc_merge:1;	
	char oc_olet;
	schar oc_prob;		
	schar oc_delay;		
	uchar oc_weight;
	schar oc_oc1, oc_oc2;
	int oc_oi;





				



				




};

extern struct objclass objects[];


























# 17 "./hack.h"

typedef struct {
	xchar x,y;
} coord;


# 1 "./def.monst.h"



struct monst {
	struct monst *nmon;
	struct permonst *data;
	unsigned m_id;
	xchar mx,my;
	xchar mdx,mdy;		

	coord mtrack[4];	
	schar mhp,mhpmax;
	char mappearance;	
		unsigned mimic:1;	
		unsigned mdispl:1;	
		unsigned minvis:1;	
		unsigned cham:1;	
		unsigned mhide:1;	
		unsigned mundetected:1;	
		unsigned mspeed:2;
		unsigned msleep:1;
		unsigned mfroz:1;
		unsigned mconf:1;
		unsigned mflee:1;	
		unsigned mfleetim:7;	
		unsigned mcan:1;	
		unsigned mtame:1;		
		unsigned mpeaceful:1;	
		unsigned isshk:1;	
		unsigned isgd:1;	
		unsigned mcansee:1;	
		unsigned mblinded:7;	
		unsigned mtrapped:1;	
		unsigned mnamelth:6;	

		unsigned wormno:5;	

	unsigned mtrapseen;	
	long mlstmv;	
	struct obj *minvent;
	long mgold;
	unsigned mxlth;		
	

	long mextra[1];		
};



extern struct monst *fmon;
extern struct monst *fallen_down;
struct monst *m_at();








# 23 "./hack.h"

# 1 "./def.gold.h"



struct gold {
	struct gold *ngold;
	xchar gx,gy;
	long amount;
};

extern struct gold *fgold;
struct gold *g_at();

# 24 "./hack.h"

# 1 "./def.trap.h"



struct trap {
	struct trap *ntrap;
	xchar tx,ty;
	unsigned ttyp:5;
	unsigned tseen:1;
	unsigned once:1;
};

extern struct trap *ftrap;
struct trap *t_at();













				
# 25 "./hack.h"

# 1 "./def.obj.h"



struct obj {
	struct obj *nobj;
	unsigned o_id;
	unsigned o_cnt_id;		
	xchar ox,oy;
	xchar odx,ody;
	uchar otyp;
	uchar owt;
	uchar quan;		
	schar spe;		


	char olet;
	char invlet;
		unsigned oinvis:1;	
		unsigned odispl:1;
		unsigned known:1;	
		unsigned dknown:1;	
		unsigned cursed:1;
		unsigned unpaid:1;	
		unsigned rustfree:1;
		unsigned onamelth:6;
	long age;		
	long owornmask;












	long oextra[1];		

};

extern struct obj *fobj;




# 26 "./hack.h"

# 1 "./def.flag.h"



struct flag {
	unsigned ident;		
	unsigned debug:1;	

	unsigned toplin:2;	
				
	unsigned cbreak:1;	
	unsigned standout:1;	
	unsigned nonull:1;	
	unsigned time:1;	
	unsigned nonews:1;	
	unsigned notombstone:1;
	unsigned end_top, end_around;	
	unsigned end_own:1;		
	unsigned no_rest_on_space:1;	
	unsigned beginner:1;
	unsigned female:1;
	unsigned invlet_constant:1;	

	unsigned move:1;
	unsigned mv:1;
	unsigned run:3;		
				
	unsigned nopick:1;	
	unsigned echo:1;	
	unsigned botl:1;	
	unsigned botlx:1;	
	unsigned nscrinh:1;	
	unsigned made_amulet:1;
	unsigned no_of_wizards:2;
				
	unsigned moonphase:3;



};

extern struct flag flags;

# 27 "./hack.h"

extern char *sprintf();






# 1 "./def.rm.h"










			



















# 33 "./def.rm.h"












struct rm {
	char scrsym;
	unsigned typ:5;
	unsigned new:1;
	unsigned seen:1;
	unsigned lit:1;
};
extern struct rm levl[80][22];
# 35 "./hack.h"

# 1 "./def.permonst.h"



struct permonst {
	char *mname,mlet;
	schar mlevel,mmove,ac,damn,damd;
	unsigned pxlth;
};

extern struct permonst mons[];















# 36 "./hack.h"

extern long *alloc();

extern xchar xdnstair, ydnstair, xupstair, yupstair; 

extern xchar dlevel;


# 1 "./hack.onames.h"



































































































































































































































# 44 "./hack.h"




extern struct obj *invent, *uwep, *uarm, *uarm2, *uarmh, *uarms, *uarmg, 
	*uleft, *uright, *fcobj;
extern struct obj *uchain;	
extern struct obj *uball;	
struct obj *o_at(), *getobj(), *sobj_at();

struct prop {







	long p_flgs;
	int (*p_tofn)();	
};

struct you {
	xchar ux, uy;
	schar dx, dy, dz;	
# 72 "./hack.h"

	xchar udisx, udisy;	
	char usym;		
	schar uluck;


	int last_str_turn:3;	
				
	unsigned udispl:1;	
	unsigned ulevel:4;	
# 84 "./hack.h"

	unsigned utrap:3;	
	unsigned utraptype:1;	


	unsigned uinshop:6;	

























	unsigned umconf:1;
	char *usick_cause;
	struct prop uprops[19+10];

	unsigned uswallow:1;		
	unsigned uswldtim:4;		
	unsigned uhs:3;			
	schar ustr,ustrmax;
	schar udaminc;
	schar uac;
	int uhp,uhpmax;
	long int ugold,ugold0,uexp,urexp;
	int uhunger;			
	int uinvault;
	struct monst *ustuck;
	int nr_killed[	55		+2];		
};

extern struct you u;

extern char *traps[];
extern char *monnam(), *Monnam(), *amonnam(), *Amonnam(),
	*doname(), *aobjnam();
extern char readchar();
extern char vowels[];

extern xchar curx,cury;	

extern coord bhitpos;	

extern xchar seehx,seelx,seehy,seely; 
extern char *save_cm,*killer;

extern xchar dlevel, maxdlevel; 

extern long moves;

extern int multi;


extern char lock[];








# 5 "hack.u_init.c"

# 1 "/usr/include/stdio.h"




extern	struct	_iobuf {
	int	_cnt;
	char	*_ptr;
	char	*_base;
	int	_bufsiz;
	short	_flag;
	char	_file;
} _iob[20];


























struct _iobuf	*fopen();
struct _iobuf	*fdopen();
struct _iobuf	*freopen();
long	ftell();
char	*fgets();

char	*sprintf();		

# 6 "hack.u_init.c"

# 1 "/usr/include/signal.h"
















































int	(*signal())();





struct	sigvec {
	int	(*sv_handler)();	
	int	sv_mask;		
	int	sv_onstack;		
};




struct	sigstack {
	char	*ss_sp;			
	int	ss_onstack;		
};








struct	sigcontext {
	int	sc_onstack;		
	int	sc_mask;		
	int	sc_sp;			
	int	sc_pc;			
	int	sc_ps;			
};





# 91 "/usr/include/signal.h"


# 7 "hack.u_init.c"




extern struct obj *addinv();
extern char *eos();
extern char plname[];

struct you zerou;
char pl_character[	20	];
char *(roles[]) = {	
			
	"Tourist", "Speleologist", "Fighter", "Knight",
	"Cave-man", "Wizard"
};

char rolesyms[	(int)(sizeof(roles) / sizeof(roles[0])) + 1];		

struct trobj {
	uchar trotyp;
	schar trspe;
	char trolet;
		unsigned trquan:6;
		unsigned trknown:1;
};


struct trobj Extra_objs[] = {
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 }
};


struct trobj Cave_man[] = {
	{ 77, 1, ')', 1, 1 },
	{ 86, 1, ')', 1, 1 },
	{ 71, 0, ')', 25, 1 },	
	{ 106, 0, '[', 1, 1 },
	{ 0, 0, 0, 0, 0}
};

struct trobj Fighter[] = {
	{ 81, 0, ')', 1, 1 },
	{ 104, 0, '[', 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

struct trobj Knight[] = {
	{ 80, 0, ')', 1, 1 },
	{ 85, 2, ')', 1, 1 },
	{ 104, 1, '[', 1, 1 },
	{ 98, 0, '[', 1, 1 },
	{ 108, 0, '[', 1, 1 },
	{ 109, 0, '[', 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

struct trobj Speleologist[] = {
	{ 105, 0, '[', 1, 1 },
	{ 0, 0, '!', 2, 0 },
	{ 2, 0, '%', 3, 1 },
	{ 93, '\177', '(', 1, 0 },
	{ 92, 0, '(', 1, 0 },
	{ 0, 0, 0, 0, 0}
};

struct trobj Tinopener[] = {
	{ 94, 0, '(', 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

struct trobj Tourist[] = {
	{ 0, 0, '%', 10, 1 },
	{ 124, 0, '!', 2, 0 },
	{ 91, 0, '(', 1, 1 },
	{ 74, 2, ')', 25, 1 },	
	{ 0, 0, 0, 0, 0 }
};

struct trobj Wizard[] = {
	{ 107, 0, '[', 1, 1 },
	{ 0, '\177', '/', 2, 0 },
	{ 0, '\177', '=', 2, 0 },
	{ 0, '\177', '!', 2, 0 },
	{ 0, '\177', '?', 3, 0 },
	{ 0, 0, 0, 0, 0 }
};

u_init(){
register int i;
char exper = 'y', pc;
extern char readchar();
	if(flags.female)	
		roles[4] = "Cave-woman";
	for(i = 0; i < 	(int)(sizeof(roles) / sizeof(roles[0])); i++)
		rolesyms[i] = roles[i][0];
	rolesyms[i] = 0;

	if(pc = pl_character[0]) {
		if('a' <= pc && pc <= 'z') pc += 'A'-'a';
		if((i = role_index(pc)) >= 0)
			goto got_suffix;	
		printf("\nUnknown role: %c\n", pc);
		pl_character[0] = pc = 0;
	}

	printf("\nAre you an experienced player? [ny] ");

	while(!index("ynYN \n\004", (exper = readchar())))
		bell();
	if(exper == '\004')		
		end_of_input();
	printf("%c\n", exper);		
	if(index("Nn \n", exper)) {
		exper = 0;
		goto beginner;
	}

	printf("\nTell me what kind of character you are:\n");
	printf("Are you");
	for(i = 0; i < 	(int)(sizeof(roles) / sizeof(roles[0])); i++) {
		printf(" a %s", roles[i]);
		if(i == 2)			
			printf(",\n\t");
		else if(i < 	(int)(sizeof(roles) / sizeof(roles[0])) - 2)
			printf(",");
		else if(i == 	(int)(sizeof(roles) / sizeof(roles[0])) - 2)
			printf(" or");
	}
	printf("? [%s] ", rolesyms);

	while(pc = readchar()) {
		if('a' <= pc && pc <= 'z') pc += 'A'-'a';
		if((i = role_index(pc)) >= 0) {
			printf("%c\n", pc);	
			(void) fflush((&_iob[1]));	
			break;
		}
		if(pc == '\n')
			break;
		if(pc == '\004')    
			end_of_input();
		bell();
	}
	if(pc == '\n')
		pc = 0;

beginner:
	if(!pc) {
		printf("\nI'll choose a character for you.\n");
		i = rn2(	(int)(sizeof(roles) / sizeof(roles[0])));
		pc = rolesyms[i];
		printf("This game you will be a%s %s.\n",
			exper ? "n experienced" : "",
			roles[i]);
		getret();
		
		(void) 	 (--((&_iob[1]))->_cnt>=0? ((int)(*((&_iob[1]))->_ptr++=(unsigned)('\n'))):_flsbuf((unsigned)('\n'),(&_iob[1])));
		(void) fflush((&_iob[1]));
	}
	if(exper) {
		roles[i][0] = pc;
	}

got_suffix:

	(void) strncpy(pl_character, roles[i], 	20	-1);
	pl_character[	20	-1] = 0;
	flags.beginner = 1;
	u = zerou;
	u.usym = '@';
	u.ulevel = 1;
	init_uhunger();
# 182 "hack.u_init.c"

	uarm = uarm2 = uarmh = uarms = uarmg = uwep = uball = uchain =
	uleft = uright = 0;

	switch(pc) {
	case 'c':
	case 'C':
		Cave_man[2].trquan = 12 + rnd(9)*rnd(9);
		u.uhp = u.uhpmax = 16;
		u.ustr = u.ustrmax = 18;
		ini_inv(Cave_man);
		break;
	case 't':
	case 'T':
		Tourist[3].trquan = 20 + rnd(20);
		u.ugold = u.ugold0 = rnd(1000);
		u.uhp = u.uhpmax = 10;
		u.ustr = u.ustrmax = 8;
		ini_inv(Tourist);
		if(!rn2(25)) ini_inv(Tinopener);
		break;
	case 'w':
	case 'W':
		for(i=1; i<=4; i++) if(!rn2(5))
			Wizard[i].trquan += rn2(3) - 1;
		u.uhp = u.uhpmax = 15;
		u.ustr = u.ustrmax = 16;
		ini_inv(Wizard);
		break;
	case 's':
	case 'S':
			u.uprops[	(19+1)		].p_flgs = 040000L;
		u.uprops[5].p_flgs = 040000L;
		u.uhp = u.uhpmax = 12;
		u.ustr = u.ustrmax = 10;
		ini_inv(Speleologist);
		if(!rn2(10)) ini_inv(Tinopener);
		break;
	case 'k':
	case 'K':
		u.uhp = u.uhpmax = 12;
		u.ustr = u.ustrmax = 10;
		ini_inv(Knight);
		break;
	case 'f':
	case 'F':
		u.uhp = u.uhpmax = 14;
		u.ustr = u.ustrmax = 17;
		ini_inv(Fighter);
		break;
	default:	
		u.uhp = u.uhpmax = 12;
		u.ustr = u.ustrmax = 16;
	}
	find_ac();
	if(!rn2(20)) {
		register int d = rn2(7) - 2;	
		u.ustr += d;
		u.ustrmax += d;
	}


	if(flags.debug) wiz_inv();


	
	while(inv_weight() > 0 && u.ustr < 118)
		u.ustr++, u.ustrmax++;
}

ini_inv(trop) register struct trobj *trop; {
register struct obj *obj;
extern struct obj *mkobj();
	while(trop->trolet) {
		obj = mkobj(trop->trolet);
		obj->known = trop->trknown;
		
		obj->cursed = 0;
		if(obj->olet == ')'){
			obj->quan = trop->trquan;
			trop->trquan = 1;
		}
		if(trop->trspe != '\177')
			obj->spe = trop->trspe;
		if(trop->trotyp != 0)
			obj->otyp = trop->trotyp;
		else
			if(obj->otyp == 158)	
				obj->otyp = 172;
		obj->owt = weight(obj);	
		obj = addinv(obj);
		if(obj->olet == '['){
			switch(obj->otyp){
			case 108:
				if(!uarms) setworn(obj, 010L);
				break;
			case 98:
				if(!uarmh) setworn(obj, 04L);
				break;
			case 109:
				if(!uarmg) setworn(obj, 020L);
				break;
			case 107:
				if(!uarm2)
					setworn(obj, 01L);
				break;
			default:
				if(!uarm) setworn(obj, 01L);
			}
		}
		if(obj->olet == ')')
			if(!uwep) setuwep(obj);

		if(--trop->trquan) continue;	
# 302 "hack.u_init.c"

		trop++;
	}
}


wiz_inv(){
register struct trobj *trop = &Extra_objs[0];
extern char *getenv();
register char *ep = getenv("INVENT");
register int type;
	while(ep && *ep) {
		type = atoi(ep);
		ep = index(ep, ',');
		if(ep) while(*ep == ',' || *ep == ' ') ep++;
		if(type <= 0 || type > 215) continue;
		trop->trotyp = type;
		trop->trolet = objects[type].oc_olet;
		trop->trspe = 4;
		trop->trknown = 1;
		trop->trquan = 1;
		ini_inv(trop);
	}
	
	trop->trotyp = 158;
	trop->trolet = '/';
	trop->trspe = 20;
	trop->trknown = 1;
	trop->trquan = 1;
	ini_inv(trop);
}


plnamesuffix() {
register char *p;
	if(p = rindex(plname, '-')) {
		*p = 0;
		pl_character[0] = p[1];
		pl_character[1] = 0;
		if(!plname[0]) {
			askname();
			plnamesuffix();
		}
	}
}

role_index(pc)
char pc;
{		
		
	register char *cp;

	if(cp = index(rolesyms, pc))
		return(cp - rolesyms);
	return(-1);
}
