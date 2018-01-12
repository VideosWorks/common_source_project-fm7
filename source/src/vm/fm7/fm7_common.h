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
	FM7_MAINMEM_BOOTROM_EXTRA,  // $FE00-$FFEF #4
	FM7_MAINMEM_BOOTROM_RAM,  // $FE00-$FFEF #5
	FM7_MAINMEM_VECTOR, // $FFF0-$FFFD
	FM7_MAINMEM_RESET_VECTOR, // $FFF0-$FFFD
	FM7_MAINMEM_NULL, // NULL
	FM7_MAINMEM_ZERO, // ZERO
	FM7_MAINMEM_KANJI_LEVEL1,
	FM7_MAINMEM_SHADOWRAM, // FM77
	
	FM7_MAINMEM_EXTRAM, //  192KB EXTRAM of FM-77, or 77AV40 768KB max. of 77AV40.
	
	FM7_MAINMEM_AV_PAGE0, // $00000-$0ffff
	FM7_MAINMEM_AV_PAGE2, // $20000-$2ffff
	FM7_MAINMEM_AV_DIRECTACCESS, // $10000-$1ffff
	FM7_MAINMEM_INITROM, // Initiator ROM, $06000 - $07fff
	// 77AV40
	FM7_MAINMEM_BACKUPED_RAM, // DICTIONARY CARD, Backuped
	FM7_MAINMEM_DICTROM,      // DICTIONARY CARD, ROM
	FM7_MAINMEM_77AV40_EXTRAROM,     // DICTIONARY CARD, Extra sub ROM
	FM7_MAINMEM_KANJI_LEVEL2,
	FM7_MAINMEM_KANJI_DUMMYADDR,
	FM7_MAINMEM_END
};

#define FM7_BOOTMODE_BASIC 0
#define FM7_BOOTMODE_DOS   1
#define FM7_BOOTMODE_MMR   2
#define FM7_BOOTMODE_ROM4  3
#define FM7_BOOTMODE_RAM   4

#define FM7_DIPSW_CYCLESTEAL         0x00000001
#define FM7_DIPSW_EXTRAM             0x00000002
#define FM7_DIPSW_EXTRAM_AV          0x00000004
#define FM7_DIPSW_DICTROM_AV         0x00000008
#define FM7_DIPSW_FM8_PROTECT_FD0F   0x00000010
#define FM7_DIPSW_CONNECT_KANJIROM   0x00000020
#define FM7_DIPSW_CONNECT_320KFDC    0x00000040
#define FM7_DIPSW_CONNECT_1MFDC      0x00000080
// '1' is Auto 5 Key, '0' is Auto 8 Key
#define FM7_DIPSW_SELECT_5_OR_8KEY   0x00000100 
#define FM7_DIPSW_AUTO_5_OR_8KEY     0x00000200

#define FM7_DIPSW_Z80_IRQ_ON         0x00001000
#define FM7_DIPSW_Z80_FIRQ_ON        0x00002000
#define FM7_DIPSW_Z80_NMI_ON         0x00004000
#define FM7_DIPSW_JIS78EMU_ON        0x00008000
#define FM7_DIPSW_JSUBCARD_ON        0x00010000
#define FM7_DIPSW_Z80CARD_ON         0x00020000
#define FM7_DIPSW_RS232C_ON          0x00040000
#define FM7_DIPSW_MODEM_ON           0x00080000
#define FM7_DIPSW_MIDI_ON            0x00100000

#define FM7_DIPSW_FRAMESKIP          0x30000000
#define FM7_DIPSW_SYNC_TO_HSYNC      0x80000000


#define MAINCLOCK_NORMAL    1798000
#define MAINCLOCK_MMR       1565000
#define MAINCLOCK_FAST_MMR  2016000
#define MAINCLOCK_SLOW      1095000

#define SUBCLOCK_NORMAL     2000000
#define SUBCLOCK_SLOW        999000

#define JCOMMCARD_CLOCK     1228000

