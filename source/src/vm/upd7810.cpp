/*
	Skelton for retropc emulator

	Origin : MESS 0.152
	Author : Takeda.Toshiya
	Date   : 2016.03.17-

	[ uPD7810 ]
*/

#include "upd7810.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

#if defined(HAS_UPD7810)
	#define CPU_MODEL upd7810
#elif defined(HAS_UPD7807)
	#define CPU_MODEL upd7807
#elif defined(HAS_UPD7801)
	#define CPU_MODEL upd7801
#elif defined(HAS_UPD78C05)
	#define CPU_MODEL upd78c05
#elif defined(HAS_UPD78C06)
	#define CPU_MODEL upd78c06
#elif defined(HAS_UPD7907)
	#define CPU_MODEL upd7907
#endif

/* ----------------------------------------------------------------------------
	MAME uPD7810
---------------------------------------------------------------------------- */

#define PAIR pair_t
#define offs_t UINT16

/*****************************************************************************/
/* src/emu/devcpu.h */

// CPU interface functions
#define CPU_INIT_NAME(name)			cpu_init_##name
#define CPU_INIT(name)				void* CPU_INIT_NAME(name)()
#define CPU_INIT_CALL(name)			CPU_INIT_NAME(name)()

#define CPU_RESET_NAME(name)			cpu_reset_##name
#define CPU_RESET(name)				void CPU_RESET_NAME(name)(upd7810_state *cpustate)
#define CPU_RESET_CALL(name)			CPU_RESET_NAME(name)(cpustate)

#define CPU_EXECUTE_NAME(name)			cpu_execute_##name
#define CPU_EXECUTE(name)			int CPU_EXECUTE_NAME(name)(upd7810_state *cpustate)
#define CPU_EXECUTE_CALL(name)			CPU_EXECUTE_NAME(name)(cpustate)

#define CPU_DISASSEMBLE_NAME(name)		cpu_disassemble_##name
#define CPU_DISASSEMBLE(name)			int CPU_DISASSEMBLE_NAME(name)(_TCHAR *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
#define CPU_DISASSEMBLE_CALL(name)		CPU_DISASSEMBLE_NAME(name)(buffer, pc, oprom, oprom)

/*****************************************************************************/
/* src/emu/didisasm.h */

// Disassembler constants
const UINT32 DASMFLAG_SUPPORTED     = 0x80000000;   // are disassembly flags supported?
const UINT32 DASMFLAG_STEP_OUT      = 0x40000000;   // this instruction should be the end of a step out sequence
const UINT32 DASMFLAG_STEP_OVER     = 0x20000000;   // this instruction should be stepped over by setting a breakpoint afterwards
const UINT32 DASMFLAG_OVERINSTMASK  = 0x18000000;   // number of extra instructions to skip when stepping over
const UINT32 DASMFLAG_OVERINSTSHIFT = 27;           // bits to shift after masking to get the value
const UINT32 DASMFLAG_LENGTHMASK    = 0x0000ffff;   // the low 16-bits contain the actual length

/*****************************************************************************/
/* src/emu/diexec.h */

// I/O line states
enum line_state
{
	CLEAR_LINE = 0,				// clear (a fired or held) line
	ASSERT_LINE				// assert an interrupt immediately
};

enum
{
	UPD7810_INTF1  = 0,
	UPD7810_INTF2  = 1,
	UPD7810_INTF0  = 2,
	UPD7810_INTFE1 = 4,
	INPUT_LINE_NMI
};

#define logerror(...)
#define fatalerror(...)

#include "mame/emu/cpu/upd7810/upd7810.c"
#ifdef USE_DEBUGGER
#include "mame/emu/cpu/upd7810/7810dasm.c"
#endif

// main

void UPD7810::initialize()
{
	opaque = CPU_INIT_CALL(upd7810);
	
	upd7810_state *cpustate = (upd7810_state *)opaque;
#if defined(HAS_UPD7810)
	cpustate->config.type = TYPE_7810;
#elif defined(HAS_UPD7807)
	cpustate->config.type = TYPE_7807;
#elif defined(HAS_UPD7801)
	cpustate->config.type = TYPE_7801;
#elif defined(HAS_UPD78C05)
	cpustate->config.type = TYPE_78C05;
#elif defined(HAS_UPD78C06)
	cpustate->config.type = TYPE_78C06;
#elif defined(HAS_UPD7907)
	cpustate->config.type = TYPE_78C06;
#endif
	cpustate->config.io_callback = this;
	cpustate->program = d_mem;
	cpustate->io = d_io;
#ifdef USE_DEBUGGER
	cpustate->emu = emu;
	cpustate->debugger = d_debugger;
	cpustate->program_stored = d_mem;
	cpustate->io_stored = d_io;
	
	d_debugger->set_context_mem(d_mem);
	d_debugger->set_context_io(d_io);
#endif
}

void UPD7810::release()
{
	free(opaque);
}

