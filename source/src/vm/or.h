/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.06.10-

	[ or gate ]
*/

#ifndef _OR_H_
#define _OR_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

#define SIG_OR_BIT_0	0x01
#define SIG_OR_BIT_1	0x02
#define SIG_OR_BIT_2	0x04
#define SIG_OR_BIT_3	0x08
#define SIG_OR_BIT_4	0x10
#define SIG_OR_BIT_5	0x20
#define SIG_OR_BIT_6	0x40
#define SIG_OR_BIT_7	0x80

class VM;
class EMU;
class OR : public DEVICE
{
private:
	outputs_t outputs;
	uint32_t bits_in;
	bool prev, first;
	
public:
	OR(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs);
		bits_in = 0;
		prev = first = true;
		set_device_name(_T("OR GATE"));
	}
	~OR() {}
	
	// common functions
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_out(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs, device, id, mask);
	}
};

#endif

