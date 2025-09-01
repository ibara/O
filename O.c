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

int
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

void
shiftwindow(struct peephole *window)
{

	free(window->line1);
	window->line1 = xstrdup(window->line2);

	free(window->line2);
	window->line2 = xstrdup(window->line3);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	const char *input = NULL, *target = TARGET;
	int arch, i, in = 0, out = 0;

	for (i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-o") == 0) {
			if (argv[++i] == NULL) {
usage:
				(void) fputs("usage: O [-o out.s] [in.s]\n",
				    stderr);
				return 1;
			}
			if (out++)
				goto usage;
			if (freopen(argv[i], "w+", stdout) == NULL) {
				(void) fprintf(stderr,
				    "O: error: couldn't open %s\n", argv[i]);
			}
		} else if (strcmp(argv[i], "-t") == 0) {
			if (argv[++i] == NULL)
				goto usage;
			target = argv[i];
		} else {
			if (in++)
				goto usage;
			if (strcmp(argv[i], "-") != 0)
				input = argv[i];
		}
	}

	if (strcmp(target, "arm64") == 0 || strcmp(target, "aarch64") == 0) {
		arch = T_ARM64;
	} else if (strcmp(target, "x86_64") == 0 ||
	    strcmp(target, "amd64") == 0 || strcmp(target, "x64") == 0) {
		arch = T_X64;
	} else {
		(void) fputs("O: error: values for -t are `arm64' and `x64'\n",
		    stderr);
		return 1;
	}

	if (input == NULL) {
		if (arch == T_X64)
			x64(stdin);
		else if (arch == T_ARM64)
			arm64(stdin);

		return 0;
	}

	if ((fp = fopen(input, "r")) == NULL) {
		(void) fprintf(stderr, "O: error: couldn't open %s\n", input);
		return 1;
	}

	if (arch == T_ARM64)
		arm64(fp);
	else if (arch == T_X64)
		x64(fp);

	(void) fclose(fp);

	return 0;
}
