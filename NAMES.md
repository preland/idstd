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
| `lset`/`lget`/`sset`/`lset2`/`wset` | the five bare list helpers (§3) | `core/data/lst/` |
| `lst_` | list helpers: fill, copy, search, order, aggregate | `core/data/lst/` |
| `buf_` | the flat store as bytes: fill, copy, compare | `core/data/buf/` |
| `pcsf_` | PC Screen Font headers: which one, where the glyphs are, their size and count | `core/data/psf/` |
| `str_` | strings: slice, search, compare, split, join, build | `core/text/str/` |
| `chr_` | one byte code: classify, case, hex digit | `core/text/chr/` |
| `fmt_` | formatting for display: width, hex | `core/text/fmt/` |
| `err_` | accumulated diagnostics | `sys/err/` |
| `sf_` | surfaces; `sf_l_` is the list surface | `gfx/px/` |
| `ppm_` | writing a surface as a PPM image; `ppm_l_` for the list surface | `gfx/px/` |
| `d2_` | 2D colour and shapes; `d2_l_` draws on the list surface | `gfx/d2/` |
| `txt_` | text on a surface; `txt_g8_` is the 8x8 face | `gfx/d2/txt/` |
| `d3_` | 3D geometry; so far `d3_cube_`, a cube's vertex and colour lists | `gfx/d3/` |
| `term_` | the character-cell terminal: screen, drawing, rendering, keys | `sys/io/term/` |
| `inp_` | input; so far `inp_live`, whether a window event lets a loop go on | `sys/win/` |
| `sys_` | the window and frame pacing; so far `sys_next` | `sys/win/` |
| `fs_` | the file backend's natives (§1.11) | `sys/io/fs/` |
| `proc_` | the child process backend's natives (§1.11) | `sys/io/ipc/proc/` |
| `sock_` | the Unix-domain socket backend's natives (§1.11) | `sys/io/ipc/sock/` |
| `gfx_` | the software window backend's natives (§1.11) | `sys/win/gfx/` |
| `gl_` `glwin_` | the OpenGL window backend's natives (§1.11) | `sys/win/gl/` |

Reserved shapes inside a prefix, so two authors do not invent two spellings of
one idea: `*_init`, `*_get`, `*_set`, `*_len`, `*_at`, `*_add`, `*_find`,
`*_all`. A fixture that exists only to give inline cases a setup or a check is
spelled `*_t_*` and marked int.

**Reserved for later phases, not yet built:** `file_`, `term_` (`sys/io/`),
`sys_`, `inp_` (`sys/win/`), `m4_`, `d3_` (`gfx/d3/`), and every other name
under `sf_`, `ppm_`, `d2_` and `txt_` — idem's flat-store surface is expected
there. Listed here so nothing else claims them.

### 1.1 `fx_` — fixed point (`core/math/`)

