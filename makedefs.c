/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* makedefs.c - NetHack version 1.0 */

static	char	SCCS_Id[] = "@(#)makedefs.c	1.4\t87/08/08";

#include	"config.h"
#include	<stdio.h>

#ifdef MSDOS
#undef	exit
#define	alloc	malloc
#define RDMODE	"r"
#define WRMODE	"w"
#else
#define RDMODE	"r+"
#define WRMODE	"w+"
#endif

/* construct definitions of object constants */
#define	OBJ_FILE	"objects.h"
#define	ONAME_FILE	"onames.h"
#define	TRAP_FILE	"trap.h"
#define	DATE_FILE	"date.h"
#define	RUMOR_FILE	"rumors"
#define	DATA_FILE	"data"

char	inline[256], outline[256];

main(argc, argv)
	int	argc;
	char	*argv[];
{
	char	*option;

	if(argc == 2) {
	    option = argv[1];
	    switch (option[1]) {

		case 'o':
		case 'O':	do_objs();
				break;
		case 't':
		case 'T':	do_traps();
				break;
		case 'r':
		case 'R':	do_rumors();
				break;

		case 'd':	do_data();
				break;

		case 'D':	do_date();
				break;
		default:
				fprintf(stderr, "Unknown option '%c'.\n", option[1]);
				exit(1);
	    }
	    exit(0);
	} else	fprintf(stderr, "Bad arg count (%d).\n", argc-1);
	exit(1);
}

do_traps() {
int	ntrap, getpid();
char	tmpfile[30];
FILE	*freopen();

	sprintf(tmpfile, "makedefs.%d", getpid());
	if(freopen(tmpfile, WRMODE, stdout) == NULL) {

		perror(tmpfile);
		exit(1);
	}
	if(freopen(TRAP_FILE, RDMODE, stdin) == NULL) {

		perror(TRAP_FILE);
		exit(1);
	}

	while(gets(inline) != NULL) {

	    puts(inline);
	    if(!strncmp(inline, "/* DO NOT REMOVE THIS LINE */", 29)) break;
	}
	ntrap = 10;
	printf("\n");

#ifdef NEWTRAPS
	printf("#define\tMGTRP\t\t%d\n", ntrap++);
	printf("#define\tSQBRD\t\t%d\n", ntrap++);
#endif
#ifdef SPIDERS
	printf("#define\tWEB\t\t%d\n", ntrap++);
#endif
#ifdef NEWCLASS
	printf("#define\tSPIKED_PIT\t%d\n", ntrap++);
	printf("#define\tLEVEL_TELEP\t%d\n", ntrap++);
#endif
#ifdef SPELLS
	printf("#define\tANTI_MAGIC\t%d\n", ntrap++);
#endif
#ifdef KAA
	printf("#define\tRUST_TRAP\t%d\n", ntrap++);
#endif
	printf("\n#define\tTRAPNUM\t%d\n", ntrap);
	fclose(stdin);
	fclose(stdout);
#ifdef MSDOS
	remove(TRAP_FILE);
#endif
	rename(tmpfile, TRAP_FILE);
}


struct	hline {
	struct	hline	*next;
	char	*line;
}	*f_line;

do_rumors(){
struct	hline	*c_line;
char	infile[30];
FILE	*freopen();

	if(freopen(RUMOR_FILE, WRMODE, stdout) == NULL) {

		perror(RUMOR_FILE);
		exit(1);
	}
#ifdef MSDOS
	sprintf(infile, "%s.bas", RUMOR_FILE);
#else
	sprintf(infile, "%s.base", RUMOR_FILE);
#endif
	if(freopen(infile, RDMODE, stdin) == NULL) {

		perror(infile);
		exit(1);
	}

	while(gets(inline) != NULL)	puts(inline);

#ifdef KAA
	sprintf(infile, "%s.kaa", RUMOR_FILE);
	if(freopen(infile, RDMODE, stdin) == NULL)	perror(infile);

	while(gets(inline) != NULL)	puts(inline);
#endif

#ifdef NEWCLASS
	sprintf(infile, "%s.mrx", RUMOR_FILE);
	if(freopen(infile, RDMODE, stdin) == NULL)	perror(infile);

	while(gets(inline) != NULL)	puts(inline);
#endif
	fclose(stdin);
	fclose(stdout);
}

