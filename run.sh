#!/usr/bin/env bash
# idstd's regression suite.
#
# Three kinds of check, and the first one matters more than it looks:
#
#   1. THE WHOLE LIBRARY COMPILES AS ONE PROGRAM, in every project. `id` has no
#      module system -- every function and every variable name in every imported
#      directory lands in one flat namespace, one type per name, with no two
#      function bodies allowed to be equal up to renaming. So a module can pass
#      its own tests and still be unbuildable next to its neighbour. Every suite
#      below links the whole library, so every build is that check.
#
#   2. Each project under .tests/ builds, runs, and matches its golden file.
#      Every line of every golden file names the value it expects in its own
#      label, and every one of those values was computed independently in python3
#      -- math.isqrt, math.atan2, an FNV-1a model, a Park-Miller model, str.split
#      -- rather than read off a run of this code.
#
#   3. The registry, and the cost regression. NAMES.md is enforced against the
#      source in both directions. hello-world's build time and binary size are
#      recorded with and without idstd: dead-code elimination has landed, so the
#      difference should stay near zero, and this is how we find out when it does
#      not. IDSTD.md 1.3 budgeted +0.6 s and +60 KB for a library this size.
#
# Everything runs through BOTH compilers. They emit byte-identical C, so running
# both is the cheapest parity check available -- and idstd is in every program's
# build now, so a parity break is a break everywhere.
#
# Usage:  ./run.sh [suite ...]     (default: everything)
#
# Note on the rule of 3: .tests/ holds six projects, which is not a violation --
# the limit is enforced against a *built project's* tree, and each .tests/<name>
# is its own project. .tests is a container of projects and is never compiled as
# one. It is also HIDDEN, which is load-bearing: the rule of 3 binds on this
# repository's root because the root is what gets imported, and both compilers
# skip hidden entries entirely (which is also the only reason a git repo can be a
# project root at all). Keeping the suite hidden leaves the root's three slots
# for core/, sys/ and gfx/.

set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ID_DEV="${ID_DEV:-$(cd "$ROOT/.." && pwd)/id_development}"
WORK="${TMPDIR:-/tmp}/idstd-test.$$"
mkdir -p "$WORK"
trap 'rm -rf "$WORK"' EXIT

if [ ! -x "$ID_DEV/bin/idc" ]; then
    echo "run.sh: cannot find bin/idc at $ID_DEV" >&2
    echo "run.sh: set ID_DEV to the id_development checkout" >&2
    exit 2
fi

# --std points at THIS working tree rather than relying on the sibling-directory
# default, so the suite tests what is checked out here and not whatever happens
# to be installed.
IDC="$ID_DEV/bin/idc --std $ROOT"
IDC_PY="python3 $ID_DEV/idc.py --std $ROOT"

pass=0; fail=0; failed=()
say()  { printf '%s\n' "$*"; }
ok()   { pass=$((pass+1)); printf '  ok    %s\n' "$1"; }
bad()  { fail=$((fail+1)); failed+=("$1"); printf '  FAIL  %s\n' "$1"; }

# Build a project with one compiler, run it, and diff stdout against the golden
# file. A test binary's exit code is its own error count (sys/err's suite exits
# nonzero on purpose), so the exit code is not a pass/fail signal here -- the
# golden file is.
check() { # name, project dir, compiler label, compiler command
    local name="$1" dir="$2" who="$3" cc="$4"
    local out="$WORK/$name.$who"
    if ! $cc "$dir" -o "$out.bin" >"$out.log" 2>&1; then
        bad "$name [$who] compile"
        sed -n '1,12p' "$out.log" | sed 's/^/        /'
        return
    fi
    "$out.bin" >"$out.txt" 2>&1
    if [ ! -f "$dir/golden.txt" ]; then
        bad "$name [$who] has no golden.txt"
        return
    fi
    if diff -u "$dir/golden.txt" "$out.txt" >"$out.diff"; then
        ok "$name [$who] ($(wc -l <"$dir/golden.txt" | tr -d ' ') golden lines)"
    else
        bad "$name [$who] golden mismatch"
        sed -n '1,20p' "$out.diff" | sed 's/^/        /'
    fi
}

