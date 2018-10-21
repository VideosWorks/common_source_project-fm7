/*
	SANYO PHC-25 Emulator 'ePHC-25'
	SEIKO MAP-1010 Emulator 'eMAP-1010'

	Author : Takeda.Toshiya
	Date   : 2010.08.03-

	[ system port ]
*/

#include "./system.h"
#include "../datarec.h"
#include "../mc6847.h"

void PHC25_SYSTEM::initialize()
{
	sysport = 0;
}

void PHC25_SYSTEM::reset()
{
	d_vdp->write_signal(SIG_MC6847_INTEXT, 1, 1);
}

void PHC25_SYSTEM::write_io8(uint32_t addr, uint32_t data)
{
	d_drec->write_signal(SIG_DATAREC_MIC, data, 0x01);
	d_drec->write_signal(SIG_DATAREC_REMOTE, ~data, 0x02);
	// bit2 : kana lock led ???
	// bit3 : printer strobe
	d_vdp->write_signal(SIG_MC6847_GM, (data & 0x20) ? 7 : 6, 7);
	d_vdp->write_signal(SIG_MC6847_CSS, data, 0x40);
	d_vdp->write_signal(SIG_MC6847_AG, data, 0x80);
}

uint32_t PHC25_SYSTEM::read_io8(uint32_t addr)
{
	return sysport;
}

void PHC25_SYSTEM::write_signal(int id, uint32_t data, uint32_t mask)
{
	sysport = (sysport & ~mask) | (data & mask);
}

#define STATE_VERSION	1

#include "../../statesub.h"

void PHC25_SYSTEM::decl_state()
{
	enter_decl_state(STATE_VERSION);

	DECL_STATE_ENTRY_UINT8(sysport);
	
	leave_decl_state();
}

void PHC25_SYSTEM::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->FputUint8(sysport);
}

bool PHC25_SYSTEM::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) {
		return false;
	}
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	sysport = state_fio->FgetUint8();
	return true;
}

bool PHC25_SYSTEM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateUint8(sysport);
	return true;
}
