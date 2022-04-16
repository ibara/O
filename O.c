/*
 * Copyright (c) 2022 Brian Callahan <bcallah@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct peephole {
	char *line1;
	char *line2;
	char *line3;
};

static char *
xstrdup(const char *s)
{
	char *p;

	if (s == NULL)
		return NULL;

	if ((p = strdup(s)) == NULL) {
		(void) fputs("O: error: xstrdup failed\n", stderr);

		exit(1);
	}

	return p;
}

static int
xorq(struct peephole *window)
{
	char buf[32], r1a, r1b;

	if (window->line1 == NULL)
		return 0;

	if (!strncmp("\tmovq $0, %r", window->line1, 12)) {
		if (strlen(window->line1) < 14)
			return 0;

		r1a = window->line1[12];
		r1b = window->line1[13];

		if (r1b == '\n') {
			(void) snprintf(buf, sizeof(buf),
			    "\txorl %%r%cd, %%r%cd\n", r1a, r1a);
		} else if (r1a == '1') {
			(void) snprintf(buf, sizeof(buf),
			    "\txorl %%r%c%cd, %%r%c%cd\n", r1a, r1b, r1a, r1b);
		} else {
			(void) snprintf(buf, sizeof(buf),
			    "\txorl %%e%c%c, %%e%c%c\n", r1a, r1b, r1a, r1b);
		}

		free(window->line1);
		window->line1 = xstrdup(buf);

		return 1;
	}

	return 0;
}

static int
xorl(struct peephole *window)
{
	char buf[32], e1a, e1b;

	if (window->line1 == NULL)
		return 0;

	if (!strncmp("\tmovl $0, %e", window->line1, 12)) {
		if (strlen(window->line1) != 15)
			return 0;

		e1a = window->line1[12];
		e1b = window->line1[13];

		(void) snprintf(buf, sizeof(buf), "\txorl %%e%c%c, %%e%c%c\n",
		    e1a, e1b, e1a, e1b);

		free(window->line1);
		window->line1 = xstrdup(buf);

		return 1;
	} else if (!strncmp("\tmovl $0, %r", window->line1, 12)) {
		if (strlen(window->line1) < 14)
			return 0;

		e1a = window->line1[12];
		e1b = window->line1[13];

		if (e1b == 'd') {
			(void) snprintf(buf, sizeof(buf),
			    "\txorl %%r%cd, %%r%cd\n", e1a, e1a);
		} else {
			(void) snprintf(buf, sizeof(buf),
			    "\txorl %%r%c%cd, %%r%c%cd\n", e1a, e1b, e1a, e1b);
		}

		free(window->line1);
		window->line1 = xstrdup(buf);

		return 1;
	}

	return 0;
}

static int
incq(struct peephole *window)
{
	char buf[32], r1a, r1b;

	if (window->line1 == NULL)
		return 0;

	if (!strncmp("\taddq $1, %r", window->line1, 12)) {
		if (strlen(window->line1) < 14)
			return 0;

		r1a = window->line1[12];
		r1b = window->line1[13];

		if (r1b == '\n') {
			(void) snprintf(buf, sizeof(buf), "\tincq %%r%c\n",
			    r1a);
		} else {
			(void) snprintf(buf, sizeof(buf), "\tincq %%r%c%c\n",
			    r1a, r1b);
		}

		free(window->line1);
		window->line1 = xstrdup(buf);

		return 1;
	}

	return 0;
}

static int
incl(struct peephole *window)
{
	char buf[32], e1a, e1b;

	if (window->line1 == NULL)
		return 0;

	if (!strncmp("\taddl $1, %e", window->line1, 12)) {
		if (strlen(window->line1) != 15)
			return 0;

		e1a = window->line1[12];
		e1b = window->line1[13];

		(void) snprintf(buf, sizeof(buf), "\tincl %%e%c%c\n", e1a,
		    e1b);

		free(window->line1);
		window->line1 = xstrdup(buf);

		return 1;
	} else if (!strncmp("\taddl $1, %r", window->line1, 12)) {
		if (strlen(window->line1) < 14)
			return 0;

		e1a = window->line1[12];
		e1b = window->line1[13];

		if (e1b == 'd') {
			(void) snprintf(buf, sizeof(buf), "\tincl %%r%cd\n",
			    e1a);
		} else {
			(void) snprintf(buf, sizeof(buf), "\tincl %%r%c%cd\n",
			    e1a, e1b);
		}

		free(window->line1);
		window->line1 = xstrdup(buf);

		return 1;
	}

	return 0;
}

static int
decq(struct peephole *window)
{
	char buf[32], r1a, r1b;

	if (window->line1 == NULL)
		return 0;

	if (!strncmp("\tsubq $1, %r", window->line1, 12)) {
		if (strlen(window->line1) < 14)
			return 0;

		r1a = window->line1[12];
		r1b = window->line1[13];

		if (r1b == '\n') {
			(void) snprintf(buf, sizeof(buf), "\tdecq %%r%c\n",
			    r1a);
		} else {
			(void) snprintf(buf, sizeof(buf), "\tdecq %%r%c%c\n",
			    r1a, r1b);
		}

		free(window->line1);
		window->line1 = xstrdup(buf);

		return 1;
	}

	return 0;
}

static int
decl(struct peephole *window)
{
	char buf[32], e1a, e1b;

	if (window->line1 == NULL)
		return 0;

	if (!strncmp("\tsubl $1, %e", window->line1, 12)) {
		if (strlen(window->line1) != 15)
			return 0;

		e1a = window->line1[12];
		e1b = window->line1[13];

		(void) snprintf(buf, sizeof(buf), "\tdecl %%e%c%c\n", e1a,
		    e1b);

		free(window->line1);
		window->line1 = xstrdup(buf);

		return 1;
	} else if (!strncmp("\tsubl $1, %r", window->line1, 12)) {
		if (strlen(window->line1) < 14)
			return 0;

		e1a = window->line1[12];
		e1b = window->line1[13];

		if (e1b == 'd') {
			(void) snprintf(buf, sizeof(buf), "\tdecl %%r%cd\n",
			    e1a);
		} else {
			(void) snprintf(buf, sizeof(buf), "\tdecl %%r%c%cd\n",
			    e1a, e1b);
		}

		free(window->line1);
		window->line1 = xstrdup(buf);

		return 1;
	}

	return 0;
}

static int
imulq(struct peephole *window)
{
	char buf[32], r1a, r1b, r1c;

	if (window->line1 == NULL)
		return 0;

	if (!strncmp("\timulq $", window->line1, 8)) {
		if (strlen(window->line1) < 19)
			return 0;

		r1a = window->line1[strlen(window->line1) - 10];
		r1b = window->line1[strlen(window->line1) - 9];
		r1c = window->line1[strlen(window->line1) - 8];

		if (r1a == ' ' && r1b == '%' && r1c == 'r') {
			r1a = r1c;
			r1b = window->line1[strlen(window->line1) - 7];

			if (window->line1[strlen(window->line1) - 3] != r1a ||
			    window->line1[strlen(window->line1) - 2] != r1b) {
				return 0;
			}

			r1c = ' ';
		} else if (window->line1[strlen(window->line1) - 4] != r1a ||
		    window->line1[strlen(window->line1) - 3] != r1b ||
		    window->line1[strlen(window->line1) - 2] != r1c) {
			return 0;
		}

		switch (window->line1[8]) {
		case '-':
			if (window->line1[9] == '1' &&
			    window->line1[10] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tnegq %%%c%c%c\n", r1a, r1b, r1c);
			} else {
				return 0;
			}

			goto success;
		case '0':
			if (window->line1[9] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\txorq %%%c%c%c, %%%c%c%c\n", r1a, r1b,
				    r1c, r1a, r1b, r1c);
			} else {
				return 0;
			}

			goto success;
		case '1':
			if (window->line1[9] == ',') {
				free(window->line1);
				window->line1 = NULL;

				return 1;
			} else if (window->line1[9] == '6' &&
			    window->line1[10] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsalq $4, %%%c%c%c\n", r1a, r1b, r1c);
			} else if (window->line1[9] == '2' &&
			    window->line1[10] == '8' &&
			    window->line1[11] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsalq $7, %%%c%c%c\n", r1a, r1b, r1c);
			} else if (window->line1[9] == '0' &&
			    window->line1[10] == '2' &&
			    window->line1[11] == '4' &&
			    window->line1[12] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsalq $10, %%%c%c%c\n", r1a, r1b, r1c);
			} else if (window->line1[9] == '6' &&
			    window->line1[10] == '3' &&
			    window->line1[11] == '8' &&
			    window->line1[12] == '4' &&
			    window->line1[13] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsalq $14, %%%c%c%c\n", r1a, r1b, r1c);
			} else {
				return 0;
			}

			goto success;
		case '2':
			if (window->line1[9] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsalq %%%c%c%c\n", r1a, r1b, r1c);
			} else if (window->line1[9] == '5' &&
			    window->line1[10] == '6' &&
			    window->line1[11] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsalq $8, %%%c%c%c\n", r1a, r1b, r1c);
			} else if (window->line1[9] == '0' &&
			    window->line1[10] == '4' &&
			    window->line1[11] == '8' &&
			    window->line1[12] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsalq $11, %%%c%c%c\n", r1a, r1b, r1c);
			} else {
				return 0;
			}

			goto success;
		case '3':
			if (window->line1[9] == '2' &&
			    window->line1[10] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsalq $5, %%%c%c%c\n", r1a, r1b, r1c);
			} else if (window->line1[9] == '2' &&
			    window->line1[10] == '7' &&
			    window->line1[11] == '6' &&
			    window->line1[12] == '8' &&
			    window->line1[13] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsalq $15, %%%c%c%c\n", r1a, r1b, r1c);
			} else {
				return 0;
			}

			goto success;
		case '4':
			if (window->line1[9] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsalq $2, %%%c%c%c\n", r1a, r1b, r1c);
			} else if (window->line1[9] == '0' &&
			    window->line1[10] == '9' &&
			    window->line1[11] == '6' &&
			    window->line1[12] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsalq $12, %%%c%c%c\n", r1a, r1b, r1c);
			} else {
				return 0;
			}

			goto success;
		case '5':
			if (window->line1[9] == '1' &&
			    window->line1[10] == '2' &&
			    window->line1[11] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsalq $9, %%%c%c%c\n", r1a, r1b, r1c);
			} else {
				return 0;
			}

			goto success;
		case '6':
			if (window->line1[9] == '4' &&
			    window->line1[10] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsalq $6, %%%c%c%c\n", r1a, r1b, r1c);
			} else if (window->line1[9] == '5' &&
			    window->line1[10] == '5' &&
			    window->line1[11] == '3' &&
			    window->line1[12] == '6' &&
			    window->line1[13] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsalq $16, %%%c%c%c\n", r1a, r1b, r1c);
			} else {
				return 0;
			}

			goto success;
		case '8':
			if (window->line1[9] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsalq $3, %%%c%c%c\n", r1a, r1b, r1c);
			} else if (window->line1[9] == '1' &&
			    window->line1[10] == '9' &&
			    window->line1[11] == '2' &&
			    window->line1[12] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsalq $13, %%%c%c%c\n", r1a, r1b, r1c);
			} else {
				return 0;
			}

			goto success;
		default:
			return 0;
		}
	}

	return 0;

success:
	free(window->line1);
	window->line1 = xstrdup(buf);

	return 1;
}

static int
imull(struct peephole *window)
{
	char buf[32], e1a, e1b, e1c;

	if (window->line1 == NULL)
		return 0;

	if (!strncmp("\timull $", window->line1, 8)) {
		if (strlen(window->line1) < 21)
			return 0;

		e1a = window->line1[strlen(window->line1) - 10];
		e1b = window->line1[strlen(window->line1) - 9];
		e1c = window->line1[strlen(window->line1) - 8];

		if (window->line1[strlen(window->line1) - 4] != e1a ||
		    window->line1[strlen(window->line1) - 3] != e1b ||
		    window->line1[strlen(window->line1) - 2] != e1c) {
			return 0;
		}

		switch (window->line1[8]) {
		case '-':
			if (window->line1[9] == '1' &&
			    window->line1[10] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tnegl %%%c%c%c\n", e1a, e1b, e1c);
			} else {
				return 0;
			}

			goto success;
		case '0':
			if (window->line1[9] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\txorl %%%c%c%c, %%%c%c%c\n", e1a, e1b,
				    e1c, e1a, e1b, e1c);
			} else {
				return 0;
			}

			goto success;
		case '1':
			if (window->line1[9] == ',') {
				free(window->line1);
				window->line1 = NULL;

				return 1;
			} else if (window->line1[9] == '6' &&
			    window->line1[10] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsall $4, %%%c%c%c\n", e1a, e1b, e1c);
			} else if (window->line1[9] == '2' &&
			    window->line1[10] == '8' &&
			    window->line1[11] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsall $7, %%%c%c%c\n", e1a, e1b, e1c);
			} else if (window->line1[9] == '0' &&
			    window->line1[10] == '2' &&
			    window->line1[11] == '4' &&
			    window->line1[12] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsall $10, %%%c%c%c\n", e1a, e1b, e1c);
			} else if (window->line1[9] == '6' &&
			    window->line1[10] == '3' &&
			    window->line1[11] == '8' &&
			    window->line1[12] == '4' &&
			    window->line1[13] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsall $14, %%%c%c%c\n", e1a, e1b, e1c);
			} else {
				return 0;
			}

			goto success;
		case '2':
			if (window->line1[9] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsall %%%c%c%c\n", e1a, e1b, e1c);
			} else if (window->line1[9] == '5' &&
			    window->line1[10] == '6' &&
			    window->line1[11] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsall $8, %%%c%c%c\n", e1a, e1b, e1c);
			} else if (window->line1[9] == '0' &&
			    window->line1[10] == '4' &&
			    window->line1[11] == '8' &&
			    window->line1[12] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsall $11, %%%c%c%c\n", e1a, e1b, e1c);
			} else {
				return 0;
			}

			goto success;
		case '3':
			if (window->line1[9] == '2' &&
			    window->line1[10] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsall $5, %%%c%c%c\n", e1a, e1b, e1c);
			} else if (window->line1[9] == '2' &&
			    window->line1[10] == '7' &&
			    window->line1[11] == '6' &&
			    window->line1[12] == '8' &&
			    window->line1[13] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsall $15, %%%c%c%c\n", e1a, e1b, e1c);
			} else {
				return 0;
			}

			goto success;
		case '4':
			if (window->line1[9] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsall $2, %%%c%c%c\n", e1a, e1b, e1c);
			} else if (window->line1[9] == '0' &&
			    window->line1[10] == '9' &&
			    window->line1[11] == '6' &&
			    window->line1[12] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsall $12, %%%c%c%c\n", e1a, e1b, e1c);
			} else {
				return 0;
			}

			goto success;
		case '5':
			if (window->line1[9] == '1' &&
			    window->line1[10] == '2' &&
			    window->line1[11] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsall $9, %%%c%c%c\n", e1a, e1b, e1c);
			} else {
				return 0;
			}

			goto success;
		case '6':
			if (window->line1[9] == '4' &&
			    window->line1[10] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsall $6, %%%c%c%c\n", e1a, e1b, e1c);
			} else if (window->line1[9] == '5' &&
			    window->line1[10] == '5' &&
			    window->line1[11] == '3' &&
			    window->line1[12] == '6' &&
			    window->line1[13] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsall $16, %%%c%c%c\n", e1a, e1b, e1c);
			} else {
				return 0;
			}

			goto success;
		case '8':
			if (window->line1[9] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsall $3, %%%c%c%c\n", e1a, e1b, e1c);
			} else if (window->line1[9] == '1' &&
			    window->line1[10] == '9' &&
			    window->line1[11] == '2' &&
			    window->line1[12] == ',') {
				(void) snprintf(buf, sizeof(buf),
				    "\tsall $13, %%%c%c%c\n", e1a, e1b, e1c);
			} else {
				return 0;
			}

			goto success;
		default:
			return 0;
		}
	}

	return 0;

success:
	free(window->line1);
	window->line1 = xstrdup(buf);

	return 1;
}

static void
one(struct peephole *window)
{

	if (xorq(window))
		return;

	if (xorl(window))
		return;

	if (incq(window))
		return;

	if (incl(window))
		return;

	if (decq(window))
		return;

	if (decl(window))
		return;

	if (imulq(window))
		return;

	(void) imull(window);
}

static int
mov(struct peephole *window)
{

	if (window->line1 == NULL || window->line2 == NULL ||
	    window->line3 == NULL) {
		return 0;
	}

	if (strncmp("\tmovl %e", window->line1, 8) != 0 &&
	    strncmp("\tmovq %r", window->line1, 8) != 0) {
		return 0;
	}

	if (strncmp("\tmovl %e", window->line2, 8) != 0 &&
	    strncmp("\tmovq %r", window->line2, 8) != 0) {
		return 0;
	}

	if (strncmp("\tmovl %e", window->line3, 8) != 0 &&
	    strncmp("\tmovq %r", window->line3, 8) != 0 &&
	    strncmp("\tmovl $0, %e", window->line3, 12) != 0 &&
	    strncmp("\tmovq $0, %r", window->line3, 12) != 0) {
		return 0;
	}

	if (strlen(window->line1) < 16 || strlen(window->line2) < 16)
		return 0;

	if (window->line1[4] == window->line2[4] &&
	    window->line1[7] == window->line2[13] &&
	    window->line1[8] == window->line2[14] &&
	    window->line1[9] == window->line2[15] &&
	    window->line1[13] == window->line2[7] &&
	    window->line1[14] == window->line2[8] &&
	    window->line1[15] == window->line2[9] &&
	    window->line1[14] == window->line3[strlen(window->line3) - 3] &&
	    window->line1[15] == window->line3[strlen(window->line3) - 2]) {
		free(window->line2);
		window->line2 = NULL;

		free(window->line1);
		window->line1 = xstrdup(window->line3);

		free(window->line3);
		window->line3 = NULL;

		return 1;
	}

	return 0;
}

static int
three(struct peephole *window)
{

	return mov(window);
}

static int
fillwindow(struct peephole *window, FILE *fp)
{
	size_t size = 0;

	if (window->line1 == NULL) {
		if (getline(&window->line1, &size, fp) == -1)
			return 0;
	}

	if (window->line2 == NULL) {
		if (getline(&window->line2, &size, fp) == -1)
			return 0;
	}

	if (getline(&window->line3, &size, fp) == -1)
		return 0;

	return 1;
}

static void
shiftwindow(struct peephole *window)
{

	free(window->line1);
	window->line1 = xstrdup(window->line2);

	free(window->line2);
	window->line2 = xstrdup(window->line3);
}

static void
O(FILE *fp)
{
	struct peephole window;
	int ret;

	window.line1 = NULL;
	window.line2 = NULL;
	window.line3 = NULL;

	while (fillwindow(&window, fp)) {
again:
		ret = three(&window);
		one(&window);

		if (ret == 1) {
			if (fillwindow(&window, fp))
				goto again;
		}

		if (window.line1 != NULL)
			(void) fputs(window.line1, stdout);

		shiftwindow(&window);
	}

	free(window.line3);
	window.line3 = NULL;

	while (window.line1 != NULL) {
		one(&window);

		if (window.line1 != NULL)
			(void) fputs(window.line1, stdout);

		shiftwindow(&window);
	}
}

int
main(int argc, char *argv[])
{
	FILE *fp;

	if (argc == 4) {
		if (strcmp(argv[2], "-o") != 0)
			goto usage;

		if (freopen(argv[3], "w+", stdout) == NULL) {
			(void) fprintf(stderr, "O: error: couldn't open %s\n",
			    argv[3]);
		}
	} else if (argc != 2) {
usage:
		(void) fputs("usage: O in.s [-o out.s]\n", stderr);

		return 1;
	}

	if (!strcmp(argv[1], "-")) {
		O(stdin);

		return 0;
	}

	if ((fp = fopen(argv[1], "r")) == NULL) {
		(void) fprintf(stderr, "O: error: couldn't open %s\n",
		    argv[1]);

		return 1;
	}

	O(fp);

	(void) fclose(fp);

	return 0;
}
