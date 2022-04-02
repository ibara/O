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

The only non-C89 function is
[`getline(3)`](https://man.openbsd.org/getline.3).

Running
-------
```
usage: O file.s
```

Output is listed on `stdout`.

You can pass this to an assembler using something like:
```
$ O file.s | as -o file.o -
```

`O` can read from `stdin` if invoked as `O -`.

License
-------
ISC License. See `LICENSE` for more information.
