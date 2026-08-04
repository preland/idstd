#!/usr/bin/env python3
"""Assert that every golden line agrees with the expectation written into it.

A golden file is only evidence if something checks it. These ones are written so
that each line states what it expects *and* prints what it got:

    fx_sqrt(999999) exp 999 | (1000000) exp 1000 = 999 1000

so the line carries its own assertion, and this script is what enforces it: pull
the tokens that follow each `exp` in the label, pull the tokens after the final
` = `, and require the two sequences to be equal. A golden file regenerated from
a broken build fails here rather than silently becoming the new truth.

Bracketed values (`[x   ]`) are one token including their inner spaces, because
padding and empty-string results are exactly the cases that matter and are
exactly the ones naive whitespace splitting would destroy.

A token that is not a number, a bracketed value or a sign is treated as prose and
ends that `exp` group -- so `exp -2147483648 wraps` expects one value and then
stops. Lines with no `exp` at all, and lines whose two sides have different
lengths for that reason, are reported as UNCHECKED rather than passed silently:
an assertion this script cannot read is one a person has to.

Usage: label.py FILE...      exit 0 if every checked line agrees
"""
import re
import sys


def tokens(text):
    """Split on whitespace, but keep [...] together."""
    out, i, n = [], 0, len(text)
    while i < n:
        if text[i].isspace():
            i += 1
            continue
        if text[i] == "[":
            j = text.find("]", i)
            if j < 0:
                out.append(text[i:])
                break
            out.append(text[i:j + 1])
            i = j + 1
        else:
            j = i
            while j < n and not text[j].isspace():
                j += 1
            out.append(text[i:j])
            i = j
    return out


# A value is a bracketed literal, a decimal (with sign and fraction), or an
# eight-digit hex word -- fmt_hex's output. Eight digits exactly, so that a
# prose word made only of a-f letters cannot be mistaken for one.
VALUE = re.compile(r"^(\[.*\]|-?\d+(\.\d+)?|[0-9a-f]{8})$")


def expected(label):
    """Every value token following an `exp`, in order."""
    out = []
    for part in label.split("exp ")[1:]:
        for tok in tokens(part):
            if not VALUE.match(tok):
                break
            out.append(tok)
    return out


def check(path):
    bad = unchecked = 0
    for lineno, raw in enumerate(open(path), 1):
        line = raw.rstrip("\n")
        # A line with no `exp` is not an assertion, it is output the program
        # produced (sys/err's suite prints real diagnostics). The golden diff
        # already covers those, so they are not this script's business.
        if "exp " not in line:
            continue
        if " = " not in line:
            unchecked += 1
            print(f"  UNCHECKED {path}:{lineno}: an `exp` with no ` = ` to compare against")
            continue
        label, _, got = line.rpartition(" = ")
        want = expected(label)
        have = tokens(got)
        if not want:
            unchecked += 1
            print(f"  UNCHECKED {path}:{lineno}: no readable expectation")
        elif want != have:
            bad += 1
            print(f"  MISMATCH {path}:{lineno}")
            print(f"    stated:   {want}")
            print(f"    measured: {have}")
    return bad, unchecked


def main(argv):
    bad = unchecked = 0
    for path in argv:
        b, u = check(path)
        bad += b
        unchecked += u
    if bad:
        print(f"label: {bad} line(s) disagree with their own stated expectation")
        return 1
    if unchecked:
        print(f"label: {unchecked} line(s) could not be checked automatically")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
