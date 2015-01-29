/*
	SHARP MZ-5500 Emulator 'EmuZ-5500'

	Author : Takeda.Toshiya
	Date   : 2008.04.10 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8255.h"
#include "../i8259.h"
#include "../../fifo.h"
#include "../../fileio.h"

#define BIT_DK	8
#define BIT_SRK	0x10
#define BIT_DC	1
#define BIT_STC	2

#define PHASE_IDLE	0

#define PHASE_SEND_EB_L	11
#define PHASE_SEND_D7_H	12
#define PHASE_SEND_D7_L	13
#define PHASE_SEND_D6_H	14
#define PHASE_SEND_D6_L	15
#define PHASE_SEND_D5_H	16
#define PHASE_SEND_D5_L	17
#define PHASE_SEND_D4_H	18
#define PHASE_SEND_D4_L	19
#define PHASE_SEND_D3_H	20
#define PHASE_SEND_D3_L	21
#define PHASE_SEND_D2_H	22
#define PHASE_SEND_D2_L	23
#define PHASE_SEND_D1_H	24
#define PHASE_SEND_D1_L	25
#define PHASE_SEND_D0_H	26
#define PHASE_SEND_D0_L	27
#define PHASE_SEND_PB_H	28
#define PHASE_SEND_PB_L	29
#define PHASE_SEND_RE_H	30
#define PHASE_SEND_RE_L	31
#define PHASE_SEND_END	32

#define PHASE_RECV_D4_H	41
#define PHASE_RECV_D4_L	42
#define PHASE_RECV_D3_H	43
#define PHASE_RECV_D3_L	44
#define PHASE_RECV_D2_H	45
#define PHASE_RECV_D2_L	46
#define PHASE_RECV_D1_H	47
#define PHASE_RECV_D1_L	48
#define PHASE_RECV_D0_H	49
#define PHASE_RECV_D0_L	50
#define PHASE_RECV_PB_H	51
#define PHASE_RECV_PB_L	52
#define PHASE_RECV_RE_H	53
#define PHASE_RECV_RE_L	54
#define PHASE_RECV_END	55

#define TIMEOUT_500MSEC	30
#define TIMEOUT_100MSEC	6

static const int key_table[256] = {
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x008,0x009,0x000,0x000,0x000,0x00d,0x000,0x000,
	0x000,0x000,0x000,0x006,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x01b,0x000,0x000,0x000,0x000,
	0x020,0x080,0x090,0x018,0x08f,0x01f,0x01c,0x01e,0x01d,0x000,0x000,0x000,0x000,0x00b,0x07f,0x000,
	0x030,0x031,0x032,0x033,0x034,0x035,0x036,0x037,0x038,0x039,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x061,0x062,0x063,0x064,0x065,0x066,0x067,0x068,0x069,0x06a,0x06b,0x06c,0x06d,0x06e,0x06f,
	0x070,0x071,0x072,0x073,0x074,0x075,0x076,0x077,0x078,0x079,0x07a,0x000,0x000,0x000,0x000,0x000,
	0x0f0,0x0f1,0x0f2,0x0f3,0x0f4,0x0f5,0x0f6,0x0f7,0x0f8,0x0f9,0x0ea,0x0eb,0x0ec,0x0ed,0x0ee,0x0ef,
	0x081,0x082,0x083,0x084,0x085,0x086,0x087,0x088,0x089,0x08a,0x00e,0x00f,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x03a,0x03b,0x02c,0x02d,0x02e,0x02f,
	0x040,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x05b,0x05c,0x05d,0x05e,0x000,
	0x000,0x000,0x05f,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000
};

static const int key_table_shift[256] = {
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x019,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x007,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x020,0x090,0x080,0x000,0x09f,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x00c,0x01a,0x000,
	0x000,0x021,0x022,0x023,0x024,0x025,0x026,0x027,0x028,0x029,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x041,0x042,0x043,0x044,0x045,0x046,0x047,0x048,0x049,0x04a,0x04b,0x04c,0x04d,0x04e,0x04f,
	0x050,0x051,0x052,0x053,0x054,0x055,0x056,0x057,0x058,0x059,0x05a,0x000,0x000,0x000,0x000,0x000,
	0x0e0,0x0e1,0x0e2,0x0e3,0x0e4,0x0e5,0x0e6,0x0e7,0x0e8,0x0e9,0x000,0x000,0x000,0x000,0x000,0x000,
	0x091,0x092,0x093,0x094,0x095,0x096,0x097,0x098,0x099,0x09a,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x02a,0x02b,0x03c,0x03d,0x03e,0x03f,
	0x060,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x07b,0x07c,0x07d,0x07e,0x000,
	0x000,0x000,0x05f,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000
};

static const int key_table_kana[256] = {
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x008,0x009,0x000,0x000,0x000,0x00d,0x000,0x000,
	0x000,0x000,0x000,0x006,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x01b,0x000,0x000,0x000,0x000,
	0x020,0x080,0x090,0x018,0x08f,0x01f,0x01c,0x01e,0x01d,0x000,0x000,0x000,0x000,0x00b,0x07f,0x000,
	0x0dc,0x0c7,0x0cc,0x0b1,0x0b3,0x0b4,0x0b5,0x0d4,0x0d5,0x0d6,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x0c1,0x0ba,0x0bf,0x0bc,0x0b2,0x0ca,0x0b7,0x0b8,0x0c6,0x0cf,0x0c9,0x0d8,0x0d3,0x0d0,0x0d7,
	0x0be,0x0c0,0x0bd,0x0c4,0x0b6,0x0c5,0x0cb,0x0c3,0x0bb,0x0dd,0x0c2,0x000,0x000,0x000,0x000,0x000,
	0x0f0,0x0f1,0x0f2,0x0f3,0x0f4,0x0f5,0x0f6,0x0f7,0x0f8,0x0f9,0x0ea,0x0eb,0x0ec,0x0ed,0x0ee,0x0ef,
	0x081,0x082,0x083,0x084,0x085,0x086,0x087,0x088,0x089,0x08a,0x00e,0x00f,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x0b9,0x0da,0x0c8,0x0ce,0x0d9,0x0d2,
	0x0de,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x0df,0x0b0,0x0d1,0x0cd,0x000,
	0x000,0x000,0x0db,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000
};

static const int key_table_kana_shift[256] = {
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x019,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x007,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x020,0x090,0x080,0x000,0x09f,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x00c,0x01a,0x000,
	0x0a6,0x000,0x000,0x0a7,0x0a9,0x0aa,0x0ab,0x0ac,0x0ad,0x0ae,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x0a8,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x0af,0x000,0x000,0x000,0x000,0x000,
	0x0e0,0x0e1,0x0e2,0x0e3,0x0e4,0x0e5,0x0e6,0x0e7,0x0e8,0x0e9,0x000,0x000,0x000,0x000,0x000,0x000,
	0x091,0x092,0x093,0x094,0x095,0x096,0x097,0x098,0x099,0x09a,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x0a4,0x000,0x0a1,0x0a5,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x0a2,0x000,0x0a3,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000
};

static const int key_table_graph[256] = {
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x008,0x009,0x000,0x000,0x000,0x00d,0x000,0x000,
	0x000,0x000,0x000,0x006,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x01b,0x000,0x000,0x000,0x000,
	0x020,0x080,0x090,0x018,0x1a0,0x1a5,0x1a2,0x1a4,0x1a3,0x000,0x000,0x000,0x000,0x00b,0x07f,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x181,0x182,0x183,0x184,0x185,0x186,0x187,0x188,0x189,0x18a,0x18b,0x18c,0x18d,0x18e,0x18f,
	0x190,0x191,0x192,0x193,0x194,0x195,0x196,0x197,0x198,0x199,0x19a,0x000,0x000,0x000,0x000,0x000,
	0x0f0,0x0f1,0x0f2,0x0f3,0x0f4,0x0f5,0x0f6,0x0f7,0x0f8,0x0f9,0x0ea,0x0eb,0x0ec,0x0ed,0x0ee,0x0ef,
	0x081,0x082,0x083,0x084,0x085,0x086,0x087,0x088,0x089,0x08a,0x00e,0x00f,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x180,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x19b,0x19c,0x19d,0x19e,0x000,
	0x000,0x000,0x19f,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000
};

static const int key_table_graph_shift[256] = {
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x019,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x007,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x020,0x090,0x080,0x000,0x1fb,0x1a5,0x1a2,0x1a4,0x1a3,0x000,0x000,0x000,0x000,0x00c,0x01a,0x000,
	0x1d0,0x1d1,0x1d2,0x1d3,0x1d4,0x1d5,0x1d6,0x1d7,0x1d8,0x1d9,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x1e1,0x1e2,0x1e3,0x1e4,0x1e5,0x1e6,0x1e7,0x1e8,0x1e9,0x1ea,0x1eb,0x1ec,0x1ed,0x1ee,0x1ef,
	0x1f0,0x1f1,0x1f2,0x1f3,0x1f4,0x1f5,0x1f6,0x1f7,0x1f8,0x1f9,0x1fa,0x000,0x000,0x000,0x000,0x000,
	0x0e0,0x0e1,0x0e2,0x0e3,0x0e4,0x0e5,0x0e6,0x0e7,0x0e8,0x0e9,0x000,0x000,0x000,0x000,0x000,0x000,
	0x091,0x092,0x093,0x094,0x095,0x096,0x097,0x098,0x099,0x09a,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000
};

#define SET_DK(v) { \
	d_pio->write_signal(SIG_I8255_PORT_B, (dk = (v) ? 1 : 0) ? 0 : BIT_DK, BIT_DK); \
}
#define SET_SRK(v) { \
	d_pio->write_signal(SIG_I8255_PORT_B, (srk = (v) ? 1 : 0) ? 0 : BIT_SRK, BIT_SRK); \
	d_pic->write_signal(SIG_I8259_IR3 | SIG_I8259_CHIP0, srk ? 0 : 1, 1); \
}

void KEYBOARD::initialize()
{
	key_stat = emu->key_buffer();
	mouse_stat = emu->mouse_buffer();
	key_buf = new FIFO(64);
	rsp_buf = new FIFO(16);
	caps = kana = graph = false;
	
	register_frame_event(this);
}

void KEYBOARD::release()
{
	key_buf->release();
	delete key_buf;
	rsp_buf->release();
	delete rsp_buf;
}

void KEYBOARD::reset()
{
	key_buf->clear();
	rsp_buf->clear();
	SET_DK(1);
	SET_SRK(1);
	dc = stc = 1;
	phase = PHASE_IDLE;
	timeout = 0;
}

void KEYBOARD::write_signal(int id, uint32 data, uint32 mask)
{
	// from 8255 port c
	dc = (data & BIT_DC) ? 0 : 1;
	stc = (data & BIT_STC) ? 0 : 1;
	drive();
}

void KEYBOARD::event_frame()
{
	if(timeout > 0) {
		timeout--;
	}
	drive();
}

void KEYBOARD::key_down(int code)
{
	if(code == 0x1d) {
		// muhenkan->graph
		if(graph) {
			graph = false;
		} else {
			graph = true;
			kana = false;
		}
		return;
	} else if(code == 0x14) {
		// caps
		caps = !caps;
		return;
	} else if(code == 0x15) {
		// kana
		if(kana) {
			kana = false;
		} else {
			kana = true;
			graph = false;
		}
		return;
	}
	int shift = key_stat[0x10];
	int ctrl = key_stat[0x11];
	int algo = key_stat[0x12];
	
	if(kana) {
		if(shift) {
			code = key_table_kana_shift[code];
		} else {
			code = key_table_kana[code];
		}
	} else if(graph) {
		if(shift) {
			code = key_table_graph_shift[code];
		} else {
			code = key_table_graph[code];
		}
	} else {
		if(shift) {
			code = key_table_shift[code];
		} else {
			code = key_table[code];
		}
	}
	if(!code) {
		return;
	}
	if(caps) {
		if(0x41 <= code && code <= 0x5a) {
			code += 0x20;
		} else if(0x61 <= code && code <= 0x7a) {
			code -= 0x20;
		}
	}
	if(ctrl) {
		key_buf->write(2);
	} else if(algo) {
		key_buf->write(4);
	}
	key_buf->write(code);
//	drive();
}

void KEYBOARD::key_up(int code)
{
	// dont check key break
}

#define NEXTPHASE() { \
	phase++; \
	timeout = TIMEOUT_100MSEC; \
}

void KEYBOARD::drive()
{
	switch(phase) {
	case PHASE_IDLE:
		if(dc && (!key_buf->empty() || !rsp_buf->empty())) {
			if(!rsp_buf->empty()) {
				send = rsp_buf->read();
			} else {
				send = key_buf->read();
			}
			send = ~send & 0x1ff;
			int parity = 0;
			for(int i = 0; i < 9; i++) {
				parity += (send & (1 << i)) ? 1 : 0;
			}
			send = (send << 1) | (parity & 1);
			
			SET_DK(0);
			SET_SRK(0);
			phase = PHASE_SEND_EB_L;
			// 500msec
			timeout = TIMEOUT_500MSEC;
		} else if(!dc && !stc) {
			recv = 0;
			SET_DK(0);
			phase = PHASE_RECV_D4_H;
			// 500msec
			timeout = TIMEOUT_500MSEC;
		}
		break;
	case PHASE_SEND_EB_L:
		if(!stc) {
			SET_DK(send & 0x200);
			SET_SRK(1);
			NEXTPHASE();
		}
		break;
	case PHASE_SEND_D7_H:
	case PHASE_SEND_D6_H:
	case PHASE_SEND_D5_H:
	case PHASE_SEND_D4_H:
	case PHASE_SEND_D3_H:
	case PHASE_SEND_D2_H:
	case PHASE_SEND_D1_H:
	case PHASE_SEND_D0_H:
	case PHASE_SEND_PB_H:
		if(stc) {
			SET_DK(send & 0x100);
			send <<= 1;
			NEXTPHASE();
		}
		break;
	case PHASE_SEND_D7_L:
	case PHASE_SEND_D6_L:
	case PHASE_SEND_D5_L:
	case PHASE_SEND_D4_L:
	case PHASE_SEND_D3_L:
	case PHASE_SEND_D2_L:
	case PHASE_SEND_D1_L:
	case PHASE_SEND_D0_L:
	case PHASE_SEND_PB_L:
		if(!stc) {
			NEXTPHASE();
		}
		break;
	case PHASE_SEND_RE_H:
		if(stc) {
			SET_DK(0);
			NEXTPHASE();
		}
		break;
	case PHASE_SEND_RE_L:
		if(!stc) {
			NEXTPHASE();
		}
		break;
	case PHASE_SEND_END:
		if(stc) {
			SET_DK(1);
			phase = PHASE_IDLE;
		}
		break;
	case PHASE_RECV_D4_H:
	case PHASE_RECV_D3_H:
	case PHASE_RECV_D2_H:
	case PHASE_RECV_D1_H:
	case PHASE_RECV_D0_H:
	case PHASE_RECV_PB_H:
		if(stc) {
			NEXTPHASE();
		}
		break;
	case PHASE_RECV_D4_L:
	case PHASE_RECV_D3_L:
	case PHASE_RECV_D2_L:
	case PHASE_RECV_D1_L:
	case PHASE_RECV_D0_L:
	case PHASE_RECV_PB_L:
		if(!stc) {
			recv = (recv << 1) | (dc ? 1 : 0);
			NEXTPHASE();
		}
		break;
	case PHASE_RECV_RE_H:
		if(stc) {
			SET_DK(1);
			NEXTPHASE();
		}
		break;
	case PHASE_RECV_RE_L:
		if(!stc) {
			NEXTPHASE();
		}
		break;
	case PHASE_RECV_END:
		if(stc) {
			recv >>= 1;
			recv = ~recv & 0x1f;
			process(recv);
			phase = PHASE_IDLE;
		}
		break;
	}
	
	// timeout
	if(phase != PHASE_IDLE && !(timeout > 0)) {
		SET_DK(1);
		SET_SRK(1);
		phase = PHASE_IDLE;
	}
}

void KEYBOARD::process(int cmd)
{
	int mx, my, mb;
	
	switch(cmd) {
	case 1:
		// mouse ???
		mx = mouse_stat[0]; mx = (mx > 126) ? 126 : (mx < -128) ? -128 : mx;
		my = mouse_stat[1]; my = (my > 126) ? 126 : (my < -128) ? -128 : my;
		mb = mouse_stat[2];
//		rsp_buf->clear();
		rsp_buf->write(0x140 | (mx & 0x3f));
		rsp_buf->write(0x140 | (my & 0x3f));
		rsp_buf->write(0x140 | ((my >> 2) & 0x30) | ((mx >> 4) & 0xc) | (mb & 3));
		break;
	case 25:
		// clear buffer ?
//		key_buf->clear();
		break;
	case 30:
		// version ???
//		rsp_buf->write(0x110);
		break;
	}
}

#define STATE_VERSION	1

void KEYBOARD::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	key_buf->save_state((void *)state_fio);
	rsp_buf->save_state((void *)state_fio);
	state_fio->FputBool(caps);
	state_fio->FputBool(kana);
	state_fio->FputBool(graph);
	state_fio->FputInt32(dk);
	state_fio->FputInt32(srk);
	state_fio->FputInt32(dc);
	state_fio->FputInt32(stc);
	state_fio->FputInt32(send);
	state_fio->FputInt32(recv);
	state_fio->FputInt32(phase);
	state_fio->FputInt32(timeout);
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
	if(!rsp_buf->load_state((void *)state_fio)) {
		return false;
	}
	caps = state_fio->FgetBool();
	kana = state_fio->FgetBool();
	graph = state_fio->FgetBool();
	dk = state_fio->FgetInt32();
	srk = state_fio->FgetInt32();
	dc = state_fio->FgetInt32();
	stc = state_fio->FgetInt32();
	send = state_fio->FgetInt32();
	recv = state_fio->FgetInt32();
	phase = state_fio->FgetInt32();
	timeout = state_fio->FgetInt32();
	return true;
}

