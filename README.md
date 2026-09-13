# `idstd` — the `id` standard library

`id` ships 30 builtins and no module system. None of them is arithmetic beyond
the operators: there is no `abs`, `min`, `max`, `sqrt`, `sin`, `rand`, no
substring, no `split`, no string ordering, no `to_float`. Every non-trivial `id`
program has re-implemented some of that, and `../idem` (a game engine in `id`)
has implemented all of it once, well, with the measurements that justify each
choice. `idstd` is that work, extracted, named, registered and tested, so that
the next program does not write it again.

It is built to the brief in `../id_development/docs/IDSTD.md`.

## Status — what is built, honestly

| module | prefix | state |
| --- | --- | --- |
| `core/math` | `fx_` `rnd_` | **built.** 36 functions: fixed point, roots, magnitudes, the 91-entry trig table and a whole-degree sin/cos over it, a new `fx_atan2`, Park–Miller |
| `core/data` | `lst_` `buf_` + the five bare helpers | **built.** 25 functions |
| `core/text` | `str_` `chr_` `fmt_` | **built.** 57 functions — the largest new-code area |
| `sys/err` | `err_` | **built.** 13 functions |
| `sys/io` | `file_` `term_` | **`term_` built.** 46 functions (41 plus 5 test fixtures): `demos/engine`'s character-cell screen, drawing, rendering and input, prefixed, which `demos/moonbuggy` and `demos/solitaire` bundled copies of. `file_` still needs `backends/fs` |
| `sys/win` | `sys_` `inp_` | **partly built.** 2 functions: `inp_live` and `sys_next`, the pure step of every windowed demo's frame loop. The window itself is blocked on link-on-demand for native backends |
| `gfx/px` `gfx/d2` | `sf_l_` `ppm_l_` `d2_` `txt_g8_` | **partly built.** 23 functions: the list surface, colour packing, rectangles, the 8x8 face and the PPM dump that gfxdemo, idml and id_nativeapp each carried — all pure `id`. idem's flat-store surface and anything calling a native backend wait on C7 |
| `gfx/d3` | `m4_` `d3_` | **not built.** Same block. Dead-code elimination has since landed, so the ~400 functions are no longer the obstacle — the X11/OpenGL link line is |
| a `flt_` float mirror | `flt_` | **deferred**, deliberately — see "Decisions" |

128 functions in total, 80 of them public and 48 internal. Everything marked built is covered by
`run.sh`, through **both** compilers, and every assertion in every golden file is
checked against its own stated expectation (see Testing).

## Using it

The standard library is now **implicit**: `bin/idc` and `idc.py` resolve it from
`--std DIR`, then `$IDSTD_HOME`, then an `idstd` directory beside the
`id_development` checkout. On this machine that is this repository, so

```sh
bin/idc myprog -o myprog        # fx_max, str_split and friends already resolve
```

just works. `--no-std` (or `IDC_NO_STD=1`) turns it off; `--std /path/to/idstd`
points at a specific checkout, which is what this repository's own `run.sh` does
so that the suite tests the working tree rather than whatever is installed.

The library also still works as an ordinary explicit import, which is what a
vendored copy or an older compiler needs — put this in a project's `import.id`:

```
import "../../idstd"
```

`run.sh` covers both paths, because they are two different resolutions and only
one of them is new.

### Initialisation — read this before you call anything

**An `export` in `id` is a declaration inside a function body: the global does
not exist until that function runs, and reading it before then segfaults.** Two
idstd modules hold state and therefore have an init that a program must call:

```
main(int argc, string[] argv) {
  fx_trig_init();          // before fx_sin, fx_cos, fx_tab, fx_atan2
  rnd_init(ticks());       // before rnd_next, rnd_range
  err_init();              // before err_report, err_count, err_say, err_mute
  run();
} return int 0;
```

The list surface and the 8x8 face in `gfx/` hold state too:

```
  sf_l_init(w, h);         // before sf_l_*, d2_l_*, ppm_l_*, txt_g8_glyph, txt_g8_draw
  txt_g8_init();           // before txt_g8_row, txt_g8_glyph, txt_g8_draw
  term_init(w, h, seed);   // before every other term_; it also seeds rnd_
```

Order among these does not matter; *before first use* does. The compiler
catches the common shape — a reachable read whose exporter is unreachable from
`main` is now a compile error — but it does not catch an init chain in the wrong
order. `fx_abs`, `fx_sqrt`, `fx_hyp`, every `lst_`, every `buf_`, every `str_`,
every `chr_` and every `fmt_` are pure and need no init at all.

`id_development` is building automatic initialisation (IDSTD.md §2 C6). Until it
lands, this list is the contract.