do_date(){
int	getpid();
long	clock, time();
char	tmpfile[30], cbuf[30], *c, *ctime();
FILE	*freopen();

	sprintf(tmpfile, "makedefs.%d", getpid());
	if(freopen(tmpfile, WRMODE, stdout) == NULL) {

		perror(tmpfile);
		exit(1);
	}
	if(freopen(DATE_FILE, RDMODE, stdin) == NULL) {

		perror(DATE_FILE);
		exit(1);
	}

	while(gets(inline) != NULL) {

	    if(!strncmp(inline, "char datestring[] = ", 20)) break;
	    puts(inline);
	}
	time(&clock);
	strcpy(cbuf, ctime(&clock));
	for(c = cbuf; *c != '\n'; c++);	*c = 0; /* strip off the '\n' */
	printf("char datestring[] = %c%s%c;\n", '"', cbuf, '"');

	fclose(stdin);
	fclose(stdout);
#ifdef MSDOS
	remove(DATE_FILE);
#endif
	rename(tmpfile, DATE_FILE);
}

do_data(){
int	getpid();
char	tmpfile[30];
FILE	*freopen();

	sprintf(tmpfile, "%s.base", DATA_FILE);
	if(freopen(tmpfile, RDMODE, stdin) == NULL) {

		perror(tmpfile);
		exit(1);
	}
	if(freopen(DATA_FILE, WRMODE, stdout) == NULL) {

		perror(DATA_FILE);
		exit(1);
	}

	while(gets(inline) != NULL) {
#ifdef KOPS
	    if(!strcmp(inline, "K	a kobold"))
		printf("K\ta Keystone Kop\n");
	    else
#endif
#ifdef KAA
	    if(!strcmp(inline, "Q	a quasit"))
		printf("Q\ta quantum mechanic\n");
	    else
#endif
#ifdef ROCKMOLE
	    if(!strcmp(inline, "r	a giant rat"))
		printf("r\ta rockmole\n");
	    else
#endif
#ifdef SPIDERS
	    if(!strcmp(inline, "s	a scorpion"))
		printf("s\ta giant spider\n");
	    else if (!strcmp(inline, "\"	an amulet"))
		printf("\"\tan amulet (or a web)\n");
	    else
#endif
#ifdef	SPELLS
	    if (!strcmp(inline, "+	a door"))
		printf("+\ta door (or a spell book)\n");
	    else
#endif
#ifdef	FOUNTAINS
	    if(!strcmp(inline, "}	water filled area")) {
		puts(inline);
		printf("{\ta fountain\n");
	    } else
#endif
#ifdef NEWCLASS
	    if(!strcmp(inline, "^	a trap")) {
		puts(inline);
		printf("\\\tan opulant throne.\n");
	    } else
#endif
		puts(inline);
	}
#ifdef KAA
	printf("9\ta giant\n");
#endif
	fclose(stdin);
	fclose(stdout);
}

#define	LINSZ	1000
#define	STRSZ	40

int	fd;
struct	objdef {

	struct	objdef	*next;
	char	string[STRSZ];
}	*more, *current;