# A golden file is only evidence if something checks it, and a golden file
# regenerated from a broken build is worse than none. Every assertion line states
# what it expects and prints what it got; label.py requires the two to agree, so
# a regenerated golden fails here instead of quietly becoming the new truth.
labels() { # name, project dir
    if python3 "$ROOT/.tests/label.py" "$2/golden.txt" >"$WORK/$1.label" 2>&1; then
        ok "$1 [labels] every line agrees with its own stated expectation"
    else
        bad "$1 [labels] a line disagrees with what it says it expects"
        sed -n '1,20p' "$WORK/$1.label" | sed 's/^/        /'
    fi
}

# ------------------------------------------------------------- the registry
#
# NAMES.md claims to be a build dependency rather than documentation; this is
# what makes that true. Every variable name the library declares must be listed
# with its type, and every function with its file, in both directions. A name
# keeps one type across the whole program including imported trees, so every
# parameter here is public API -- and a registry nobody checks drifts within a
# day. This check was added after an audit found the library had quietly claimed
# `w`, `x` and `y` while NAMES.md said all three were deliberately left free.
if [ $# -eq 0 ]; then
    say "names"
    if python3 "$ROOT/.tests/names.py" "$ROOT/NAMES.md" "$ROOT/core" "$ROOT/sys" "$ROOT/gfx" >"$WORK/names.out" 2>&1; then
        ok "$(cat "$WORK/names.out")"
    else
        bad "NAMES.md does not match the library"
        sed -n '1,20p' "$WORK/names.out" | sed 's/^/        /'
    fi
fi

suites=("$@")
if [ ${#suites[@]} -eq 0 ]; then
    suites=(math data text err vendor)
fi

for m in "${suites[@]}"; do
    dir="$ROOT/.tests/$m"
    [ -d "$dir" ] || { bad "$m (no such suite)"; continue; }
    say "$m"
    if [ "$m" = "vendor" ]; then
        # The explicit-import path: --no-std, so nothing resolves unless
        # .tests/vendor/import.id does the work.
        check "$m" "$dir" "idc"    "$ID_DEV/bin/idc --no-std"
        check "$m" "$dir" "idc.py" "python3 $ID_DEV/idc.py --no-std"
    else
        check "$m" "$dir" "idc"    "$IDC"
        check "$m" "$dir" "idc.py" "$IDC_PY"
    fi
    labels "$m" "$dir"
done

# ------------------------------------------------------------ cost regression
#
# Wall time is best-of-three, because a single timing on a shared machine is
# noise. Binary size is exact. Both are reported rather than asserted against a
# threshold: the number is the point, and a threshold would either be so loose it
# never fires or so tight it fires on a busy afternoon. What IS asserted is that
# the with-idstd build still works at all.
if [ $# -eq 0 ]; then
    say "cost"
    hello="$ROOT/.tests/hello"
    best() { # label, compiler command -> prints "seconds bytes"
        local cc="$2" t bestt=999 sz=0
        for _ in 1 2 3; do
            local s0 s1
            s0=$(date +%s.%N)
            $cc "$hello" -o "$WORK/hello.bin" >/dev/null 2>&1 || { echo "FAIL 0"; return; }
            s1=$(date +%s.%N)
            t=$(awk "BEGIN{print $s1-$s0}")
            bestt=$(awk "BEGIN{print ($t<$bestt)?$t:$bestt}")
        done
        sz=$(stat -c %s "$WORK/hello.bin")
        printf '%.2f %s\n' "$bestt" "$sz"
    }
    read -r t_no  s_no  <<<"$(best nostd "$ID_DEV/bin/idc --no-std")"
    read -r t_std s_std <<<"$(best std   "$IDC")"
    if [ "$s_std" = "0" ] || [ "$s_no" = "0" ]; then
        bad "cost regression (a hello-world build failed)"
    else
        ok "hello-world --no-std      ${t_no}s  ${s_no} bytes"
        ok "hello-world + idstd       ${t_std}s  ${s_std} bytes"
        say "        idstd costs $(awk "BEGIN{printf \"%+.2f\", $t_std-$t_no}")s and \
$(awk "BEGIN{printf \"%+d\", $s_std-$s_no}") bytes on a program that calls none of it"
        say "        (dead-code elimination is live, so this should stay near zero -- it is how we know it still is)"
    fi
fi

say ""
say "$pass passed, $fail failed"
if [ $fail -gt 0 ]; then
    printf 'failed: %s\n' "${failed[*]}"
    exit 1
fi