## The fixed-point convention is part of the library

`id` has no cast, and the parts of it that matter for graphics and simulation are
integer. So integers carry meaning only by convention, and publishing that
convention is as much a part of a standard library as the functions are — it is
the closest thing the language has to types.

| quantity | scale |
| --- | --- |
| screen position, 2D | whole pixels |
| 2D sub-pixel motion | centipixels (×100) |
| world position, 3D | millunits (×1000) |
| angle | millidegrees (×1000) |
| trig result | ×1000 (`fx_sin` returns −1000…1000) |
| matrix entry | ×1000 |
| scale factor | per-mille (1000 = unscaled) |
| colour | packed `0xRRGGBB` |
| time | milliseconds |

And the rule that goes with it: **multiply before dividing, always**, and a
function's name says which scale it speaks. Overflow is the real hazard — `int`
is 32-bit and wraps *silently* — so anything reaching 10⁶ accumulates in a `word`
and narrows once, inside the helper, so no caller repeats it. `fx_mul`, `fx_div`,
`fx_lerp`, `fx_hyp` and `fx_hyp3` own that narrowing.

Alongside it, the **record-as-list** convention, which is what a struct is here:
parallel lists indexed by an integer id for many-of-a-kind; one fixed-slot list
with named zero-action accessors for one-of-a-kind; and slot numbers spoken only
through named functions, never as bare literals at a call site.

## `NAMES.md` is a build dependency

`id` has one flat namespace for the whole program, one type per name — parameters
and locals included — and no two function bodies may match up to renaming. All
three rules cross the import boundary. So a library that is in every program
reserves names and *logic* for every program, and
[`NAMES.md`](NAMES.md) is the registry that makes that auditable rather than
accidental. Adding a function to `idstd` means adding it there first.

Three consequences worth knowing before you read the source:

- **Every parameter name in this library is public API.** A parameter named `a`
  makes `a` an `int` in every program that imports idstd. `NAMES.md` §2 is the
  whole vocabulary, kept deliberately short (34 names), with `w`, `h`, `x`, `y`,
  `z`, `src`, `name`, `key` and `fb` left unclaimed on purpose. This is the
  single most invasive thing here, and `id_development`'s C4 (per-unit name-type
  checking) is what makes it go away.
- **Every constant is a `conf.id` global carrying its module prefix**
  (`NAMES.md` §4). A function that only returns a constant is a compile error,
  which also ends the old hazard of a library constant function taking its
  literal away from every program.
- **Every export becomes a raw C global with no prefix**, so it collides with
  libc. Every idstd export carries its module prefix (`NAMES.md` §3).

## Decisions

The five open questions in IDSTD.md §8, settled:

1. **Fixed point is the primary surface**, exactly as idem wrote it. A `flt_`
   mirror (`flt_sqrt`, `flt_sin`, …) is **deferred**, not rejected: `float` works
   end to end in both compilers and a standard library has a wider audience than
   a game engine, but nothing in this pass needs it. The one place idstd speaks
   `float` today is `str_to_float`, because there is no `to_float` builtin and
   the integer parser cannot be made into one — the fraction's *length* is what
   scales it.
2. **Graphics is out of scope for this pass.** It needed dead-code elimination
   and link-on-demand for native backends. DCE has since landed and costs are now
   negligible (see the cost regression), so the remaining blocker is only the
   second: without link-on-demand, every hello-world links X11 and OpenGL because
   the library contains a framebuffer. `COMPILER-ASKS.md` says so under C7.
3. **There is a published public surface.** `NAMES.md` §1 marks every function
   pub or int. The internals are the loop bodies and fold steps that exist only
   because a block holds three actions; they are the set that should be exempt
   from the *user-facing* duplicate-logic diagnostic.
4. **Versioning: the hazard is recorded, the policy is not decided.** Once names
   are reserved program-wide, *removing* an idstd function is a breaking change
   and **adding one can break a program that already compiled** — a new library
   function that duplicates a user's helper is a compile error in the user's
   file. That is a genuinely unusual compatibility surface: most libraries can
   add. Until a policy exists, treat every addition to `NAMES.md` as a
   potentially breaking change and say so in the commit message.
5. **Migrating idem onto idstd is not part of this pass.** It is the library's
   best acceptance test and should be scheduled as one.
6. **A function duplicated across any two projects belongs here** (decided
   2026-09-13, standing practice). Whatever the reason for the second copy, the
   function is added to idstd with its cases and both projects call it; a
   project never keeps a copy of something idstd has. `idc/tools/dupscan.sh`,
   given each project root separately, finds them. This is also what brings
   graphics into scope despite decision 2: the terminal engine, the framebuffer
   and text code and the GL kit are each carried by several projects, and their
   native-calling parts wait on C7.