do_objs(){
register int index = 0;
register int propct = 0;
#ifdef SPELLS
register int nspell = 0;
#endif
FILE	*freopen();
register char *sp;
char	*limit();
int skip;

	fd = open(OBJ_FILE, 0);
	if(fd < 0) {
		perror(OBJ_FILE);
		exit(1);
	}

	if(freopen(ONAME_FILE, WRMODE, stdout) == NULL) {
		perror(ONAME_FILE);
		exit(1);
	}

	current = 0; newobj();
	skipuntil("objects[] = {");

	while(getentry(&skip)) {
		if(!*(current->string)){
			if (skip) index++;
			continue;
		}
		for(sp = current->string; *sp; sp++)
			if(*sp == ' ' || *sp == '\t' || *sp == '-')
				*sp = '_';

		/* Do not process duplicates caused by #ifdef/#else pairs. */
		/* M. Stephenson					   */
		if (! duplicate()) {

		    if(!strncmp(current->string, "RIN_", 4))
			    specprop(current->string+4, propct++);
		    for(sp = current->string; *sp; sp++) capitalize(sp);
		    /* avoid trouble with stupid C preprocessors */
		    if(!strncmp(current->string, "WORTHLESS_PIECE_OF_", 19))
			printf("/* #define\t%s\t%d */\n", current->string, index++);
		    else  {
#ifdef SPELLS
			if(!strncmp(current->string, "SPE_", 4))  nspell++;
			printf("#define\t%s\t%d\n", limit(current->string), index++);
#else
			if(strncmp(current->string, "SPE_", 4))
			    printf("#define\t%s\t%d\n", limit(current->string), index++);
#endif
		    }
		    newobj();
		}
	}
	printf("\n#define	CORPSE		DEAD_HUMAN\n");
#ifdef KOPS
	printf("#define	DEAD_KOP		DEAD_KOBOLD\n");
#endif
#ifdef SPIDERS
	printf("#define	DEAD_GIANT_SPIDER	DEAD_GIANT_SCORPION\n");
#endif
#ifdef ROCKMOLE
	printf("#define	DEAD_ROCKMOLE		DEAD_GIANT_RAT\n");
#endif
#ifndef KAA
	printf("#define DEAD_QUASIT		DEAD_QUANTUM_MECHANIC\n");
	printf("#define DEAD_VIOLET_FUNGI	DEAD_VIOLET_FUNGUS\n");
#endif
	printf("#define	LAST_GEM	(JADE+1)\n");
	printf("#define	LAST_RING	%d\n", propct);
#ifdef SPELLS
	printf("#define MAXSPELL	%d\n", nspell+1);
#endif
	printf("#define	NROFOBJECTS	%d\n", index-1);
	exit(0);
}

static	char	temp[32];

char *
limit(name)	/* limit a name to 30 characters length */
	char	*name;
{
	strncpy(temp, name, 30);
	temp[30] = 0;
	return(temp);
}

newobj()
{
	extern	long	*alloc();

	more = current;
	current = (struct objdef *)alloc(sizeof(struct objdef));
	current->next = more;
}

struct inherent {

	char	*attrib,
		*monsters;
}	abilities[] = { "Regeneration", "TVi",
			"See_invisible", "I",
			"Poison_resistance", "abcghikqsuvxyADFQSVWXZ&",
			"Fire_resistance", "gD&",
			"Cold_resistance", "gFY",
			"Teleportation", "LNt",
			"Teleport_control", "t",
			"", "" };

specprop(name, count)

	char	*name;
	int	count;
{
	int	i;
	char	*tname, *limit();

	tname = limit(name);
	capitalize(tname);
	for(i = 0; strlen(abilities[i].attrib); i++)
	    if(!strcmp(abilities[i].attrib, tname)) {

		printf("#define\tH%s\tu.uprops[%d].p_flgs\n", tname, count);
		printf("#define\t%s\t((H%s) || index(\"%s\", u.usym))\n",
			tname, tname, abilities[i].monsters);
		return(0);
	    }

	printf("#define\t%s\tu.uprops[%d].p_flgs\n", tname, count);
	return(0);
}

char line[LINSZ], *lp = line, *lp0 = line, *lpe = line;
int xeof;

readline(){
register int n = read(fd, lp0, (line+LINSZ)-lp0);
	if(n < 0){
		printf("Input error.\n");
		exit(1);
	}
	if(n == 0) xeof++;
	lpe = lp0+n;
}

char
nextchar(){
	if(lp == lpe){
		readline();
		lp = lp0;
	}
	return((lp == lpe) ? 0 : *lp++);
}

skipuntil(s) char *s; {
register char *sp0, *sp1;
loop:
	while(*s != nextchar())
		if(xeof) {
			printf("Cannot skipuntil %s\n", s);
			exit(1);
		}
	if(strlen(s) > lpe-lp+1){
		register char *lp1, *lp2;
		lp2 = lp;
		lp1 = lp = lp0;
		while(lp2 != lpe) *lp1++ = *lp2++;
		lp2 = lp0;	/* save value */
		lp0 = lp1;
		readline();
		lp0 = lp2;
		if(strlen(s) > lpe-lp+1) {
			printf("error in skipuntil");
			exit(1);
		}
	}
	sp0 = s+1;
	sp1 = lp;
	while(*sp0 && *sp0 == *sp1) sp0++, sp1++;
	if(!*sp0){
		lp = sp1;
		return(1);
	}
	goto loop;
}

