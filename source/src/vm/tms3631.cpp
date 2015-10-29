/*
	Skelton for retropc emulator

	Origin : Neko Project 2
	Author : Takeda.Toshiya
	Date   : 2015.09.29-

	[ TMS3631 ]
*/

#include "tms3631.h"

// from tms3631c.c
static const uint16 np2_freq_table[] = {
	0,	0x051B, 0x0569, 0x05BB, 0x0613, 0x066F, 0x06D1,
		0x0739, 0x07A7, 0x081B, 0x0897, 0x091A, 0x09A4, 0,0,0,
	0,	0x0A37, 0x0AD3, 0x0B77, 0x0C26, 0x0CDF, 0x0DA3,
		0x0E72, 0x0F4E, 0x1037, 0x112E, 0x1234, 0x1349, 0,0,0,
	0,	0x146E, 0x15A6, 0x16EF, 0x184C, 0x19BE, 0x1B46,
		0x1CE5, 0x1E9D, 0x206F, 0x225D, 0x2468, 0x2692, 0,0,0,
	0,	0x28DD, 0x2B4C, 0x2DDF, 0x3099, 0x337D, 0x368D,
		0x39CB, 0x3D3B, 0x40DF, 0x44BA, 0x48D1, 0x4D25, 0x51BB, 0,0
};

void TMS3631::reset()
{
	datareg = maskreg = 0;
	memset(ch, 0, sizeof(ch));
	channel = 0;
	set_key = false;
}

void TMS3631::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_TMS3631_ENVELOP1) {
		envelop1 = (envelop1 & ~mask) | (data & mask);
	} else if(id == SIG_TMS3631_ENVELOP2) {
		envelop2 = (envelop2 & ~mask) | (data & mask);
	} else if(id == SIG_TMS3631_DATAREG) {
		// from board14.c
		data = (datareg & ~mask) | (data & mask);
		if(data & 0x80) {
			if(!(datareg & 0x80)) {
				set_key = true;
				channel = 0;
			} else if(set_key) {
				ch[channel].freq = freq_table[data & 0x3f];
				set_key = false;
			} else if(!(data & 0x40) && (datareg & 0x40)) {
				set_key = true;
				channel = (channel + 1) & 7;
			}
		}
		datareg = data;
	} else if(id == SIG_TMS3631_MASKREG) {
		maskreg = (maskreg & ~mask) | (data & mask);
	}
}

void TMS3631::mix(int32* buffer, int cnt)
{
	// from tms3631g.c
	for(int i = 0; i < cnt; i++) {
		int data = 0;
		for(int j = 0; j < 2; j++) {
			if((maskreg & (1 << j)) && ch[j].freq != 0) {
				for(int k = 0; k < 4; k++) {
					ch[j].count += ch[j].freq;
					data += (ch[j].count & 0x10000) ? 1 : -1;
				}
			}
		}
		int vol_l = data * vol;
		int vol_r = data * vol;
		for(int j = 2; j < 5; j++) {
			if((maskreg & (1 << j)) && ch[j].freq != 0) {
				for(int k = 0; k < 4; k++) {
					ch[j].count += ch[j].freq;
					vol_l += feet[(ch[j].count >> 16) & 15];
				}
			}
		}
		for(int j = 5; j < 8; j++) {
			if((maskreg & (1 << j)) && ch[j].freq != 0) {
				for(int k = 0; k < 4; k++) {
					ch[j].count += ch[j].freq;
					vol_r += feet[(ch[j].count >> 16) & 15];
				}
			}
		}
		*buffer++ += vol_l; // L
		*buffer++ += vol_r; // R
	}
}

void TMS3631::init(int rate, int volume)
{
	// from tms3631c.c
	for(int i = 0; i < 64; i++) {
		freq_table[i] = (uint32)((double)np2_freq_table[i] * 11025.0 / (double)rate / 2.0 + 0.5);
	}
	for(int i = 0; i < 16; i++) {
		int data = 0;
		for(int j = 0; j < 4; j++) {
			data += (volume / 8) * ((i & (1 << j)) ? 1 : -1);
		}
		feet[i] = data;
	}
	vol = volume / 8;
}

#define STATE_VERSION	1

void TMS3631::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(envelop1);
	state_fio->FputUint8(envelop2);
	state_fio->FputUint8(datareg);
	state_fio->FputUint8(maskreg);
	state_fio->Fwrite(ch, sizeof(ch), 1);
	state_fio->FputUint8(channel);
	state_fio->FputBool(set_key);
}

bool TMS3631::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	envelop1 = state_fio->FgetUint8();
	envelop2 = state_fio->FgetUint8();
	datareg = state_fio->FgetUint8();
	maskreg = state_fio->FgetUint8();
	state_fio->Fread(ch, sizeof(ch), 1);
	channel = state_fio->FgetUint8();
	set_key = state_fio->FgetBool();
	return true;
}
