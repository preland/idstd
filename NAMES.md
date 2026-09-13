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

---

## 1. Function prefixes — one owner per prefix

| prefix | module | directory |
| --- | --- | --- |
| `fx_` | fixed-point arithmetic, roots, trig, the inverse tangent | `core/math/` |
| `rnd_` | random numbers | `core/math/` |
| `lset`/`lget`/`sset`/`lset2`/`wset` | the five bare list helpers (§3) | `core/data/lst/` |
| `lst_` | list helpers: fill, copy, search, order, aggregate | `core/data/lst/` |
| `buf_` | the flat store as bytes: fill, copy, compare | `core/data/buf/` |
| `str_` | strings: slice, search, compare, split, join, build | `core/text/str/` |
| `chr_` | one byte code: classify, case, hex digit | `core/text/chr/` |
| `fmt_` | formatting for display: width, hex | `core/text/fmt/` |
| `err_` | accumulated diagnostics | `sys/err/` |

Reserved shapes inside a prefix, so two authors do not invent two spellings of
one idea: `*_init`, `*_get`, `*_set`, `*_len`, `*_at`, `*_add`, `*_find`,
`*_all`.

**Reserved for later phases, not yet built:** `file_`, `term_` (`sys/io/`),
`sys_`, `inp_` (`sys/win/`), `sf_`, `ppm_`, `d2_`, `txt_`, `m4_`, `d3_` (`gfx/`).
Listed here so nothing else claims them.

### 1.1 `fx_` — fixed point (`core/math/`)

| function | signature | vis | file |
| --- | --- | --- | --- |
| `fx_abs` | `(int) -> int` | pub | `fx/base/base.id` |
| `fx_min` | `(int,int) -> int` | pub | `fx/base/base.id` |
| `fx_max` | `(int,int) -> int` | pub | `fx/base/base.id` |
| `fx_fdiv` | `(int,int) -> int` | pub | `fx/base/div.id` |
| `fx_fdown` | `(int,int,int) -> int` | int | `fx/base/div.id` |
| `fx_clamp` | `(int,int,int) -> int` | pub | `fx/lim.id` |
| `fx_sign` | `(int) -> int` | pub | `fx/lim.id` |
| `fx_lerp` | `(int,int,int) -> int` | pub | `fx/lim.id` |
| `fx_mul` | `(int,int,int) -> int` | pub | `fx/wide/mul.id` |
| `fx_div` | `(int,int,int) -> int` | pub | `fx/wide/mul.id` |
| `fx_pow` | `(int,int) -> int` | pub | `fx/wide/mul.id` |
| `fx_sqrt` | `(int) -> int` | pub | `fx/wide/sqrt.id` |
| `fx_sqbit` | `(int) -> int` | int | `fx/wide/sqrt.id` |
| `fx_sqloop` | `(int[],int) -> int` | int | `fx/wide/sqrt.id` |
| `fx_sqstep` | `(int[],int) -> void` | int | `fx/wide/bits/bits.id` |
| `fx_sqtake` | `(int[],int) -> void` | int | `fx/wide/bits/bits.id` |
| `fx_hypfix` | `(word) -> int` | int | `fx/wide/bits/bits.id` |
| `fx_wroot` | `(word) -> int` | pub | `fx/wide/bits/root.id` |
| `fx_wsmall` | `(word) -> int` | int | `fx/wide/bits/root.id` |
| `fx_sq` | `(int) -> word` | pub | `fx/wide/bits/root.id` |
| `fx_hyp` | `(int,int) -> int` | pub | `fx/wide/bits/hyp.id` |
| `fx_hyp3` | `(int,int,int) -> int` | pub | `fx/wide/bits/hyp.id` |
| `fx_hyp_sm2` | `(int,int) -> word` | int | `fx/wide/bits/hyp.id` |
| `fx_trig_init` | `() -> void` | pub | `trig/tab.id` |
| `fx_tab` | `(int) -> int` | int | `trig/tab.id` |
| `fx_norm_deg` | `(int) -> int` | pub | `trig/tab.id` |
| `fx_sin` | `(int) -> int` | pub | `trig/sin/sin.id` |
| `fx_sin_qr` | `(int) -> int` | int | `trig/sin/sin.id` |
| `fx_cos` | `(int) -> int` | pub | `trig/sin/sin.id` |
| `fx_sin_lin` | `(int) -> int` | int | `trig/sin/lin.id` |
| `fx_sin_interp` | `(int,int) -> int` | int | `trig/sin/lin.id` |
| `fx_sin_deg` | `(int) -> int` | pub | `trig/sin/deg.id` |
| `fx_cos_deg` | `(int) -> int` | pub | `trig/sin/deg.id` |
| `fx_sin_q` | `(int,int) -> int` | int | `trig/ang/fold.id` |
| `fx_sin_q01` | `(int,int) -> int` | int | `trig/ang/fold.id` |
| `fx_sin_q23` | `(int,int) -> int` | int | `trig/ang/fold.id` |
| `fx_atan2` | `(int,int) -> int` | pub | `trig/ang/atan/atan.id` |
| `fx_atan_absq` | `(int,int) -> int` | int | `trig/ang/atan/atan.id` |
| `fx_atan_fold` | `(int,int,int) -> int` | int | `trig/ang/atan/atan.id` |
| `fx_atan_q` | `(int,int) -> int` | int | `trig/ang/quad/quad.id` |
| `fx_atan_oct_v` | `(int,int) -> int` | int | `trig/ang/quad/quad.id` |
| `fx_atan_oct` | `(int,int) -> int` | int | `trig/ang/atan/oct.id` |
| `fx_atan_loop` | `(int[],int,int) -> void` | int | `trig/ang/atan/oct.id` |
| `fx_atan_step` | `(int[],int,int) -> void` | int | `trig/ang/atan/oct.id` |
| `fx_atan_hi` | `(int,int,int) -> int` | int | `trig/ang/atan/hi.id` |
| `fx_atan_cross` | `(int,int,int) -> word` | int | `trig/ang/atan/hi.id` |

