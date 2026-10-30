#!/usr/bin/env python3
"""Check one function's compiler-reported static stack usage."""

from __future__ import annotations

import argparse
from pathlib import Path


def fail(message: str) -> None:
    raise SystemExit(f"stack-usage check failed: {message}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("su_file", type=Path)
    parser.add_argument("function")
    parser.add_argument("expected_bytes", type=int)
    args = parser.parse_args()

    if not args.su_file.is_file():
        fail(f"missing stack-usage file: {args.su_file}")

    matches: list[tuple[int, str]] = []
    for raw in args.su_file.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line:
            continue

        # GCC/Clang .su format is normally:
        # source:line:column:function<TAB>bytes<TAB>static|dynamic...
        fields = line.split("\t")
        if len(fields) < 3:
            fields = line.split()
        if len(fields) < 3:
            continue

        location = fields[0]
        name = location.rsplit(":", 1)[-1]
        if name != args.function:
            continue

        try:
            usage = int(fields[1], 0)
        except ValueError:
            fail(f"non-integer usage for {args.function}: {fields[1]!r}")
        matches.append((usage, fields[2]))

    if not matches:
        fail(f"{args.function!r} not found in {args.su_file}")

    if len(matches) != 1:
        fail(f"expected one {args.function!r} record, found {len(matches)}")

    usage, kind = matches[0]
    if usage != args.expected_bytes:
        fail(
            f"{args.function} uses {usage} B, expected "
            f"{args.expected_bytes} B ({kind})"
        )

    if not kind.startswith("static"):
        fail(f"{args.function} usage is not static: {kind!r}")

    print(
        f"stack-usage check: PASS "
        f"{args.function}={usage} B ({kind})"
    )


if __name__ == "__main__":
    main()