## Testing

```sh
./run.sh              # all suites, plus the cost regression
./run.sh math text    # one or more suites
```

`run.sh` builds each project under `.tests/` with `bin/idc`, runs it, and diffs
stdout against a golden file. It used to build each with `python3 idc.py` too, as
a parity check; `idc.py` cannot parse a `given` case, and the library's
module-state cases (`gfx/`, `sys/io/term`) need them, so that leg is gone — the
same move `id_development`'s own suite made. Every inline case also runs on
every `bin/idc` build of anything that imports the library.

Everything a suite asserts is a number computed independently: `fx_sqrt` against
a known table and the exact ends of its range, `rnd_next` against the reference
Park–Miller sequence from a stated seed, `fx_atan2` against `math.atan2`,
`str_split` against a case worked out by hand. "It ran" is not a test, and a
matching hash is evidence about one input, not about correctness.

`.tests/` is a **hidden** directory, and that is load-bearing rather than
tasteful: the rule of 3 binds on this repository's root because the root is the
imported directory, and both compilers skip hidden entries entirely — which is
also the only reason a git repository can be a project root at all. Keeping the
suite hidden leaves the root's three slots for `core/`, `sys/` and `gfx/`.

### The cost regression, and why it is nearly zero

`run.sh` records hello-world's build time and binary size with and without idstd
attached, because IDSTD.md §1.3 budgeted +0.6 s and +60 KB for a library this
size and that number decides whether graphics can ever be in the default import.

It is now wrong, in the right direction. **Dead-code elimination has landed**, and
only functions reachable from `main` reach the emitted C:

| build | emitted C | binary | wall |
| --- | --- | --- | --- |
| hello-world, `--no-std` | 416 lines | 16 360 B | 0.18 s |
| hello-world, idstd attached, calls none of it | 424 lines | 16 552 B | 0.20 s |
| one `fx_max(1, 2)` call | 434 lines | | |
| one `fx_atan2(3, 4)` call | 608 lines | | |

`id_fx_max` does not appear in the first program's C at all. So 128 functions
cost a program that ignores them **+0.02 s and +192 bytes** — parse-and-check
time and alignment, not code. Keep measuring it anyway: this is the number that
says when DCE stops working.

## Layout

```
idstd/
  README.md  NAMES.md  COMPILER-ASKS.md  run.sh     (none of these count toward the rule of 3)
  core/
    math/   fx/{base,lim,wide/}  trig/{tab,sin,ang/}  rnd.id
    data/   lst/{lst,grow,w/}  buf/{buf,cmp}
    text/   str/{make,scan,part}  chr/  fmt/
  sys/
    err/    err.id  mute.id  k/
    io/     term/{scr,draw,out}          the character-cell terminal
    win/    win.id                       inp_live, sys_next
  gfx/
    px/     l/{surf,px}  ppm/{dump,row}  t.id
    d2/     col.id  rect.id  txt/{font,g8,draw}
    d3/     geom.id  face.id  mesh/{build,push,coord}
  .tests/   one project per suite, hidden so it does not count
```

Every directory holds at most 3 `.id` files and subdirectories combined, at every
level including the root — the rule binds on imported trees, and idstd is an
imported tree in every build on this machine. Lay the skeleton out before adding
functions; idem records parallel workstreams repeatedly breaking each other's
builds by violating this mid-restructure, and the blast radius here is every
program rather than one engine.

## What is deliberately not here

| excluded | why |
| --- | --- |
| `str_of_int`, `str_of_word` | the runtime's own C helpers are already spelled `id_str_of_int` / `id_str_of_word`, and an `id` function of that name is a hard C compilation failure. Use `"" + n`, or `fmt_int(n, w)` for a column |
| `fx_hypot` | `fx_hyp` *is* the `int` case; a second function with that body is a duplicate-logic compile error |
| a game engine's model (`ent_`, `scn_`, `run_`, `sim_`, `ui_`) | that is idem's, not a standard library's |
| image, DEFLATE, zstd, `.blend`, font decoders | ~600 functions of general code and far too much to link into hello-world before DCE. Strong v2 candidates, probably as a separate opt-in `idfmt` |
| anything needing a filesystem *walk* | `id` cannot see directories; that is why `bin/idc` is a shell driver. A library cannot fix it |
| audio | there is no audio backend anywhere. Do not stub it |

`COMPILER-ASKS.md` records what `idstd` needs from `id_development` and what this
pass found to be wrong in the spec.