| function | signature | vis | file |
| --- | --- | --- | --- |
| `fx_abs` | `(int) -> int` | pub | `fx/base/base.id` |
| `fx_min` | `(int,int) -> int` | pub | `fx/base/base.id` |
| `fx_max` | `(int,int) -> int` | pub | `fx/base/base.id` |
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
| `idstd_fx_sqstep` | `(int[],int) -> void` | int | `fx/wide/bits/bits.id` |
| `idstd_fx_sqtake` | `(int[],int) -> void` | int | `fx/wide/bits/bits.id` |
| `idstd_fx_hypfix` | `(word) -> int` | int | `fx/wide/bits/bits.id` |
| `fx_wroot` | `(word) -> int` | pub | `fx/wide/bits/root.id` |
| `idstd_fx_wsmall` | `(word) -> int` | int | `fx/wide/bits/root.id` |
| `fx_sq` | `(int) -> word` | pub | `fx/wide/bits/root.id` |
| `fx_hyp` | `(int,int) -> int` | pub | `fx/wide/bits/hyp.id` |
| `fx_hyp3` | `(int,int,int) -> int` | pub | `fx/wide/bits/hyp.id` |
| `idstd_fx_hyp_sm2` | `(int,int) -> word` | int | `fx/wide/bits/hyp.id` |
| `fx_trig_init` | `() -> void` | pub | `trig/tab.id` |
| `idstd_fx_tab` | `(int) -> int` | int | `trig/tab.id` |
| `fx_norm_deg` | `(int) -> int` | pub | `trig/tab.id` |
| `fx_sin` | `(int) -> int` | pub | `trig/sin/sin.id` |
| `idstd_fx_sin_qr` | `(int) -> int` | int | `trig/sin/sin.id` |
| `fx_cos` | `(int) -> int` | pub | `trig/sin/sin.id` |
| `idstd_fx_sin_lin` | `(int) -> int` | int | `trig/sin/lin.id` |
| `idstd_fx_sin_interp` | `(int,int) -> int` | int | `trig/sin/lin.id` |
| `fx_sin_deg` | `(int) -> int` | pub | `trig/sin/deg.id` |
| `fx_cos_deg` | `(int) -> int` | pub | `trig/sin/deg.id` |
| `idstd_fx_sin_q` | `(int,int) -> int` | int | `trig/ang/fold.id` |
| `idstd_fx_sin_q01` | `(int,int) -> int` | int | `trig/ang/fold.id` |
| `idstd_fx_sin_q23` | `(int,int) -> int` | int | `trig/ang/fold.id` |
| `fx_atan2` | `(int,int) -> int` | pub | `trig/ang/atan/atan.id` |
| `idstd_fx_atan_absq` | `(int,int) -> int` | int | `trig/ang/atan/atan.id` |
| `idstd_fx_atan_fold` | `(int,int,int) -> int` | int | `trig/ang/atan/atan.id` |
| `idstd_fx_atan_q` | `(int,int) -> int` | int | `trig/ang/quad/quad.id` |
| `idstd_fx_atan_oct_v` | `(int,int) -> int` | int | `trig/ang/quad/quad.id` |
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

### 1.3 the five bare helpers (`core/data/lst/`)

These carry no prefix because they are language-level, and because the
compiler's own diagnostic names `lset` by that spelling. Defining a second copy
of any of them anywhere in a program is a duplicate-logic error — which is the
point: there is one.

| function | signature | vis | file |
| --- | --- | --- | --- |
| `lset` | `(int[],int,int) -> void` | pub | `lst/lst.id` |
| `lget` | `(int[],int) -> int` | pub | `lst/lst.id` |
| `sset` | `(string[],int,string) -> void` | pub | `lst/lst.id` |
| `lset2` | `(int[][],int,int[]) -> void` | pub | `lst/grow.id` |
| `wset` | `(word[],int,word) -> void` | pub | `lst/w/pick.id` |

### 1.4 `lst_`, `buf_` and `pcsf_` (`core/data/`)

