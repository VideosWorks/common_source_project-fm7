/*
 * FM77AV/FM16β ALU [77av_alu.cpp]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Mar 28, 2015 : Initial
 *
 */

#include "77av_alu.h"


FMALU::FMALU(VM *parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	p_emu = parent_emu;
	p_vm = parent_vm;
}

FMALU::~FMALU()
{
}

uint8 FMALU::do_read(uint32 addr, uint32 bank)
{
	uint32 raddr;
	uint32 offset;
	
	if(((1 << bank) & read_signal(SIG_DISPLAY_MULTIPAGE)) != 0) return 0xff;
	//if(is_400line) offset = 0x8000;
	
	if(is_400line) {
		if((addr & 0xffff) < 0x8000) {
			raddr = addr + 0x8000 * bank;
			return target->read_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS);
		}
		return 0xff;
	} else {
		raddr = addr + 0x4000 * bank;
		return target->read_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS);
	}
	return 0xff;
}

uint8 FMALU::do_write(uint32 addr, uint32 bank, uint8 data)
{
	uint32 raddr;
	uint8 readdata;

	if(((1 << bank) & read_signal(SIG_DISPLAY_MULTIPAGE)) != 0) return 0xff;
	if((command_reg & 0x40) != 0) { // Calculate before writing
		readdata = do_read(addr, bank);
		if((command_reg & 0x20) != 0) { // NAND
			readdata = readdata & cmp_status_reg;
			readdata = readdata | (data & ~cmp_status_reg);
		} else { // AND
			readdata = readdata & ~cmp_status_reg;
			readdata = readdata | (data & cmp_status_reg);
		}
	} else {
		readdata = data;
	}
	if(is_400line) {
		if((addr & 0xffff) < 0x8000) {
			raddr = addr + 0x8000 * bank;
			target->write_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS, readdata);
		}
	} else {
		raddr = addr + 0x4000 * bank;
		target->write_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS, readdata);
	}
	return readdata;
}


uint8 FMALU::do_pset(uint32 addr)
{
	uint32 i;
	uint32 raddr = addr;  // Use banked ram.
	uint8 bitmask;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		if((color_reg & (1 << i)) == 0) {
			bitmask = 0x00;
		} else {
			bitmask = ~mask_reg;
		}
		srcdata = do_read(addr, i);
		srcdata = srcdata & mask_reg;
		srcdata = srcdata | bitmask;
		do_write(addr, i, srcdata);
	}
	return 0xff;
}

uint8 FMALU::do_blank(uint32 addr)
{
	uint32 i;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i);
		srcdata = srcdata & mask_reg;
		do_write(addr, i, srcdata);
	}
	return 0xff;
}

uint8 FMALU::do_or(uint32 addr)
{
	uint32 i;
	uint8 bitmask;
	uint8 srcdata;
	
	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i);
		if((color_reg & (1 << i)) == 0) {
			bitmask = srcdata; // srcdata | 0x00
		} else {
			bitmask = 0xff; // srcdata | 0xff
		}
		bitmask = bitmask & ~mask_reg;
		srcdata = (srcdata & mask_reg) | bitmask;
		do_write(addr, i, srcdata);
	}
	return 0xff;
}

uint8 FMALU::do_and(uint32 addr)
{
	uint32 i;
	uint8 bitmask;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i);
		if((color_reg & (1 << i)) == 0) {
			bitmask = 0x00; // srcdata & 0x00
		} else {
			bitmask = srcdata; // srcdata & 0xff;
		}
		bitmask = bitmask & ~mask_reg;
		srcdata = (srcdata & mask_reg) | bitmask;
		do_write(addr, i, srcdata);
	}
	return 0xff;
}

uint8 FMALU::do_xor(uint32 addr)
{
	uint32 i;
	uint8 bitmask;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i);
		if((color_reg & (1 << i)) == 0) {
			bitmask = srcdata ^ 0x00;
		} else {
			bitmask = srcdata ^ 0xff;
		}
		bitmask = bitmask & ~mask_reg;
		srcdata = (srcdata & mask_reg) | bitmask;
		do_write(addr, i, srcdata);
	}
	return 0xff;
}

