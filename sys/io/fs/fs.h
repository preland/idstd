/* fs.h -- the C target's realisation of the `fs` backend ABI.
 *
 * The ABI itself lives in this directory's `native` declarations (handle.id,
 * data.id, path/), written in `id`'s types, because it is the part that is not
 * about C: an interpreter or an LLVM target has to provide the same functions
 * with the same meanings, and reads them from there. The compiler checks every
 * call against them. This header is one target's answer to that declaration -- the C
 * prototypes the object file exports -- and nothing above it (no `.id` file,
 * no compiler source) knows it exists.
 *
 * Why a backend at all, rather than builtins: adding `fs_*` to BUILTIN_NAMES
 * would mean implementing them once per code generator -- the C runtime, the
 * LLVM lowering, the wasm lowering -- and the C runtime is a string constant
 * inside `demos/idc_in_id_parse/back/out/sink/runtime.id`, so "add file
 * I/O" would have meant writing C into an `id` source file. As a backend the
 * implementation is C source in a C file, linked by whoever links.
 *
 * ABI notes:
 *   - Every entry point is a `native` declaration, which idc emits as the
 *     prototype `<ctype> id_<name>(<params>)` and calls as `id_<name>(args)`.
 *     So each function is named with the `id_` prefix; each returns `int`, so a
 *     size or a count tops out at INT_MAX, which is also true of every other
 *     length in `id` (`len` returns `int`).
 *   - Argument lowering mirrors idc's: `id` int -> C int, `id` string -> char*,
 *     `id` int[] -> IdList*, `id` word -> long long.
 *
 * Bytes cross two ways: as an `int[]`, one byte per cell (a list is a pointer
 * the id side already owns -- the same seam gfx uses for a framebuffer), or as
 * an address in the flat store, through id_store_span below.
 */
#ifndef ID_FS_H
#define ID_FS_H

/* Growable list, byte-for-byte identical to the IdList in idc.py's RUNTIME (and
 * to the one in gfx.h). An `id` `int[]` lowers to `IdList*`; each element is
 * one cell, and for an int list the cell holds the int directly. If idc's
 * runtime layout ever changes, this struct must change with it. */
typedef struct { int len, cap; long long* data; } IdList;

/* Open `path`. `mode` is one of "r", "w", "a" (a trailing "+" is accepted and
 * means read/write, as in C). Returns a handle >= 0, or -1 -- ask id_fs_error
 * for why. Handles are small integers, and there are FS_MAX_HANDLES of them. */
extern int id_fs_open(const char* path, const char* mode);

/* Read up to `n` bytes into `buf`, one byte per cell (0..255). `n` is clamped
 * to the list's length, so the caller sizes the buffer and the backend can
 * never write past it. Returns the number of bytes read -- 0 at end of file --
 * or -1 on error. Cells past the returned count are left alone. */
extern int id_fs_read(int handle, IdList* buf, int n);

/* Write the first `n` cells of `buf` as bytes (each cell is taken modulo 256).
 * `n` is clamped to the list's length. Returns the number of bytes written, or
 * -1 on error. */
extern int id_fs_write(int handle, IdList* buf, int n);

/* The flat store (`alloc`) as the generated program exports it: `n` bytes at
 * `addr`, after the same bounds check as `peek8`. The pointer is good only
 * until the next `alloc`, which may move the store. */
extern unsigned char* id_store_span(long long addr, long long n);

/* Read up to `n` bytes from `handle` straight into the store at `addr`, one
 * byte per byte. An `addr`/`n` outside the store aborts, as `poke8` would.
 * Returns the number of bytes read -- 0 at end of file -- or -1 on error. */
extern int id_fs_read_mem(int handle, long long addr, int n);

/* Write the `n` bytes at `addr` in the store. Returns `n`, or -1. */
extern int id_fs_write_mem(int handle, long long addr, int n);

/* Flush and close a handle. Returns 0, or -1 (including for a handle that was
 * never open). */
extern int id_fs_close(int handle);

/* Size of `path` in bytes, or -1 if it cannot be stat'd. */
extern int id_fs_size(const char* path);

/* 1 if `path` exists, 0 if it does not. Never -1: "does it exist" has no
 * error case worth propagating. */
extern int id_fs_exists(const char* path);

/* Delete `path`. Returns 0, or -1. */
extern int id_fs_remove(const char* path);

/* Create one directory level at `path`, mode 0777 masked by the process
 * umask -- the same default `mkdir(1)` uses. The parent must already exist;
 * a multi-level create is `fs_mkdir_p`, built on this in `id` (path/edit/
 * mkdir/deep.id). Returns 0, or -1 (EEXIST if `path` is already there). */
extern int id_fs_mkdir(const char* path);

/* `path`'s modification time, whole seconds since the epoch (the same
 * resolution `stat -c %Y` reports), or -1 if it cannot be stat'd. */
extern int id_fs_mtime(const char* path);

/* Set `path`'s permission bits to `mode` exactly, e.g. 493 for 0755
 * (rwxr-xr-x) -- `id` has no octal literal, so a caller spells the decimal
 * equivalent of whatever POSIX mode it wants, executable bits included.
 * Returns 0, or -1. */
extern int id_fs_chmod(const char* path, int mode);

/* Create a new, empty file (`dir` 0) or directory (`dir` 1) named
 * /tmp/<prefix> plus six characters no other process has, and write that
 * path into the store at `addr`, where `n` bytes are available. Returns the
 * path's length, or -1: EINVAL for a `prefix` containing '/' or a `dir` other
 * than 0 or 1, ENAMETOOLONG when the path would not fit in `n`. */
extern int id_fs_mktemp_mem(const char* prefix, int dir, long long addr, int n);

/* Rename/move `old_path` to `new_path`, replacing `new_path` if it exists
 * and both are the same kind of thing (POSIX `rename`'s own rule -- a file
 * never silently replaces a directory or the reverse). Returns 0, or -1. */
extern int id_fs_rename(const char* old_path, const char* new_path);

/* The errno of the last fs_* call that returned -1, or 0 if none has. Reading
 * it does not clear it. This exists because every entry point returns `int`,
 * so a failure has exactly one bit of room to say so; the detail has to be
 * fetched separately. */
/* One directory's entries into `buf`, newline-separated, with a trailing '/' on
 * the names that are directories. `.` and `..` are omitted, and the listing is
 * sorted in byte order so that two tools walking the same tree agree on it.
 *
 * Returns how many bytes the whole listing needs, which may be more than were
 * written: that is what lets a caller size its buffer without a second entry
 * point -- call, and if the answer exceeds the buffer, grow it and call again.
 * -1 means the directory could not be read, with the reason in id_fs_error. */
extern int id_fs_list(const char* path, IdList* buf, int n);

extern int id_fs_error(void);

#define FS_MAX_HANDLES 16

#endif /* ID_FS_H */
