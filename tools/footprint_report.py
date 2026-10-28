#!/usr/bin/env python3
"""Report ArghtOS SRAM-resident footprint from an LLD map used by the M23 harness."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

LINE_RE = re.compile(
    r"^(?P<vma>[0-9a-f]+)\s+[0-9a-f]+\s+(?P<size>[0-9a-f]+)\s+\d+\s+\s*(?P<object>\S+):\((?P<section>[^)]+)\)"
)
OUT_RE = re.compile(
    r"^(?P<vma>[0-9a-f]+)\s+[0-9a-f]+\s+(?P<size>[0-9a-f]+)\s+\d+\s+(?P<section>\.[A-Za-z0-9_.]+)$"
)

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--map", required=True, dest="map_path")
    ap.add_argument("--msp", type=int, default=0, help="ArghtOS-only MSP bytes")
    ap.add_argument("--soft-target", type=int, default=1536)
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()

    lines = Path(args.map_path).read_text().splitlines()
    needles = {
        "kernel": "arghtos_kernel",
        "port_c": "arghtos_port",
        "context_asm": "arghtos_context",
        "board_adapter": "m23_board_stub",
    }
    totals = {k: 0 for k in needles}
    output_sections: dict[str, int] = {}
    cfg = 0
    crash = 0

    for raw in lines:
        line = raw.strip()
        mo = OUT_RE.match(line)
        if mo:
            output_sections[mo.group("section")] = int(mo.group("size"), 16)

        m = LINE_RE.match(line)
        if not m:
            continue
        size = int(m.group("size"), 16)
        obj = Path(m.group("object")).name
        sec = m.group("section")

        if sec == ".arghtos_cfg":
            cfg += size
        elif sec == ".arghtos_crash":
            crash += size

        if sec.startswith((".text", ".rodata", ".data", ".bss")):
            for bucket, needle in needles.items():
                if needle in obj:
                    totals[bucket] += size
                    break

    linker_meta = output_sections.get(".ARM.exidx", 0)
    core = sum(totals.values()) + cfg + crash + linker_meta + args.msp

    result = {
        "kernel_code_data": totals["kernel"],
        "port_c_code_data": totals["port_c"],
        "context_asm": totals["context_asm"],
        "board_adapter": totals["board_adapter"],
        "task_descriptors": cfg,
        "crash_record": crash,
        "linker_resident_metadata": linker_meta,
        "arghtos_only_msp": args.msp,
        "arghtos_resident_total": core,
        "soft_target": args.soft_target,
        "delta_to_target": args.soft_target - core,
        "target_met": core <= args.soft_target,
    }

    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        for key, value in result.items():
            print(f"{key:28s} {value}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
