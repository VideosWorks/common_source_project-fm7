/*
	CASIO PV-1000 Emulator 'ePV-1000'

	Author : Takeda.Toshiya
	Date   : 2006.11.16 -

	[ psg ]
*/

#ifndef _PSG_H_
#define _PSG_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class PSG : public DEVICE
{
private:
	struct {
		int count;
		int period;
		bool signal;
	} ch[3];
	int diff;
	int volume_l, volume_r;
	
public:
	PSG(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
	}
	~PSG() {}
	
	// common functions
	void reset();
	void write_io8(uint32 addr, uint32 data);
	void mix(int32* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void init(int rate);
};

#endif

