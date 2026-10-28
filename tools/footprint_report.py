#!/usr/bin/env python3
"""Report complete ArghtOS SRAM-resident footprint from an LLD map."""

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

RESIDENT_OUTPUT_PREFIXES = (
    ".text",
    ".rodata",
    ".data",
    ".bss",
    ".noinit",
    ".arghtos_cfg",
    ".arghtos_crash",
    ".ARM.exidx",
)

APP_INPUT_PREFIXES = (
    ".app_text",
    ".app_rodata",
    ".app_data",
    ".app_bss",
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
    app_owned_inside_resident_outputs = 0

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

        if sec.startswith(APP_INPUT_PREFIXES):
            app_owned_inside_resident_outputs += size

        if sec.startswith((".text", ".rodata", ".data", ".bss", ".noinit")):
            for bucket, needle in needles.items():
                if needle in obj:
                    totals[bucket] += size
                    break

    linker_meta = output_sections.get(".ARM.exidx", 0)

    # Authority for the total is physical output-section occupancy, not the sum
    # of input symbols. This includes linker alignment/fill bytes and remains
    # valid when LTO merges kernel/port objects into a synthetic lto.o.
    resident_output_bytes = sum(
        size
        for section, size in output_sections.items()
        if section.startswith(RESIDENT_OUTPUT_PREFIXES)
    )
    linked_arghtos_bytes = (
        resident_output_bytes - app_owned_inside_resident_outputs
    )
    core = linked_arghtos_bytes + args.msp

    attributed_without_msp = (
        sum(totals.values()) + cfg + crash + linker_meta
    )
    unattributed_or_alignment = linked_arghtos_bytes - attributed_without_msp

    result = {
        "kernel_code_data": totals["kernel"],
        "port_c_code_data": totals["port_c"],
        "context_asm": totals["context_asm"],
        "board_adapter": totals["board_adapter"],
        "task_descriptors": cfg,
        "crash_record": crash,
        "linker_resident_metadata": linker_meta,
        "app_owned_excluded": app_owned_inside_resident_outputs,
        "linked_arghtos_before_msp": linked_arghtos_bytes,
        "unattributed_or_alignment": unattributed_or_alignment,
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
