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

static int
xorq(const char *line)
{

	if (!strncmp("\tmovq $0, %r", line, 12)) {
		(void) fprintf(stdout, "\txorq %%r%c%c, %%r%c%c\n", line[12],
		    line[13], line[12], line[13]);

		return 1;
	}

	return 0;
}

static int
xorl(const char *line)
{

	if (!strncmp("\tmovl $0, %e", line, 12)) {
		(void) fprintf(stdout, "\txorl %%e%c%c, %%e%c%c\n", line[12],
		    line[13], line[12], line[13]);

		return 1;
	}

	return 0;
}

static void
xor(const char *line)
{

	if (xorq(line))
		return;

	if (xorl(line))
		return;

	(void) fputs(line, stdout);
}

static int
nostack(const char *line, FILE *fp)
{
	char *line2 = NULL, *line3 = NULL, *line4 = NULL;
	size_t size = 0;

	if (strcmp("\tpushq %rbp\n", line) != 0) {
		xor(line);

		return 0;
	}

	if (getline(&line2, &size, fp) == -1) {
		(void) fputs(line, stdout);

		return 1;
	}

	if (strcmp("\tmovq %rsp, %rbp\n", line2) != 0) {
		(void) fputs(line, stdout);
		xor(line2);

		free(line2);

		return 0;
	}

	if (getline(&line3, &size, fp) == -1) {
		(void) fputs(line, stdout);
		(void) fputs(line2, stdout);

		free(line2);

		return 1;
	}

	if (!strcmp("\tleave\n", line3)) {
		free(line3);
		free(line2);

		return 0;
	}

	if (!strncmp("\tsub", line3, 4) || !strncmp("\tpush", line3, 5)) {
		(void) fputs(line, stdout);
		(void) fputs(line2, stdout);
		(void) fputs(line3, stdout);
	} else {
		xor(line3);

		while (getline(&line4, &size, fp) != -1) {
			if (!strcmp("\tleave\n", line4)) {
				free(line4);
				free(line3);
				free(line2);

				return 0;
			} else {
				xor(line4);
			}
		}

		free(line4);
	}

	free(line3);
	free(line2);

	return 0;
}

static void
O(FILE *fp)
{
	char *line = NULL;
	size_t size = 0;

	while (getline(&line, &size, fp) != -1) {
#ifdef SAFE
		xor(line);
#else
		if (nostack(line, fp) == 1)
			break;
#endif
	}

	free(line);
}

static int
usage(void)
{

	(void) fputs("usage: O in.s [-o out.s]\n", stderr);

	return 1;
}

int
main(int argc, char *argv[])
{
	FILE *fp;

	if (argc == 4) {
		if (strcmp(argv[2], "-o") != 0)
			return usage();

		if (freopen(argv[3], "w+", stdout) == NULL) {
			(void) fprintf(stderr, "O: error: couldn't open %s\n",
			    argv[3]);
		}
	} else if (argc != 2) {
		return usage();
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
