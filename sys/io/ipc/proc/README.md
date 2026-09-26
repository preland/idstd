# `proc` — child processes for `id`

`id` has no way to start another process and watch it run. `fs_run` (in
`sys/io/fs`) can run a command, but only to completion, with no handle, no
timeout and no way to kill it early -- fine for "build this," useless for
"start QEMU, talk to it while it runs, and stop it when done," which is what
`tools/qmon` needs.

This backend supplies that: spawn (with or without resource limits), write to
the child's stdin, read what it writes to stdout and stderr separately, wait
for it with a timeout, kill it.

```sh
bin/idc demos/procdemo -o procdemo && ./procdemo
```

## The seam

| function | meaning |
| --- | --- |
| `proc_spawn(argv_lines)` | start a child with pipes on its stdin, stdout and stderr; `argv_lines` is the program and its arguments, one per line (see below). Returns a handle ≥ 0, or −1 |
| `proc_spawn_limited(argv_lines, mem_kb, cpu_seconds)` | exactly like `proc_spawn`, but caps the child's address space (`RLIMIT_AS`) to `mem_kb` kilobytes -- the unit `ulimit -v` takes -- and its CPU time (`RLIMIT_CPU`) to `cpu_seconds` before it execs -- 0 in either leaves that resource unlimited. **This is the only call that applies limits; `proc_spawn` never does.** Returns a handle ≥ 0, or −1 |
| `proc_read(h, buf, n, timeout_ms)` | up to `n` bytes of the child's stdout into `buf`, waiting up to `timeout_ms` for at least one. Returns the count, `0` if none arrived (including the child having closed stdout), or −1 |
| `proc_read_err(h, buf, n, timeout_ms)` | exactly like `proc_read`, but for the child's stderr -- a separate pipe, so reading one stream never consumes or blocks on the other |
| `proc_write(h, buf, n, timeout_ms)` | write up to `n` bytes of `buf` to the child's stdin, waiting up to `timeout_ms` for the pipe to accept at least one. Returns the count written (may be short), `0` if the pipe stayed full for the timeout, or −1 (`EPIPE` if the child is no longer reading) |
| `proc_wait(h, timeout_ms)` | wait up to `timeout_ms` for the child to exit. Returns its exit status (0..255, or 128+signal), or −1 (`proc_error()` is `ETIMEDOUT` if it is still running) |
| `proc_kill(h)` | send `SIGKILL`. `0`, or −1. Does not reap -- call `proc_wait` after |
| `proc_close_in(h)` | close the write end of the child's stdin, so it sees EOF on its next read of it, without touching stdout, stderr or the child. `0` (including if already closed), or −1 |
| `proc_close(h)` | close whichever pipes are still open and reap the child if it has already exited (never blocks). `0`, or −1 |
| `proc_error()` | the `errno` of the last call that returned −1 |

`n` is clamped to `len(buf)`, so the caller sizes the buffer and the backend
can never write past it -- the same contract as `fs_read`.

Every child now gets three real pipes -- stdin, stdout, stderr -- instead of
stdin/stderr being `/dev/null`. A caller that does not want to feed a child's
stdin and does not want it blocking on a read of it calls `proc_close_in`
right after spawning, for the same immediate-EOF effect `/dev/null` used to
give for free.

### Reading stdout and stderr without deadlocking

A child that writes a lot to stderr while its parent only reads stdout can
fill the stderr pipe and block on the next write, and a parent that only
ever calls `proc_read` would never notice -- classic two-pipe deadlock.
Nothing here can hit that as long as a caller drains **both** streams every
round rather than reading one to exhaustion before touching the other:
`proc_read` and `proc_read_err` each return within `timeout_ms` whether or
not data arrived, so alternating bounded calls on both, followed by a
non-blocking `proc_wait`, always makes progress on whichever stream the
child is actually filling and always reaches the exit check -- it cannot
park on one pipe forever. `.tests/check/drain/round.id`'s `drain_round` is
exactly that loop body, written once and reused by every check in
`.tests/check` that needs to capture both streams to exit.

### Why `argv_lines` is one string, not a `string[]`

idc's native boundary lowers `int`, `int[]` and a single `string` (see
`sys/win/gfx/gfx.h` and `sys/win/gl/gl.h`: "there is no `id` string arg on
this second tier except the window title"). A `string[]` has no lowering at
all -- an `IdList` cell is one uniform 64-bit word, and a string is not one.
So the caller builds one string with idstd's `str_join(argv, "\n")` -- the
program name, then each argument, one per line -- and this backend splits it
back on `\n`. None of `tools/qmon`'s arguments can contain a newline (they
are flag names, file paths and a fixed QEMU command line), so the join loses
nothing. A caller that needs a literal newline in an argument cannot use this
backend as it stands; nothing in this repository does.

### Why bytes cross as `int[]`

Same reason as `fs_read`: the flat store (`alloc`/`peek8`/`poke8`) is a
`static` inside the *generated* program, unreachable from a separately
compiled object. A list is a pointer the `id` side already owns and hands
over.

## What is *not* here

No process groups, no signals besides `SIGKILL`, no resource limit besides
address space and CPU time (no `RLIMIT_NOFILE`, no `RLIMIT_CORE`, ...). This
is the smallest seam `tools/qmon` -- and now an `id` test runner that needs
to read a child's stderr and cap its memory -- needs; a caller that needs
more adds one more `native` declaration beside these, and its C, when
something actually needs it.

## Portability

Linux only. `fork`, `execvp`, `pipe`, `waitpid` and `kill` are POSIX, but the
retry-with-timeout shape of `proc_wait` was only proven on Linux, so
`backend.id` declares no darwin sources; the compiler's missing-platform
diagnostic (`docs/BACKENDS.md`) names the gap exactly if something reaches a
`proc_*` native while building for another platform.
