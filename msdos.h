/*	SCCS Id: @(#)msdos.h	1.4	87/08/08
/* msdos.h - function declarations for msdos.c */

extern char *alllevels, *allbones;
extern char levels[], bones[], permbones[], SAVEF[], hackdir[];
extern int ramdisk, count_only;

#define CTRL(ch) (ch & 0x37)
#define ABORT CTRL('A')
#define COUNT 0x1
#define WRITE 0x2

#ifdef LINT_ARGS		/* arg. checking enabled */

void	append_slash(char *);
void	chdrive(char *);
int	check_then_creat(char *, int);
void	copybones(int);
int	dosh();
int	dotogglepickup();
long	filesize(char *);
void	flushout();
long	freediskspace(char *);
void	gameDiskPrompt();
char *	getlogin();
void	getreturn(char *);
char *	getenv();
int	getuid();
char *	let_to_name(char);
void	msexit(int);
void	msmsg(char *, ...);
void	name_file(char *, int);
void	pushch(char);
void	read_config_file();
#ifdef DGK
int	savelev(int, xchar, int);
#endif
int	saveDiskPrompt(int);
void	set_lock_and_bones();
int	tgetch();

#else

extern long	filesize(), freediskspace();
extern char	*getlogin(), *let_to_name();
extern void	append_slash(), chdrive(), copybones();
extern void	gameDiskPrompt(), getreturn(), msexit(), msmsg(), name_file();
extern void	pushch(), read_config_file(), set_lock_and_bones();

#endif
