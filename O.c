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
		if (strlen(window->line1) != 15) {
			(void) fputs("O: error: xorq failed\n", stderr);

			exit(1);
		}

		r1a = window->line1[12];
		r1b = window->line1[13];

		(void) snprintf(buf, sizeof(buf), "\txorq %%r%c%c, %%r%c%c\n",
		    r1a, r1b, r1a, r1b);

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
		if (strlen(window->line1) != 15) {
			(void) fputs("O: error: xorl failed\n", stderr);

			exit(1);
		}

		e1a = window->line1[12];
		e1b = window->line1[13];

		(void) snprintf(buf, sizeof(buf), "\txorl %%e%c%c, %%e%c%c\n",
		    e1a, e1b, e1a, e1b);

		free(window->line1);
		window->line1 = xstrdup(buf);

		return 1;
	}

	return 0;
}

static void
one(struct peephole *window)
{

	if (xorq(window))
		return;

	if (xorl(window))
		return;
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

		(void) fputs(window.line1, stdout);

		shiftwindow(&window);
	}

	free(window.line3);
	window.line3 = NULL;

	while (window.line1 != NULL) {
		one(&window);

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