| function | signature | vis | file |
| --- | --- | --- | --- |
| `lst_fill` | `(int[],int,int) -> void` | pub | `lst/grow.id` |
| `lst_set_all` | `(int[],int,int) -> void` | pub | `lst/grow.id` |
| `lst_pick` | `(int,int,int) -> int` | pub | `lst/w/pick.id` |
| `lst_index_of` | `(int[],int) -> int` | pub | `lst/w/find.id` |
| `lst_find` | `(int[],int) -> int` | pub | `lst/w/find.id` |
| `lst_last` | `(int[]) -> int` | pub | `lst/w/find.id` |
| `lst_min` | `(int[]) -> int` | pub | `lst/w/ord/agg.id` |
| `lst_max` | `(int[]) -> int` | pub | `lst/w/ord/agg.id` |
| `lst_sum` | `(int[]) -> int` | pub | `lst/w/ord/agg.id` |
| `lst_copy` | `(int[]) -> int[]` | pub | `lst/w/ord/move.id` |
| `lst_swap` | `(int[],int,int) -> void` | pub | `lst/w/ord/move.id` |
| `lst_reverse` | `(int[]) -> void` | pub | `lst/w/ord/move.id` |
| `lst_sort` | `(int[]) -> void` | pub | `lst/w/ord/sort.id` |
| `idstd_lst_sort_loop` | `(int[],int,int) -> void` | int | `lst/w/ord/sort.id` |
| `idstd_lst_sift` | `(int[],int) -> void` | int | `lst/w/ord/sort.id` |
| `buf_fill` | `(word,int,int) -> void` | pub | `buf/buf.id` |
| `buf_zero` | `(word,int) -> void` | pub | `buf/buf.id` |
| `buf_copy` | `(word,word,int) -> void` | pub | `buf/buf.id` |
| `buf_cmp` | `(word,word,int) -> int` | pub | `buf/cmp.id` |
| `idstd_buf_cmp_loop` | `(int[],word,word,int) -> void` | int | `buf/cmp.id` |
| `idstd_buf_cmp_one` | `(int,word,word,int) -> int` | int | `buf/cmp.id` |
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
| `str_pad` | `(string,int) -> string` | pub | `str/make/fill/pad/pad.id` |
| `idstd_str_pad_build` | `(string,word,int) -> string` | int | `str/make/fill/pad/fill.id` |
| `idstd_str_pad_fill` | `(string,word,int) -> void` | int | `str/make/fill/pad/fill.id` |
| `str_repeat` | `(string,int) -> string` | pub | `str/make/fill/rep/rep.id` |
| `idstd_str_rep_width` | `(string,int) -> int` | int | `str/make/fill/rep/loop.id` |
| `idstd_str_rep_build` | `(string,word,int,int) -> string` | int | `str/make/fill/rep/loop.id` |
| `idstd_str_rep_loop` | `(string,word,int) -> void` | int | `str/make/fill/rep/loop.id` |
| `str_trim` | `(string) -> string` | pub | `str/make/fill/trim/trim.id` |
| `idstd_str_trim_end` | `(string) -> int` | int | `str/make/fill/trim/trim.id` |
| `idstd_str_trim_slice` | `(string,int,int) -> string` | int | `str/make/fill/trim/trim.id` |
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
| `str_split` | `(string,string) -> string[]` | pub | `str/part/split/split.id` |
| `idstd_str_split_loop` | `(string[],string,string,int) -> void` | int | `str/part/split/split.id` |
| `idstd_str_split_one` | `(string[],string,string,int) -> int` | int | `str/part/split/split.id` |
| `idstd_str_split_bound` | `(string,string,int) -> int` | int | `str/part/split/bound.id` |
| `idstd_str_split_push` | `(string[],string,int,int) -> void` | int | `str/part/split/bound.id` |
| `idstd_str_split_cut` | `(int,int) -> int` | int | `str/part/cut.id` |
| `str_eol` | `(string,int) -> int` | pub | `str/part/cut.id` |
| `str_join` | `(string[],string) -> string` | pub | `str/part/join/join.id` |
| `idstd_str_join_len` | `(string[],string) -> int` | int | `str/part/join/join.id` |
| `idstd_str_join_blit` | `(string[],string,word) -> void` | int | `str/part/join/join.id` |
| `idstd_str_join_one` | `(int[],string[],string,word) -> void` | int | `str/part/join/one.id` |
| `idstd_str_join_one_adv` | `(int[],string[],string,word,int) -> void` | int | `str/part/join/one.id` |
| `idstd_str_join_put` | `(string,word,int) -> int` | int | `str/part/join/one.id` |
| `idstd_str_join_base` | `(string[],string) -> int` | int | `str/part/join/sep.id` |
| `idstd_str_join_sep` | `(string,word,int,int) -> int` | int | `str/part/join/sep.id` |
| `chr_is_digit` | `(int) -> int` | pub | `chr/cls.id` |
| `chr_is_upper` | `(int) -> int` | pub | `chr/cls.id` |
| `chr_is_lower` | `(int) -> int` | pub | `chr/cls.id` |
| `chr_is_alpha` | `(int) -> int` | pub | `chr/cls2.id` |
| `chr_is_alnum` | `(int) -> int` | pub | `chr/cls2.id` |
| `chr_is_space` | `(int) -> int` | pub | `chr/cls2.id` |
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
| `err_init` | `() -> void` | pub, **required init** | `err.id` |
| `err_report` | `(string,int,string) -> void` | pub | `err.id` |
| `err_count` | `() -> int` | pub | `err.id` |
| `err_mute` | `(int) -> void` | pub | `mute.id` |
| `err_say` | `(string) -> void` | pub | `mute.id` |
| `idstd_err_keep_init` | `() -> void` | int | `k/keep.id` |
| `idstd_err_keep_init2` | `() -> void` | int | `k/keep.id` |
| `idstd_err_keep` | `(string,int,string) -> void` | int | `k/keep.id` |
| `idstd_err_keep2` | `(int,string) -> void` | int | `k/k2.id` |
| `err_clear` | `() -> void` | pub | `k/k2.id` |
| `idstd_err_drop` | `() -> void` | int | `k/k2.id` |
| `idstd_err_drop2` | `() -> void` | int | `k/k3.id` |
| `err_nmsg` | `() -> int` | pub | `k/k3.id` |

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
| `d2_pack` | `(int,int,int) -> int` | pub | `d2/col.id` |
| `d2_l_rect` | `(int,int,int,int,int) -> void` | pub | `d2/rect.id` |
| `idstd_d2_l_row` | `(int,int,int,int) -> void` | int | `d2/rect.id` |
| `txt_g8_init` | `() -> void` | pub, **required init** | `d2/txt/font.id` |
| `idstd_txt_g8_row` | `(int,int) -> int` | int | `d2/txt/g8.id` |
| `txt_g8_glyph` | `(int,int,int,int,int) -> void` | pub | `d2/txt/g8.id` |
| `idstd_txt_g8_bits` | `(int,int,int,int,int) -> void` | int | `d2/txt/g8.id` |
| `txt_g8_draw` | `(int,int,string,int,int) -> void` | pub | `d2/txt/draw.id` |
| `txt_g8_width` | `(string,int) -> int` | pub | `d2/txt/draw.id` |
| `idstd_txt_g8_t_len` | `() -> int` | int, test fixture | `d2/txt/draw.id` |