getentry(skip) int *skip; {
int inbraces = 0, inparens = 0, stringseen = 0, commaseen = 0;
int prefix = 0;
char ch;
#define	NSZ	10
char identif[NSZ], *ip;
	current->string[0] = current->string[4] = 0;
	/* read until {...} or XXX(...) followed by ,
	   skip comment and #define lines
	   deliver 0 on failure
	 */
	while(1) {
		ch = nextchar();
	swi:
		if(letter(ch)){
			ip = identif;
			do {
				if(ip < identif+NSZ-1) *ip++ = ch;
				ch = nextchar();
			} while(letter(ch) || digit(ch));
			*ip = 0;
			while(ch == ' ' || ch == '\t') ch = nextchar();
			if(ch == '(' && !inparens && !stringseen)
				if(!strcmp(identif, "WAND") ||
				   !strcmp(identif, "RING") ||
				   !strcmp(identif, "POTION") ||
				   !strcmp(identif, "SPELL") ||
				   !strcmp(identif, "SCROLL"))
				(void) strncpy(current->string, identif, 3),
				current->string[3] = '_',
				prefix = 4;
		}
		switch(ch) {
		case '/':
			/* watch for comment */
			if((ch = nextchar()) == '*')
				skipuntil("*/");
			goto swi;
		case '{':
			inbraces++;
			continue;
		case '(':
			inparens++;
			continue;
		case '}':
			inbraces--;
			if(inbraces < 0) return(0);
			continue;
		case ')':
			inparens--;
			if(inparens < 0) {
				printf("too many ) ?");
				exit(1);
			}
			continue;
		case '\n':
			/* watch for #define at begin of line */
			if((ch = nextchar()) == '#'){
				register char pch;
				/* skip until '\n' not preceded by '\\' */
				do {
					pch = ch;
					ch = nextchar();
				} while(ch != '\n' || pch == '\\');
				continue;
			}
			goto swi;
		case ',':
			if(!inparens && !inbraces){
				if(prefix && !current->string[prefix]) {
#ifndef SPELLS
					*skip = strncmp(current->string, "SPE_", 4);
#else
					*skip = 1;
#endif
					current->string[0] = 0;
				}
				if(stringseen) return(1);
				printf("unexpected ,\n");
				exit(1);
			}
			commaseen++;
			continue;
		case '\'':
			if((ch = nextchar()) == '\\') ch = nextchar();
			if(nextchar() != '\''){
				printf("strange character denotation?\n");
				exit(1);
			}
			continue;
		case '"':
			{
				register char *sp = current->string + prefix;
				register char pch;
				register int store = (inbraces || inparens)
					&& !stringseen++ && !commaseen;
				do {
					pch = ch;
					ch = nextchar();
					if(store && sp < current->string+STRSZ)
						*sp++ = ch;
				} while(ch != '"' || pch == '\\');
				if(store) *--sp = 0;
				continue;
			}
		}
	}
}

duplicate() {

	char	s[STRSZ];
	register char	*c;
	register struct	objdef	*testobj;

	strcpy (s, current->string);
	for(c = s; *c != 0; c++) capitalize(c);

	for(testobj = more; testobj != 0; testobj = testobj->next)
		if(! strcmp(s, testobj->string)) return(1);

	return(0);
}

capitalize(sp) register char *sp; {
	if('a' <= *sp && *sp <= 'z') *sp += 'A'-'a';
}

letter(ch) register char ch; {
	return( ('a' <= ch && ch <= 'z') ||
		('A' <= ch && ch <= 'Z') );
}

digit(ch) register char ch; {
	return( '0' <= ch && ch <= '9' );
}

/* a copy of the panic code from hack.pri.c, edited for standalone use */

boolean	panicking = 0;

panic(str,a1,a2,a3,a4,a5,a6)
char *str;
{
	if(panicking++) exit(1);    /* avoid loops - this should never happen*/
	fputs(" ERROR:  ", stdout);
	printf(str,a1,a2,a3,a4,a5,a6);
#ifdef DEBUG
# ifdef UNIX
	if(!fork())
		abort();	/* generate core dump */
# endif
#endif
	exit(1);
}

#if defined(SYSV) || defined(GENIX)
rename(oldname, newname)
	char	*oldname, *newname;
{
	if (strcmp(oldname, newname)) {

		unlink(newname);
		link(oldname, newname);
		unlink(oldname);
	}
}
#endif

#ifdef __TURBOC__
int getpid() {
	return(1);
}
#endif
