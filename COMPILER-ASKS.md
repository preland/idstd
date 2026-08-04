# What `idstd` needs from `id_development`

Written from the other side of the seam: everything here was hit while building
`core/math`, `core/data`, `core/text` and `sys/err`, not predicted. Items are
ordered by how much the library depends on them. The IDSTD.md §2 labels are kept
so the two documents line up.

Nothing in this file has been changed in `id_development` by me — that repository
is read-only from here, as briefed.

---

## Already landed while this pass was running — thank you, and one consequence

- **C1, implicit import.** Working. `bin/idc prog` resolves `fx_max` with no
  manifest and no flag, and `--std DIR` / `--no-std` / `IDC_NO_STD=1` all behave.
  `run.sh` passes `--std` at this working tree so the suite tests what is checked
  out rather than what is installed.
- **C3, transitive imports.** Not exercised yet — `idstd` has no `import.id` of
  its own, because nothing it contains needs a backend. `gfx/` and `sys/io` are
  the first users.
- **C2, dead-code elimination.** Also working, and it is the single biggest
  change to this library's economics. Measured on this checkout:

  | build | emitted C | binary | wall |
  | --- | --- | --- | --- |
  | hello-world, `--no-std` | 416 lines | 16 360 B | 0.18 s |
  | hello-world, idstd attached, calls none of it | 424 lines | 16 552 B | 0.20 s |
  | one `fx_max(1, 2)` call | 434 lines | | |
  | one `fx_atan2(3, 4)` call | 608 lines | | |

  `id_fx_max` does not appear in the first program's C at all, and `id_str_split`
  does not appear in the fourth's. So a 128-function library costs a program that
  does not use it **+0.02 s and +192 bytes** — which is parse-and-check time and
  alignment noise, not code. IDSTD.md §1.3 budgeted +0.6 s and +60 KB. That
  paragraph is now wrong in the right direction and worth updating, because it is
  the paragraph that made graphics unaffordable in the default library.

  **The consequence for §8's decision 2:** with DCE working, the argument against
  shipping `gfx/` in the implicitly-imported library is now only C7
  (link-on-demand for native backends), not code size. A framebuffer that nothing
  calls costs nothing; an X11 dependency that nothing calls still costs every
  program a link line.

---

## C4 — stop the library reserving the user's local names (**the one that matters**)

Still the most invasive thing about an always-on library, and this pass produced
a measurement rather than an argument.

Writing the obvious signature `str_cmp(string a, string b)` produced **28 compile
errors, 25 of them inside `core/math/fx/`**, pointing at code that had not
changed:

```
core/math/fx/base.id:21: error: argument 'a' of 'fx_max' expects int, got string
core/math/fx/base.id:25: error: cannot order string and string
core/math/fx/wide/sqrt.id:27: error: cannot initialize int[] 'sq' with a string[] value
...
core/text/str/scan/ord/cmp.id:20: error: variable 'a' is declared string here but
    int elsewhere; a name must keep one type across the whole program
```

The one message naming the cause is the twenty-sixth. That is the experience
inside a *single* tree, by an author who knew the rule and had written it into
`NAMES.md` two hours earlier. A user who declares `string a` in their own program
gets the same cascade, in library files they have never opened, for a library
they did not know they were importing.

**Recommendation unchanged: option (a), per-unit name-type checking.** The rule
earns its keep within one person's program; it does not earn it across a seam.
Until then, `NAMES.md` §2 is a 34-name vocabulary that every `id` program on this
machine is now bound by, and `w`, `h`, `x`, `y`, `z`, `src`, `name`, `key` and
`fb` are left unclaimed only because I chose not to claim them.

Two smaller asks in the same family:

- **Order the diagnostics cause-first.** Even without (a), reporting the
  `declared X here but Y elsewhere` message *before* its consequences would turn
  28 confusing errors into 1 clear one plus 27.
- **Say which side is the library.** None of the 28 messages contains the word
  `idstd`, and half of them name a path the user did not write.

## C5 — duplicate-logic diagnostics that know what a standard library is

Unchanged from IDSTD.md, and `NAMES.md` §1 now supplies what it asked for: every
function is marked **pub** or **int**(ernal). 128 functions, of which 47 are
internal — loop bodies, fold steps and blit helpers that exist only because a
block holds three actions.

Those 47 should not participate in the *user-facing* uniqueness check. A user
whose helper happens to match `str_join_sep`'s body is being told to call a
function they cannot reasonably discover, and the suggestion "call it instead" is
wrong for an internal.

