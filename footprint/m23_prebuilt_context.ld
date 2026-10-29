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
    /*
     * R5l generated application stacks are load-initialized so each task's
     * initial saved Cortex-M23 context already exists before arghtos_init().
     * The RAM footprint is unchanged; this trades load-image bytes/startup
     * initialization for smaller resident scheduler code.
     */
    .app_stacks : ALIGN(8) {
        *(.app_stacks*)
    } > RAM
    /DISCARD/ : {
        *(.ARM.exidx*)
        *(.ARM.extab*)
        *(.comment*)
        *(.ARM.attributes*)
    }
}
