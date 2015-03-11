/*
 * Common definition of FM-7 [FM7_COMMON]
 *  Author: K.Ohta
 *  Date  : 2015.01.01-
 *
 */


#ifndef _FM7_COMMON_H_
#define _FM7_COMMON_H_

enum {
	FM7_MAINMEM_OMOTE = 0, // $0000-$7FFF
	FM7_MAINMEM_URA,       // $8000-$FBFF #1
	FM7_MAINMEM_BASICROM,  // $8000-$FBFF #2
	FM7_MAINMEM_BIOSWORK,  // $FC00-$FC7F
	FM7_MAINMEM_SHAREDRAM, // $FC80-$FCFF
	FM7_MAINMEM_MMIO, // $FD00-$FDFF
	FM7_MAINMEM_BOOTROM_BAS,  // $FE00-$FFEF #1
	FM7_MAINMEM_BOOTROM_DOS,  // $FE00-$FFEF #2
	FM7_MAINMEM_BOOTROM_MMR, // $FE00-$FFEF #3
	FM7_MAINMEM_BOOTROM_RAM,  // $FE00-$FFEF #4
	FM7_MAINMEM_VECTOR, // $FFF0-$FFFD
	FM7_MAINMEM_NULL, // NULL
	FM7_MAINMEM_ZERO, // ZERO
	
	FM7_MAINMEM_EXTRAM, //  192KB EXTRAM of FM-77, or 77AV40 768KB max. of 77AV40.
	
	FM7_MAINMEM_AV_PAGE0, // $00000-$0ffff
	FM7_MAINMEM_AV_DIRECTACCESS, // $10000-$1ffff
	FM7_MAINMEM_AV_PAGE2,  // $20000-$2ffff
	FM7_MAINMEM_INITROM, // Initiator ROM, $06000 - $07fff
	// 77AV40
	FM7_MAINMEM_BACKUPED_RAM, // DICTIONARY CARD, Backuped
	FM7_MAINMEM_DICTROM,      // DICTIONARY CARD, ROM
	FM7_MAINMEM_77AV40_EXTRAROM,     // DICTIONARY CARD, Extra sub ROM
	FM7_MAINMEM_END
};

#define FM7_BOOTMODE_BASIC 0
#define FM7_BOOTMODE_DOS   1
#define FM7_BOOTMODE_MMR   2
#define FM7_BOOTMODE_ROM4  3
#define FM7_BOOTMODE_RAM   4

#define FM7_SUBMEM_OFFSET_DPALETTE    0x100000
#define FM7_SUBMEM_OFFSET_APALETTE_B  0x200000
#define FM7_SUBMEM_OFFSET_APALETTE_R  0x300000
#define FM7_SUBMEM_OFFSET_APALETTE_G  0x400000
#define FM7_SUBMEM_OFFSET_APALETTE_HI 0x500000
#define FM7_SUBMEM_OFFSET_APALETTE_LO 0x500001

enum {
	EVENT_BEEP_OFF = 0,
	EVENT_FM7SUB_DISPLAY_NMI = 32,
	EVENT_FM7SUB_HDISP,
	EVENT_FM7SUB_HBLANK,
	EVENT_FM7SUB_VSTART,
	EVENT_FM7SUB_VSYNC,
	ID_KEYBOARD_RXRDY_OK = 64,
	ID_KEYBOARD_RXRDY_BUSY,
	ID_KEYBOARD_ACK,
	ID_KEYBOARD_AUTOREPEAT_FIRST = 0x200,
	ID_KEYBOARD_AUTOREPEAT = 0x400
};


enum {
	FM7_SUBMEM_VRAMBANK_0 = 0,	// $0000-$BFFF #1
	FM7_SUBMEM_CONSOLERAM,		// $C000-$CFFF
	FM7_SUBMEM_WORKRAM,		// $D000-$D37F
	FM7_SUBMEM_SHAREDRAM,		// $D380-$D3FF
	FM7_SUBMEM_MMIO,		// $D400-$D7FF
	FM7_SUBMEM_MONITOR_C,		// $D800-#FFFF #1
	FM7_SUBMEM_NULL,
	FM7_SUBMEM_ZERO,
	// 77AV
	FM7_SUBMEM_VRAMBANK_1,	// $0000-$BFFF #2
	// 77AV : $E000-$FFFF
	FM7_SUBMEM_MONITOR_A,
	FM7_SUBMEM_MONITOR_B,
	FM7_SUBMEM_MONITOR_CG,
	FM7_SUBMEM_CGROM,		// 77AV: CGROM #2($D800-$DFFF)
	// 77AV40 
	FM7_SUBMEM_VRAMBANK_2,		// $0000-$BFFF #3
	FM7_SUBMEM_MONITOR_RAM,		// $E000-$DFFF
	FM7_SUBMEM_CGRAM,		// $D800-$DFFF
	// 77(L4)
	FM7_SUBMEM_77_WORKRAM,
	FM7_SUBMEM_77_GVRAM,
	FM7_SUBMEM_77_TEXTRAM,
	FM7_SUBMEM_77_SUBMONITOR,
	FM7_SUBMEM_END
};

