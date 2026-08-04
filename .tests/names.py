#!/usr/bin/env python3
"""Enforce NAMES.md against the library source.

`NAMES.md` claims to be a build dependency rather than documentation. This is
what makes that true: every variable name the library declares -- parameter,
local or export -- must appear in NAMES.md, with the type NAMES.md says it has,
and every function must be listed with its file.

That is not pedantry. A name keeps one type across the whole program, imported
trees included, so every parameter in this library is public API: declaring
`int w` here makes `string w` a compile error in every `id` program on the
machine. A registry nobody checks drifts within a day -- this script was written
after the audit that found the library had quietly claimed `w`, `x` and `y` while
NAMES.md said all three were deliberately left free.

It parses `id` rather than importing anything, which is crude but exact enough:
declarations in this language are `TYPE name = ...`, parameters are `TYPE name`
inside a function header, and exports are `export TYPE name = ...`.

Usage: names.py NAMES.md DIR...      exit 0 if the registry covers the code
"""
import os
import re
import sys

TYPE = r"(?:int|word|float|string|void)(?:\[\])*"
FUNC = re.compile(r"^([a-z_][a-z_0-9]*)\s*\(([^)]*)\)\s*\{")
DECL = re.compile(r"^\s*(?:export\s+)?(" + TYPE + r")\s+([a-z_][a-z_0-9]*)\s*=")
PARAM = re.compile(r"^\s*(" + TYPE + r")\s+([a-z_][a-z_0-9]*)\s*$")


def id_files(dirs):
    for d in dirs:
        for root, subdirs, files in os.walk(d):
            subdirs[:] = [s for s in subdirs if not s.startswith(".")]
            for f in sorted(files):
                if f.endswith(".id") and f != "import.id":
                    yield os.path.join(root, f)


def scan(dirs):
    """-> {name: {type}}, {function name: path}"""
    names, funcs = {}, {}
    for path in id_files(dirs):
        for raw in open(path):
            line = raw.split("//")[0]
            m = FUNC.match(line)
            if m:
                funcs[m.group(1)] = path
                for p in m.group(2).split(","):
                    q = PARAM.match(p)
                    if q:
                        names.setdefault(q.group(2), set()).add(q.group(1))
                continue
            m = DECL.match(line)
            if m:
                names.setdefault(m.group(2), set()).add(m.group(1))
    return names, funcs


def main(argv):
    registry = open(argv[0]).read()
    names, funcs = scan(argv[1:])
    bad = []

    for name, types in sorted(names.items()):
        if len(types) > 1:
            bad.append(f"{name} is declared as {sorted(types)} -- "
                       "a name keeps ONE type across the whole program")
        if f"`{name}`" not in registry:
            bad.append(f"variable '{name}' ({sorted(types)[0]}) is declared in the "
                       "library but is not in NAMES.md section 2")

    for fn in sorted(funcs):
        if f"`{fn}`" not in registry:
            bad.append(f"function '{fn}' ({funcs[fn]}) is not in NAMES.md section 1")

    # The reverse direction: NAMES.md must not promise functions that are gone.
    for row in re.findall(r"^\| `([a-z_][a-z_0-9]*)` \| `\(", registry, re.M):
        if row not in funcs:
            bad.append(f"NAMES.md lists '{row}', which no longer exists in the library")

    if bad:
        for b in bad:
            print("  " + b)
        print(f"names: {len(bad)} registry problem(s)")
        return 1
    print(f"names: {len(funcs)} functions and {len(names)} variable names, all registered")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
