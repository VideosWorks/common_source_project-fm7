/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.05.01 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8259.h"
#include "../../fifo.h"

void KEYBOARD::initialize()
{
	key_buf = new FIFO(64);
	register_frame_event(this);
}

void KEYBOARD::release()
{
	key_buf->release();
	delete key_buf;
}

void KEYBOARD::reset()
{
	memset(table, 0, sizeof(table));
	key_buf->clear();
	kbstat = kbdata = kbint = kbmsk = 0;
}

void KEYBOARD::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x600:
		// data
//		kbstat |= 2;
		break;
	case 0x602:
		// command
		break;
	case 0x604:
		kbmsk = data;
		break;
	}
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x600:
		kbint &= ~1;
		d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR1, 0, 0);
		kbstat &= ~1;
		return kbdata;
	case 0x602:
		return kbstat;
	case 0x604:
		return kbint | 0xfc;
	}
	return 0;
}

void KEYBOARD::event_frame()
{
	if(!(kbstat & 1) && !key_buf->empty()) {
		kbstat |= 1;
		kbdata = key_buf->read();
	}
	if((kbstat & 1) && (kbmsk & 1) && !(kbint & 1)) {
		kbint |= 1;
		d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR1, 1, 1);
	}
//	kbstat &= ~2;
}

void KEYBOARD::key_down(int code)
{
//	if(!table[code]) {
		table[code] = 1;
		if(code = key_table[code]) {
			// $11:CTRL, $10:SHIFT
			key_buf->write(0xa0 | (table[0x11] ? 8 : 0) | (table[0x10] ? 4 : 0));
			key_buf->write(code);
		}
//	}
}

void KEYBOARD::key_up(int code)
{
//	if(table[code]) {
		table[code] = 0;
		if(code = key_table[code]) {
			key_buf->write(0xb0);
			key_buf->write(code);
		}
//	}
}

#define STATE_VERSION	1

void KEYBOARD::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	key_buf->save_state((void *)state_fio);
	state_fio->FputUint8(kbstat);
	state_fio->FputUint8(kbdata);
	state_fio->FputUint8(kbint);
	state_fio->FputUint8(kbmsk);
	state_fio->Fwrite(table, sizeof(table), 1);
}

bool KEYBOARD::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	if(!key_buf->load_state((void *)state_fio)) {
		return false;
	}
	kbstat = state_fio->FgetUint8();
	kbdata = state_fio->FgetUint8();
	kbint = state_fio->FgetUint8();
	kbmsk = state_fio->FgetUint8();
	state_fio->Fread(table, sizeof(table), 1);
	return true;
}