#define FM7_SUBMEM_OFFSET_DPALETTE    0x0100000
#define FM7_SUBMEM_OFFSET_APALETTE_B  0x0200000
#define FM7_SUBMEM_OFFSET_APALETTE_R  0x0300000
#define FM7_SUBMEM_OFFSET_APALETTE_G  0x0400000
#define FM7_SUBMEM_OFFSET_APALETTE_HI 0x0500000
#define FM7_SUBMEM_OFFSET_APALETTE_LO 0x0500001

enum {
	EVENT_BEEP_OFF = 0,
	EVENT_BEEP_CYCLE,
	EVENT_UP_BREAK,
	EVENT_TIMERIRQ_ON,
	EVENT_TIMERIRQ_OFF,
	EVENT_FD_MOTOR_ON,
	EVENT_FD_MOTOR_OFF,
	EVENT_FM7MAINIO_ATTENTION,
	EVENT_FM7MAINIO_SUBBUSY_SET,
	EVENT_FM7MAINIO_SUBBUSY_RESET,
	EVENT_MOUSE_TIMEOUT,
	EVENT_FM7SUB_DISPLAY_NMI = 32,
	EVENT_FM7SUB_DISPLAY_NMI_OFF,
	EVENT_FM7SUB_HDISP,
	EVENT_FM7SUB_HBLANK,
	EVENT_FM7SUB_VSTART,
	EVENT_FM7SUB_VSYNC,
	EVENT_FM7SUB_HALT,
	EVENT_FM7SUB_CLR_BUSY,
	EVENT_FM7SUB_CLR_CRTFLAG,
	EVENT_FM7SUB_PROC,
	EVENT_PRINTER_RESET_COMPLETED,
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
	FM7_SUBMEM_AV_HIDDEN_RAM, // $D500-$D7FF
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
	FM7_JCOMMCARD_BUS_BA = 0x400000,
	FM7_JCOMMCARD_BUS_BS,
};

enum {
	FM7_MAINIO_IS_BASICROM = 0x200000,
	FM7_MAINIO_BOOTMODE,
	FM7_MAINIO_BOOTRAM_RW,
	FM7_MAINIO_READ_FD0F,
	FM7_MAINIO_PUSH_FD0F,
	FM7_MAINIO_CLOCKMODE,
	FM7_MAINIO_INITROM_ENABLED,
	FM7_MAINIO_EXTBANK,
	FM7_MAINIO_EXTROM,
	FM7_MAINIO_MODE320,
	FM7_MAINIO_SUBMONITOR_ROM,
	FM7_MAINIO_SUBMONITOR_RAM,
	FM7_MAINIO_MMR_ENABLED,
	FM7_MAINIO_FASTMMR_ENABLED,
	FM7_MAINIO_WINDOW_ENABLED,
	FM7_MAINIO_WINDOW_OFFSET,
	FM7_MAINIO_MMR_SEGMENT,
	FM7_MAINIO_MMR_EXTENDED,
	FM7_MAINIO_WINDOW_FAST,
	FM7_MAINMEM_REFRESH_FAST,
	FM7_MAINIO_UART0_SYNDET,
	FM7_MAINIO_UART0_RXRDY,
	FM7_MAINIO_UART0_TXRDY,
	FM7_MAINIO_UART0_DCD,
	FM7_MAINIO_MODEM_SYNDET,
	FM7_MAINIO_MODEM_RXRDY,
	FM7_MAINIO_MODEM_TXRDY,
	FM7_MAINIO_MIDI_SYNDET,
	FM7_MAINIO_MIDI_RXRDY,
	FM7_MAINIO_MIDI_TXRDY,
	
	FM7_MAINIO_MMR_BANK = 0x200100,

};

