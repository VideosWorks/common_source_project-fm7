/*
 * Common definition of FM-7 [FM7_COMMON]
 *  Author: K.Ohta
 *  Date  : 2015.01.01-
 *
 */


#ifndef _FM7_COMMON_H_
#define _FM7_COMMON_H_

enum 
{
   FM7_MAINMEM_OMOTE = 0, // $0000-$7FFF
   FM7_MAINMEM_URA,       // $8000-$FBFF #1
   FM7_MAINMEM_BASICROM,  // $8000-$FBFF #2
   FM7_MAINMEM_BIOSWORK,  // $FC00-$FC7F
   FM7_MAINMEM_SHAREDRAM, // $FC80-$FCFF
   FM7_MAINMEM_MMIO, // $FD00-$FDFF
   FM7_MAINMEM_BOOTROM_BAS,  // $FE00-$FFEF #1
   FM7_MAINMEM_BOOTROM_DOS,  // $FE00-$FFEF #2
   FM7_MAINMEM_BOOTROM_ROM3, // $FE00-$FFEF #3
   FM7_MAINMEM_BOOTROM_RAM,  // $FE00-$FFEF #4
   FM7_MAINMEM_VECTOR, // $FFF0-$FFFD
   FM7_MAINMEM_VECTOR_RESET, // $FFFE-$FFFF
   
   FM7_MAINMEM_77EXTRAM, // 77AVL4, 192KB EXTRAM.
   FM7_MAINMEM_77_NULLRAM, // 0x00 
   FM7_MAINMEM_77_SHADOWRAM, // 0x200
   
   FM7_MAINMEM_AV_PAGE0,
   FM7_MAINMEM_AV_SUBMEM,
   FM7_MAINMEM_AV_JCARD,
   FM7_MAINMEM_AV_EXTRAM786K,
   FM7_MAINMEN_AV_INITROM,
   FM7_MAINMEM_END
};

#define FM7_BOOTMODE_BASIC 0
#define FM7_BOOTMODE_DOS   1
#define FM7_BOOTMODE_ROM3  2
#define FM7_BOOTMODE_RAM   4


#endif // _FM7_COMMON_H_
