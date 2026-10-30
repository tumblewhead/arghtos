#!/usr/bin/env python3
"""Verify the canonical R6aj Cortex-M23 prebuilt-frame relocations."""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path


SLOT_BYTES = 128
PERIODIC_COUNT = 23
ASYNC_ID = 23
HW_R0_OFFSET = 96
HW_LR_OFFSET = 116
HW_PC_OFFSET = 120


def fail(message: str) -> None:
    raise SystemExit(f"R6aj prebuilt-context check failed: {message}")


def relocations(path: Path) -> dict[int, str]:
    proc = subprocess.run(
        ["readelf", "-rW", str(path)],
        check=True,
        text=True,
        stdout=subprocess.PIPE,
    )
    result: dict[int, str] = {}
    for line in proc.stdout.splitlines():
        match = re.match(r"^\s*([0-9A-Fa-f]+)\s+\S+\s+\S+\s+\S+\s+(\S+)", line)
        if match is None:
            continue
        offset = int(match.group(1), 16)
        symbol = match.group(2)
        result[offset] = symbol
    return result


def main() -> None:
    if len(sys.argv) != 2:
        fail("usage: check_r6aj_prebuilt_context.py <object>")

    path = Path(sys.argv[1])
    rel = relocations(path)

    guard_offsets = {
        task_id * SLOT_BYTES + HW_LR_OFFSET
        for task_id in range(PERIODIC_COUNT + 1)
    }
    periodic_pc_offsets = {
        task_id * SLOT_BYTES + HW_PC_OFFSET
        for task_id in range(PERIODIC_COUNT)
    }
    periodic_r0_offsets = {
        task_id * SLOT_BYTES + HW_R0_OFFSET
        for task_id in range(PERIODIC_COUNT)
    }
    async_r0 = ASYNC_ID * SLOT_BYTES + HW_R0_OFFSET
    async_lr = ASYNC_ID * SLOT_BYTES + HW_LR_OFFSET
    async_pc = ASYNC_ID * SLOT_BYTES + HW_PC_OFFSET

    for offset in sorted(guard_offsets):
        if rel.get(offset) != "arghtos_task_return_guard":
            fail(
                f"LR relocation at 0x{offset:x} is "
                f"{rel.get(offset)!r}, expected arghtos_task_return_guard"
            )

    for offset in sorted(periodic_pc_offsets):
        if rel.get(offset) != "arghtos_task_start_trampoline":
            fail(
                f"periodic PC relocation at 0x{offset:x} is "
                f"{rel.get(offset)!r}, expected arghtos_task_start_trampoline"
            )

    for offset in sorted(periodic_r0_offsets):
        if rel.get(offset) != "task_stub":
            fail(
                f"periodic R0 relocation at 0x{offset:x} is "
                f"{rel.get(offset)!r}, expected task_stub"
            )

    if async_r0 in rel:
        fail(f"async R0 at 0x{async_r0:x} unexpectedly has a relocation")

    if rel.get(async_lr) != "arghtos_task_return_guard":
        fail(
            f"async LR relocation at 0x{async_lr:x} is "
            f"{rel.get(async_lr)!r}, expected arghtos_task_return_guard"
        )

    if rel.get(async_pc) != "task_stub":
        fail(
            f"async PC relocation at 0x{async_pc:x} is "
            f"{rel.get(async_pc)!r}, expected task_stub"
        )

    trampoline_count = sum(
        1 for symbol in rel.values() if symbol == "arghtos_task_start_trampoline"
    )
    guard_count = sum(
        1 for symbol in rel.values() if symbol == "arghtos_task_return_guard"
    )
    task_stub_count = sum(1 for symbol in rel.values() if symbol == "task_stub")

    if trampoline_count != PERIODIC_COUNT:
        fail(
            f"found {trampoline_count} trampoline relocations, "
            f"expected {PERIODIC_COUNT}"
        )
    if guard_count != PERIODIC_COUNT + 1:
        fail(
            f"found {guard_count} return-guard relocations, "
            f"expected {PERIODIC_COUNT + 1}"
        )
    if task_stub_count != PERIODIC_COUNT + 1:
        fail(
            f"found {task_stub_count} task_stub relocations, "
            f"expected {PERIODIC_COUNT + 1}"
        )

    print(
        "R6aj prebuilt-context relocation check: PASS "
        f"({PERIODIC_COUNT} periodic frames + 1 direct async frame)"
    )


if __name__ == "__main__":
    main()
