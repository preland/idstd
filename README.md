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
| `core/math` | `fx_` `rnd_` | **built.** Fixed point, roots, magnitudes, the 91-entry trig table, the inverse tangent, Park–Miller |
| `core/data` | `lst_` `buf_` + the five bare helpers | **built.** |
| `core/text` | `str_` `chr_` `fmt_` | **built.** |
| `sys/err` | `err_` | **built.** |
| `sys/io` | `file_` `term_` | **not built.** Needs `backends/fs`, and a `term_`-prefixed rewrite of `id_development/demos/engine`, whose functions are named `clear()`, `render()`, `drain()` |
| `sys/win` | `sys_` `inp_` | **not built.** Blocked on link-on-demand for native backends |
| `gfx/px` `gfx/d2` `gfx/d3` | `sf_` `d2_` `txt_` `m4_` `d3_` | **not built.** Same block, plus ~400 functions that would be linked into hello-world until dead-code elimination lands |
| a `flt_` float mirror | `flt_` | **deferred**, deliberately — see "Decisions" |

Everything marked built is covered by `run.sh`, through **both** compilers.

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

Order among the three does not matter; *before first use* does. The compiler
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
- **Every zero-action function returning a bare int literal takes that literal
  away from every program.** So idstd reserves the block **7000–7999** for
  `<prefix>_base() + n` constants and promises never to define a bare-literal
  constant outside it (`NAMES.md` §4).
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
2. **Graphics is out of scope for this pass.** It depends on dead-code
   elimination and link-on-demand for native backends, or every hello-world links
   X11 and OpenGL.
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

## Testing

```sh
./run.sh              # both compilers, all suites, plus the cost regression
./run.sh math text    # one or more suites
```

`run.sh` builds each project under `.tests/` with `bin/idc` **and** with
`python3 idc.py`, runs it, and diffs stdout against a golden file. The two
compilers emit byte-identical C, so running both is the cheapest parity check
available — and idstd is now in every program's build, so a parity break is a
break everywhere.

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

### The cost regression

`run.sh` records hello-world's build time and binary size with and without idstd
attached. There is no dead-code elimination yet, so **every function in the
library reaches the emitted C of every program**, and that number is the tax.
It is also the number that will tell us whether DCE is working once it lands:

| build | wall | binary |
| --- | --- | --- |
| hello-world, `--no-std` | *see `run.sh` output* | |
| hello-world, with idstd | | |

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
