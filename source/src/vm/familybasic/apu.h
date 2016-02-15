/*
	Nintendo Family BASIC Emulator 'eFamilyBASIC'

	Origin : nester
	Author : Takeda.Toshiya
	Date   : 2010.08.11-

	[ APU ]
*/

#ifndef _APU_H_
#define _APU_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define APUQUEUE_SIZE	4096
#define APUQUEUE_MASK	(APUQUEUE_SIZE - 1)

class APU : public DEVICE
{
private:
	DEVICE *d_cpu, *d_mem;
	
	// rectangle
	typedef struct {
		uint8 regs[4];
		bool enabled;
		int32 phaseacc;
		int32 freq;
		int32 output_vol;
		bool fixed_envelope;
		bool holdnote;
		uint8 volume;
		int32 sweep_phase;
		int32 sweep_delay;
		bool sweep_on;
		uint8 sweep_shifts;
		uint8 sweep_length;
		bool sweep_inc;
		int32 freq_limit;
		bool sweep_complement;
		int32 env_phase;
		int32 env_delay;
		uint8 env_vol;
		int vbl_length;
		uint8 adder;
		int duty_flip;
		bool enabled_cur;
		bool holdnote_cur;
		int vbl_length_cur;
	} rectangle_t;
	
	// triangle
	typedef struct {
		uint8 regs[3];
		bool enabled;
		int32 freq;
		int32 phaseacc;
		int32 output_vol;
		uint8 adder;
		bool holdnote;
		bool counter_started;
		int write_latency;
		int vbl_length;
		int linear_length;
		bool enabled_cur;
		bool holdnote_cur;
		bool counter_started_cur;
		int vbl_length_cur;
	} triangle_t;
	
	// noise
	typedef struct {
		uint8 regs[3];
		bool enabled;
		int32 freq;
		int32 phaseacc;
		int32 output_vol;
		int32 env_phase;
		int32 env_delay;
		uint8 env_vol;
		bool fixed_envelope;
		bool holdnote;
		uint8 volume;
		int vbl_length;
		uint8 xor_tap;
		bool enabled_cur;
		bool holdnote_cur;
		int vbl_length_cur;
		
		int shift_reg;
		int noise_bit;
	} noise_t;
	
	// dmc
	typedef struct {
		uint8 regs[4];
		bool enabled;
		int32 freq;
		int32 phaseacc;
		int32 output_vol;
		uint32 address;
		uint32 cached_addr;
		int dma_length;
		int cached_dmalength;
		uint8 cur_byte;
		bool looping;
		bool irq_gen;
		bool irq_occurred;
		int32 freq_cur;
		int32 phaseacc_cur;
		int dma_length_cur;
		int cached_dmalength_cur;
		bool enabled_cur;
		bool looping_cur;
		bool irq_gen_cur;
		bool irq_occurred_cur;
	} dmc_t;
	
	// queue
	typedef struct {
		uint32 timestamp, addr;
		uint32 data;
	} queue_t;
	
	rectangle_t rectangle[2];
	triangle_t triangle;
	noise_t noise;
	dmc_t dmc;
	
	int32 cycle_rate;
	int32 decay_lut[16];
	int vbl_lut[32];
	int trilength_lut[128];
	
	uint32 enable_reg;
	uint32 enable_reg_cur;
	int count_rate;
	
	queue_t queue[APUQUEUE_SIZE];
	int q_head, q_tail;
	uint32 elapsed_cycles;
	double ave, max, min;
	
	int32 create_rectangle(rectangle_t *chan);
	int32 create_triangle(triangle_t *chan);
	int32 create_noise(noise_t *chan);
	inline void dmc_reload(dmc_t *chan);
	int32 create_dmc(dmc_t *chan);
	void enqueue(queue_t *d);
	queue_t* dequeue();
	void write_data_sync(uint32 addr, uint32 data);
	void write_data_cur(uint32 addr, uint32 data);
	
	int volume_l, volume_r;
	
public:
	APU(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
	}
	~APU() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void event_frame();
	void event_vline(int v, int clock);
	void mix(int32* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_memory(DEVICE* device)
	{
		d_mem = device;
	}
	void initialize_sound(int rate, int samples);
};

#endif

