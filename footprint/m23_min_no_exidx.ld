ENTRY(footprint_entry)
MEMORY
{
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}
SECTIONS
{
    . = ORIGIN(RAM);
    .text : ALIGN(4) {
        *(.text*)
        *(.rodata*)
        *(.app_text*)
        *(.app_rodata*)
    } > RAM
    .arghtos_cfg : ALIGN(4) { KEEP(*(.arghtos_cfg)) } > RAM
    .arghtos_crash : ALIGN(4) { KEEP(*(.arghtos_crash)) } > RAM
    .data : ALIGN(4) {
        *(.data*)
        *(.app_data*)
    } > RAM
    .bss (NOLOAD) : ALIGN(8) {
        *(.bss*)
        *(COMMON)
    } > RAM
    .app_stacks (NOLOAD) : ALIGN(8) {
        *(.app_stacks*)
    } > RAM
    /DISCARD/ : {
        *(.ARM.exidx*)
        *(.ARM.extab*)
        *(.comment*)
        *(.ARM.attributes*)
    }
}