The public 81 should keep participating — "there is already one, call it" is the
right answer for `fx_max`.

## C6 — automatic initialisation

Still needed. Three functions must be called from `main` today or the program
does not compile: `fx_trig_init`, `rnd_init`, `err_init`. The diagnostic is good
and it fires correctly:

```
core/math/trig/tab.id:39: error: 'fx_sintab' is exported by 'fx_trig_init', which
    nothing calls -- an export is initialised when its declaring function runs, so
    this reads an uninitialised global. Call 'fx_trig_init' from main's setup chain
```

Two notes from using it:

- It names a **library file and line** as the site of a user's mistake. With C6
  the whole class disappears; without it, this message should lead with the
  user's call to `fx_sin`.
- `rnd_init(int seed)` takes an argument, so an automatic chain has to decide the
  seed. Please make the default **a fixed literal, not `ticks()`** — a program
  that is reproducible unless it asks not to be is the better default, and
  `rnd_init(ticks())` is one line for anyone who wants the other.

## C7 — link a native backend only when its symbols are reachable

Not yet blocking, because `idstd` names no backend. It blocks `sys/io` (`fs`),
`sys/win` and all of `gfx/` — i.e. everything left in the brief. With C2 landed,
this is the *only* remaining obstacle to graphics being in the default library.

## C8 — diagnostics hygiene

- `bin/idc --version` reporting the resolved `idstd` would make "which library am
  I actually compiling against" answerable; `run.sh` works around it with `--std`.
- A stdlib frame in a diagnostic should look different from a user frame. Every
  error quoted in this document names a library path, and a beginner's first
  compile error now probably does too.

---

## Things in IDSTD.md that turned out to be wrong or unbuildable

Recorded so the spec can be corrected rather than re-attempted.

### `str_of_int` and `str_of_word` cannot exist (§3.3 contradicts §1.7)

§3.3 lists both under "to write new". §1.7 lists both as names to avoid. §1.7 is
right, and it is a **hard failure**, not a style issue: `id` function names get an
`id_` prefix and the runtime's own helpers are already `id_str_of_int` and
`id_str_of_word`, so either definition produces

```
error: conflicting types for 'id_str_of_int'; have 'int(int)'
note:  previous definition of 'id_str_of_int' with type 'char *(int)'
```

in both compilers, plus an "internal error: the self-hosted compiler emitted C
that does not compile" wrapper from `bin/idc`. `"" + n` is the int→string path;
`fmt_int(n, w)` is the aligned one. **The `bin/idc` wrapper is worth a look on
its own** — a user-caused name collision is reported as a compiler bug, complete
with "this is a bug in the self-hosted compiler; please report it."

### `fx_hypot` cannot coexist with `fx_hyp` (§3.1)

§3.1 asks for "`fx_hypot` for the `int` case". `fx_hyp(int a, int b) -> int`
*is* the int case. A second function with that body is a duplicate-logic compile
error, so the ask is not "write one more function" but "rename the existing one",
and renaming it would break idem. Not done; `fx_hyp` is the name.

### `buf_cmp` needed a decision the spec did not make (§3.2)

There is no `break`, so an ordering comparison over the flat store visits every
byte even when the first settles it. `buf_cmp` answers -1/0/1 like `memcmp` and
is O(n) always. Documented in its own header rather than silently O(n).

### The 3-entries-per-directory rule and a repository root

The rule binds on the repository root, because the root is what gets imported.
That leaves three slots for `core/`, `sys/` and `gfx/` and none for a test
directory. The suite therefore lives in **`.tests/`**, hidden, because both
compilers skip hidden entries — which is also the only reason a git repository
can be a project root at all (`.git` would otherwise be entry number four).

This works and is stable, but it is a workaround wearing a filename. If a
standard library is meant to live in its own repository, the entry rule probably
wants either an exemption list beside `import.id` or a way for a manifest to
declare which subdirectory is the library.

### Two smaller ones

- **An empty list literal in an argument position** has no known type:
  `str_join([], ",")` is `error: an empty list literal needs a known list type
  here`, and the compiler then guesses `int[]` and reports a second, confusing
  type error. Legal code requires a typed local first. Fine, but the second error
  is noise.
- **The `t_sset` shape.** A test function that declares a list, writes to it,
  prints it and calls the next link is four actions and cannot exist. This is
  working as intended, but it is worth knowing that a *test* is the code the
  3-action limit chafes against hardest: assertions are inherently
  "set up, act, observe, continue".