void UPD7810::reset()
{
	upd7810_state *cpustate = (upd7810_state *)opaque;
	
	CPU_RESET_CALL(CPU_MODEL);
	
	cpustate->program = d_mem;
	cpustate->io = d_io;
#ifdef USE_DEBUGGER
	cpustate->emu = emu;
	cpustate->debugger = d_debugger;
	cpustate->program_stored = d_mem;
	cpustate->io_stored = d_io;
#endif
	icount = 0;
	busreq = false;
}

int UPD7810::run(int clock)
{
	upd7810_state *cpustate = (upd7810_state *)opaque;
	
	if(clock == -1) {
		if(busreq) {
			// don't run cpu!
			return 1;
		} else {
			// run only one opcode
			return CPU_EXECUTE_CALL(upd7810);
		}
	} else {
		icount += clock;
		int first_icount = icount;
		
		// run cpu while given clocks
		while(icount > 0 && !busreq) {
			icount -= CPU_EXECUTE_CALL(upd7810);
		}
		// if busreq is raised, spin cpu while remained clock
		if(icount > 0 && busreq) {
			icount = 0;
		}
		return first_icount - icount;
	}
}

void UPD7810::write_signal(int id, uint32_t data, uint32_t mask)
{
	upd7810_state *cpustate = (upd7810_state *)opaque;
	
	switch(id) {
	case SIG_UPD7810_INTF1:
		set_irq_line(cpustate, UPD7810_INTF1, (data & mask) ? ASSERT_LINE : CLEAR_LINE);
		break;
	case SIG_UPD7810_INTF2:
		set_irq_line(cpustate, UPD7810_INTF2, (data & mask) ? ASSERT_LINE : CLEAR_LINE);
		break;
	case SIG_UPD7810_INTF0:
		set_irq_line(cpustate, UPD7810_INTF0, (data & mask) ? ASSERT_LINE : CLEAR_LINE);
		break;
	case SIG_UPD7810_INTFE1:
		set_irq_line(cpustate, UPD7810_INTFE1, (data & mask) ? ASSERT_LINE : CLEAR_LINE);
		break;
	case SIG_UPD7810_NMI:
		set_irq_line(cpustate, INPUT_LINE_NMI, (data & mask) ? ASSERT_LINE : CLEAR_LINE);
		break;
	case SIG_UPD7810_RXD:
		rxd_status = ((data & mask) != 0);
		break;
	case SIG_CPU_BUSREQ:
		busreq = ((data & mask) != 0);
		break;
	// from upd7810.c
	case UPD7810_TXD:
		write_signals(&outputs_txd, (data & mask) ? 0xffffffff : 0);
		break;
	case UPD7810_RXD:
		write_signals(&outputs_rxd, (data & mask) ? 0xffffffff : 0);
		break;
	case UPD7810_TO:
		write_signals(&outputs_to, (data & mask) ? 0xffffffff : 0);
		break;
	}
}

uint32_t UPD7810::read_signal(int id)
{
	switch(id) {
	// from upd7801.c
	case UPD7810_RXD:
		return rxd_status ? 1 : 0;
	}
	return 0;
}

uint32_t UPD7810::get_pc()
{
	upd7810_state *cpustate = (upd7810_state *)opaque;
	return cpustate->ppc.w.l;
}

uint32_t UPD7810::get_next_pc()
{
	upd7810_state *cpustate = (upd7810_state *)opaque;
	return cpustate->pc.w.l;
}

#ifdef USE_DEBUGGER
void UPD7810::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem->write_data8w(addr, data, &wait);
}

uint32_t UPD7810::read_debug_data8(uint32_t addr)
{
	int wait;
	return d_mem->read_data8w(addr, &wait);
}

void UPD7810::write_debug_io8(uint32_t addr, uint32_t data)
{
	int wait;
	d_io->write_io8w(addr, data, &wait);
}

uint32_t UPD7810::read_debug_io8(uint32_t addr) {
	int wait;
	return d_io->read_io8w(addr, &wait);
}

bool UPD7810::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	upd7810_state *cpustate = (upd7810_state *)opaque;
	return false;
}

void UPD7810::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	upd7810_state *cpustate = (upd7810_state *)opaque;
}

// disassembler

int UPD7810::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	uint8_t oprom[8];
	uint8_t *opram = oprom;
	
	for(int i = 0; i < 8; i++) {
		int wait;
		oprom[i] = d_mem->read_data8w(pc + i, &wait);
	}
	return CPU_DISASSEMBLE_CALL(CPU_MODEL) & DASMFLAG_LENGTHMASK;
}
#endif

#define STATE_VERSION	3

void UPD7810::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(opaque, sizeof(upd7810_state), 1);
	state_fio->FputInt32(icount);
	state_fio->FputBool(busreq);
}

bool UPD7810::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(opaque, sizeof(upd7810_state), 1);
	icount = state_fio->FgetInt32();
	busreq = state_fio->FgetBool();
	
	// post process
	upd7810_state *cpustate = (upd7810_state *)opaque;
	cpustate->program = d_mem;
	cpustate->io = d_io;
#ifdef USE_DEBUGGER
	cpustate->emu = emu;
	cpustate->debugger = d_debugger;
	cpustate->program_stored = d_mem;
	cpustate->io_stored = d_io;
#endif
	return true;
}