enum {
	FM7_MAINIO_CMT_RECV = 0x1000, // Input data changed
	FM7_MAINIO_CMT_INVERT, // Set / reset invert cmt data.
	FM7_MAINIO_TIMERIRQ, // Timer from SUB.
	FM7_MAINIO_LPTIRQ,
	FM7_MAINIO_LPT_BUSY,
	FM7_MAINIO_LPT_ERROR,
	FM7_MAINIO_LPT_ACK,
	FM7_MAINIO_LPT_PAPER_EMPTY,
	FM7_MAINIO_LPT_DET1,
	FM7_MAINIO_LPT_DET2,
	FM7_MAINIO_KEYBOARDIRQ, 
	FM7_MAINIO_KEYBOARDIRQ_MASK,
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
	FM7_MAINIO_JOYPORTA_CHANGED, // Joystick
	FM7_MAINIO_JOYPORTB_CHANGED, // Joystick
	FM7_MAINIO_FDC_DRQ,
	FM7_MAINIO_FDC_IRQ,
	FM7_MAINIO_KANJI1_ADDR_HIGH,
	FM7_MAINIO_KANJI1_ADDR_LOW,
	FM7_MAINIO_KANJI2_ADDR_HIGH,
	FM7_MAINIO_KANJI2_ADDR_LOW,
	FM7_MAINIO_HOT_RESET,
	FM7_MAINIO_DMA_INT,
	FM7_MAINIO_RUN_Z80,
	FM7_MAINIO_RUN_6809,
	
	FM7_JOYSTICK_EMULATE_MOUSE_0,
	FM7_JOYSTICK_EMULATE_MOUSE_1,
	FM7_JOYSTICK_MOUSE_STROBE,

	
};
// SUB
enum {
	SIG_DISPLAY_VBLANK = 0x4000,
	SIG_DISPLAY_HBLANK,
	SIG_DISPLAY_VSYNC,
	SIG_DISPLAY_DISPLAY,
	SIG_DISPLAY_CLOCK,
	SIG_DISPLAY_DIGITAL_PALETTE,
	SIG_DISPLAY_ANALOG_PALETTE,
	SIG_DISPLAY_HALT,
	SIG_DISPLAY_BUSY,
	SIG_DISPLAY_CHANGE_MODE,
	SIG_DISPLAY_MULTIPAGE,
	SIG_DISPLAY_MODE_IS_400LINE,
	SIG_DISPLAY_PLANES,
	SIG_DISPLAY_X_WIDTH,
	SIG_DISPLAY_Y_HEIGHT,
	SIG_DISPLAY_EXTRA_MODE,
	SIG_DISPLAY_STATREG_FD12,
	SIG_DISPLAY_MODE320,
	SIG_DISPLAY_BANK_OFFSET,
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
	DISPLAY_ADDR_APALETTE_B = 0x10000000,
	DISPLAY_ADDR_APALETTE_R = 0x20000000,
	DISPLAY_ADDR_APALETTE_G = 0x30000000,
	DISPLAY_VRAM_DIRECT_ACCESS = 0x80000000
};

enum {
	SIG_FM7_SUB_HALT = 0x2000,
	SIG_FM7_SUB_CANCEL,
	SIG_FM7_SUB_BANK,
	SIG_FM7_SUB_KEY_FIRQ,
	SIG_FM7_SUB_KEY_MASK,
	SIG_FM7_SUB_USE_CLR
};

// KEYBOARD
enum {
	SIG_FM7KEY_KEY_UP = 0x800,
	SIG_FM7KEY_KEY_DOWN,
	SIG_FM7KEY_READ, // D400 = high , D401 = low
	SIG_FM7KEY_SET_INSLED, // D40D: Write = OFF / Read = ON
	SIG_FM7KEY_SET_KANALED, //
	SIG_FM7KEY_SET_CAPSLED, //
	SIG_FM7KEY_RXRDY,
	SIG_FM7KEY_ACK, 	// D431
	SIG_FM7KEY_BREAK_KEY,
	SIG_FM7KEY_PUSH_TO_ENCODER,
	SIG_FM7KEY_LED_STATUS,
};

enum {
	KEYMODE_STANDARD = 0,
	KEYMODE_16BETA,
	KEYMODE_SCAN
};
// KANJIROM
enum {
	KANJIROM_ADDR_HI = 0x10000,
	KANJIROM_ADDR_LO,
	KANJIROM_DATA_HI,
	KANJIROM_DATA_LO,
	KANJIROM_READSTAT,
	KANJIROM_DIRECTADDR = 0x20000,
};

#endif // _FM7_COMMON_H_
