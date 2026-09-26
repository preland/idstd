# The `idstd` name registry

`id` has **no module system**. Every function and *every variable name* in a
program — the user's tree and every imported tree — lands in one flat namespace,
and three rules make that dangerous for a library that is imported by default:

1. **A name keeps one type, program-wide.** If `a` is an `int` in idstd, it is an
   `int` in *every program that imports idstd*, including every parameter and
   local a user writes. The error is reported in the library file, pointing at
   innocent code, for a declaration the user made.
2. **No two functions may share a body up to renaming.** Two functions with the
   same signature whose bodies differ only in the spelling of their own params
   and locals are a compile error — and that check crosses the import boundary,
   so a user writing `biggest(int p, int q)` collides with `fx_max`. What
   distinguishes two functions is *operators, literals, and the names of called
   functions and imported globals*.
3. **A zero-action function returning a bare int literal has the same logic as
   every other one returning that literal.** Every bare integer constant idstd
   defines takes that literal away from every user program. §5 is the answer.

So this file is **normative and a build dependency**. Adding a function or a
variable name to idstd means adding it here first. Everything in §1–§5 is a
promise to every program on the machine.

---

## 0. Public and internal

Every function is marked **pub** or **int**(ernal) in §1's tables.

- **pub** — part of the library's surface. Named, documented, and covered by the
  compatibility promise in `README.md`.
- **int** — an implementation detail: a loop body split out to fit the 3-action
  limit, a fold step, a table accessor. It is still a global name and still
  reserved, but it is not something a user should call, and it is the set that
  `id_development`'s C5 (duplicate-logic diagnostics that know what the standard
  library is) should exempt from the *user-facing* uniqueness check.

There is no language-level enforcement of this split. It is a manifest, which is
exactly what IDSTD.md §2 C5 asks for.

**Every internal function is spelled `idstd_<name>`.** `fx_sqbit` is
`idstd_fx_sqbit`; `term_render_row` is `idstd_term_render_row`. This is on top
of, not instead of, the pub/int split above: the prefix says "do not call this
from outside idstd" the same way `int` always did, and it also makes an
internal name impossible for a user program to collide with by accident,
which the bare `int` marking never prevented — `idc_in_id_calc` broke on a
parameter named `cn` colliding with the compiler's own `cn`, and that
parameter was never idstd's to begin with. §2 states the same rule for every
parameter and local variable, which is where the collision hazard actually
lives, since a parameter name is invisible in a diff of the calling program.
A **pub** function keeps its bare name — `fx_max`, `str_split` — because that
name *is* the library's surface and changing it would be the breaking change
§0 exists to avoid.

---

## 1. Function prefixes — one owner per prefix

| prefix | module | directory |
| --- | --- | --- |
| `fx_` | fixed-point arithmetic, roots, trig, the inverse tangent | `core/math/` |
| `rnd_` | random numbers | `core/math/` |
| `lset`/`lget`/`sset`/`lset2`/`wset` | the five bare list helpers (§3) | `core/data/seq/lst/` |
| `lst_` | list helpers: fill, copy, search, order, aggregate | `core/data/seq/lst/` |
| `buf_` | the flat store as bytes: fill, copy, compare | `core/data/seq/buf/` |
| `pcsf_` | PC Screen Font headers: which one, where the glyphs are, their size and count | `core/data/psf/` |
| `hmap_` `hset_` | a string-keyed hash table over `str_hash`, and the string set that shares its shape | `core/data/hmap/` |
| `str_` | strings: slice, search, compare, split, join, build | `core/text/str/` |
| `chr_` | one byte code: classify, case, hex digit | `core/text/chr/` |
| `fmt_` | formatting for display: width, hex | `core/text/fmt/` |
| `re_` | a backtracking regular-expression matcher over the flat-array VM in §1.14 | `core/text/str/part/split/re/` |
| `err_` | accumulated diagnostics | `sys/err/` |
| `sf_` | surfaces; `sf_l_` is the list surface | `gfx/px/` |
| `ppm_` | writing a surface as a PPM image; `ppm_l_` for the list surface | `gfx/px/` |
| `d2_` | 2D colour and shapes; `d2_l_` draws on the list surface | `gfx/plane/` |
| `txt_` | text on a surface; `txt_g8_` is the 8x8 face | `gfx/plane/txt/` |
| `d3_` | 3D geometry; so far `d3_cube_`, a cube's vertex and colour lists | `gfx/space/` |
| `term_` | the character-cell terminal: screen, drawing, rendering, keys | `sys/io/term/` |
| `inp_` | input; so far `inp_live`, whether a window event lets a loop go on | `sys/win/` |
| `sys_` | the window and frame pacing; so far `sys_next` | `sys/win/` |
| `fs_` | the file backend's natives (§1.11) | `sys/io/fs/` |
| `proc_` | the child process backend's natives (§1.11) | `sys/io/ipc/proc/` |
| `sock_` | the Unix-domain socket backend's natives (§1.11) | `sys/io/ipc/sock/` |
| `env_` | the process environment backend's natives (§1.11) | `sys/io/ipc/env/` |
| `gfx_` | the software window backend's natives (§1.11) | `sys/win/gfx/` |
| `gl_` `glwin_` | the OpenGL window backend's natives (§1.11) | `sys/win/gl/` |
| `inf_` | DEFLATE decompression (§1.15) | `core/data/seq/byte/inflate/` |
| `gz_` | the gzip container around a DEFLATE stream (§1.15) | `core/data/seq/byte/inflate/api/` |
| `sha256_` | SHA-256 (§1.16) | `core/data/seq/byte/sha256/` |

Reserved shapes inside a prefix, so two authors do not invent two spellings of
one idea: `*_init`, `*_get`, `*_set`, `*_len`, `*_at`, `*_add`, `*_find`,
`*_all`. A fixture that exists only to give inline cases a setup or a check is
spelled `*_t_*` and marked int.

**Reserved for later phases, not yet built:** `file_`, `term_` (`sys/io/`),
`sys_`, `inp_` (`sys/win/`), `m4_`, `d3_` (`gfx/space/`), and every other name
under `sf_`, `ppm_`, `d2_` and `txt_` — idem's flat-store surface is expected
there. Listed here so nothing else claims them.

### 1.1 `fx_` — fixed point (`core/math/`)

| function | signature | vis | file |
| --- | --- | --- | --- |
| `fx_abs` | `(int) -> int` | pub | `fx/base/cmp.id` |
| `fx_min` | `(int,int) -> int` | pub | `fx/base/cmp.id` |
| `fx_max` | `(int,int) -> int` | pub | `fx/base/cmp.id` |
| `fx_fdiv` | `(int,int) -> int` | pub | `fx/base/div.id` |
| `idstd_fx_fdown` | `(int,int,int) -> int` | int | `fx/base/div.id` |
| `fx_clamp` | `(int,int,int) -> int` | pub | `fx/lim.id` |
| `fx_sign` | `(int) -> int` | pub | `fx/lim.id` |
| `fx_lerp` | `(int,int,int) -> int` | pub | `fx/lim.id` |
| `fx_mul` | `(int,int,int) -> int` | pub | `fx/wide/mul.id` |
| `fx_div` | `(int,int,int) -> int` | pub | `fx/wide/mul.id` |
| `fx_pow` | `(int,int) -> int` | pub | `fx/wide/mul.id` |
| `fx_sqrt` | `(int) -> int` | pub | `fx/wide/sqrt.id` |
| `idstd_fx_sqbit` | `(int) -> int` | int | `fx/wide/sqrt.id` |
| `idstd_fx_sqloop` | `(int[],int) -> int` | int | `fx/wide/sqrt.id` |
| `idstd_fx_sqstep` | `(int[],int) -> void` | int | `fx/wide/bits/digits.id` |
| `idstd_fx_sqtake` | `(int[],int) -> void` | int | `fx/wide/bits/digits.id` |
| `idstd_fx_hypfix` | `(word) -> int` | int | `fx/wide/bits/digits.id` |
| `fx_wroot` | `(word) -> int` | pub | `fx/wide/bits/root.id` |
| `idstd_fx_wsmall` | `(word) -> int` | int | `fx/wide/bits/root.id` |
| `fx_sq` | `(int) -> word` | pub | `fx/wide/bits/root.id` |
| `fx_hyp` | `(int,int) -> int` | pub | `fx/wide/bits/hyp.id` |
| `fx_hyp3` | `(int,int,int) -> int` | pub | `fx/wide/bits/hyp.id` |
| `idstd_fx_hyp_sm2` | `(int,int) -> word` | int | `fx/wide/bits/hyp.id` |
| `fx_trig_init` | `() -> void` | pub | `trig/tab.id` |
| `idstd_fx_tab` | `(int) -> int` | int | `trig/tab.id` |
| `fx_norm_deg` | `(int) -> int` | pub | `trig/tab.id` |
| `fx_sin` | `(int) -> int` | pub | `trig/sin/wave.id` |
| `idstd_fx_sin_qr` | `(int) -> int` | int | `trig/sin/wave.id` |
| `fx_cos` | `(int) -> int` | pub | `trig/sin/wave.id` |
| `idstd_fx_sin_lin` | `(int) -> int` | int | `trig/sin/lin.id` |
| `idstd_fx_sin_interp` | `(int,int) -> int` | int | `trig/sin/lin.id` |
| `idstd_fx_sin_t_tab30` | `() -> int` | int, test fixture | `trig/sin/lin.id` |
| `fx_sin_deg` | `(int) -> int` | pub | `trig/sin/deg.id` |
| `fx_cos_deg` | `(int) -> int` | pub | `trig/sin/deg.id` |
| `idstd_fx_sin_q` | `(int,int) -> int` | int | `trig/ang/fold.id` |
| `idstd_fx_sin_q01` | `(int,int) -> int` | int | `trig/ang/fold.id` |
| `idstd_fx_sin_q23` | `(int,int) -> int` | int | `trig/ang/fold.id` |
| `fx_atan2` | `(int,int) -> int` | pub | `trig/ang/atan/atan2.id` |
| `idstd_fx_atan_absq` | `(int,int) -> int` | int | `trig/ang/atan/atan2.id` |
| `idstd_fx_atan_fold` | `(int,int,int) -> int` | int | `trig/ang/atan/atan2.id` |
| `idstd_fx_atan_q` | `(int,int) -> int` | int | `trig/ang/quad.id` |
| `idstd_fx_atan_oct_v` | `(int,int) -> int` | int | `trig/ang/quad.id` |
| `idstd_fx_atan_oct` | `(int,int) -> int` | int | `trig/ang/atan/oct.id` |
| `idstd_fx_atan_loop` | `(int[],int,int) -> void` | int | `trig/ang/atan/oct.id` |
| `idstd_fx_atan_step` | `(int[],int,int) -> void` | int | `trig/ang/atan/oct.id` |
| `idstd_fx_atan_hi` | `(int,int,int) -> int` | int | `trig/ang/atan/hi.id` |
| `idstd_fx_atan_cross` | `(int,int,int) -> word` | int | `trig/ang/atan/hi.id` |

**`fx_hypot` is deliberately absent.** IDSTD.md §3.1 asks for it "for the `int`
case", but `fx_hyp` already takes two `int`s and answers an `int`; a second
function with that body is a duplicate-logic *compile error*, not a redundancy.
See `COMPILER-ASKS.md`.

### 1.2 `rnd_` — random (`core/math/rnd/`)

| function | signature | vis |
| --- | --- | --- |
| `rnd_init` | `(int) -> void` | pub, **required init** |
| `rnd_next` | `() -> int` | pub |
| `idstd_rnd_step` | `() -> int` | int |
| `rnd_range` | `(int,int) -> int` | pub |
| `idstd_rnd_t_setup` | `() -> void` | int, test fixture |
| `idstd_rnd_t_setup_neg` | `() -> void` | int, test fixture |
| `idstd_rnd_t_state` | `() -> int` | int, test fixture |

### 1.3 the five bare helpers (`core/data/seq/lst/`)

These carry no prefix because they are language-level, and because the
compiler's own diagnostic names `lset` by that spelling. Defining a second copy
of any of them anywhere in a program is a duplicate-logic error — which is the
point: there is one.

