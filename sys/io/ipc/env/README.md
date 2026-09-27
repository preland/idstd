# `env` — process environment variables for `id`

`id` has no way to read the process environment: no `getenv`, nothing. The
compiler driver (`bin/idc`, a 2000-line bash script) reads `IDC_MEM_LIMIT`,
`IDSTD_HOME`, `IDC_NO_STD` and `IDC_WASI_*` this way today, because it had no
other way to see them — this backend is what lets that move to `id`.

```sh
bin/idc demos/envdemo -o envdemo && ./envdemo
```

## The seam

| function | meaning |
| --- | --- |
| `env_has(name)` | `1` if `name` is set (even to an empty string), `0` if it is not. Never −1 |
| `env_get(name, buf, n)` | `name`'s value, one byte per cell into `buf`. Returns the bytes the value *needs* — grow and retry if that exceeds `n`, the same contract as `fs_list` — or −1 if `name` is not set |
| `env_set(name, value)` | sets `name` to `value`, overwriting any existing value. `0` on success, `-1` on failure |
| `env_error()` | the `errno` of the last call that returned −1 |

`n` is clamped to `len(buf)`, the same contract as `fs_read` and `fs_list`.

### Telling "unset" from "set to empty"

That is the one real design question here, and it is why there are two
functions instead of one. `env_get` alone cannot answer it: `0` bytes needed
is the right answer both for `FOO=` and for `FOO` never set, so a caller that
only calls `env_get` cannot tell a variable that is present but empty from one
that was never exported. Splitting the question in two settles it without a
third return channel: `env_has` answers *whether*, `env_get` answers *what*,
and `env_get` returning −1 (`env_error()` is `ENOENT`) is itself already a
clean presence check when the value is not going to be used anyway.

### Why it lives under `sys/io/ipc/`

It is not inter-process communication and does not belong next to `proc` and
`sock` by subject — it is here because every other directory in this tree,
root to leaf, is already at the 3-entries-per-directory limit (`README.md`,
"Layout") and `ipc/` was the one with a slot free. A dedicated `sys/env/`
(or a shared `sys/os/` alongside `sys/err/`) reads better and should be where
this moves the next time something restructures `sys/`; nothing about the
functions above depends on which directory holds them.

### Why bytes cross as `int[]`

Same reason as `fs_read` and `fs_list`: the flat store (`alloc`/`peek8`/
`poke8`) is a `static` inside the *generated* program, unreachable from a
separately compiled object.

## What is *not* here

No `unsetenv`, no listing every variable (`environ`). `env_set` covers the one
write a `given` fixture needs (forcing a value before the function under test
reads it); a caller that needs to unset a variable or enumerate the
environment adds one more `native` declaration beside these, and its C, when
something actually needs it.

## Portability

`env_posix.c` is `getenv(3)`, plain libc with no platform-specific behaviour,
which is why one file serves both platform keys in `backend.id` — the same
shape as `sys/io/fs`.