### 1.8 `d3_` — cube geometry (`gfx/d3/`)

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
| `idstd_term_idx` | `(int,int) -> int` | int | `scr/cell/cell.id` |
| `idstd_term_put` | `(int,int,int,int) -> void` | int | `scr/cell/cell.id` |
| `idstd_term_put_attr` | `(int,int,int) -> void` | int | `scr/cell/cell.id` |
| `term_set` | `(int,int,int,int) -> void` | pub | `scr/cell/set.id` |
| `term_in` | `(int,int) -> int` | pub | `scr/cell/set.id` |
| `idstd_term_blank` | `() -> void` | int | `scr/cell/set.id` |
| `term_clear` | `() -> void` | pub | `scr/cell/clear.id` |
| `idstd_term_clear_from` | `(int) -> void` | int | `scr/cell/clear.id` |
| `idstd_term_blank_at` | `(int) -> void` | int | `scr/cell/clear.id` |
| `term_text` | `(int,int,string,int) -> void` | pub | `draw/draw.id` |
| `term_hline` | `(int,int,int,int,int) -> void` | pub | `draw/draw.id` |
| `term_vline` | `(int,int,int,int,int) -> void` | pub | `draw/draw.id` |
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
| `term_setup` | `() -> void` | pub | `out/more/tty/tty.id` |
| `idstd_term_setup_tail` | `() -> void` | int | `out/more/tty/tty.id` |
| `term_done` | `() -> void` | pub | `out/more/tty/tty.id` |
| `idstd_term_done_msg` | `(string,string) -> void` | int | `out/more/tty/end/done.id` |
| `idstd_term_done_cls` | `() -> void` | int | `out/more/tty/end/done.id` |
| `idstd_term_t_setup` | `() -> void` | int, test fixture | `out/more/tty/end/t.id` |
| `idstd_term_t_mark` | `() -> void` | int, test fixture | `out/more/tty/end/t.id` |
| `idstd_term_t_sum` | `() -> int` | int, test fixture | `out/more/tty/end/t.id` |
| `idstd_term_t_asum` | `() -> int` | int, test fixture | `out/more/tty/end/t2.id` |
| `idstd_term_t_pal` | `() -> int` | int, test fixture | `out/more/tty/end/t2.id` |

### 1.9 `inp_`, `sys_` — the frame loop's pure part (`sys/win/`)

| function | signature | vis | file |
| --- | --- | --- | --- |
| `inp_live` | `(int) -> int` | pub | `win.id` |
| `sys_next` | `(int) -> int` | pub | `win.id` |

