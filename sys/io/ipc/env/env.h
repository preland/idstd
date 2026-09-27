/* env.h -- the C target's realisation of the `env` backend ABI.
 *
 * `id` has no way to read the process environment: no `getenv`, nothing. The
 * compiler driver needs one -- IDC_MEM_LIMIT, IDSTD_HOME, IDC_NO_STD and
 * IDC_WASI_* are all read out of the environment today, by the bash driver,
 * because `id` had no other way to see them. This backend is that seam, the
 * same shape as sys/io/fs: a `native` declaration in `id`'s own types for
 * each entry point, and this header as the C target's answer to it.
 *
 * ABI notes, identical to fs.h's: every entry point returns `int` (idc emits
 * the prototype `<ctype> id_<name>(<params>)`), `id` string -> C `char*`,
 * `id` int[] -> IdList*.
 */
#ifndef ID_ENV_H
#define ID_ENV_H

/* Growable list, byte-for-byte identical to the IdList in idc.py's RUNTIME
 * (and to sys/io/fs/fs.h's copy). An `id` `int[]` lowers to `IdList*`. */
typedef struct { int len, cap; long long* data; } IdList;

/* 1 if `name` is set in the environment (even to an empty string), 0 if it
 * is not. Never -1: "is it set" has no error case worth propagating, the
 * same reasoning as id_fs_exists. */
extern int id_env_has(const char* name);

/* The value of `name`, one byte per cell into `buf` (`n` clamped to the
 * list's length, so the caller sizes the buffer and this can never write
 * past it -- the same contract as id_fs_list). Returns how many bytes the
 * whole value needs, which may be more than were written -- grow the buffer
 * and call again if so -- or -1 if `name` is not set at all, which is how a
 * caller tells "unset" apart from "set to the empty string" (0 bytes
 * needed, nothing missing) without a separate presence flag in the answer.
 * id_env_has is the cheaper way to ask only the yes/no question. */
extern int id_env_get(const char* name, IdList* buf, int n);

/* Sets `name` to `value` in this process's environment (overwriting any
 * existing value), the same as setenv(3) with overwrite=1. Returns 0 on
 * success, -1 on failure (id_env_error() gives why). This exists so a case's
 * `given` fixture can force a specific process-environment value -- see
 * demos/gfxdemo/loop/frame.id's gfx_force_no_display -- before the function
 * under test reads it; a case is its own process (docs/TESTS.md), so this
 * never reaches another case. */
extern int id_env_set(const char* name, const char* value);

/* The errno of the last env_* call that returned -1. Reading it does not
 * clear it -- the same contract as id_fs_error. */
extern int id_env_error(void);

#endif /* ID_ENV_H */
