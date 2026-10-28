#!/usr/bin/env python3
"""Reject target-specific leakage into ArghtOS portable sources."""

from pathlib import Path
import re
import sys

FILES = [
    Path("include/arghtos/arghtos.h"),
    Path("include/arghtos/arghtos_port.h"),
    Path("src/arghtos_internal.h"),
    Path("src/arghtos_kernel_port.h"),
    Path("src/arghtos_kernel.c"),
]

FORBIDDEN = [
    r"\bPRIMASK\b",
    r"\bPSP\b",
    r"\bMSP\b",
    r"\bPendSV\b",
    r"\bSVC\b",
    r"\bNVIC\b",
    r"\bWFI\b",
    r"\bEXC_RETURN\b",
    r"\bCortex-M\w*\b",
    r"__asm\b",
    r"\bcore_cm\w*\.h\b",
    r"\bcmsis\w*\.h\b",
]

PATTERNS = [re.compile(x, re.IGNORECASE) for x in FORBIDDEN]

def main() -> int:
    bad = []
    for path in FILES:
        text = path.read_text()
        for lineno, line in enumerate(text.splitlines(), 1):
            for pattern in PATTERNS:
                if pattern.search(line):
                    bad.append((path, lineno, pattern.pattern, line.strip()))
    if bad:
        for path, lineno, pattern, line in bad:
            print(f"{path}:{lineno}: forbidden {pattern}: {line}", file=sys.stderr)
        return 1
    print("portable source boundary: PASS")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