### 1.11 Natives — `fs_`, `proc_`, `sock_`, `gfx_`, `gl_`, `glwin_` (`sys/io/`, `sys/win/`)

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
| `fs_read` | `(int,int[],int) -> int` | native | `fs/data.id` |
| `fs_write` | `(int,int[],int) -> int` | native | `fs/data.id` |
| `fs_list` | `(string,int[],int) -> int` | native | `fs/data.id` |
| `fs_open` | `(string,string) -> int` | native | `fs/handle.id` |
| `fs_close` | `(int) -> int` | native | `fs/handle.id` |
| `fs_error` | `() -> int` | native | `fs/handle.id` |
| `fs_size` | `(string) -> int` | native | `fs/path/path.id` |
| `fs_exists` | `(string) -> int` | native | `fs/path/path.id` |
| `fs_remove` | `(string) -> int` | native | `fs/path/path.id` |
| `fs_run` | `(string) -> int` | native | `fs/path/run.id` |
| `proc_spawn` | `(string) -> int` | native | `ipc/proc/handle.id` |
| `proc_close` | `(int) -> int` | native | `ipc/proc/handle.id` |
| `proc_error` | `() -> int` | native | `ipc/proc/handle.id` |
| `proc_read` | `(int,int[],int,int) -> int` | native | `ipc/proc/io.id` |
| `proc_wait` | `(int,int) -> int` | native | `ipc/proc/wait.id` |
| `proc_kill` | `(int) -> int` | native | `ipc/proc/wait.id` |
| `sock_connect` | `(string,int) -> int` | native | `ipc/sock/handle.id` |
| `sock_close` | `(int) -> int` | native | `ipc/sock/handle.id` |
| `sock_error` | `() -> int` | native | `ipc/sock/handle.id` |
| `sock_send` | `(int,int[],int) -> int` | native | `ipc/sock/io.id` |
| `sock_recv` | `(int,int[],int,int) -> int` | native | `ipc/sock/io.id` |
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
| `gl_begin_frame` | `(int,int,int) -> int` | native | `gl/frame/frame.id` |
| `gl_end_frame` | `() -> int` | native | `gl/frame/frame.id` |
| `gl_read_pixels` | `(int[]) -> int` | native | `gl/frame/frame.id` |
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
| `buf_cmp_one_v` `charat_v2` `chr_hex_v` `chr_lower_v` `chr_upper_v` `fx_abs_v` `fx_abs_v2` `fx_max_v` `fx_min_v` `len_v` `str_findat_v` `str_join_sep_v` `str_ws_start_v` | the result of calling the function the name starts with (`<call>_v`; a second one in the same function is `<call>_v2`), named because a call cannot be a call's argument |
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

**`int[]`**

| name | meaning |
| --- | --- |
| `xs` | the generic list a helper operates on |
| `sq` | the square root's 2-slot working state: remainder, root so far |
| `bs` | a 1- or 2-slot fold cell threaded through a loop by reference |
| `idstd_xs` | a file's bytes, one per cell, in the PSF header reader |
| `idstd_psh` | a PSF header as `pcsf_head` answers it: [glyph offset, bytes per glyph, height, width, glyph count], or empty |

**`int[][]`** — `kidsl`, a list of lists (`lset2`'s target). Spelled as idem spells it, so a program importing both keeps one vocabulary.

**`string[]`** — `strs`, a generic list of strings.

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
| `sf_l_w` | `int` | `sf_l_init` | the list surface's width |
| `sf_l_h` | `int` | `sf_l_init` | its height |
| `sf_l_px` | `int[]` | `idstd_sf_l_alloc` | its pixels, `0xRRGGBB`, row-major — what a backend presents |
| `txt_g8` | `int[]` | `txt_g8_init` | the 8x8 face, 95 glyphs x 8 row masks |
| `term_w` | `int` | `idstd_term_scr_init` | the terminal screen's width in cells |
| `term_h` | `int` | `idstd_term_scr_init` | its height |
| `term_scr` | `int[]` | `idstd_term_scr_alloc` | each cell's byte code, row-major |
| `term_attr` | `int[]` | `idstd_term_scr_alloc` | each cell's attribute |
| `term_pal` | `string[]` | `idstd_term_pal_init` | attribute -> SGR parameters |

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