uint8 FMALU::do_not(uint32 addr)
{
	uint32 i;
	uint8 bitmask;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i);
		bitmask = ~srcdata;
		
		bitmask = bitmask & ~mask_reg;
		srcdata = (srcdata & mask_reg) | bitmask;
		do_write(addr, i, srcdata);
	}
	return 0xff;
}


uint8 FMALU::do_tilepaint(uint32 addr)
{
	uint32 i;
	uint8 bitmask;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i);
		bitmask = tile_reg[i] & ~mask_reg;
		srcdata = (srcdata & mask_reg) | bitmask;
		do_write(addr, i, srcdata);
	}
	return 0xff;
}

uint8 FMALU::do_compare(uint32 addr)
{
	uint32 offset = 0x4000;
	uint8 r, g, b, t;
	uint8 disables = ~bank_disable_reg & 0x0f;
	uint8 tmpcol;
	int i;
	int j;

	b = do_read(addr, 0);
	r = do_read(addr, 1);
	g = do_read(addr, 2);
	if(planes >= 4) {
		t = do_read(addr, 3);
	} else {
		disables = disables & 0x07;
	}
	cmp_status_reg = 0x00;
	tmpcol  = (b & 0x80) ? 1 : 0;
	tmpcol |= (r & 0x80) ? 2 : 0;
	tmpcol |= (g & 0x80) ? 4 : 0;
	if(planes >= 4) {
		tmpcol |= (t & 0x80) ? 8 : 0;
	}
	tmpcol = tmpcol & disables;
	for(i = 7; i >= 0; i--) {
		for(j = 0; j < 8; j++) {
			if((cmp_color_data[j] & 0x80) != 0) continue;
			if((cmp_color_data[j] & disables) == tmpcol) {
				cmp_status_reg = cmp_status_reg | (1 << i);
				break;
			}
		}
	}
	return 0xff;
}

uint8 FMALU::do_alucmds(uint32 addr)
{
	if(addr >= 0x8000) {
		mask_reg = 0xff;
		return 0xff;
	}
	if(((command_reg & 0x40) != 0) && ((command_reg & 0x07) != 7)) do_compare(addr);
	//printf("ALU: CMD %02x ADDR=%08x\n", command_reg, addr);
	switch(command_reg & 0x07) {
		case 0:
			return do_pset(addr);
			break;
		case 1:
			return do_blank(addr);
			break;
		case 2:
			return do_or(addr);
			break;
		case 3:
			return do_and(addr);
			break;
		case 4:
			return do_xor(addr);
			break;
		case 5:
			return do_not(addr);
			break;
		case 6:
			return do_tilepaint(addr);
			break;
		case 7:
			return do_compare(addr);
			break;
	}
	return 0xff;
}

