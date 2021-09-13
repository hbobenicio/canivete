# canivete

My experimental army-knife CLI utility.

## Dependencies

### Dev dependencies

#### Tools

- C11/C17 compiler (clang recomended)
- make
- strip (optional, for build releases)
- find (optional, for make clean/distclean targets)
- docker (optional, for static linking builds)

#### Libraries

- openssl 1.1
  - Ubuntu 20.04: `libssl-dev`
- sqlite 3
  - Ubuntu 20.04: `libsqlite3-dev`

### Runtime Dependencies

#### Libraries

These runtime shared libraries are only required for running the dynamically linked default builds.

Naturally, the static build does not require these.

- openssl 1.1
  - Ubuntu 20.04: `libssl1.1`
- sqlite 3
  - Ubuntu 20.04: `libsqlite3-0`