// SIGNALS
// MAIN
enum {
	FM7_MAINCLOCK_SLOW = 0,
	FM7_MAINCLOCK_HIGH,
	FM7_MAINCLOCK_MMRSLOW,
	FM7_MAINCLOCK_MMRHIGH
};

enum {
	FM7_MAINIO_IS_BASICROM = 0x10000,
	FM7_MAINIO_BOOTMODE,
	FM7_MAINIO_READ_FD0F,
	FM7_MAINIO_CLOCKMODE,
	FM7_MAINIO_INITROM_ENABLED,
	FM7_MAINIO_EXTBANK,
	FM7_MAINIO_EXTROM,
	FM7_MAINIO_MMR_ENABLED,
	FM7_MAINIO_WINDOW_ENABLED,
	FM7_MAINIO_MMR_SEGMENT,
	FM7_MAINI_MMR_BANK,
};

enum {
	FM7_MAINIO_CMT_RECV = 0x1000, // Input data changed
	FM7_MAINIO_CMT_INVERT, // Set / reset invert cmt data.
	FM7_MAINIO_TIMERIRQ, // Timer from SUB.
	FM7_MAINIO_LPTIRQ,
	FM7_MAINIO_KEYBOARDIRQ, 
	FM7_MAINIO_PUSH_KEYBOARD,
	FM7_MAINIO_PUSH_BREAK, // FIRQ
	FM7_MAINIO_SUB_ATTENTION, // FIRQ
	FM7_MAINIO_SUB_BUSY, // SUB is BUSY.
	FM7_MAINIO_SUB_CANCEL, // SUB is BUSY.
	FM7_MAINIO_EXTDET,
	FM7_MAINIO_BEEP, // BEEP From sub
	FM7_MAINIO_PSG_IRQ,
	FM7_MAINIO_OPN_IRQ,
	FM7_MAINIO_WHG_IRQ,
 	FM7_MAINIO_THG_IRQ,
	FM7_MAINIO_OPNPORTA_CHANGED, // Joystick
	FM7_MAINIO_OPNPORTB_CHANGED, // Joystick
	FM7_MAINIO_FDC_DRQ,
	FM7_MAINIO_FDC_IRQ,
};
// SUB
enum {
	SIG_DISPLAY_VBLANK = 0x4000,
	SIG_DISPLAY_HBLANK,
	SIG_DISPLAY_DIGITAL_PALETTE,
	SIG_DISPLAY_ANALOG_PALETTE,
	SIG_DISPLAY_HALT,
	SIG_DISPLAY_CHANGE_MODE
};
enum {
	DISPLAY_MODE_8_200L = 0,
	DISPLAY_MODE_8_400L,
	DISPLAY_MODE_8_200L_TEXT,
	DISPLAY_MODE_8_400L_TEXT,
	DISPLAY_MODE_4096,
	DISPLAY_MODE_256k
};

enum {
	DISPLAY_ADDR_MULTIPAGE = 0,
	DISPLAY_ADDR_OFFSET_H,
	DISPLAY_ADDR_OFFSET_L,
	DISPLAY_ADDR_DPALETTE,
	DISPLAY_ADDR_APALETTE_B = 0x1000,
	DISPLAY_ADDR_APALETTE_R = 0x2000,
	DISPLAY_ADDR_APALETTE_G = 0x3000
};

enum {
	SIG_FM7_SUB_HALT = 0x2000,
	SIG_FM7_SUB_CANCEL,
	SIG_FM7_SUB_BANK,
	SIG_FM7_SUB_MULTIPAGE,
	SIG_FM7_SUB_KEY_FIRQ,
};

// KEYBOARD
enum {
	SIG_FM7KEY_KEY_UP = 0x800,
	SIG_FM7KEY_KEY_DOWN,
	SIG_FM7KEY_READ, // D400 = high , D401 = low
	SIG_FM7KEY_SET_INSLED, // D40D: Write = OFF / Read = ON
	SIG_FM7KEY_SET_KANALED, //
	SIG_FM7KEY_SET_CAPSLED, //
	// D431
	SIG_FM7KEY_PUSH_TO_ENCODER
};

enum {
	KEYMODE_STANDARD = 0,
	KEYMODE_16BETA,
	KEYMODE_SCAN
};

#endif // _FM7_COMMON_H_
