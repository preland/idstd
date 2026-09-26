/* proc.h -- the `id` child-process backend ABI (Linux only).
 *
 * The whole of `BUILTIN_NAMES` gives a program stdin, stdout and a filesystem
 * (through `fs`) but no way to start another process and watch it run, so a
 * tool that has to drive something long-lived -- QEMU, in `tools/qmon` --
 * could not exist in `id` at all. This backend is the missing seam: spawn,
 * write to its stdin, read what it writes on stdout and stderr separately,
 * wait for it with a timeout, kill it, and cap its memory and CPU time
 * before it execs.
 *
 * ABI notes, matching sys/io/fs/fs.h:
 *   - Every entry point is a `native` declaration in this directory's .id
 *     files, so it is named with the `id_` prefix and returns `int`: a status
 *     fits one bit of room, hence -1 everywhere and id_proc_error() to fetch
 *     the errno behind it.
 *   - Argument lowering mirrors idc's: `id` int -> C int, `id` string ->
 *     char*, `id` int[] -> IdList* (below).
 *
 * Why argv is one newline-joined string rather than a `string[]`: idc's
 * native boundary only lowers int, int[] and a single string (see gfx.h and
 * gl.h -- "there is no id string arg on this second tier except the window
 * title"); a list of strings has no lowering at all, because an IdList cell
 * is one uniform 64-bit word and a string is not one. The caller builds the
 * line with idstd's str_join(argv, "\n"); this side splits on '\n'. None of
 * qmon's arguments can contain a newline, so the join is lossless.
 *
 * Why bytes read from or written to the child cross as `int[]`: the same
 * reason fs_read does (fs.h) -- the flat store is a `static` inside the
 * *generated* program, unreachable from a separately compiled object, so a
 * list the id side already owns is the seam.
 *
 * Every child now gets three real pipes -- stdin, stdout, stderr -- instead
 * of stdin/stderr being /dev/null. A caller that wants the old
 * immediate-EOF stdin calls id_proc_close_in right after spawning; nothing
 * here defaults to /dev/null any more.
 *
 * SIGPIPE: a write to a child that has already closed its stdin (exited, or
 * closed fd 0 itself) raises SIGPIPE, whose default action kills the whole
 * `id` program. id_proc_write ignores SIGPIPE (once, lazily) so that case
 * surfaces as an ordinary -1/EPIPE instead.
 *
 * Resource limits: id_proc_spawn_limited is the ONLY call that applies them
 * -- id_proc_spawn never does. Both a nonzero mem_kb (RLIMIT_AS) and a
 * nonzero cpu_seconds (RLIMIT_CPU) are set in the child, after fork and
 * before execvp; 0 in either leaves that resource unlimited. If setrlimit
 * itself fails, the child _exit(126)s (127 stays reserved for "exec
 * itself failed").
 */
#ifndef ID_PROC_H
#define ID_PROC_H

/* Growable list, byte-for-byte identical to the IdList in idc.py's RUNTIME
 * (and to sys/io/fs/fs.h's copy). An `id` `int[]` lowers to `IdList*`. */
typedef struct { int len, cap; long long* data; } IdList;

/* Start `argv_lines` (the program name, then its arguments, one per line) as
 * a child process, with pipes for its stdin, stdout and stderr. Returns a
 * handle >= 0, or -1 -- ask id_proc_error for why. No resource limits are
 * applied; use id_proc_spawn_limited for that. */
extern int id_proc_spawn(const char* argv_lines);

/* Exactly like id_proc_spawn, but the child's address space is capped to
 * `mem_kb` (RLIMIT_AS, both soft and hard) and its CPU time to
 * `cpu_seconds` (RLIMIT_CPU) before it execs -- a 0 in either argument
 * leaves that resource unlimited. Negative values return -1/EINVAL. This is
 * the only proc_spawn* call that applies limits. */
extern int id_proc_spawn_limited(const char* argv_lines, int mem_kb, int cpu_seconds);

/* Read up to `n` bytes of the child's stdout into `buf`, one byte per cell,
 * waiting up to `timeout_ms` for at least one byte to arrive. `n` is clamped
 * to the list's length. Returns the number of bytes read, 0 if none arrived
 * within the timeout (which includes the child having closed its stdout),
 * or -1 on error. */
extern int id_proc_read(int handle, IdList* buf, int n, int timeout_ms);

/* Exactly like id_proc_read, but for the child's stderr instead of its
 * stdout -- a separate pipe, so reading one stream never consumes or blocks
 * on the other. */
extern int id_proc_read_err(int handle, IdList* buf, int n, int timeout_ms);

/* Write up to `n` bytes of `buf` to the child's stdin, waiting up to
 * `timeout_ms` for the pipe to accept at least one byte. Returns the number
 * of bytes written (which may be less than `n` -- a short write, exactly
 * like a partial read), 0 if the pipe stayed full for the whole timeout, or
 * -1 on error (EPIPE if the child is no longer reading). */
extern int id_proc_write(int handle, IdList* buf, int n, int timeout_ms);

/* Wait up to `timeout_ms` for the child to exit. Returns its exit status
 * (0..255, or 128+signal if it was killed by one) if it exited in time, or
 * -1 -- ask id_proc_error for why, which is ETIMEDOUT if it is still
 * running. Calling it again after a successful wait returns the same status
 * without touching the child again. */
extern int id_proc_wait(int handle, int timeout_ms);

/* Send SIGKILL to the child. Returns 0, or -1. Does not reap it -- follow
 * with id_proc_wait to collect the exit status. */
extern int id_proc_kill(int handle);

/* Close the write end of the child's stdin, so the child sees EOF on its
 * next read of it, without touching stdout, stderr or the child itself.
 * Returns 0 (including if stdin was already closed), or -1 for a handle
 * that was never open. */
extern int id_proc_close_in(int handle);

/* Close the handle: closes whichever of the stdin/stdout/stderr pipes are
 * still open and reaps the child if it has already exited, without
 * blocking if it has not. Returns 0, or -1 (including for a handle that was
 * never open). */
extern int id_proc_close(int handle);

/* The errno of the last proc_* call that returned -1, or 0 if none has.
 * Reading it does not clear it. */
extern int id_proc_error(void);

#define PROC_MAX_HANDLES 8

#endif /* ID_PROC_H */