**`fx_hypot` is deliberately absent.** IDSTD.md §3.1 asks for it "for the `int`
case", but `fx_hyp` already takes two `int`s and answers an `int`; a second
function with that body is a duplicate-logic *compile error*, not a redundancy.
See `COMPILER-ASKS.md`.

### 1.2 `rnd_` — random (`core/math/rnd/`)

| function | signature | vis |
| --- | --- | --- |
| `rnd_init` | `(int) -> void` | pub, **required init** |
| `rnd_next` | `() -> int` | pub |
| `rnd_step` | `() -> int` | int |
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

### 1.4 `lst_` and `buf_` (`core/data/`)

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
| `lst_sort_loop` | `(int[],int,int) -> void` | int | `lst/w/ord/sort.id` |
| `lst_sift` | `(int[],int) -> void` | int | `lst/w/ord/sort.id` |
| `buf_fill` | `(word,int,int) -> void` | pub | `buf/buf.id` |
| `buf_zero` | `(word,int) -> void` | pub | `buf/buf.id` |
| `buf_copy` | `(word,word,int) -> void` | pub | `buf/buf.id` |
| `buf_cmp` | `(word,word,int) -> int` | pub | `buf/cmp.id` |
| `buf_cmp_loop` | `(int[],word,word,int) -> void` | int | `buf/cmp.id` |
| `buf_cmp_one` | `(int,word,word,int) -> int` | int | `buf/cmp.id` |

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
| `str_upper_alloc` | `(string,int) -> word` | int | `str/make/case/up.id` |
| `str_upper_blit` | `(string,word,int) -> void` | int | `str/make/case/up.id` |
| `str_upper_step` | `(string,word,int) -> void` | int | `str/make/case/step.id` |
| `str_lower` | `(string) -> string` | pub | `str/make/case/low.id` |
| `str_lower_alloc` | `(string,int) -> word` | int | `str/make/case/low.id` |
| `str_lower_blit` | `(string,word,int) -> void` | int | `str/make/case/low.id` |
| `str_lower_step` | `(string,word,int) -> void` | int | `str/make/case/step.id` |
| `str_pad` | `(string,int) -> string` | pub | `str/make/fill/pad/pad.id` |
| `str_pad_build` | `(string,word,int) -> string` | int | `str/make/fill/pad/fill.id` |
| `str_pad_fill` | `(string,word,int) -> void` | int | `str/make/fill/pad/fill.id` |
| `str_repeat` | `(string,int) -> string` | pub | `str/make/fill/rep/rep.id` |
| `str_rep_width` | `(string,int) -> int` | int | `str/make/fill/rep/loop.id` |
| `str_rep_build` | `(string,word,int,int) -> string` | int | `str/make/fill/rep/loop.id` |
| `str_rep_loop` | `(string,word,int) -> void` | int | `str/make/fill/rep/loop.id` |
| `str_trim` | `(string) -> string` | pub | `str/make/fill/trim/trim.id` |
| `str_trim_end` | `(string) -> int` | int | `str/make/fill/trim/trim.id` |
| `str_trim_slice` | `(string,int,int) -> string` | int | `str/make/fill/trim/trim.id` |
| `str_ws_start` | `(string,int) -> int` | int | `str/make/fill/trim/ws.id` |
| `str_ws_end` | `(string,int) -> int` | int | `str/make/fill/trim/ws.id` |
| `chr_is_space_at` | `(string,int) -> int` | int | `str/make/fill/trim/ws.id` |
| `str_eqat` | `(string,int,string) -> int` | pub | `str/scan/eq.id` |
| `str_starts` | `(string,string) -> int` | pub | `str/scan/eq.id` |
| `str_ends` | `(string,string) -> int` | pub | `str/scan/eq.id` |
| `str_find` | `(string,string) -> int` | pub | `str/scan/find.id` |
| `str_findat` | `(string,string,int) -> int` | pub | `str/scan/find.id` |
| `str_find_end` | `(string,int,string) -> int` | int | `str/scan/find.id` |
| `str_cmp` | `(string,string) -> int` | pub | `str/scan/ord/cmp.id` |
| `str_cmp_sign` | `(string,string,int) -> int` | int | `str/scan/ord/cmp.id` |
| `str_cmp_run` | `(string,string,int) -> int` | int | `str/scan/ord/cmp.id` |
| `str_hash` | `(string) -> int` | pub | `str/scan/ord/hash.id` |
| `str_hash_run` | `(string,int,int) -> int` | int | `str/scan/ord/hash.id` |
| `str_to_float` | `(string) -> float` | pub | `str/scan/ord/num/float.id` |
| `str_frac` | `(string,int) -> float` | int | `str/scan/ord/num/float.id` |
| `str_frac_run` | `(string,int,float,float) -> float` | int | `str/scan/ord/num/float.id` |
| `chr_is_digit_at` | `(string,int) -> int` | int | `str/scan/ord/num/digit.id` |
| `str_sgn` | `(string) -> int` | int | `str/scan/ord/num/sgn.id` |
| `str_split` | `(string,string) -> string[]` | pub | `str/part/split/split.id` |
| `str_split_loop` | `(string[],string,string,int) -> void` | int | `str/part/split/split.id` |
| `str_split_one` | `(string[],string,string,int) -> int` | int | `str/part/split/split.id` |
| `str_split_bound` | `(string,string,int) -> int` | int | `str/part/split/bound.id` |
| `str_split_push` | `(string[],string,int,int) -> void` | int | `str/part/split/bound.id` |
| `str_split_cut` | `(int,int) -> int` | int | `str/part/cut.id` |
| `str_eol` | `(string,int) -> int` | pub | `str/part/cut.id` |
| `str_join` | `(string[],string) -> string` | pub | `str/part/join/join.id` |
| `str_join_len` | `(string[],string) -> int` | int | `str/part/join/join.id` |
| `str_join_blit` | `(string[],string,word) -> void` | int | `str/part/join/join.id` |
| `str_join_one` | `(int[],string[],string,word) -> void` | int | `str/part/join/one.id` |
| `str_join_one_adv` | `(int[],string[],string,word,int) -> void` | int | `str/part/join/one.id` |
| `str_join_put` | `(string,word,int) -> int` | int | `str/part/join/one.id` |
| `str_join_base` | `(string[],string) -> int` | int | `str/part/join/sep.id` |
| `str_join_sep` | `(string,word,int,int) -> int` | int | `str/part/join/sep.id` |
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
| `fmt_pad_width` | `(string,int) -> int` | int | `fmt/pad.id` |
| `fmt_pad_build` | `(string,word,int) -> string` | int | `fmt/pad.id` |
| `fmt_pad_fill` | `(string,word,int) -> void` | int | `fmt/pad.id` |
| `fmt_hex` | `(int) -> string` | pub | `fmt/hex.id` |
| `fmt_hex_run` | `(word,int,int) -> void` | int | `fmt/hex.id` |

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
| `err_keep_init` | `() -> void` | int | `k/keep.id` |
| `err_keep_init2` | `() -> void` | int | `k/keep.id` |
| `err_keep` | `(string,int,string) -> void` | int | `k/keep.id` |
| `err_keep2` | `(int,string) -> void` | int | `k/k2.id` |
| `err_clear` | `() -> void` | pub | `k/k2.id` |
| `err_drop` | `() -> void` | int | `k/k2.id` |
| `err_drop2` | `() -> void` | int | `k/k3.id` |
| `err_nmsg` | `() -> int` | pub | `k/k3.id` |

