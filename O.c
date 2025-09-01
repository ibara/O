/*
 * Copyright (c) 2022, 2025 Brian Callahan <bcallah@openbsd.org>
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

#include "O.h"

char *
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

	window.line1 = NULL;
	window.line2 = NULL;
	window.line3 = NULL;

	while (fillwindow(&window, fp)) {
again:
		ret = three(&window);
		if (ret == 0)
			ret = two(&window);
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
	int t;

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