| function | signature | vis | file |
| --- | --- | --- | --- |
| `lset` | `(int[],int,int) -> void` | pub | `seq/lst/access.id` |
| `lget` | `(int[],int) -> int` | pub | `seq/lst/access.id` |
| `sset` | `(string[],int,string) -> void` | pub | `seq/lst/access.id` |
| `lset2` | `(int[][],int,int[]) -> void` | pub | `seq/lst/w/pick.id` |
| `wset` | `(word[],int,word) -> void` | pub | `seq/lst/w/pick.id` |

### 1.4 `lst_`, `buf_` and `pcsf_` (`core/data/`)

| function | signature | vis | file |
| --- | --- | --- | --- |
| `lst_fill` | `(int[],int,int) -> void` | pub | `seq/lst/make/grow.id` |
| `lst_set_all` | `(int[],int,int) -> void` | pub | `seq/lst/make/grow.id` |
| `lst_extend` | `(int[],int[]) -> void` | pub | `seq/lst/make/grow.id` |
| `lst_slice` | `(int[],int,int) -> int[]` | pub | `seq/lst/make/slice.id` |
| `lst_pick` | `(int,int,int) -> int` | pub | `seq/lst/w/pick.id` |
| `lst_index_of` | `(int[],int) -> int` | pub | `seq/lst/w/find.id` |
| `lst_find` | `(int[],int) -> int` | pub | `seq/lst/w/find.id` |
| `lst_last` | `(int[]) -> int` | pub | `seq/lst/w/find.id` |
| `lst_min` | `(int[]) -> int` | pub | `seq/lst/w/ord/agg.id` |
| `lst_max` | `(int[]) -> int` | pub | `seq/lst/w/ord/agg.id` |
| `lst_sum` | `(int[]) -> int` | pub | `seq/lst/w/ord/agg.id` |
| `lst_copy` | `(int[]) -> int[]` | pub | `seq/lst/w/ord/move.id` |
| `lst_swap` | `(int[],int,int) -> void` | pub | `seq/lst/w/ord/move.id` |
| `lst_reverse` | `(int[]) -> void` | pub | `seq/lst/w/ord/move.id` |
| `lst_sort` | `(int[]) -> void` | pub | `seq/lst/w/ord/sort.id` |
| `idstd_lst_sort_loop` | `(int[],int,int) -> void` | int | `seq/lst/w/ord/sort.id` |
| `idstd_lst_sift` | `(int[],int) -> void` | int | `seq/lst/w/ord/sort.id` |
| `buf_fill` | `(word,int,int) -> void` | pub | `seq/buf/bytes.id` |
| `buf_zero` | `(word,int) -> void` | pub | `seq/buf/bytes.id` |
| `buf_copy` | `(word,word,int) -> void` | pub | `seq/buf/bytes.id` |
| `buf_cmp` | `(word,word,int) -> int` | pub | `seq/buf/cmp.id` |
| `idstd_buf_cmp_loop` | `(int[],word,word,int) -> void` | int | `seq/buf/cmp.id` |
| `idstd_buf_cmp_one` | `(int,word,word,int) -> int` | int | `seq/buf/cmp.id` |
| `idstd_buf_t_setup` | `() -> void` | int, test fixture | `seq/buf/known.id` |
| `idstd_buf_t_mark` | `() -> void` | int, test fixture | `seq/buf/known.id` |
| `idstd_buf_t_sum` | `() -> int` | int, test fixture | `seq/buf/known.id` |
| `pcsf_head` | `(int[]) -> int[]` | pub | `psf/head.id` |
| `idstd_pcsf_v1` | `(int[]) -> int[]` | int | `psf/head.id` |
| `idstd_pcsf_v2` | `(int[]) -> int[]` | int | `psf/head.id` |
| `idstd_pcsf_is1` | `(int[]) -> int` | int | `psf/magic.id` |
| `idstd_pcsf_is2` | `(int[]) -> int` | int | `psf/magic.id` |
| `idstd_pcsf_u32` | `(int[],int) -> int` | int | `psf/magic.id` |

`lst_find` answers **whether** a value is present (0/1); `lst_index_of` answers
**where** (-1 for absent). They are two functions because two callers want two
different things, and because `lst_find` delegating to `lst_index_of` is what
keeps them from being one duplicate-logic error.

### 1.5 `str_`, `chr_`, `fmt_` (`core/text/`)

| function | signature | vis | file |
| --- | --- | --- | --- |
| `str_slice` | `(string,int,int) -> string` | pub | `str/make/cut.id` |
| `str_blit` | `(string,int,int,word) -> void` | pub | `str/make/cut.id` |
| `str_upper` | `(string) -> string` | pub | `str/make/case/up.id` |
| `idstd_str_upper_alloc` | `(string,int) -> word` | int | `str/make/case/up.id` |
| `idstd_str_upper_blit` | `(string,word,int) -> void` | int | `str/make/case/up.id` |
| `idstd_str_upper_step` | `(string,word,int) -> void` | int | `str/make/case/step.id` |
| `str_lower` | `(string) -> string` | pub | `str/make/case/low.id` |
| `idstd_str_lower_alloc` | `(string,int) -> word` | int | `str/make/case/low.id` |
| `idstd_str_lower_blit` | `(string,word,int) -> void` | int | `str/make/case/low.id` |
| `idstd_str_lower_step` | `(string,word,int) -> void` | int | `str/make/case/step.id` |
| `str_pad` | `(string,int) -> string` | pub | `str/make/fill/pad/right.id` |
| `idstd_str_pad_build` | `(string,word,int) -> string` | int | `str/make/fill/pad/fill.id` |
| `idstd_str_pad_fill` | `(string,word,int) -> void` | int | `str/make/fill/pad/fill.id` |
| `idstd_str_t_setup` | `() -> void` | int, test fixture | `str/make/fill/pad/known.id` |
| `idstd_str_t_mark` | `() -> void` | int, test fixture | `str/make/fill/pad/known.id` |
| `idstd_str_t_read` | `() -> string` | int, test fixture | `str/make/fill/pad/known.id` |
| `str_repeat` | `(string,int) -> string` | pub | `str/make/fill/rep/repeat.id` |
| `idstd_str_rep_width` | `(string,int) -> int` | int | `str/make/fill/rep/loop.id` |
| `idstd_str_rep_build` | `(string,word,int,int) -> string` | int | `str/make/fill/rep/loop.id` |
| `idstd_str_rep_loop` | `(string,word,int) -> void` | int | `str/make/fill/rep/loop.id` |
| `str_trim` | `(string) -> string` | pub | `str/make/fill/trim/both.id` |
| `idstd_str_trim_end` | `(string) -> int` | int | `str/make/fill/trim/both.id` |
| `idstd_str_trim_slice` | `(string,int,int) -> string` | int | `str/make/fill/trim/both.id` |
| `idstd_str_ws_start` | `(string,int) -> int` | int | `str/make/fill/trim/ws.id` |
| `idstd_str_ws_end` | `(string,int) -> int` | int | `str/make/fill/trim/ws.id` |
| `idstd_chr_is_space_at` | `(string,int) -> int` | int | `str/make/fill/trim/ws.id` |
| `str_eqat` | `(string,int,string) -> int` | pub | `str/scan/eq.id` |
| `str_starts` | `(string,string) -> int` | pub | `str/scan/eq.id` |
| `str_ends` | `(string,string) -> int` | pub | `str/scan/eq.id` |
| `str_find` | `(string,string) -> int` | pub | `str/scan/find.id` |
| `str_findat` | `(string,string,int) -> int` | pub | `str/scan/find.id` |
| `idstd_str_find_end` | `(string,int,string) -> int` | int | `str/scan/find.id` |
| `str_cmp` | `(string,string) -> int` | pub | `str/scan/ord/cmp.id` |
| `idstd_str_cmp_sign` | `(string,string,int) -> int` | int | `str/scan/ord/cmp.id` |
| `idstd_str_cmp_run` | `(string,string,int) -> int` | int | `str/scan/ord/cmp.id` |
| `str_hash` | `(string) -> int` | pub | `str/scan/ord/hash.id` |
| `idstd_str_hash_run` | `(string,int,int) -> int` | int | `str/scan/ord/hash.id` |
| `str_to_float` | `(string) -> float` | pub | `str/scan/ord/num/float.id` |
| `idstd_str_frac` | `(string,int) -> float` | int | `str/scan/ord/num/float.id` |
| `idstd_str_frac_run` | `(string,int,float,float) -> float` | int | `str/scan/ord/num/float.id` |
| `idstd_chr_is_digit_at` | `(string,int) -> int` | int | `str/scan/ord/num/digit.id` |
| `idstd_str_sgn` | `(string) -> int` | int | `str/scan/ord/num/sgn.id` |
| `str_split` | `(string,string) -> string[]` | pub | `str/part/split/field.id` |
| `idstd_str_split_loop` | `(string[],string,string,int) -> void` | int | `str/part/split/field.id` |
| `idstd_str_split_one` | `(string[],string,string,int) -> int` | int | `str/part/split/field.id` |
| `idstd_str_split_bound` | `(string,string,int) -> int` | int | `str/part/split/bound.id` |
| `idstd_str_split_push` | `(string[],string,int,int) -> void` | int | `str/part/split/bound.id` |
| `idstd_str_split_cut` | `(int,int) -> int` | int | `str/part/cut.id` |
| `str_eol` | `(string,int) -> int` | pub | `str/part/cut.id` |
| `str_join` | `(string[],string) -> string` | pub | `str/part/join/build.id` |
| `idstd_str_join_len` | `(string[],string) -> int` | int | `str/part/join/build.id` |
| `idstd_str_join_blit` | `(string[],string,word) -> void` | int | `str/part/join/build.id` |
| `idstd_str_join_one` | `(int[],string[],string,word) -> void` | int | `str/part/join/one.id` |
| `idstd_str_join_one_adv` | `(int[],string[],string,word,int) -> void` | int | `str/part/join/one.id` |
| `idstd_str_join_put` | `(string,word,int) -> int` | int | `str/part/join/one.id` |
| `idstd_str_join_base` | `(string[],string) -> int` | int | `str/part/join/sep.id` |
| `idstd_str_join_sep` | `(string,word,int,int) -> int` | int | `str/part/join/sep.id` |
| `chr_is_digit` | `(int) -> int` | pub | `chr/range.id` |
| `chr_is_upper` | `(int) -> int` | pub | `chr/range.id` |
| `chr_is_lower` | `(int) -> int` | pub | `chr/range.id` |
| `chr_is_alpha` | `(int) -> int` | pub | `chr/composite.id` |
| `chr_is_alnum` | `(int) -> int` | pub | `chr/composite.id` |
| `chr_is_space` | `(int) -> int` | pub | `chr/composite.id` |
| `chr_upper` | `(int) -> int` | pub | `chr/case.id` |
| `chr_lower` | `(int) -> int` | pub | `chr/case.id` |
| `chr_hex` | `(int) -> int` | pub | `chr/case.id` |
| `fmt_int` | `(int,int) -> string` | pub | `fmt/int.id` |
| `fmt_pad` | `(string,int) -> string` | pub | `fmt/int.id` |
| `idstd_fmt_pad_width` | `(string,int) -> int` | int | `fmt/pad.id` |
| `idstd_fmt_pad_build` | `(string,word,int) -> string` | int | `fmt/pad.id` |
| `idstd_fmt_pad_fill` | `(string,word,int) -> void` | int | `fmt/pad.id` |
| `fmt_hex` | `(int) -> string` | pub | `fmt/hex.id` |
| `idstd_fmt_hex_run` | `(word,int,int) -> void` | int | `fmt/hex.id` |

**`str_of_int` and `str_of_word` are deliberately absent**, even though IDSTD.md
§3.3 lists them. `id` function names get an `id_` prefix in C and the runtime's
own helpers are already spelled `id_str_of_int` / `id_str_of_word`, so defining
either is a hard C compilation failure in *both* compilers — verified, and
IDSTD.md §1.7 says so two sections earlier. `"" + n` is the int→string path;
`fmt_int(n, w)` is the aligned one.

`str_pad` pads on the **right** (left-aligned text); `fmt_pad` pads on the
**left** (right-aligned, for a column of numbers). Two functions rather than one
with a flag, because a flag would be a bare literal at every call site.

### 1.6 `err_` (`sys/err/`)

