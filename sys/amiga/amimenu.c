/*    SCCS Id: @(#)amimenu.c    3.2    96/02/04			   */
/*    Copyright (c) Olaf 'Rhialto' Seibert, 1989		   */
/*    Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1992, 1993, 1996 */
/* NetHack may be freely redistributed.  See license for details.  */

/*  Originally by John Toebes.  */

#define TEXT(nam,str) \
static struct IntuiText nam = \
  {1,0,JAM2,0,0,0L,(UBYTE*)str,0L}  /* 0,1 == C_BLACK,C_WHITE */

    /* Commands */
    TEXT(T_HELP,  "?     Display help menu");
    TEXT(T_amp,   "&     Explain a command");
    TEXT(T_O,     "O     Set options");
    TEXT(T_SHELL, "!     AmigaDos commands");
    TEXT(T_v,     "v     Version number");
    TEXT(T_CR,    "^R    Redraw screen");
    TEXT(T_CP,    "^P    Repeat last message");
    TEXT(T_Q,     "#quit Quit game");
    TEXT(T_S,     "S     Save the game");

    /* Inventory */
    TEXT(T_i,     "i   Inventory");
    TEXT(T_p,     "p   Pay your bill");
    TEXT(T_d,     "d   Drop an object");
    TEXT(T_D,     "D   Drop several things");
    TEXT(T_COMMA, ",   Pickup an object");
    TEXT(T_AT,    "@   Toggle pickup");
    TEXT(T_SLASH, "/   Identify something");
    TEXT(T_C,     "C   Christen a monster");

    /* Actions */
    TEXT(T_a,     "a   Apply/use something");
    TEXT(T_e,     "e   Eat something");
    TEXT(T_q,     "q   Quaff a potion");
    TEXT(T_r,     "r   Read scroll/book");
    TEXT(T_t,     "t   Throw/shoot weapon");
    TEXT(T_z,     "z   Zap a wand");
    TEXT(T_Z,     "Z   Cast a spell");

    /* Preparations */
    TEXT(T_w,     "w   Wield a weapon");
    TEXT(T_P,     "P   Put on ring");
    TEXT(T_R,     "R   Remove ring");
    TEXT(T_T,     "T   Take off armor");
    TEXT(T_W,     "W   Wear armor");
    TEXT(T_WPN,   ")   Current weapon");
    TEXT(T_ARMOR, "[   Current armor");
    TEXT(T_RING,  "=   Current rings");
    TEXT(T_AMU,  "\"   Current amulet");
    TEXT(T_TOOL,  "(   Current tools");

    /* Movement */
    TEXT(T_o,     "o   Open door");
    TEXT(T_c,     "c   Close door");
    TEXT(T_KICK,  "^D  Kick door");
    TEXT(T_s,     "s   Search");
    TEXT(T_UP,    "<   Go up stairs");
    TEXT(T_DOWN,  ">   Go down stairs");
    TEXT(T_CT,    "^T  Teleport");
    TEXT(T_WAIT,  ".   Wait a moment");
    TEXT(T_E,     "E   Engrave msg on floor");

    /* Extended */
    TEXT(T_Ma, "M-a  #adjust inventory letters");
    TEXT(T_Mc, "M-c  #chat with someone");
    TEXT(T_Md, "M-d  #dip an object into something");
#ifdef WEAPON_SKILLS
    TEXT(T_Me, "M-e  #enhance weapon skills");
#endif
    TEXT(T_Mf, "M-f  #force a lock");
    TEXT(T_Mi, "M-i  #invoke an object's special powers");
    TEXT(T_Mj, "M-j  #jump to another location");
    TEXT(T_Ml, "M-l  #loot a box on the floor");
    TEXT(T_Mm, "M-m  Use a #monster's special ability");
    TEXT(T_Mn, "M-n  #name an item or type of object");
    TEXT(T_Mo, "M-o  #offer a sacrifice to the gods");
    TEXT(T_Mp, "M-p  #pray to the gods for help");
    TEXT(T_Mr, "M-r  #rub a lamp");
    TEXT(T_Ms, "M-s  #sit down");
    TEXT(T_Mt, "M-t  #turn undead");
    TEXT(T_Mu, "M-u  #untrap something");
    TEXT(T_Mv, "M-v  long #version information");
    TEXT(T_Mw, "M-w  #wipe off your face");