void FMALU::do_line(void)
{
	int x_begin = line_xbegin.w.l;
	int x_end = line_xend.w.l;
	int y_begin = line_ybegin.w.l;
	int y_end = line_yend.w.l;
	uint32 total_bytes;
	int xx, yy;
	int delta;
	int tmp;
	int width, height;
	int count;
	bool direction = false;
	uint8 lmask[8] = {0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01};
	uint8 rmask[8] = {0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	uint8 vmask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	double usec;
	//is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
	//planes = target->read_signal(SIG_DISPLAY_PLANES) & 0x07;
	is_400line = false;
	planes = 3;
	screen_width = target->read_signal(SIG_DISPLAY_X_WIDTH) * 8;
	screen_height = target->read_signal(SIG_DISPLAY_Y_HEIGHT);
	//screen_width = 320;
	//screen_height = 200;

	if((command_reg & 0x80) == 0) return;
	oldaddr = 0xffffffff;

	int cpx_t = x_begin;
	int cpy_t = y_begin;
	int16 ax = x_end - x_begin;
	int16 ay = y_end - y_begin;

	line_style = line_pattern;
	// Got from HD63484.cpp .
	if(abs(ax) >= abs(ay)) {
		while(ax) {
			put_dot(cpx_t, cpy_t);
			if(ax > 0) {
				cpx_t++;
				ax--;
			} else {
				cpx_t--;
				ax++;
			}
			cpy_t = y_begin + ay * (cpx_t - x_begin) / (x_end - x_begin);
		}
	} else {
		while(ay) {
			put_dot(cpx_t, cpy_t);
			if(ay > 0) {
				cpy_t++;
				ay--;
			} else {
				cpy_t--;
				ay++;
			}
			cpx_t = x_begin + ax * (cpy_t - y_begin) / (y_end - y_begin);
		}
	}

	usec = (double)total_bytes / 16.0;
	if(usec >= 1.0) {
		register_event(this, EVENT_FMALU_BUSY_OFF, usec, false, &eventid_busy) ;
	} else {
       		busy_flag = false;
	}
}

void FMALU::put_dot(int x, int y)
{
	uint16 addr;
	uint32 bank_offset = target->read_signal(SIG_DISPLAY_BANK_OFFSET);
	uint8 vmask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	uint16 tmp8a;
	uint8 mask;

	if((x < 0) || (y < 0)) return;
	if((x >= screen_width) || (y >= screen_height)) return;
	
	addr = ((y * screen_width) >> 3) + (x >> 3);
	addr = addr + line_addr_offset.w.l;
	addr = addr & 0x7fff;
	if(!is_400line) {
		if(screen_width == 640) {
			addr = addr & 0x3fff;
		} else {
			addr = addr & 0x1fff;
		}
	}
	if((line_style.b.h & 0x80) != 0) {
	  	mask_reg = ~vmask[x & 7];
	}
  	if(oldaddr != addr) do_alucmds((uint32) addr);
	oldaddr = addr;
	tmp8a = (line_style.w.l & 0x8000) >> 15;
	line_style.w.l = (line_pattern.w.l << 1) | tmp8a;
}

void FMALU::write_data8(uint32 id, uint32 data)
{
	//printf("ALU: ADDR=%02x DATA=%02x\n", id, data);
	if(id == ALU_CMDREG) {
		command_reg = data;
		return;
	}
	//if((command_reg & 0x80) == 0) return;
	switch(id) {
		case ALU_LOGICAL_COLOR:
			color_reg = data;
			break;
		case ALU_WRITE_MASKREG:
			mask_reg = data;
			break;
		case ALU_BANK_DISABLE:
			bank_disable_reg = data;
			break;
		case ALU_TILEPAINT_B:
			tile_reg[0] = data;
			break;
		case ALU_TILEPAINT_R:
			tile_reg[1] = data;
			break;
		case ALU_TILEPAINT_G:
			tile_reg[2] = data;
			break;
		case ALU_TILEPAINT_L:
			tile_reg[3] = data;
			break;
		case ALU_OFFSET_REG_HIGH:
			is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
			if(is_400line) {
				line_addr_offset.b.h = data & 0x3f;
			} else {
				line_addr_offset.b.h = data & 0x1f;
			}
			break;
		case ALU_OFFSET_REG_LO:
			line_addr_offset.b.l = data;
			break;
		case ALU_LINEPATTERN_REG_HIGH:
			line_pattern.b.h = data;
			break;
		case ALU_LINEPATTERN_REG_LO:
			line_pattern.b.l = data;
			break;
		case ALU_LINEPOS_START_X_HIGH:
			line_xbegin.b.h = data;
			break;
		case ALU_LINEPOS_START_X_LOW:  
			line_xbegin.b.l = data;
			break;
		case ALU_LINEPOS_START_Y_HIGH:
			line_ybegin.b.h = data;
			break;
		case ALU_LINEPOS_START_Y_LOW:  
			line_ybegin.b.l = data;
			break;
		case ALU_LINEPOS_END_X_HIGH:
			line_xend.b.h = data;
			break;
		case ALU_LINEPOS_END_X_LOW:  
			line_xend.b.l = data;
			break;
		case ALU_LINEPOS_END_Y_HIGH:
			line_yend.b.h = data;
			break;
		case ALU_LINEPOS_END_Y_LOW:
			line_yend.b.l = data;
			do_line();
			break;
		default:
			if((id >= ALU_CMPDATA_REG + 0) && (id < ALU_CMPDATA_REG + 8)) {
				cmp_color_data[id - ALU_CMPDATA_REG] = data;
			} else 	if((id >= ALU_WRITE_PROXY) && (id < (ALU_WRITE_PROXY + 0x18000))) {
				is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
				do_alucmds(id - ALU_WRITE_PROXY);
			}
			break;
	}
}

uint32 FMALU::read_data8(uint32 id)
{
  //if((command_reg & 0x80) == 0) return 0xff;
  
	switch(id) {
		case ALU_CMDREG:
			return (uint32)command_reg;
			break;
		case ALU_LOGICAL_COLOR:
			return (uint32)color_reg;
			break;
		case ALU_WRITE_MASKREG:
			return (uint32)mask_reg;
			break;
		case ALU_CMP_STATUS_REG:
			return (uint32)cmp_status_reg;
			break;
		case ALU_BANK_DISABLE:
			return (uint32)bank_disable_reg;
			break;
		default:
			if((id >= ALU_WRITE_PROXY) && (id < (ALU_WRITE_PROXY + 0x18000))) {
				return do_alucmds(id - ALU_WRITE_PROXY);
			}
			return 0xffffffff;
			break;
	}
}

uint32 FMALU::read_signal(int id)
{
	uint32 val = 0x00000000;
	switch(id) {
		case SIG_ALU_BUSYSTAT:
			if(busy_flag) val = 0xffffffff;
			break;
	}
	return val;
}

void FMALU::event_callback(int event_id, int err)
{
	switch(event_id) {
		case EVENT_FMALU_BUSY_ON:
			busy_flag = true;
			if(eventid_busy >= 0) cancel_event(this, eventid_busy);
			eventid_busy = -1;
			break;
		case EVENT_FMALU_BUSY_OFF:
			busy_flag = false;
			eventid_busy = -1;
			break;
	}
}

void FMALU::initialize(void)
{
	busy_flag = false;
	eventid_busy = -1;
}

void FMALU::reset(void)
{
	int i;
	busy_flag = false;
	if(eventid_busy >= 0) cancel_event(this, eventid_busy);
	eventid_busy = -1;
	
  	command_reg = 0;        // D410 (RW)
	color_reg = 0;          // D411 (RW)
	mask_reg = 0;           // D412 (RW)
	cmp_status_reg = 0;     // D413 (RO)
	for(i = 0; i < 8; i++) cmp_color_data[i] = 0; // D413-D41A (WO)
	bank_disable_reg = 0;   // D41B (RW)
	for(i = 0; i < 4; i++) tile_reg[i] = 0;        // D41C-D41F (WO)
	
	line_addr_offset.d = 0; // D420-D421 (WO)
	line_pattern.d = 0;     // D422-D423 (WO)
	line_xbegin.d = 0;      // D424-D425 (WO)
	line_ybegin.d = 0;      // D426-D427 (WO)
	line_xend.d = 0;        // D428-D429 (WO)
	line_yend.d = 0;        // D42A-D42B (WO)

	oldaddr = 0xffffffff;
	
	planes = target->read_signal(SIG_DISPLAY_PLANES) & 0x07;
	is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
	
	screen_width = target->read_signal(SIG_DISPLAY_X_WIDTH) * 8;
	screen_height = target->read_signal(SIG_DISPLAY_Y_HEIGHT);
}
