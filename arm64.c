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

static void
add(struct peephole *window)
{
	int c, i = 0, j = 0, r;
	char imm[3];

	if (window->line1 == NULL)
		return;

	if (strncmp("\tadd\tx", window->line1, 6) == 0) {
		c = window->line1[i++];
		while (c != ',') {
			if (c == '\n' || c == '\0')
				return;
			c = window->line1[i++];
		}
		++i;
		c = window->line1[i++];
		while (c != ',') {
			if (c == '\n' || c == '\0')
				return;
			c = window->line1[i++];
		}
		r = i;
		++i;

		(void) memset(imm, 0, 3);

		c = window->line1[i++];
		while (c != '\n') {
			if (j == 2)
				return;
			imm[j++] = c;
			c = window->line1[i++];
		}
		if (strcmp(imm, "#0") == 0) {
			window->line1[1] = 'm';
			window->line1[2] = 'o';
			window->line1[3] = 'v';
			window->line1[r - 1] = '\n';
			window->line1[r] = '\0';
		}
	}
}

void
one(struct peephole *window)
{

	add(window);
}

static int
merge_immediate(struct peephole *window)
{
	int c, i = 0, j = 0, r;
	char buf[64], imm[4], r1[4], r2[4], tmp[32];

	if (window->line1 == NULL || window->line2 == NULL)
		return 0;

	if (strncmp("\tmov\tw", window->line1, 6) != 0 &&
	    strncmp("\tmov\tx", window->line1, 6) != 0) {
		return 0;
	}

	if (strncmp("\tlsl\t", window->line2, 5) != 0 &&
	    strncmp("\tadd\t", window->line2, 5) != 0 &&
	    strncmp("\tsub\t", window->line2, 5) != 0) {
		return 0;
	}

	(void) memset(r1, 0, 4);

	c = window->line1[i + 5];

	while (c != ',') {
		if (i == 3)
			return 0;
		r1[i++] = c;
		c = window->line1[i + 5];
	}

	while (c == ',' || c == ' ') {
		c = window->line1[i + 5];
		if (c == '\n')
			return 0;
		++i;
	}

	(void) memset(imm, 0, 4);

	while (c != '\n') {
		if (j == 3)
			return 0;
		imm[j++] = c;
		c = window->line1[i + 5];
		++i;
	}

	i = 0;
	c = window->line2[i + 5];

	while (c != ',') {
		++i;
		c = window->line2[i + 5];
	}

	++i;
	c = window->line2[i + 5];

	while (c != ',') {
		++i;
		c = window->line2[i + 5];
	}

	r = i + 7;

	i += 2;
	c = window->line2[i + 5];

	j = 0;
	(void) memset(r2, 0, 4);

	while (c != '\n') {
		if (j == 3)
			return 0;
		r2[j++] = c;
		++i;
		c = window->line2[i + 5];
	}

	r1[0] = 'x';

	if (strcmp(r1, r2) == 0) {
		(void) memset(tmp, 0, 32);
		for (i = 0; i < r; ++i) {
			if (i == 31)
				return 0;
			tmp[i] = window->line2[i];
		}
		(void) snprintf(buf, 64, "%s%s\n", tmp, imm);

		free(window->line1);
		window->line1 = xstrdup(buf);

		free(window->line2);
		window->line2 = xstrdup(window->line3);

		free(window->line3);
		window->line3 = NULL;

		return 1;
	}

	return 0;
}

static int
eor(struct peephole *window)
{
	int c, i = 0, j = 0;
	char buf[32], imm[4], r1[4], r2[4], r3[4], r4[4];

	if (window->line1 == NULL || window->line2 == NULL)
		return 0;

	if (strncmp("\tmov\tx", window->line1, 6) != 0 ||
	    strncmp("\teor\tx", window->line2, 6) != 0) {
		return 0;
	}

	(void) memset(r1, 0, 4);

	c = window->line1[i + 5];

	while (c != ',') {
		if (i == 3)
			return 0;
		r1[i++] = c;
		c = window->line1[i + 5];
	}

	i += 2;
	c = window->line1[i + 5];

	(void) memset(imm, 0, 4);

	while (c != '\n') {
		if (j == 3)
			return 0;
		imm[j++] = c;
		++i;
		c = window->line1[i + 5];
	}

	if (strcmp(imm, "#-1") != 0)
		return 0;

	i = 0;
	c = window->line2[i + 5];

	(void) memset(r2, 0, 4);

	while (c != ',') {
		if (i == 3)
			return 0;
		r2[i++] = c;
		c = window->line2[i + 5];
	}

	i += 2;
	c = window->line2[i + 5];
	j = 0;

	(void) memset(r3, 0, 4);

	while (c != ',') {
		if (j == 3)
			return 0;
		r3[j++] = c;
		++i;
		c = window->line2[i + 5];
	}

	i += 2;
	c = window->line2[i + 5];
	j = 0;

	(void) memset(r4, 0, 4);

	while (c != '\n') {
		if (j == 3)
			return 0;
		r4[j++] = c;
		++i;
		c = window->line2[i + 5];
	}

	if (strcmp(r1, r4) != 0 || strcmp(r2, r3) != 0)
		return 0;

	(void) snprintf(buf, 32, "\tmvn\t%s, %s\n", r2, r2);

	free(window->line1);
	window->line1 = xstrdup(buf);

	free(window->line2);
	window->line2 = xstrdup(window->line3);

	free(window->line3);
	window->line3 = NULL;

	return 1;
}

static int
mov(struct peephole *window)
{
	int c, i = 0, j = 0;
	char r1[4], r2[4], r3[4], r4[4];

	if (window->line1 == NULL || window->line2 == NULL)
		return 0;

	if (strncmp("\tmov\tx", window->line1, 6) != 0 ||
	    strncmp("\tmov\tx", window->line2, 6) != 0) {
		return 0;
	}

	(void) memset(r1, 0, 4);

	c = window->line1[i + 5];

	while (c != ',') {
		if (i == 3)
			return 0;
		r1[i++] = c;
		c = window->line1[i + 5];
	}

	while (c == ',' || c == ' ') {
		c = window->line1[i + 5];
		if (c == '\n')
			return 0;
		++i;
	}

	(void) memset(r2, 0, 4);

	while (c != '\n') {
		if (j == 3)
			return 0;
		r2[j++] = c;
		c = window->line1[i + 5];
		++i;
	}

	i = 0;
	(void) memset(r3, 0, 4);

	c = window->line2[i + 5];

	while (c != ',') {
		if (i == 3)
			return 0;
		r3[i++] = c;
		c = window->line2[i + 5];
	}

	while (c == ',' || c == ' ') {
		c = window->line2[i + 5];
		if (c == '\n')
			return 0;
		++i;
	}

	j = 0;
	(void) memset(r4, 0, 4);

	while (c != '\n') {
		if (j == 3)
			return 0;
		r4[j++] = c;
		c = window->line2[i + 5];
		++i;
	}

	if (r1[0] != 'x' || r2[0] != 'x' || r3[0] != 'x' || r4[0] != 'x')
		return 0;

	if (strcmp(r1, r4) != 0 || strcmp(r2, r3) != 0)
		return 0;

	free(window->line2);
	window->line2 = xstrdup(window->line3);

	free(window->line3);
	window->line3 = NULL;

	return 1;
}

int
two(struct peephole *window)
{

	if (merge_immediate(window))
		return 1;

	if (eor(window))
		return 1;

	return mov(window);
}

int
three(struct peephole *window)
{

	return 0;
}