#define IFLAGS ITEMENABLED|ITEMTEXT|HIGHCOMP
#define IDATA(cmd,str,off) \
    MOFF,off,WDT,9,IFLAGS,0,(APTR)&str,(APTR)0,(signed char)(cmd),0L,0

/* Commands */

#define MOFF 0

#undef  WDT
#define WDT 184

static struct MenuItem cmdsub[] = {
    { &cmdsub[1], IDATA('?', T_HELP,   0) },	/*   Display help */
    { &cmdsub[2], IDATA('&', T_amp,   10) },	/*   Explain a command */
    { &cmdsub[3], IDATA('O', T_O,     20) },	/*   Set options */
    { &cmdsub[4], IDATA('!', T_SHELL, 30) },	/*   AmigaDos commands */
    { &cmdsub[5], IDATA('v', T_v,     40) },	/*   Version number */
    { &cmdsub[6], IDATA(022, T_CR,    50) },	/*R  Redraw screen */
    { &cmdsub[7], IDATA(020 ,T_CP,    60) },	/*P  Repeat last message */
    { &cmdsub[8], IDATA('Q', T_Q,     70) },	/*   Quit game */
    { NULL,   IDATA('S', T_S,     80) },	/*   Save the game */
};

/* Inventory */

#undef  WDT
#define WDT 184

static struct MenuItem invsub[] = {
    { &invsub[1], IDATA('i', T_i,      0) },	/*   Inventory */
    { &invsub[2], IDATA('p', T_p,     10) },	/*   Pay your bill */
    { &invsub[3], IDATA('d', T_d,     20) },	/*   Drop an object */
    { &invsub[4], IDATA('D', T_D,     30) },	/*   Drop several things */
    { &invsub[5], IDATA(',', T_COMMA, 40) },	/*   Pickup an object */
    { &invsub[6], IDATA('/', T_SLASH, 50) },	/*   Identify something */
    { NULL,   IDATA('C', T_C,     60) },	/*   Christen a monster */
};

/* Actions */

#undef  WDT
#define WDT 184

static struct MenuItem actsub[] = {
    { &actsub[1], IDATA('a', T_a,     0) },	/*   Apply/use something */
    { &actsub[2], IDATA('e', T_e,    10) },	/*   Eat something */
    { &actsub[3], IDATA('q', T_q,    20) },	/*   Quaff a potion */
    { &actsub[4], IDATA('r', T_r,    30) },	/*   Read a scroll/spellbook */
    { &actsub[5], IDATA('t', T_t,    40) },	/*   Throw/shoot weapon */
    { &actsub[6], IDATA('z', T_z,    50) },	/*   Zap a wand */
    { NULL      , IDATA('Z', T_Z,    60) },	/*   Cast a spell */
};

/* Preparations */

#undef  WDT
#define WDT 144

static struct MenuItem armsub[] = {
    { &armsub[1], IDATA('w', T_w,      0) },	/*   Wield a weapon */
    { &armsub[2], IDATA('R', T_R,     10) },	/*   Remove ring */
    { &armsub[3], IDATA('P', T_P,     20) },	/*   Put on ring */
    { &armsub[4], IDATA('T', T_T,     30) },	/*   Take off armor */
    { &armsub[5], IDATA('W', T_W,     40) },	/*   Wear armor */
    { &armsub[6], IDATA(')', T_WPN,   50) },	/*   Current weapon */
    { &armsub[7], IDATA('[', T_ARMOR, 60) },	/*   Current armor */
    { &armsub[8], IDATA('=', T_RING,  70) },	/*   Current rings */
    { &armsub[9], IDATA('"', T_AMU,   80) },	/*   Current amulet */
    { NULL  , IDATA('(', T_TOOL,  90) },	/*   Current tools */
};