| function | signature | vis | file |
| --- | --- | --- | --- |
| `err_init` | `() -> void` | pub, **required init** | `report.id` |
| `err_report` | `(string,int,string) -> void` | pub | `report.id` |
| `err_count` | `() -> int` | pub | `report.id` |
| `err_mute` | `(int) -> void` | pub | `mute.id` |
| `err_say` | `(string) -> void` | pub | `mute.id` |
| `idstd_err_t_mute` | `() -> int` | int, test fixture | `mute.id` |
| `idstd_err_keep_init` | `() -> void` | int | `k/keep.id` |
| `idstd_err_keep_init2` | `() -> void` | int | `k/keep.id` |
| `idstd_err_keep` | `(string,int,string) -> void` | int | `k/keep.id` |
| `idstd_err_keep2` | `(int,string) -> void` | int | `k/clear.id` |
| `err_clear` | `() -> void` | pub | `k/clear.id` |
| `idstd_err_drop` | `() -> void` | int | `k/clear.id` |
| `idstd_err_drop2` | `() -> void` | int | `k/count.id` |
| `err_nmsg` | `() -> int` | pub | `k/count.id` |
| `idstd_err_t_one` | `() -> void` | int, test fixture | `k/count.id` |

### 1.7 `sf_`, `ppm_`, `d2_`, `txt_` — the list surface (`gfx/`)

The pure-`id` half of the framebuffer kit that `demos/gfxdemo`, idml's id
backend and `id_nativeapp` carried: a surface that is one `int[]`, rectangles,
an 8x8 face and a PPM dump. None of it calls a native backend. idem's
`sf_`/`ppm_`/`d2_`/`txt_` functions are a different surface (the flat store)
and these names are chosen not to meet them: `sf_l_`/`ppm_l_`/`d2_l_` are the
list surface, `txt_g8_` the 8x8 face.

| function | signature | vis | file |
| --- | --- | --- | --- |
| `sf_l_init` | `(int,int) -> void` | pub, **required init** | `px/l/surf.id` |
| `idstd_sf_l_alloc` | `(int) -> void` | int | `px/l/surf.id` |
| `idstd_sf_l_fill` | `(int) -> void` | int | `px/l/surf.id` |
| `sf_l_idx` | `(int,int) -> int` | pub | `px/l/px.id` |
| `sf_l_pset` | `(int,int,int) -> void` | pub | `px/l/px.id` |
| `idstd_sf_l_t_setup` | `() -> void` | int, test fixture | `px/t.id` |
| `idstd_sf_l_t_len` | `() -> int` | int, test fixture | `px/t.id` |
| `idstd_sf_l_t_sum` | `() -> int` | int, test fixture | `px/t.id` |
| `idstd_ppm_l_head` | `() -> void` | int | `px/ppm/dump.id` |
| `ppm_l_dump` | `() -> void` | pub | `px/ppm/dump.id` |
| `idstd_ppm_l_rows` | `() -> void` | int | `px/ppm/dump.id` |
| `idstd_ppm_l_row` | `(int) -> void` | int | `px/ppm/row.id` |
| `idstd_ppm_l_px` | `(int,int) -> void` | int | `px/ppm/row.id` |
| `d2_pack` | `(int,int,int) -> int` | pub | `plane/col.id` |
| `d2_l_rect` | `(int,int,int,int,int) -> void` | pub | `plane/rect.id` |
| `idstd_d2_l_row` | `(int,int,int,int) -> void` | int | `plane/rect.id` |
| `txt_g8_init` | `() -> void` | pub, **required init** | `plane/txt/font.id` |
| `idstd_txt_g8_t_row1` | `() -> int` | int, test fixture | `plane/txt/font.id` |
| `idstd_txt_g8_row` | `(int,int) -> int` | int | `plane/txt/g8.id` |
| `txt_g8_glyph` | `(int,int,int,int,int) -> void` | pub | `plane/txt/g8.id` |
| `idstd_txt_g8_bits` | `(int,int,int,int,int) -> void` | int | `plane/txt/g8.id` |
| `txt_g8_draw` | `(int,int,string,int,int) -> void` | pub | `plane/txt/draw.id` |
| `txt_g8_width` | `(string,int) -> int` | pub | `plane/txt/draw.id` |
| `idstd_txt_g8_t_len` | `() -> int` | int, test fixture | `plane/txt/draw.id` |

### 1.8 `d3_` — cube geometry (`gfx/space/`)

The pure half of the GL kit `demos/fpsmaze`, `demos/gl3dgame` and `demos/gl3d`
shared: a cube's vertex and colour lists, built for a triangle submit. The
submit, the matrices and the frame (`gl_*`) are native and not here.

| function | signature | vis | file |
| --- | --- | --- | --- |
| `d3_cube_x` | `(int,int) -> int` | pub | `geom.id` |
| `d3_cube_y` | `(int,int) -> int` | pub | `geom.id` |
| `d3_cube_z` | `(int,int) -> int` | pub | `geom.id` |
| `d3_cube_corner` | `(int) -> int` | pub | `face.id` |
| `d3_cube_verts` | `(int) -> int[]` | pub | `mesh/build.id` |
| `d3_cube_colors` | `(int) -> int[]` | pub | `mesh/build.id` |
| `idstd_d3_cube_cfill` | `(int[],int,int) -> void` | int | `mesh/build.id` |
| `idstd_d3_cube_vfill` | `(int[],int,int) -> void` | int | `mesh/push.id` |
| `idstd_d3_cube_vert` | `(int[],int,int) -> void` | int | `mesh/push.id` |
| `idstd_d3_cube_xyz` | `(int[],int,int) -> void` | int | `mesh/push.id` |
| `idstd_d3_cube_px` | `(int[],int,int) -> void` | int | `mesh/coord.id` |
| `idstd_d3_cube_py` | `(int[],int,int) -> void` | int | `mesh/coord.id` |
| `idstd_d3_cube_pz` | `(int[],int,int) -> void` | int | `mesh/coord.id` |

### 1.10 `term_` — the character-cell terminal (`sys/io/term/`)

`demos/engine`, carried as a copy under `demos/moonbuggy/engine` and
`demos/solitaire/engine`, with every function given the prefix. Files are
relative to `sys/io/term/`.

| function | signature | vis | file |
| --- | --- | --- | --- |
| `term_init` | `(int,int,int) -> void` | pub, **required init** | `scr/init.id` |
| `idstd_term_pal_init` | `() -> void` | int | `scr/init.id` |
| `term_sgr` | `(int) -> string` | pub | `scr/init.id` |
| `idstd_term_scr_init` | `(int,int) -> void` | int | `scr/alloc.id` |
| `idstd_term_scr_alloc` | `(int) -> void` | int | `scr/alloc.id` |
| `idstd_term_scr_fill` | `(int) -> void` | int | `scr/alloc.id` |
| `idstd_term_idx` | `(int,int) -> int` | int | `scr/cell/raw.id` |
| `idstd_term_put` | `(int,int,int,int) -> void` | int | `scr/cell/raw.id` |
| `idstd_term_put_attr` | `(int,int,int) -> void` | int | `scr/cell/raw.id` |
| `term_set` | `(int,int,int,int) -> void` | pub | `scr/cell/set.id` |
| `term_in` | `(int,int) -> int` | pub | `scr/cell/set.id` |
| `idstd_term_blank` | `() -> void` | int | `scr/cell/set.id` |
| `term_clear` | `() -> void` | pub | `scr/cell/clear.id` |
| `idstd_term_clear_from` | `(int) -> void` | int | `scr/cell/clear.id` |
| `idstd_term_blank_at` | `(int) -> void` | int | `scr/cell/clear.id` |
| `term_text` | `(int,int,string,int) -> void` | pub | `draw/stroke.id` |
| `term_hline` | `(int,int,int,int,int) -> void` | pub | `draw/stroke.id` |
| `term_vline` | `(int,int,int,int,int) -> void` | pub | `draw/stroke.id` |
| `term_box` | `(int,int,int,int,int) -> void` | pub | `draw/box.id` |
| `idstd_term_box_edges` | `(int,int,int,int,int) -> void` | int | `draw/box.id` |
| `idstd_term_box_vedges` | `(int,int,int,int,int) -> void` | int | `draw/box.id` |
| `idstd_term_box_corners` | `(int,int,int,int,int) -> void` | int | `draw/corner.id` |
| `idstd_term_box_corners2` | `(int,int,int,int,int) -> void` | int | `draw/corner.id` |
| `term_render` | `() -> void` | pub | `out/render.id` |
| `idstd_term_render_rows` | `(int) -> void` | int | `out/render.id` |
| `idstd_term_render_row` | `(int) -> void` | int | `out/render.id` |
| `idstd_term_render_body` | `() -> void` | int | `out/frame.id` |
| `idstd_term_finish` | `() -> void` | int | `out/frame.id` |
| `idstd_term_rowpos` | `(int) -> string` | int | `out/frame.id` |
| `idstd_term_row` | `(int) -> string` | int | `out/more/row.id` |
| `idstd_term_cell` | `(int,int) -> string` | int | `out/more/row.id` |
| `idstd_term_sgr_at` | `(int,int) -> string` | int | `out/more/row.id` |
| `idstd_term_attr_new` | `(int,int) -> int` | int | `out/more/attr.id` |
| `idstd_term_attr_diff` | `(int) -> int` | int | `out/more/attr.id` |
| `term_key` | `() -> int` | pub | `out/more/tty/key.id` |
| `idstd_term_drain` | `(int,int) -> int` | int | `out/more/tty/key.id` |
| `term_setup` | `() -> void` | pub | `out/more/tty/mode.id` |
| `idstd_term_setup_tail` | `() -> void` | int | `out/more/tty/mode.id` |
| `term_done` | `() -> void` | pub | `out/more/tty/mode.id` |
| `idstd_term_done_msg` | `(string,string) -> void` | int | `out/more/tty/end/done.id` |
| `idstd_term_done_cls` | `() -> void` | int | `out/more/tty/end/done.id` |
| `idstd_term_t_setup` | `() -> void` | int, test fixture | `out/more/tty/end/setup.id` |
| `idstd_term_t_mark` | `() -> void` | int, test fixture | `out/more/tty/end/setup.id` |
| `idstd_term_t_sum` | `() -> int` | int, test fixture | `out/more/tty/end/setup.id` |
| `idstd_term_t_asum` | `() -> int` | int, test fixture | `out/more/tty/end/check.id` |
| `idstd_term_t_pal` | `() -> int` | int, test fixture | `out/more/tty/end/check.id` |

### 1.9 `inp_`, `sys_` — the frame loop's pure part (`sys/win/`)

| function | signature | vis | file |
| --- | --- | --- | --- |
| `inp_live` | `(int) -> int` | pub | `loop.id` |
| `sys_next` | `(int) -> int` | pub | `loop.id` |

### 1.11 Natives — `fs_`, `proc_`, `sock_`, `env_`, `gfx_`, `gl_`, `glwin_` (`sys/io/`, `sys/win/`)

