/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2011.12.18-

	[ PC-80S31K ]
*/

#ifndef _PC80S31K_H_
#define _PC80S31K_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

class UPD765A;

class PC80S31K : public DEVICE
{
private:
	UPD765A *d_fdc;
	DEVICE *d_cpu, *d_pio;
	
	uint8_t rom[0x2000];	// PC-8801M*
	uint8_t ram[0x4000];
	
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t* wbank[8];
	uint8_t* rbank[8];
	
public:
	PC80S31K(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		set_device_name(_T("PC80S31K"));
	}
	~PC80S31K() {}
	
	// common functions
	void initialize();
	void reset();
	uint32_t read_data8(uint32_t addr);
	uint32_t fetch_op(uint32_t addr, int *wait);
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t get_intr_ack();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	const _TCHAR *get_device_name()
	{
		return _T("PC-80S31K");
	}
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_fdc(UPD765A* device)
	{
		d_fdc = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
};

#endif