/* Movement */

#undef  WDT
#define WDT 192

static struct MenuItem movsub[] = {
    { &movsub[1], IDATA('o', T_o,     0) },	/*   Open door */
    { &movsub[2], IDATA('c', T_c,    10) },	/*   Close door */
    { &movsub[3], IDATA(004, T_KICK, 20) },	/*D  Kick door */
    { &movsub[4], IDATA('s', T_s,    30) },	/*   Search */
    { &movsub[5], IDATA('<', T_UP,   40) },	/*   Go up stairs */
    { &movsub[6], IDATA('>', T_DOWN, 50) },	/*   Go down stairs */
    { &movsub[7], IDATA(024, T_CT,   60) },	/*T  Teleport */
    { &movsub[8], IDATA('.', T_WAIT, 70) },	/*   Wait a moment */
    { NULL  , IDATA('E', T_E,    80) },		/*   Engrave msg on floor */
};

#undef	WDT
#define WDT 312

#undef MOFF
#define MOFF -(442+312+8+3-640)

#ifdef WEAPON_SKILLS
# define WS1	1
# define WS10	10
#else
# define WS1	0
# define WS10	0
#endif

static struct MenuItem extsub[] = {
    { &extsub[ 1], IDATA(128+'a', T_Ma,   0) },
    { &extsub[ 2], IDATA(128+'c', T_Mc,  10) },
    { &extsub[ 3], IDATA(128+'d', T_Md,  20) },
#ifdef WEAPON_SKILLS
    { &extsub[ 4], IDATA(128+'e', T_Me,  30) },
#endif
    { &extsub[ 4+WS1], IDATA(128+'f', T_Mf,  30+WS10) },
    { &extsub[ 5+WS1], IDATA(128+'i', T_Mi,  40+WS10) },
    { &extsub[ 6+WS1], IDATA(128+'j', T_Mj,  50+WS10) },
    { &extsub[ 7+WS1], IDATA(128+'l', T_Ml,  60+WS10) },
    { &extsub[ 8+WS1], IDATA(128+'m', T_Mm,  70+WS10) },
    { &extsub[ 9+WS1], IDATA(128+'n', T_Mn,  80+WS10) },
    { &extsub[10+WS1], IDATA(128+'o', T_Mo,  90+WS10) },
    { &extsub[11+WS1], IDATA(128+'p', T_Mp, 100+WS10) },
    { &extsub[12+WS1], IDATA(128+'r', T_Mr, 110+WS10) },
    { &extsub[13+WS1], IDATA(128+'s', T_Ms, 120+WS10) },
    { &extsub[14+WS1], IDATA(128+'t', T_Mt, 130+WS10) },
    { &extsub[15+WS1], IDATA(128+'u', T_Mu, 140+WS10) },
    { &extsub[16+WS1], IDATA(128+'v', T_Mv, 150+WS10) },
    { NULL       , IDATA(128+'w', T_Mw, 160+WS10) },
};
#undef WS1
#undef WS10
#undef WDT
#undef MOFF

/* Menustrip */

/* Width = #letters * 8 + 8 + 10 */

struct Menu HackMenu[] = {
   { &HackMenu[1], 0,0, 72,0,MENUENABLED,"Commands",     &cmdsub[0] }, /*8*/
   { &HackMenu[2], 82,0, 80,0,MENUENABLED,"Inventory",    &invsub[0] }, /*9*/
   { &HackMenu[3],172,0, 64,0,MENUENABLED,"Actions",      &actsub[0] }, /*7*/
   { &HackMenu[4],246,0,104,0,MENUENABLED,"Preparations", &armsub[0] }, /*12*/
   { &HackMenu[5],360,0, 72,0,MENUENABLED,"Movement",     &movsub[0] },	/*8*/
   { NULL,   442,0, 72,0,MENUENABLED,"Extended", &extsub[0] }, /*8*/
};
