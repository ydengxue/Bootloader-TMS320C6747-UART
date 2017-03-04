/*
 *  Copyright 2008 by Spectrum Digital Incorporated.
 *  All rights reserved.  Property of Spectrum Digital Incorporated.
 */

/*
 *  Linker command file
 *
 */

-l rts6740_elf.lib
-stack           0x00004000      /* Stack Size */
-heap            0x01000000      /* Heap Size */

MEMORY
{
    L2RAM:                o = 0x11800000  l = 0x00020000
    SHAREDRAM:            o = 0x80000000  l = 0x00020000
    EMIFB_SDRAM           o = 0xC0000000  l = 0x02000000
}

SECTIONS
{
    .stack              >   L2RAM
    .fardata.l2data     >   L2RAM
    .bss                >   L2RAM
    .neardata           >   L2RAM
    .rodata             >   L2RAM
    .far                >   SHAREDRAM | EMIFB_SDRAM
    .fardata            >   SHAREDRAM | EMIFB_SDRAM
    .text               >   SHAREDRAM | EMIFB_SDRAM
    .const              >   SHAREDRAM | EMIFB_SDRAM
    .switch             >   SHAREDRAM | EMIFB_SDRAM
    .cio                >   SHAREDRAM | EMIFB_SDRAM
    .sysmem             >   EMIFB_SDRAM
    .cinit              >   EMIFB_SDRAM
    .init_code          >   EMIFB_SDRAM
}
