/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

struct func_tab {
	char f_char;
	int (*f_funct)();
};

extern struct func_tab list[];
