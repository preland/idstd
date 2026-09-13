# prefix_check.awk -- every idstd parameter, local variable and internal
# function must be spelled idstd_<name> (NAMES.md section 0 and section 2).
# A pub function (NAMES.md section 1, "pub" column) keeps its bare name, and
# an export keeps its module-prefixed name (section 3) -- both are exempt.
#
# Usage: awk -f prefix_check.awk NAMES.md FILE...
#
# NAMES.md must be the first file: NR==FNR is true only while awk is on that
# first file, the standard two-file awk idiom for "read a table from file 1,
# then check every line of every later file against it" -- so the pub set is
# built before a single source line is checked, in one pass over argv. This
# mirrors .tests/names.py's own argv shape so the two checks read the same
# way, without sharing code (names.py is being ported to id separately and
# this file must not depend on it).
#
# Deliberately match()/substr() rather than one regex with several capturing
# groups: gawk's 3-arg match() numbers groups by position in the pattern, and
# the TYPE alternation below needs its own group for "(\[\])*", which silently
# shifts every group after it. Stripping the line piece by piece with 2-arg
# match() (which only ever sets RSTART/RLENGTH, never numbered groups) side-
# steps that miscount entirely.

BEGIN {
    bad = 0
}

# ---- pass over NAMES.md: collect the pub-function set ----------------------
NR == FNR {
    if ($0 ~ /^\| `[a-z_][a-z_0-9]*` \|.*\| *pub/) {
        line = $0
        sub(/^\| `/, "", line)
        sub(/`.*/, "", line)
        pubs[line] = 1
    }
    next
}

# length of a TYPE token (int/word/float/string/void, any number of []) at
# the start of s, or 0 if s does not start with one.
function type_len(s) {
    if (match(s, /^(int|word|float|string|void)(\[\])*/)) {
        return RLENGTH
    }
    return 0
}

# Report one bad name.
function report(kind, name, extra) {
    bad++
    if (extra == "") {
        printf "  %s:%d: %s '%s' lacks the idstd_ prefix\n", FILENAME, FNR, kind, name > "/dev/stderr"
    } else {
        printf "  %s:%d: %s '%s' of '%s' lacks the idstd_ prefix\n", FILENAME, FNR, kind, name, extra > "/dev/stderr"
    }
}

# Check one "TYPE name" parameter (already trimmed of surrounding whitespace).
function check_param(p, fname,    tl, rest) {
    if (p == "") return
    tl = type_len(p)
    if (tl == 0) return                          # not a recognisable "TYPE name"
    rest = substr(p, tl + 1)
    sub(/^[ \t]+/, "", rest)
    if (rest !~ /^[A-Za-z_][A-Za-z0-9_]*$/) return
    if (rest !~ /^idstd_/) report("parameter", rest, fname)
}

# Check one full source line for a local/exported declaration.
function check_decl(line,    rest, is_export, tl, name, after) {
    rest = line
    is_export = 0
    if (match(rest, /^[ \t]*export[ \t]+/)) {
        is_export = 1
        rest = substr(rest, RSTART + RLENGTH)
    } else if (match(rest, /^[ \t]+/)) {
        rest = substr(rest, RSTART + RLENGTH)
    }
    tl = type_len(rest)
    if (tl == 0) return
    rest = substr(rest, tl + 1)
    if (!match(rest, /^[ \t]+/)) return
    rest = substr(rest, RLENGTH + 1)
    if (!match(rest, /^[A-Za-z_][A-Za-z0-9_]*/)) return
    name = substr(rest, RSTART, RLENGTH)
    after = substr(rest, RLENGTH + 1)
    if (after !~ /^[ \t]*[=;]/) return            # not "name = ..." / "name;"
    if (!is_export && name !~ /^idstd_/) report("local", name, "")
}

# ---- every later file: the actual source to check ---------------------------
{
    line = $0
    sub(/\/\/.*/, "", line)                  # drop comments
    gsub(/"(\\.|[^"\\])*"/, "\"\"", line)    # blank out string contents

    if (match(line, /^([A-Za-z_][A-Za-z0-9_]*)\(([^)]*)\)[ \t]*\{[ \t]*$/)) {
        fullmatch = substr(line, RSTART, RLENGTH)
        parenpos = index(fullmatch, "(")
        closeparen = index(fullmatch, ")")
        fname = substr(fullmatch, 1, parenpos - 1)
        params = substr(fullmatch, parenpos + 1, closeparen - parenpos - 1)

        if (fname !~ /^idstd_/ && !(fname in pubs)) {
            report("internal function", fname, "")
        }
        n = split(params, plist, ",")
        for (i = 1; i <= n; i++) {
            p = plist[i]
            gsub(/^[ \t]+|[ \t]+$/, "", p)
            check_param(p, fname)
        }
        next
    }

    check_decl(line)
}

END {
    if (bad > 0) {
        printf "prefix_check: %d name(s) missing the idstd_ prefix\n", bad > "/dev/stderr"
        exit 1
    }
    print "prefix_check: every parameter, local and internal function is prefixed"
    exit 0
}
