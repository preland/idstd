/* env_posix.c -- the `env` backend for POSIX hosts (Linux, macOS).
 *
 * getenv(3) behind the three entry points in env.h. Nothing here is
 * platform-specific -- getenv is plain libc -- which is why one file serves
 * both platform keys in backend.id, the same shape as fs_posix.c.
 */
#include "env.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

static int env_last_errno = 0;

/* Record why a call is about to return -1, and return it. Every failure
 * path goes through here so id_env_error is never stale in one place and
 * fresh in another -- the same discipline fs_posix.c's fs_fail keeps. */
static int env_fail(int err) {
    env_last_errno = err;
    return -1;
}

int id_env_has(const char* name) {
    if (!name || !*name) return 0;
    return getenv(name) != NULL ? 1 : 0;
}

int id_env_get(const char* name, IdList* buf, int n) {
    const char* v;
    size_t len, i;
    if (!name || !buf) return env_fail(EINVAL);
    if (n < 0) return env_fail(EINVAL);
    if (n > buf->len) n = buf->len;
    v = getenv(name);
    if (!v) return env_fail(ENOENT);
    len = strlen(v);
    if (len > 0x7fffffff) return env_fail(EOVERFLOW);
    for (i = 0; i < len && (int)i < n; i++) {
        buf->data[i] = (long long)(unsigned char)v[i];
    }
    return (int)len;
}

int id_env_error(void) {
    return env_last_errno;
}
