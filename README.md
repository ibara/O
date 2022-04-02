O
=
`O` is a peephole optimizer for
[qbe](https://c9x.me/compile/).

It is meant to be used in conjunction with
[cproc](https://sr.ht/~mcf/cproc/).

Why?
----
`O` is the subject of two
[blog](https://briancallahan.net/blog/20220330.html)
[posts](https://briancallahan.net/blog/20220402.html).

Building
--------
Just run `make`.

The only non-C89 functions are
[`getline(3)`](https://man.openbsd.org/getline.3)
and
[`snprintf(3)`](https://man.openbsd.org/snprintf.3).

Running
-------
```
usage: O in.s [-o out.s]
```

Output is listed on `stdout` unless `-o` is passed on the command line.

You can pass this to an assembler using something like:
```
$ O file.s | as -o file.o -
```

`O` can read from `stdin` if invoked as `O -`.

License
-------
ISC License. See `LICENSE` for more information.