A native is a function whose body is a backend's C: `native fs_open(string
path, string mode) return int;`. Each backend lives inside the module that
wraps it, with its sources and its `backend.id`, and `bin/idc` compiles and
links it only for a build that reaches one of its natives. A native's name is
reserved program-wide exactly like any other function's, so a program that
defines `fs_open` collides with it, whether or not it calls it. Its
parameters reserve nothing (§2). Vis is `native`: callable today, and the
seam the pub functions over it will call. Files are relative to the
backend's directory.

| function | signature | vis | file |
| --- | --- | --- | --- |
| `fs_read` | `(int,int[],int) -> int` | native | `fs/data/cells.id` |
| `fs_write` | `(int,int[],int) -> int` | native | `fs/data/cells.id` |
| `fs_list` | `(string,int[],int) -> int` | native | `fs/data/cells.id` |
| `fs_read_mem` | `(int,word,int) -> int` | native | `fs/data/store.id` |
| `fs_write_mem` | `(int,word,int) -> int` | native | `fs/data/store.id` |
| `fs_open` | `(string,string) -> int` | native | `fs/handle.id` |
| `fs_close` | `(int) -> int` | native | `fs/handle.id` |
| `fs_error` | `() -> int` | native | `fs/handle.id` |
| `fs_size` | `(string) -> int` | native | `fs/path/check.id` |
| `fs_exists` | `(string) -> int` | native | `fs/path/check.id` |
| `fs_remove` | `(string) -> int` | native | `fs/path/check.id` |
| `fs_run` | `(string) -> int` | native | `fs/path/run.id` |
| `fs_mkdir` | `(string) -> int` | native | `fs/path/edit/mkdir/one.id` |
| `fs_mtime` | `(string) -> int` | native | `fs/path/edit/meta.id` |
| `fs_chmod` | `(string,int) -> int` | native | `fs/path/edit/meta.id` |
| `fs_rename` | `(string,string) -> int` | native | `fs/path/edit/meta.id` |
| `fs_mktemp_mem` | `(string,int,word,int) -> int` | native | `fs/path/edit/mktemp/make.id` |
| `proc_spawn` | `(string) -> int` | native | `ipc/proc/handle/create.id` |
| `proc_spawn_limited` | `(string,int,int) -> int` | native | `ipc/proc/handle/create.id` |
| `proc_close` | `(int) -> int` | native | `ipc/proc/handle/close.id` |
| `proc_close_in` | `(int) -> int` | native | `ipc/proc/handle/close.id` |
| `proc_error` | `() -> int` | native | `ipc/proc/handle/error.id` |
| `proc_read` | `(int,int[],int,int) -> int` | native | `ipc/proc/io.id` |
| `proc_read_err` | `(int,int[],int,int) -> int` | native | `ipc/proc/io.id` |
| `proc_write` | `(int,int[],int,int) -> int` | native | `ipc/proc/io.id` |
| `proc_wait` | `(int,int) -> int` | native | `ipc/proc/wait.id` |
| `proc_kill` | `(int) -> int` | native | `ipc/proc/wait.id` |
| `sock_connect` | `(string,int) -> int` | native | `ipc/sock/handle.id` |
| `sock_close` | `(int) -> int` | native | `ipc/sock/handle.id` |
| `sock_error` | `() -> int` | native | `ipc/sock/handle.id` |
| `sock_send` | `(int,int[],int) -> int` | native | `ipc/sock/io.id` |
| `sock_recv` | `(int,int[],int,int) -> int` | native | `ipc/sock/io.id` |
| `env_has` | `(string) -> int` | native | `ipc/env/var.id` |
| `env_get` | `(string,int[],int) -> int` | native | `ipc/env/var.id` |
| `env_error` | `() -> int` | native | `ipc/env/var.id` |
| `gfx_poll` | `() -> int` | native | `gfx/events.id` |
| `gfx_width` | `() -> int` | native | `gfx/events.id` |
| `gfx_height` | `() -> int` | native | `gfx/events.id` |
| `gfx_mouse_x` | `() -> int` | native | `gfx/mouse.id` |
| `gfx_mouse_y` | `() -> int` | native | `gfx/mouse.id` |
| `gfx_mouse_buttons` | `() -> int` | native | `gfx/mouse.id` |
| `gfx_open` | `(int,int,string) -> int` | native | `gfx/window.id` |
| `gfx_present` | `(int[]) -> int` | native | `gfx/window.id` |
| `gfx_close` | `() -> int` | native | `gfx/window.id` |
| `gl_draw_tris` | `(int[],int[],int) -> int` | native | `gl/frame/draw.id` |
| `gl_draw_points` | `(int[],int[],int,int) -> int` | native | `gl/frame/draw.id` |
| `gl_begin_frame` | `(int,int,int) -> int` | native | `gl/frame/cycle.id` |
| `gl_end_frame` | `() -> int` | native | `gl/frame/cycle.id` |
| `gl_read_pixels` | `(int[]) -> int` | native | `gl/frame/cycle.id` |
| `gl_mat_identity` | `() -> int` | native | `gl/mat/build.id` |
| `gl_mat_perspective` | `(int,int,int,int) -> int` | native | `gl/mat/build.id` |
| `gl_mat_translate` | `(int,int,int) -> int` | native | `gl/mat/build.id` |
| `gl_mat_rotate_x` | `(int) -> int` | native | `gl/mat/rotate.id` |
| `gl_mat_rotate_y` | `(int) -> int` | native | `gl/mat/rotate.id` |
| `gl_mat_rotate_z` | `(int) -> int` | native | `gl/mat/rotate.id` |
| `gl_mat_mul` | `(int,int) -> int` | native | `gl/mat/use.id` |
| `gl_set_projection` | `(int) -> int` | native | `gl/mat/use.id` |
| `gl_set_modelview` | `(int) -> int` | native | `gl/mat/use.id` |
| `glwin_mouse_x` | `() -> int` | native | `gl/win/mouse.id` |
| `glwin_mouse_y` | `() -> int` | native | `gl/win/mouse.id` |
| `glwin_mouse_buttons` | `() -> int` | native | `gl/win/mouse.id` |
| `gl_width` | `() -> int` | native | `gl/win/size.id` |
| `gl_height` | `() -> int` | native | `gl/win/size.id` |
| `gl_aspect_x1000` | `() -> int` | native | `gl/win/size.id` |
| `glwin_open` | `(int,int,string) -> int` | native | `gl/win/window.id` |
| `glwin_poll` | `() -> int` | native | `gl/win/window.id` |
| `glwin_close` | `() -> int` | native | `gl/win/window.id` |

### 1.12 `hmap_`, `hset_` — a string-keyed hash table (`core/data/hmap/`)

Open addressing (linear probing) over `str_hash`, three parallel lists the
caller owns (`string[] keys`, `int[] vals`, `int[] used`) plus a 1-slot
`int[] cnt` for the live entry count — see `base/init.id` for why (no records,
so this is the same "record-as-list" convention README.md names; chaining
would need a bucket that is itself a list of records, which `id` cannot
express cheaply). `hmap_init` allocates capacity 8; `hmap_put` doubles and
rehashes through `grow/` once the table passes 75% full
(`base/slot.id`'s `idstd_hmap_full`). A string set is the same table with the
value ignored (`hset_add`); `hmap_has`, `hmap_get` and `hmap_count` serve both.
No deletion — `base/init.id` says why.

| function | signature | vis | file |
| --- | --- | --- | --- |
| `hmap_init` | `(string[],int[],int[],int[]) -> void` | pub | `base/init.id` |
| `idstd_hmap_fill` | `(string[],int[],int[],int) -> void` | int | `base/init.id` |
| `idstd_hmap_fill_one` | `(string[],int[],int[]) -> void` | int | `base/init.id` |
| `idstd_hmap_probe` | `(string[],int[],string,int) -> int` | int | `base/slot.id` |
| `idstd_hmap_slot` | `(string[],int[],string) -> int` | int | `base/slot.id` |
| `idstd_hmap_full` | `(int[],int[]) -> int` | int | `base/slot.id` |
| `hmap_get` | `(string[],int[],int[],string,int) -> int` | pub | `rw/get.id` |
| `hmap_has` | `(string[],int[],string) -> int` | pub | `rw/get.id` |
| `hmap_count` | `(int[]) -> int` | pub | `rw/get.id` |
| `hmap_put` | `(string[],int[],int[],int[],string,int) -> void` | pub | `rw/put.id` |
| `hset_add` | `(string[],int[],int[],int[],string) -> void` | pub | `rw/put.id` |
| `idstd_hmap_grow` | `(string[],int[],int[]) -> void` | int | `grow/rehash.id` |
| `idstd_hmap_rehash` | `(string[],int[],int[],string[],int[],int[],int) -> void` | int | `grow/rehash.id` |
| `idstd_hmap_rehash_slot` | `(string[],int[],int[],string[],int[],int[],int) -> void` | int | `grow/rehash.id` |
| `idstd_hmap_rehash_place` | `(string[],int[],int[],int,string,int) -> void` | int | `grow/place.id` |
| `idstd_hmap_copy_old` | `(string[],int[],int[],string[],int[],int[],int,int) -> void` | int | `grow/place.id` |
| `idstd_hmap_copy_old_one` | `(string[],int[],int[],string[],int[],int[],int) -> void` | int | `grow/place.id` |
| `idstd_hmap_copy_new` | `(string[],int[],int[],string[],int[],int[],int) -> void` | int | `grow/copy.id` |
| `idstd_hmap_copy_new_one` | `(string[],int[],int[],string[],int[],int[],int) -> void` | int | `grow/copy.id` |
### 1.13 `fs_mkdir_p` — recursive directory creation (`sys/io/fs/path/edit/mkdir/`)

Built in `id` on top of the single-level `fs_mkdir` native rather than more C
(IDSTD.md's own preference where a native and a loop both work). `1` on
success, `0` on the first level that fails.

| function | signature | vis | file |
| --- | --- | --- | --- |
| `fs_mkdir_p` | `(string) -> int` | pub | `mkdir/deep.id` |
| `idstd_fs_mkdir_p_loop` | `(string,int,string) -> int` | int | `mkdir/deep.id` |
| `idstd_fs_mkdir_p_step` | `(string,int,string) -> int` | int | `mkdir/deep.id` |
| `idstd_fs_mkdir_p_t_setup` | `() -> void` | int, test fixture | `mkdir/fixture.id` |
| `idstd_fs_mkdir_p_t_clean` | `() -> int` | int, test fixture | `mkdir/fixture.id` |
### 1.13a `fs_mktemp` — scratch files and directories (`sys/io/fs/path/edit/mktemp/`)

`/tmp/<prefix>` plus six characters `mkstemp`/`mkdtemp` chose, created before it returns, so two processes never share one. `dir` 0 makes a file, 1 a directory; `""` on failure. Every test case that writes a scratch file names it with this, never a fixed `/tmp` path.

| function | signature | vis | file |
| --- | --- | --- | --- |
| `fs_mktemp` | `(string,int) -> string` | pub | `mktemp/make.id` |
| `idstd_fs_mktemp_take` | `(string,int,int) -> string` | int | `mktemp/make.id` |
| `idstd_fs_mktemp_t_setup` | `() -> void` | int, test fixture | `mktemp/fixture.id` |
| `idstd_fs_mktemp_t_check` | `() -> int` | int, test fixture | `mktemp/fixture.id` |
### 1.14 `re_` — regular expressions (`core/text/str/part/split/re/`)

**Where this lives, and why.** `re_` is a text module and belongs beside
`str_`/`chr_`/`fmt_` under `core/text/`, but by the time it was added
`core/text/` (`chr/`, `fmt/`, `str/`), `core/text/str/` (`make/`, `part/`,
`scan/`) and every ancestor up through the project root (`core/`, `sys/`,
`gfx/`) were already at the rule-of-3 ceiling — confirmed against `bin/idc`
directly, not assumed: a 4-entry root is a hard compile error. The one spare
slot anywhere in the text tree was the third entry of `str/part/split/`
(`bound.id`, `field.id`, and now `re/`), so that is where the module's own,
otherwise-unrelated three-per-level tree hangs. This is a placement of last
resort, not a claim that regexes are a kind of string-split helper; it is
recorded here because it is exactly the kind of decision `id_development`
should be told about (see the commit and the task report for this change).

**Files and match:** `int[]` throughout — a compiled pattern is
`idstd_prog` (flat 3-int instructions: op, arg1, arg2; opcodes 0 CHAR,
1 ANY, 2 CLASS, 3 SPLIT, 4 JMP, 5 SAVE, 6 MATCH, 7 BOL, 8 EOL) alongside
`idstd_cls` (character classes, 9 ints each: a negate flag then a 256-bit
membership map). `re_compile` builds both from a pattern string in two
internal passes: `idstd_parse_*` (an AST, as four more parallel `int[]`:
node type, left child, right child, value) and `idstd_emit_*`
(AST -> `idstd_prog`, also numbering capture groups left to right).
`idstd_run` and friends are the backtracking matcher over the compiled
form. **No-match convention:** every position a caller can read back — a
capture's start/end, `idstd_find`-style spans — is `-1` for "no match" or
"this group did not participate", consistently with `str_find` and the rest
of `core/text`; the one string-valued reader, `re_group`, answers `""` for
the same case (check `re_group_start` first to tell that apart from a group
that matched a genuinely empty span).

| function | signature | vis | file |
| --- | --- | --- | --- |
| `idstd_cls_new` | `(int[]) -> int` | int | `class/bits/build.id` |
| `idstd_cls_set` | `(int[],int,int) -> void` | int | `class/bits/build.id` |
| `idstd_cls_setrange` | `(int[],int,int,int) -> void` | int | `class/bits/build.id` |
| `idstd_cls_test` | `(int[],int,int) -> int` | int | `class/bits/test.id` |
| `idstd_parse_class` | `(string,int,int[],int[],int[],int[],int[]) -> int` | int | `class/body/top.id` |
| `idstd_class_body` | `(string,int,int,int[],int[],int[],int[],int[]) -> int` | int | `class/body/top.id` |
| `idstd_class_loop` | `(string,int,int,int[]) -> int` | int | `class/body/top.id` |
| `idstd_class_item_ahead` | `(string,int) -> int` | int | `class/body/item/parse.id` |
| `idstd_class_item` | `(string,int,int,int[]) -> int` | int | `class/body/item/parse.id` |
| `idstd_class_maybe_range` | `(string,int,int,int,int[]) -> int` | int | `class/body/item/parse.id` |
| `idstd_class_range_ahead` | `(string,int) -> int` | int | `class/body/item/range/extend.id` |
| `idstd_class_range` | `(string,int,int,int,int[]) -> int` | int | `class/body/item/range/extend.id` |
| `idstd_ast_push` | `(int[],int[],int[],int[],int,int,int,int) -> void` | int | `parse/ast/build.id` |
| `idstd_ast_wrap1` | `(int[],int[],int[],int[],int,int) -> void` | int | `parse/ast/build.id` |
| `idstd_ast_wrap2` | `(int[],int[],int[],int[],int,int) -> void` | int | `parse/ast/build.id` |
| `idstd_parse_alt` | `(string,int,int[],int[],int[],int[],int[]) -> int` | int | `parse/alt/pipe.id` |
| `idstd_alt_tail` | `(string,int,int,int[],int[],int[],int[],int[]) -> int` | int | `parse/alt/pipe.id` |
| `idstd_alt_combine` | `(int[],int[],int[],int[],int) -> void` | int | `parse/alt/pipe.id` |
| `idstd_parse_concat` | `(string,int,int[],int[],int[],int[],int[]) -> int` | int | `parse/alt/seq/concat.id` |
| `idstd_concat_run` | `(string,int,int[],int[],int[],int[],int[]) -> int` | int | `parse/alt/seq/concat.id` |
| `idstd_concat_loop` | `(string,int,int[],int[],int[],int[],int[]) -> int` | int | `parse/alt/seq/concat.id` |
| `idstd_atom_ahead` | `(string,int) -> int` | int | `parse/alt/seq/rep/quant.id` |
| `idstd_parse_repeat` | `(string,int,int[],int[],int[],int[],int[]) -> int` | int | `parse/alt/seq/rep/quant.id` |
| `idstd_repeat_op` | `(string,int,int,int[],int[],int[],int[]) -> int` | int | `parse/alt/seq/rep/quant.id` |
| `idstd_parse_atom` | `(string,int,int[],int[],int[],int[],int[]) -> int` | int | `parse/atom/dispatch.id` |
| `idstd_parse_simple` | `(string,int,int,int[],int[],int[],int[]) -> int` | int | `parse/atom/dispatch.id` |
| `idstd_parse_escape_atom` | `(string,int,int[],int[],int[],int[]) -> int` | int | `parse/atom/dispatch.id` |
| `idstd_parse_group` | `(string,int,int[],int[],int[],int[],int[]) -> int` | int | `parse/atom/group/paren.id` |
| `idstd_group_close` | `(string,int,int,int[],int[],int[],int[]) -> int` | int | `parse/atom/group/paren.id` |
| `idstd_prog_push3` | `(int[],int,int,int) -> void` | int | `vm/emit/glue.id` |
| `idstd_emit_concat` | `(int,int[],int[],int[],int[],int[],int[],int) -> int` | int | `vm/emit/glue.id` |
| `idstd_compile_ok` | `(int[],int[],int[],int[],int[],int[]) -> int` | int | `vm/emit/glue.id` |
| `idstd_emit` | `(int,int[],int[],int[],int[],int[],int[],int) -> int` | int | `vm/emit/disp/walk.id` |
| `idstd_emit_split_first` | `(int,int[],int[],int[],int[],int[],int[],int,int) -> int` | int | `vm/emit/disp/walk.id` |
| `idstd_emit_plus` | `(int,int[],int[],int[],int[],int[],int[],int) -> int` | int | `vm/emit/disp/quant/plus.id` |
| `idstd_emit_alt` | `(int,int[],int[],int[],int[],int[],int[],int) -> int` | int | `vm/emit/disp/quant/branch/alt.id` |
| `idstd_emit_group` | `(int,int[],int[],int[],int[],int[],int[],int) -> int` | int | `vm/emit/disp/quant/branch/alt.id` |
| `idstd_caps_init` | `(int) -> int[]` | int | `vm/match/caps/slots.id` |
| `idstd_caps_restore` | `(int[],int[]) -> void` | int | `vm/match/caps/slots.id` |
| `idstd_max_save` | `(int[]) -> int` | int | `vm/match/caps/scan/prep.id` |
| `idstd_max_save_step` | `(int[],int,int) -> int` | int | `vm/match/caps/scan/prep.id` |
| `idstd_finish` | `(int,int[],int[]) -> int` | int | `vm/match/caps/scan/prep.id` |
| `idstd_run` | `(int[],int[],string,int,int,int[],int) -> int` | int | `vm/match/op/simple.id` |
| `idstd_op_char` | `(int[],int[],string,int,int,int[],int) -> int` | int | `vm/match/op/simple.id` |
| `idstd_op_any` | `(int[],int[],string,int,int,int[],int) -> int` | int | `vm/match/op/simple.id` |
| `idstd_op_class` | `(int[],int[],string,int,int,int[],int) -> int` | int | `vm/match/op/more/tests.id` |
| `idstd_op_bol` | `(int[],int[],string,int,int,int[],int) -> int` | int | `vm/match/op/more/tests.id` |
| `idstd_op_eol` | `(int[],int[],string,int,int,int[],int) -> int` | int | `vm/match/op/more/tests.id` |
| `idstd_op_jmp` | `(int[],int[],string,int,int,int[],int) -> int` | int | `vm/match/op/more/ctrl/basic.id` |
| `idstd_op_save` | `(int[],int[],string,int,int,int[],int) -> int` | int | `vm/match/op/more/ctrl/basic.id` |
| `idstd_op_match` | `(int,int) -> int` | int | `vm/match/op/more/ctrl/basic.id` |
| `idstd_op_split` | `(int[],int[],string,int,int,int[],int) -> int` | int | `vm/match/op/more/ctrl/split/attempt.id` |
| `idstd_search_from` | `(int[],int[],string,int,int[]) -> int` | int | `vm/match/op/more/ctrl/split/attempt.id` |
| `idstd_search_try` | `(int[],int[],string,int,int[]) -> int` | int | `vm/match/op/more/ctrl/split/attempt.id` |
| `re_compile` | `(string,int[],int[]) -> int` | pub | `vm/api/compile.id` |
| `re_exec` | `(int[],int[],string,int[]) -> int` | pub | `vm/api/compile.id` |
| `re_full` | `(int[],int[],string,int[]) -> int` | pub | `vm/api/compile.id` |
| `re_group_start` | `(int[],int) -> int` | pub | `vm/api/group.id` |
| `re_group_end` | `(int[],int) -> int` | pub | `vm/api/group.id` |
| `re_group` | `(string,int[],int) -> string` | pub | `vm/api/group.id` |

**Supported syntax:** literal bytes; backslash escapes any byte as itself
(`\(`, `\.`, `\\`, ...); `.` (any byte, never end of string); `[...]` and
`[^...]` classes with `a-z`-style ranges; `^` and `$` anchors; `*` `+` `?`
quantifiers, greedy; `(...)` numbered capturing groups (`1..`; group `0` is
always the whole match); `|` alternation. **Deliberately not implemented:**
non-greedy quantifiers (`*?` etc.), `{m,n}` counted repetition, `\d`/`\w`/`\s`
shorthand classes and other backslash letter-escapes (a lone backslash always
escapes the one byte after it), backreferences, and lookaround — none of
which the three motivating uses (`idc`'s `conf.id` key pattern, a case-line
matcher, an identifier checker) need.
### 1.15 `inf_`, `gz_` — DEFLATE and gzip decompression (`core/data/seq/byte/inflate/`)

Moved from the editor's `lib/zip/inf` (`docs/HACKING.md`-style port, not a rewrite): same
algorithm, same decomposition, every parameter and local given the `idstd_` prefix and
every internal function renamed to it. `inf_inflate` (raw DEFLATE, RFC 1951) and
`gz_decompress` (the gzip container, RFC 1952, around the same stream) are the two public
entry points; `inf_failed` is the third, added so a malformed stream is a value a caller
checks (`inf_fail`, set by `idstd_inf_fail`) instead of a trap -- `id` has no early return,
so `idstd_inf_run`'s block loop and `gz_decompress`'s own second phase both check it too,
to stop working from state a failure left invalid. Everything else is a fold step or a
bit-reader the block-type limit split out.

| function | signature | vis | file |
| --- | --- | --- | --- |
| `inf_inflate` | `(int[],int) -> int[]` | pub | `api/deflate.id` |
| `idstd_inf_init` | `(int[],int) -> void` | int | `api/deflate.id` |
| `idstd_inf_run` | `() -> void` | int | `api/deflate.id` |
| `idstd_gz_extra` | `(int[],int,int) -> int` | int | `api/gzhdr.id` |
| `idstd_gz_name` | `(int[],int,int) -> int` | int | `api/gzhdr.id` |
| `idstd_gz_comment` | `(int[],int,int) -> int` | int | `api/gzhdr.id` |
| `idstd_gz_offset` | `(int[]) -> int` | int | `api/gzip.id` |
| `idstd_gz_skip0` | `(int[],int) -> int` | int | `api/gzip.id` |
| `gz_decompress` | `(int[]) -> int[]` | pub | `api/gzip.id` |
| `idstd_inf_emit` | `(int) -> void` | int | `bit/emit.id` |
| `idstd_inf_tabs` | `() -> void` | int | `bit/emit.id` |
| `idstd_inf_tabs2` | `() -> void` | int | `bit/emit.id` |
| `idstd_inf_count` | `(int[],int[]) -> void` | int | `bit/huff/make.id` |
| `idstd_inf_bump` | `(int[],int) -> void` | int | `bit/huff/make.id` |
| `idstd_inf_starts` | `(int[]) -> int[]` | int | `bit/huff/make.id` |
| `idstd_inf_walk` | `(int[],int[],int[]) -> void` | int | `bit/huff/put/place.id` |
| `idstd_inf_put` | `(int[],int[],int[],int) -> void` | int | `bit/huff/put/place.id` |
| `idstd_inf_decode` | `(int[],int[]) -> int` | int | `bit/huff/put/place.id` |
| `idstd_inf_dec_step` | `(int[],int[],int[]) -> void` | int | `bit/huff/put/step.id` |
| `idstd_inf_dec_pick` | `(int[],int[],int) -> void` | int | `bit/huff/put/step.id` |
| `idstd_inf_dec_next` | `(int[],int) -> void` | int | `bit/huff/put/step.id` |
| `idstd_inf_t_de_last` | `() -> int` | int | `bit/huff/put/table.id` |
| `idstd_inf_fail_reset` | `() -> void` | int | `bit/huff/put/table.id` |
| `inf_failed` | `() -> int` | pub | `bit/huff/put/table.id` |
| `idstd_inf_fill2` | `(int[],int[],int[]) -> void` | int | `bit/huff/tree/fill.id` |
| `idstd_inf_tree` | `(int[]) -> int[][]` | int | `bit/huff/tree/huff.id` |
| `idstd_inf_grow` | `(int[],int[][]) -> void` | int | `bit/huff/tree/huff.id` |
| `idstd_inf_fill` | `(int[],int[],int[]) -> void` | int | `bit/huff/tree/huff.id` |
| `idstd_inf_t_lb_first` | `() -> int` | int | `bit/huff/tree/read_tab.id` |
| `idstd_inf_t_le_last` | `() -> int` | int | `bit/huff/tree/read_tab.id` |
| `idstd_inf_t_db_first` | `() -> int` | int | `bit/huff/tree/read_tab.id` |
| `idstd_inf_bit` | `() -> int` | int | `bit/read/bit.id` |
| `idstd_inf_bit2` | `(int,int) -> int` | int | `bit/read/bit.id` |
| `idstd_inf_bits` | `(int) -> int` | int | `bit/read/bit.id` |
| `idstd_inf_take` | `(int[]) -> void` | int | `bit/read/take.id` |
| `idstd_inf_t_stream` | `() -> void` | int | `bit/read/take.id` |
| `idstd_inf_t_ready` | `() -> void` | int | `bit/read/take.id` |
| `idstd_inf_fixed` | `() -> void` | int | `blk/dyn/fixed/fix.id` |
| `idstd_inf_fixed2` | `(int[][]) -> void` | int | `blk/dyn/fixed/fix.id` |
| `idstd_inf_fix_lit` | `() -> int[]` | int | `blk/dyn/fixed/fix.id` |
| `idstd_inf_fix_tail` | `(int[]) -> void` | int | `blk/dyn/fixed/tail.id` |
| `idstd_inf_t_fixed_a` | `() -> void` | int | `blk/dyn/fixed/tail.id` |
| `idstd_inf_t_match_stream` | `() -> void` | int | `blk/dyn/fixed/tail.id` |
| `idstd_inf_dynamic` | `() -> void` | int | `blk/dyn/hdr/drive.id` |
| `idstd_inf_hdr` | `(int[]) -> void` | int | `blk/dyn/hdr/drive.id` |
| `idstd_inf_dyn` | `(int[]) -> void` | int | `blk/dyn/hdr/drive.id` |
| `idstd_inf_hdr_lit` | `(int[]) -> void` | int | `blk/dyn/hdr/fields.id` |
| `idstd_inf_hdr_dist` | `(int[]) -> void` | int | `blk/dyn/hdr/fields.id` |
| `idstd_inf_hdr_clen` | `(int[]) -> void` | int | `blk/dyn/hdr/fields.id` |
| `idstd_inf_t_dyn_direct` | `() -> void` | int | `blk/dyn/hdr/more/direct.id` |
| `idstd_inf_fail_x_fixture` | `() -> void` | int | `blk/dyn/hdr/more/direct.id` |
| `idstd_inf_dyn2` | `(int[],int[][]) -> void` | int | `blk/dyn/hdr/more/split.id` |
| `idstd_inf_t_init_one` | `() -> void` | int | `blk/dyn/hdr/more/split.id` |
| `idstd_inf_t_init_ff` | `() -> void` | int | `blk/dyn/hdr/more/split.id` |
| `idstd_inf_clens` | `(int) -> int[]` | int | `blk/dyn/len/clen.id` |
| `idstd_inf_clen_loop` | `(int[],int) -> void` | int | `blk/dyn/len/clen.id` |
| `idstd_inf_clen_one` | `(int[],int[],int) -> void` | int | `blk/dyn/len/clen.id` |
| `idstd_inf_lens` | `(int[],int[],int) -> int[]` | int | `blk/dyn/len/lens.id` |
| `idstd_inf_lens_one` | `(int[],int[],int[]) -> void` | int | `blk/dyn/len/lens.id` |
| `idstd_inf_rep` | `(int[],int) -> void` | int | `blk/dyn/len/lens.id` |
| `idstd_inf_body` | `(int[][],int[][]) -> void` | int | `blk/dyn/len/sp/body.id` |
| `idstd_inf_sym` | `(int[][],int[][],int[]) -> void` | int | `blk/dyn/len/sp/body.id` |
| `idstd_inf_end` | `(int,int[][],int[]) -> void` | int | `blk/dyn/len/sp/body.id` |
| `idstd_inf_match` | `(int,int[],int[]) -> void` | int | `blk/dyn/len/sp/mt/copy.id` |
| `idstd_inf_back` | `(int,int) -> void` | int | `blk/dyn/len/sp/mt/copy.id` |
| `idstd_inf_copy` | `(int[]) -> void` | int | `blk/dyn/len/sp/mt/copy.id` |
| `idstd_inf_dist` | `(int) -> int` | int | `blk/dyn/len/sp/mt/dist.id` |
| `idstd_inf_len` | `(int) -> int` | int | `blk/dyn/len/sp/mt/dist.id` |
| `idstd_inf_match2` | `(int,int) -> void` | int | `blk/dyn/len/sp/mt/finish.id` |
| `idstd_inf_t_emitted` | `() -> void` | int | `blk/dyn/len/sp/mt/finish.id` |
| `idstd_inf_t_block2_empty` | `() -> void` | int | `blk/dyn/len/sp/mt/finish.id` |
| `idstd_inf_fix_dist` | `() -> int[]` | int | `blk/dyn/len/sp/run/dist.id` |
| `idstd_inf_t_dyn_stream` | `() -> void` | int | `blk/dyn/len/sp/run/dist.id` |
| `idstd_inf_t_dyn_ready` | `() -> void` | int | `blk/dyn/len/sp/run/dist.id` |
| `idstd_inf_split` | `(int[],int,int) -> void` | int | `blk/dyn/len/sp/run/split.id` |
| `idstd_inf_split2` | `(int[],int,int,int[][]) -> void` | int | `blk/dyn/len/sp/run/split.id` |
| `idstd_inf_t_dyn2_probe` | `() -> void` | int | `blk/dyn/len/sp/run/split.id` |
| `idstd_inf_t_stored_ready` | `() -> void` | int | `blk/hdr/at.id` |
| `idstd_inf_t_stored_one` | `() -> void` | int | `blk/hdr/at.id` |
| `idstd_inf_t_block2_ready` | `() -> void` | int | `blk/hdr/at.id` |
| `idstd_inf_block` | `(int[]) -> void` | int | `blk/hdr/blk.id` |
| `idstd_inf_block2` | `() -> void` | int | `blk/hdr/blk.id` |
| `idstd_inf_kind` | `(int) -> void` | int | `blk/hdr/blk.id` |
| `idstd_inf_comp` | `(int) -> void` | int | `blk/hdr/comp.id` |
| `idstd_inf_t_outlen` | `() -> int` | int | `blk/hdr/comp.id` |
| `idstd_inf_t_empty_block` | `() -> void` | int | `blk/hdr/comp.id` |
| `idstd_inf_stored` | `() -> void` | int | `blk/raw.id` |
| `idstd_inf_raw` | `(int) -> void` | int | `blk/raw.id` |
| `idstd_inf_fail` | `(string) -> void` | int | `blk/raw.id` |

### 1.16 `sha256_` — SHA-256 (`core/data/seq/byte/sha256/`)

New code. `sha256_hash` is the only public entry point; `int` is 32-bit two's complement
and wraps on `+`/`*`, matching FIPS 180-4's mod-2^32 addition directly, so nothing here
needs `word` except the padding's 64-bit length field and the `ushr`-on-negative-`int`
workaround `idstd_sha256_rotr`, `idstd_sha256_s0` and `idstd_sha256_s1` carry (`bin/idc`'s
`ushr` answers a sign-extending shift for a negative `int` rather than the zero-filling one
`docs/SPEC.md` section 2.3 specifies; see the workspace report).

| function | signature | vis | file |
| --- | --- | --- | --- |
| `idstd_sha256_compress` | `(int[],int[],int[]) -> void` | int | `hash/compress.id` |
| `idstd_sha256_addback` | `(int[],int[]) -> void` | int | `hash/compress.id` |
| `idstd_sha256_block` | `(int[],int[],int[],int) -> void` | int | `hash/compress.id` |
| `idstd_sha256_pad` | `(int[]) -> int[]` | int | `hash/pad.id` |
| `idstd_sha256_append_len` | `(int[],int) -> void` | int | `hash/pad.id` |
| `idstd_sha256_len_byte` | `(int[],word,int) -> void` | int | `hash/pad.id` |
| `sha256_hash` | `(int[]) -> string` | pub | `hash/top.id` |
| `idstd_sha256_blocks` | `(int[],int[],int[]) -> void` | int | `hash/top.id` |
| `idstd_sha256_hex` | `(int[]) -> string` | int | `hash/top.id` |
| `idstd_sha256_rotr` | `(int,int) -> int` | int | `round/bit.id` |
| `idstd_sha256_ch` | `(int,int,int) -> int` | int | `round/bit.id` |
| `idstd_sha256_maj` | `(int,int,int) -> int` | int | `round/bit.id` |
| `idstd_sha256_bsig0` | `(int) -> int` | int | `round/sig.id` |
| `idstd_sha256_bsig1` | `(int) -> int` | int | `round/sig.id` |
| `idstd_sha256_round` | `(int[],int[],int[],int) -> void` | int | `round/step.id` |
| `idstd_sha256_shift` | `(int[],int,int) -> void` | int | `round/step.id` |
| `idstd_sha256_sch_ext` | `(int[],int) -> void` | int | `sched/ext.id` |
| `idstd_sha256_s0` | `(int) -> int` | int | `sched/ext.id` |
| `idstd_sha256_s1` | `(int) -> int` | int | `sched/ext.id` |
| `idstd_sha256_sched` | `(int[],int) -> int[]` | int | `sched/make.id` |
| `idstd_sha256_sch16` | `(int[],int[],int,int) -> void` | int | `sched/make.id` |
| `idstd_sha256_sch64` | `(int[]) -> void` | int | `sched/make.id` |

---

## 2. Variable names and their one permitted type

**Every parameter and every local variable idstd declares is spelled
`idstd_<name>`.** A parameter or local is never bare: `a` is always
`idstd_a`, `ret_i` is always `idstd_ret_i`, whatever function it is declared
in. This is newer than the table below and stricter than what it used to say:
before this rule, a bare name like `a` or `cn` *was* reserved program-wide the
moment idstd declared it, and `idc_in_id_calc` broke on exactly that — a
parameter named `cn` colliding with the compiler's own function `cn`, in a
file that never mentioned idstd. The `idstd_` prefix retires that hazard: a
prefixed local cannot collide with anything a user program or another
imported tree would plausibly write, so this table stops being *reserved*
vocabulary and goes back to being what a naming table normally is — a record
of what each spelling means, kept so two functions do not invent two
spellings of one idea.

**A `native` declaration's parameters are not prefixed, and are not listed
here** (decided 2026-09-13). They keep the spelling of the C header they
mirror — `native fs_open(string path, string mode)`, as `fs.h` has it —
because they are not variables: a native has no body, so its parameters enter
no symbol table and reserve no name in any unit (`id_development`'s
`idc/tests/backends.sh` builds a program that exports a name one of them
uses). The collision the prefix exists to prevent cannot happen through them,
so the rule has nothing to do there, and neither `.tests/prefix_check.awk` nor
`.tests/tool/names` reads them.

**The table below still names the meanings, spelled without the `idstd_`
prefix** — that prefix is now mechanical and uniform rather than a per-name
choice, so writing it into 92 rows would only repeat §0's rule 92 times. Read
every name in this section as `idstd_<name>` when you see it declared or used
in the actual source.

The list below predates the prefix and is kept for its history: it was
deliberately short — 54 names — and drawn from `idem/docs/NAMES.md` §2 wherever
a meaning already had a spelling there, so that a program importing both kept
one vocabulary. **It is known incomplete as of this rename**: the library also
declares `ret_i`, `ret_s`, `ret_f` (a function's own return-value local, one
per type, everywhere a value is built up before the closing `return`) and a
family of `<call>_v`-suffixed locals (`fx_sqbit_v`, `str_slice_v`, `charat_v`,
and others — the temporary a call's result is assigned to, since `id` forbids
a call as a call argument) that this section has never listed, a gap
`.tests/names.py` was already failing on before this rename touched anything.
The `<call>_v` locals are now registered below, as one row per type: every
`<x>_v` name means "the result of calling `<x>`", and `<x>_v2` is a second
such result in the same function.

**`int`**

| name | meaning |
| --- | --- |
| `a` `b` `c` | generic operands of an arithmetic helper |
| `x` `y` | the coordinates of a point — `fx_atan2(y, x)`. Claimed reluctantly: they are `int` in every `id` program that has ever existed, and the inverse tangent has no other honest spelling for its arguments |
| `i` `j` | loop indices |
| `n` | a count, length or limit |
| `m` | a result being built by a helper (a min, a root, a power, a width) |
| `bsum` | a test fixture's byte-sum accumulator, read back from the flat store |
| `v` | a value on its way into or out of a list slot |
| `t` | an interpolation parameter, per-mille |
| `lo` `hi` | inclusive lower / upper bound of a clamp or a random range |
| `sc` | a fixed-point scale (1000 for this library's units) |
| `bt` | the current bit (always a power of 4) in the bit-by-bit square root |
| `deg` | an angle in millidegrees |
| `ndg` | an angle normalised into [0, 360000) millidegrees |
| `dg` | an angle in whole degrees, any int -- `fx_sin_deg`'s argument |
| `rm` | millidegree remainder within a quadrant, 0..89999 |
| `q` | a quadrant, or a running result inside a formatter |
| `sv` | a ×1000 sine or cosine value |
| `ax` `ay` | the absolute value of a coordinate, inside the inverse tangent |
| `seed` | a PRNG seed supplied by a caller |
| `sd` `nx` | the PRNG's current and next state value |
| `mn` | the lower of two bounds after ordering them |
| `sp` | the span (count of values) of an inclusive range |
| `at` | an index answer, or the answer so far in a fold; -1 for none |
| `hit` | a 0/1 match result |
| `p` | a write offset into the flat store, as an int |
| `c` | one byte code, 0..255, or -1 for past the end |
| `hv` | a rolling hash value |
| `wv` | a value on its way into a `word[]` slot — `v`'s counterpart for `wset`, since a name keeps one type |
| `ln` | a source line number |
| `more` | a 0/1 "there is another one after this" flag |
| `buf_cmp_one_v` `charat_v2` `chr_hex_v` `chr_lower_v` `chr_upper_v` `fx_abs_v` `fx_abs_v2` `fx_max_v` `fx_min_v` `len_v` `str_findat_v` `str_join_sep_v` `str_ws_start_v` `idstd_inf_bit_v` `idstd_inf_bits_v` `idstd_inf_bits_v2` `idstd_inf_bits_v3` `idstd_inf_decode_v` `idstd_inf_dist_v` `idstd_lst_last_v` | the result of calling the function the name starts with (`<call>_v`; a second one in the same function is `<call>_v2`), named because a call cannot be a call's argument |
| `idstd_flg` | a gzip header's flag byte |
| `idstd_s0` `idstd_s1` | SHA-256's two "small sigma" terms in a message-schedule extension, or the "big sigma" terms in a compression round — `idstd_sha256_sch_ext`, `idstd_sha256_round` |
| `idstd_t1` `idstd_t2` | SHA-256 compression round's two temporaries, T1 and T2 (FIPS 180-4 section 6.2.2) |
| `idstd_u` | a value already narrowed back from the `word`-masked `ushr` workaround (`idstd_sha256_s0`/`idstd_sha256_s1`; see `round/bit.id`) |
| `idstd_xlen` | a gzip FEXTRA field's byte length |
| `d` | the difference of two bytes, in a comparison that answers an ordering |
| `k` | a quotient being adjusted — `fx_fdiv`'s floor step, one below `n` or not |
| `sg` | the sign of a product, -1, 0 or 1 |
| `sn` | the next candidate in a bit-by-bit search — `n`'s successor, since a name keeps one type |
| `w` `h` | a width and a height in pixels or cells — `sf_l_init`, `d2_l_rect`. Claimed when the surface arrived, for the reason `x` and `y` were: a rectangle has no other honest spelling |
| `cv` | a packed `0xRRGGBB` colour value. **Not `col`**: `demos/galaxy` exports a global named `col`, and an export's name is reserved in every unit of that program, library included |
| `cr` `cg` `cb` | one colour channel, 0..255 — `d2_pack`'s arguments, spelled as idem's `d2_rgb` spells them |
| `mag` | a whole-number magnification of the 8x8 face |
| `bits` | one row of a glyph as a mask, bit 128 leftmost |
| `bit` | the mask bit being tested, 128 down to 1 |
| `half` | a cube's half-extent, in the caller's units (millunits in every demo so far) |
| `cix` | a cube corner, 0..7, its bits choosing -half or +half on x, y and z. **Not `cn`**: the compiler's own source defines a function `cn`, and a variable may not share a function's name in one build |
| `ev` | one polled window event: a key code, -1 for none, -2 for close |
| `attr` | a terminal cell's colour attribute, an index into `term_pal` |
| `idstd_i` | an offset into a byte list, in `idstd_pcsf_u32` |
| `idstd_v` | the value of a field read out of a byte list |
| `idstd_hit` | a 0/1 match result, in the PSF magic tests |
| `idstd_val` | the value half of a key/value pair, in `hmap_put` and the rehash it may trigger |
| `idstd_def` | `hmap_get`'s default, returned when the key is absent |
| `idstd_slot` | the table slot a key resolves to, in `idstd_hmap_slot`'s callers |
| `idstd_hash_v` | `str_hash(key)`, before it is folded into a starting bucket |
| `idstd_nslot` | a slot in the *new* table, while `core/data/hmap/grow/` rehashes into it |
| `idstd_oldcap` | a hash table's capacity before a resize, in `core/data/hmap/grow/` |
| `idstd_ncap` | a hash table's capacity after a resize, in `core/data/hmap/grow/` |
| `idstd_cap` | the length of the *old* table's parallel lists, in `idstd_hmap_copy_old` |
| `idstd_made_i` | `fs_mkdir`'s own 0/-1, on its way to the 1/0 `idstd_ok` the `fs_mkdir_p` family returns (`idstd_made_i + 1`) |
| `idstd_ok` | a 1/0 success flag — the `fs_mkdir_p` family and its test fixtures |
| `idstd_prefix` | the leading part of a scratch path's name, in `fs_mktemp` |
| `idstd_dir` | 0 for a scratch file, 1 for a scratch directory, in `fs_mktemp` |
| `idstd_got` | the native's answer: a length, or -1, in `idstd_fs_mktemp_take` |
| `idstd_tmp_s` | the scratch path a `fs_mktemp` fixture made |
| `idstd_gone` | `fs_remove`'s 0/-1 as a fixture cleans up |
| `op` | a compiled regex instruction's opcode (`idstd_prog[pc]`), 0..8 |
| `pc` | an index into `idstd_prog`, always a multiple of 3 |
| `a1` `a2` | a compiled instruction's two operands, `idstd_prog[pc + 1]` and `[pc + 2]` |
| `req_end` | the byte offset a regex match must end at exactly, or -1 for "anywhere" — `idstd_run`'s full-match/search switch |
| `ok` | a 0/1 "the attempt succeeded" flag |
| `gn` | the next unassigned regex capture-group number; `mygn` is the one an `idstd_emit_group` call claims for itself before passing `gn + 1` on to its child |
| `mygn` | see `gn` |
| `ng` | the number of capture groups a compiled regex pattern has |
| `loop` | a 0/1 "add the loop-back jump too" flag — `idstd_emit_split_first`'s only difference between STAR and OPT |
| `node` `left` `right` `child` `root` | an index into the regex parser's AST, i.e. into its four parallel `idstd_ty`/`idstd_lf`/`idstd_rt`/`idstd_vl` lists |
| `l0` `l1` `l2` `l3` `lj` | a regex bytecode position captured mid-`idstd_emit_*`, to patch a jump target once the code it points at is known |
| `neg` | a character class's negate flag, 0 or 1 — `[^...]` |
| `id` `base` | a character class's own base index into `idstd_cls` (9 ints per class) |
| `bi` | a bit index, 0..31, within one word of a character class's bitmap |
| `ov` | a character class's bitmap word, read before a bit is or'd into it |
| `c1` `c2` | a character class range's first and last byte code -- `[c1-c2]` |
| `k1` | the position just past a class item's own byte(s), before a possible `-end` is read |
| `dash` `nc` | the byte at a possible class range's `-`, and the one after it |
| `ec` | an escaped byte in a regex pattern, the one after its `\` |
| `end` `start` | a regex search or parse's starting/ending byte offset |
| `g` `g1` `g2` | the next free capture-group number an `idstd_emit`/`idstd_emit_*` call returns, threaded through a node's children |
| `j2` `j3` | a second and third parse position, in a regex parse function that already uses `i`/`j` |
| `l` `rn` `val` | `idstd_ast_push`'s own left-child, right-child and value fields — its parameters are the only place these three names are used |
| `parse_repeat_v` | the result of calling `idstd_parse_repeat`, named because a call cannot be a call's argument (the `<call>_v` family) |

**`word`** — 64-bit, and only ever an intermediate: an address in the flat
store, or a product too wide for an `int`. Narrowed at the point of return,
never stored.

| name | meaning |
| --- | --- |
| `ad` | an address in the flat store |
| `sa` | a flat-store address bytes are read *from* |
| `da` | a flat-store address bytes are written *to* |
| `wp` | a wide product or numerator, narrowed once on return |
| `sm` | a wide sum of squares |
| `hs` | a wide square, used to test a root candidate |
| `peek8_v` | the result of calling `peek8`, named because a call cannot be a call's argument (the `<call>_v` family in the `int` table) |
| `idstd_bl` | SHA-256 padding's 64-bit big-endian bit-length field |
| `idstd_wn` | a byte count widened to `word` before multiplying by 8, so a long message's bit-length does not wrap the way `int * 8` would |
| `idstd_wx` | an `int`'s bit pattern widened to `word` and masked to 32 bits, the workaround for `ushr` on a negative `int` (see `round/bit.id`) |

**`float`** — only in `str_to_float`, the one place the library speaks floats.

| name | meaning |
| --- | --- |
| `fv` | a float value being accumulated |
| `dv` | the place value of the next fraction digit (0.1, 0.01, …) |

**`string`**

| name | meaning |
| --- | --- |
| `s` | a generic string |
| `txt` | the second string: the needle being searched for or matched against, or the right-hand side of a comparison. `str_cmp` takes `(s, txt)` and **not** `(a, b)`, because `a` and `b` are the `int` operands of `fx_min`/`fx_max` — see §5 |
| `sep` | a separator |
| `path` | a file path, in a diagnostic |
| `msg` | a diagnostic message |
| `esc` | the escape byte, `chr(27)`, that starts a terminal control sequence |
| `idstd_sofar` | the path prefix built so far, walking `fs_mkdir_p`'s path one character at a time |
| `pat` | a regular-expression pattern string |
| `sub` | the substring a regex capture group matched |

**`int[]`**

| name | meaning |
| --- | --- |
| `xs` | the generic list a helper operates on |
| `sq` | the square root's 2-slot working state: remainder, root so far |
| `bs` | a 1- or 2-slot fold cell threaded through a loop by reference |
| `idstd_xs` | a file's bytes, one per cell, in the PSF header reader |
| `idstd_ys` | the second list in a two-list helper: what `lst_extend` appends onto `idstd_xs` |
| `idstd_psh` | a PSF header as `pcsf_head` answers it: [glyph offset, bytes per glyph, height, width, glyph count], or empty |
| `idstd_vals` | a hash table's values, parallel to `idstd_keys` -- `core/data/hmap/` |
| `idstd_used` | a hash table's occupancy, 1 per slot -- `core/data/hmap/` |
| `idstd_cnt` | a hash table's live entry count: a 1-slot fold cell like `bs`, threaded by reference so `hmap_put` can bump it |
| `idstd_nvals` | `idstd_vals`'s counterpart in the *new* table, while `core/data/hmap/grow/` rehashes |
| `idstd_nused` | `idstd_used`'s counterpart in the *new* table, while `core/data/hmap/grow/` rehashes |
| `prog` | a compiled regex pattern's bytecode: flat 3-int instructions |
| `cls` | a compiled regex pattern's character classes: flat 9-int entries |
| `ty` `lf` `rt` `vl` | the regex parser's AST, as four parallel lists: node type, left child, right child, value |
| `caps` | regex capture slots, 2 ints per group, -1 for "did not participate" |
| `capv` | the same, before `idstd_finish` hands it back to a caller's own `caps` |
| `snap` | a copy of `caps` taken before a regex SPLIT's first branch, to restore if that branch fails |
| `idstd_buf` | a growable byte buffer being built or read — a padded message, a decoded run of code lengths, a gzip member's raw bytes |
| `idstd_dst` | DEFLATE's end-of-block flag, a 1-cell fold cell threaded through `idstd_inf_body`'s loop by reference |
| `idstd_row` | a Huffman tree's symbol list, in canonical code order — the second half of an `idstd_inf_tree` pair |
| `idstd_tab` | a Huffman tree's per-length code counts, 16 cells — the first half of an `idstd_inf_tree` pair |
| `idstd_rc` | SHA-256's 64 round constants, K (FIPS 180-4 section 4.2.2) |
| `idstd_sch` | SHA-256's 64-word message schedule for one block |
| `idstd_wst` | SHA-256's 8-word compression working state, [a, b, c, d, e, f, g, h] |
| `idstd_hst` | SHA-256's running 8-word hash state, H0..H7, updated block by block |
| `idstd_ret_li` | a function's own `int[]` return-value local, built up before the closing `return` — `ret_i`/`ret_s`'s counterpart for a list |
| `idstd_inf_clens_v` `idstd_inf_fix_dist_v` `idstd_inf_fix_lit_v` `idstd_lst_slice_v` `idstd_lst_slice_v2` `idstd_inf_starts_v` | the result of calling the function the name starts with (the `<call>_v` family, `int[]`-typed) |

**`int[][]`** — `kidsl`, a list of lists (`lset2`'s target). Spelled as idem spells it, so a program importing both keeps one vocabulary.

| name | meaning |
| --- | --- |
| `idstd_rows` | the literal/length alphabet's Huffman tree, a `[counts, symbols]` pair |
| `idstd_grid` | the distance alphabet's Huffman tree, a `[counts, symbols]` pair |
| `idstd_inf_tree_v` `idstd_inf_tree_v2` | the result of calling `idstd_inf_tree` (the `<call>_v` family, `int[][]`-typed) |

**`string[]`** — `strs`, a generic list of strings; `idstd_keys` a hash table's keys, `core/data/hmap/`; `idstd_nkeys` `idstd_keys`'s counterpart in the *new* table, while `core/data/hmap/grow/` rehashes.

**`word[]`** — `ws`, a generic list of words.

`r` is **not** available as an `int`: it reads as both "red" and "result", and a
library that reserved it would make every graphics program's `r` a compile error.
`z`, `key`, `src`, `name` and `fb` are **left unclaimed on purpose** — they are
the names a user program most wants, and idstd taking one would be a tax with no
benefit. `fmt_int`'s width parameter is `n` rather than the `w` that reads better
for exactly this reason. `w` and `h` were on that list until `gfx/` arrived and
argued for them above; since C4 a parameter name is only reserved within
idstd's own unit, but an *export's* name still is not — which is what ruled out
`col`.

`x` and `y` were on that list until `fx_atan2` was written, and moving them off
it is the honest record of a name being spent. `.tests/tool/names` is what makes
this table binding rather than aspirational: it fails the build if the library
declares a name this section does not list, or lists one the library no longer
declares.

---

## 3. Exported globals

**An exported name becomes a raw C global with no prefix of any kind**, so
`export int time` collides with libc at link time. Every idstd export therefore
carries its module prefix — no exceptions.

Each is `NULL`/uninitialised until its declaring function runs, and reading one
before that segfaults. The compiler now rejects a *reachable* read whose exporter
is unreachable from `main`, which catches a wired-up-nowhere init but not an
out-of-order one. See `README.md` "Initialisation" for the required order.

| global | type | owner | contents |
| --- | --- | --- | --- |
| `fx_sintab` | `int[]` | `fx_trig_init` | sin(0°…90°) × 1000, 91 entries |
| `rnd_st` | `int[]` | `rnd_init` | 1-element PRNG state |
| `err_n` | `int[]` | `err_init` | 2 slots: diagnostic count, mute flag |
| `err_fs` | `string[]` | `idstd_err_keep_init` | the kept diagnostics' file paths |
| `err_ls` | `int[]` | `idstd_err_keep_init` | their line numbers |
| `err_ms` | `string[]` | `idstd_err_keep_init2` | their messages |
| `buf_ad` | `word` | `idstd_buf_t_setup` | test fixture: an 8-byte buffer, all bytes 9 |
| `buf_ad2` | `word` | `idstd_buf_t_setup` | test fixture: an 8-byte buffer, all bytes 3 |
| `str_ad` | `word` | `idstd_str_t_setup` | test fixture: a 16-byte buffer, sentinel-filled with '.' |
| `sf_l_w` | `int` | `sf_l_init` | the list surface's width |
| `sf_l_h` | `int` | `sf_l_init` | its height |
| `sf_l_px` | `int[]` | `idstd_sf_l_alloc` | its pixels, `0xRRGGBB`, row-major — what a backend presents |
| `txt_g8` | `int[]` | `txt_g8_init` | the 8x8 face, 95 glyphs x 8 row masks |
| `term_w` | `int` | `idstd_term_scr_init` | the terminal screen's width in cells |
| `term_h` | `int` | `idstd_term_scr_init` | its height |
| `term_scr` | `int[]` | `idstd_term_scr_alloc` | each cell's byte code, row-major |
| `term_attr` | `int[]` | `idstd_term_scr_alloc` | each cell's attribute |
| `term_pal` | `string[]` | `idstd_term_pal_init` | attribute -> SGR parameters |
| `inf_in` | `int[]` | `idstd_inf_init` | the DEFLATE input, one byte per cell |
| `inf_bp` | `int[]` | `idstd_inf_init` | 1-element bit position into `inf_in` |
| `inf_out` | `int[]` | `idstd_inf_init` | the decoded output, one byte per cell |
| `inf_fail` | `int[]` | `idstd_inf_fail_reset` | 1-element flag: 0 until `idstd_inf_fail` runs, 1 after |
| `inf_lb` | `int[]` | `idstd_inf_tabs` | length-code base values, RFC 1951 section 3.2.5 |
| `inf_le` | `int[]` | `idstd_inf_tabs` | length-code extra-bit counts |
| `inf_db` | `int[]` | `idstd_inf_tabs2` | distance-code base values |
| `inf_de` | `int[]` | `idstd_inf_tabs2` | distance-code extra-bit counts |

---

## 4. Constants live in `conf.id`

A function that only returns a constant is a compile error (`docs/SPEC.md`
§7.2): it is a veiled reference to the constant. A constant of idstd's is
declared in idstd's own `conf.id` and read with `(import name)`.

That also retires the rule this section used to state. A zero-action function
returning a bare literal had the same *logic* as every other one returning that
literal, program-wide — idem's first whole-engine build failed on
`ast_k_repeat()` and `inp_hold()` both returning `120` — so an always-imported
library defining one took the literal away from every user program, and
constants had to be spelled `<prefix>_base() + n` inside a reserved block
7000–7999. A `conf.id` constant is a global, not a function body: it has no
fingerprint and collides with nothing but its own name.

Its name is reserved program-wide, exactly like an export, so it carries its
module prefix (§3): `fx_one`, not `one`.

**No constant exists yet.** Every integer in the library so far is an argument
to arithmetic (`1000`, `360000`, `16807`) or a byte code inside a comparison.

---

## 5. Traps this library was written around

Each one cost real time, here or in idem. They are rules about writing library
code, not observations about the language.

- **The duplicate-logic rule crosses the import boundary.** `fx_min`/`fx_max`
  coexist only because `<` differs from `>`; `fx_abs` is `} return int
  fx_max(a, 0 - a);` with an *empty body* precisely so there is no third
  comparison to collide with either. Before writing a near-duplicate, check that
  it differs from its sibling by an operator, a literal, or a called name. If it
  does not, you have one function with a parameter, not two functions.
- **Delegation dodges the rule.** `lst_find` calls `lst_index_of`; `str_starts`
  calls `str_eqat`; `buf_zero` calls `buf_fill`; `fmt_int` calls `fmt_pad`. Each
  is a zero-action function whose whole content is the name it calls, which is
  part of the fingerprint.
- **`len(s)` is a `strlen` every time it is evaluated and is not memoised;
  `charat` memoises the last string's length.** So every per-character loop in
  `core/text` is `while (charat(s, i) >= 0)`, never `while (i < len(s))`.
- **Alternating `charat` between two strings defeats that memo**, and each call
  then pays a full `strlen`. `str_eqat` is fine once per line and wrong per
  token; `str_findat` and `str_cmp` say so in their own comments.
- **`s = s + chr(c)` in a loop is quadratic in time *and retained memory*** —
  measured in idem at 667 ms and 1.8 GB for 1000 frames of a 1920-character line,
  against 1 ms and 5.8 MB through `alloc`/`poke8`/`str_of_mem`. Every builder in
  `core/text` goes through the store. This is a requirement of the
  implementation, not advice.
- **An unconditional neighbour read needs a clamped index.** `idstd_fx_sin_lin` reads
  `idstd_fx_tab(i)` and `idstd_fx_tab(i + 1)` and evaluates both even when the fraction is 0;
  an out-of-range list read *aborts the process*. `idstd_fx_tab` clamps.
- **`int` is 32 bits and wraps silently.** A helper that answers an `int` cannot
  return a value past 2^31 however wide its intermediates are.
- **`fx_sqrt` is exact only below 2^31 and is silently wrong above it.** Go
  through `fx_hyp`/`fx_wroot`.
- **A comparison is an `int`, so a branchless fixup is legal and idiomatic**:
  `nx + 2147483647 * (nx <= 0)`. Remember `*` binds tighter than `<=`.
- **Two side-effecting calls in one expression may evaluate in either order.**
  `"" + rnd_next() + rnd_next()` can print the two draws swapped. Give each call
  its own statement.
- **Bitwise binds tighter than comparison here**, the opposite of C. `hv ^ c *
  16777619` is not what you meant; `(hv ^ c) * 16777619` is. Because the two
  languages disagree, `bin/idc` rejects a comparison mixed with an
  unparenthesized bitwise operand: write `(flags & 4) == 4`.
- **`id` function names get an `id_` prefix in C**, so they collide with the
  runtime's own helpers. The full reserved set, read out of the emitted prelude:
  `add_check`, `alloc`, `arena_free_all`, `arena_head`, `arena_hooked`,
  `arena_link`, `arena_unlink`, `args`, `at`, `box_f`, `charat`, `chr`, `concat`,
  `die`, `files`, `flush`, `getkey`, `i`, `idiv`, `imod`, `index_error`, `input`,
  `len`, `list_get`, `list_grow`, `list_len`, `list_lit`, `list_new`, `list_pop`,
  `list_push`, `list_set`, `mem_alloc`, `mem_of_str`, `mem_size`, `memcopy`,
  `mul_check`, `oom_error`, `peek8`…`peek64`, `peek_n`, `poke8`…`poke64`,
  `poke_n`, `pop_error`, `print`, `put`, `raw_active`, `read_all`, `realloc`,
  `s`, `sar`, `sdiv`, `shl`, `sleep_ms`, `smod`, `store`, `store_cap`,
  `store_grow`, `store_used`, `str_of_float`, **`str_of_int`**, `str_of_mem`,
  **`str_of_word`**, `strcmp`, `string`, `strlen`, `term_raw`, `term_restore`,
  `ticks`, `to_int`.

---

## 6. Rules for adding to this file

- **A new function**: use your module's prefix, mark it pub or int, and add its
  row in the same commit that adds the code.
- **A new local variable name**: check §2 first. If the meaning already has a
  spelling there, use it. If it is genuinely new, add a row — and remember you
  are taking that name away from every `id` program on the machine.
- **A new constant family**: take a base from §4's block and register it.
- **A new prefix**: add a row to §1 and say which directory owns it.
- **A new native**: add its row to §1.11 with vis `native`, in the same commit
  as its declaration. Its name is taken from every program; its parameters are
  not (§2).
- **New code spells every parameter, every local and every internal function
  `idstd_<name>`** (decided 2026-09-13); a pub function keeps its module prefix.
  A name here is taken from every program, functions included: a parameter
  `cn` broke the compiler, which has a function named `cn`. `core/data/psf/`
  is written this way; the older modules are renamed separately.

---

## 7. What one-type-per-name actually costs, measured here

Writing `str_cmp(string a, string b)` — the obvious signature, and the one every
C programmer types — produced **28 compile errors**, every one of them reported
inside `core/math/fx/`:

```
core/math/fx/base.id:21: error: argument 'a' of 'fx_max' expects int, got string
core/math/fx/base.id:25: error: cannot order string and string
core/math/fx/wide/sqrt.id:27: error: cannot initialize int[] 'sq' with a string[] value
...
core/text/str/scan/ord/cmp.id:20: error: variable 'a' is declared string here but
    int elsewhere; a name must keep one type across the whole program
```

The one message that names the real cause is the twenty-sixth. Every earlier one
points at correct, untouched code in a different module, and describes a
consequence rather than a cause. This is the rule inside a *single* tree; a user
program that declares `string a` gets the same cascade, in files it has never
opened, for a library it did not know it was importing.

That is the whole argument for IDSTD.md §2's C4, and it is recorded here as a
measurement rather than a prediction.