---

## 2. Variable names and their one permitted type

**This table is the most invasive thing in the library.** A parameter named `a`
makes `a` an `int` in every program that imports idstd, so idstd's local
vocabulary *is* public API until `id_development` lands C4 (per-unit name-type
checking). Every name below is therefore chosen as if a user would read it, and
kept as small as the code allows.

The whole list is deliberately short — 54 names — and it is drawn from
`idem/docs/NAMES.md` §2 wherever a meaning already had a spelling there, so that
a program importing both keeps one vocabulary.

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
| `d` | the difference of two bytes, in a comparison that answers an ordering |
| `k` | a quotient being adjusted — `fx_fdiv`'s floor step, one below `n` or not |
| `sg` | the sign of a product, -1, 0 or 1 |
| `sn` | the next candidate in a bit-by-bit search — `n`'s successor, since a name keeps one type |

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

**`int[]`**

| name | meaning |
| --- | --- |
| `xs` | the generic list a helper operates on |
| `sq` | the square root's 2-slot working state: remainder, root so far |
| `bs` | a 1- or 2-slot fold cell threaded through a loop by reference |

**`int[][]`** — `kidsl`, a list of lists (`lset2`'s target). Spelled as idem spells it, so a program importing both keeps one vocabulary.

**`string[]`** — `strs`, a generic list of strings.

**`word[]`** — `ws`, a generic list of words.

`r` is **not** available as an `int`: it reads as both "red" and "result", and a
library that reserved it would make every graphics program's `r` a compile error.
`w`, `h`, `z`, `key`, `src`, `name` and `fb` are **left unclaimed on purpose** —
they are the names a user program most wants, and idstd taking one would be a tax
with no benefit. `fmt_int`'s width parameter is `n` rather than the `w` that
reads better for exactly this reason. `gfx/` will need some of these and will
have to argue for each, in this table, before the code is written.

`x` and `y` were on that list until `fx_atan2` was written, and moving them off
it is the honest record of a name being spent. `.tests/names.py` is what makes
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
| `err_fs` | `string[]` | `err_keep_init` | the kept diagnostics' file paths |
| `err_ls` | `int[]` | `err_keep_init` | their line numbers |
| `err_ms` | `string[]` | `err_keep_init2` | their messages |

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
- **An unconditional neighbour read needs a clamped index.** `fx_sin_lin` reads
  `fx_tab(i)` and `fx_tab(i + 1)` and evaluates both even when the fraction is 0;
  an out-of-range list read *aborts the process*. `fx_tab` clamps.
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
