/*
	SHARP MZ-80K/C Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'

	Author : Takeda.Toshiya
	Date   : 2010.08.18-

	SHARP MZ-80A Emulator 'EmuZ-80A'
	Modify : Hideki Suga
	Date   : 2014.12.10 -

	[ display ]
*/

#include "display.h"

void DISPLAY::initialize()
{
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	delete fio;
	
	// create pc palette
	palette_pc[0] = RGB_COLOR(0, 0, 0);
#if defined(_MZ1200) || defined(_MZ80A)
	palette_pc[1] = RGB_COLOR(0, 255, 0);
#else
	if(config.monitor_type) {
		palette_pc[1] = RGB_COLOR(0, 255, 0);
	} else {
		palette_pc[1] = RGB_COLOR(255, 255, 255);
	}
#endif
	
	// register event
	register_vline_event(this);
}

void DISPLAY::reset()
{
	vgate = true;
#if defined(_MZ1200) || defined(_MZ80A)
	reverse = false;
#endif
}

void DISPLAY::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_DISPLAY_VGATE) {
		vgate = ((data & mask) != 0);
#if defined(_MZ1200) || defined(_MZ80A)
	} else if(id == SIG_DISPLAY_REVERSE) {
		reverse = ((data & mask) != 0);
#endif
	}
}

void DISPLAY::event_vline(int v, int clock)
{
	if(0 <= v && v < 200) {
		// draw one line
#if defined(_MZ80A)
		int ptr = (v >> 3) * 40 + ((int)*e200_ptr << 3);	// scroll
#else
		int ptr = (v >> 3) * 40;
#endif
		for(int x = 0; x < 320; x += 8) {
			uint8 code = vram_ptr[(ptr++) & 0x7ff];
			uint8 pat = font[(code << 3) | (v & 7)];
#if defined(_MZ1200) || defined(_MZ80A)
			if(reverse) {
				pat = ~pat;
			}
#endif
			uint8* dest = &screen[v][x];
			
			dest[0] = (pat & 0x80) >> 7;
			dest[1] = (pat & 0x40) >> 6;
			dest[2] = (pat & 0x20) >> 5;
			dest[3] = (pat & 0x10) >> 4;
			dest[4] = (pat & 0x08) >> 3;
			dest[5] = (pat & 0x04) >> 2;
			dest[6] = (pat & 0x02) >> 1;
			dest[7] = (pat & 0x01) >> 0;
		}
	}
}

#if defined(_MZ80K)
void DISPLAY::update_config()
{
	if(config.monitor_type) {
		palette_pc[1] = RGB_COLOR(0, 255, 0);
	} else {
		palette_pc[1] = RGB_COLOR(255, 255, 255);
	}
}
#endif

void DISPLAY::draw_screen()
{
	// copy to real screen
	if(true || vgate) {
		for(int y = 0; y < 200; y++) {
			scrntype* dest = emu->screen_buffer(y);
			uint8* src = screen[y];
			
			for(int x = 0; x < 320; x++) {
				dest[x] = palette_pc[src[x] & 1];
			}
		}
	} else {
		for(int y = 0; y < 200; y++) {
			scrntype* dest = emu->screen_buffer(y);
			memset(dest, 0, sizeof(scrntype) * 320);
		}
	}
}

#define STATE_VERSION	1

void DISPLAY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputBool(vgate);
#if defined(_MZ1200) || defined(_MZ80A)
	state_fio->FputBool(reverse);
#endif
}

bool DISPLAY::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	vgate = state_fio->FgetBool();
#if defined(_MZ1200) || defined(_MZ80A)
	reverse = state_fio->FgetBool();
#endif
	return true;
}

