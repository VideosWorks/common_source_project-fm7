/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'

	Author : Takeda.Toshiya
	Date   : 2009.03.15-

	[ pseudo sub cpu ]
*/

#include "psub.h"
#include "../datarec.h"
#include "../i8255.h"
#include "../../fifo.h"

//#define DEBUG_COMMAND

#define EVENT_1SEC	0
#define EVENT_DRIVE	1
#define EVENT_REPEAT	2

#define CMT_EJECT	0x00
#define CMT_STOP	0x01
#define CMT_PLAY	0x02
#define CMT_FAST_FWD	0x03
#define CMT_FAST_REW	0x04
#define CMT_APSS_PLUS	0x05
#define CMT_APSS_MINUS	0x06
#define CMT_REC		0x0a

// TODO: XFER = 0xe8 ???

static const uint8_t keycode[256] = {	// normal
	0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x08, 0x09, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0xe8, 0x00, 0x00, 0x00,
	0x20, 0x0e, 0x0f, 0x11, 0x0b, 0x1d, 0x1e, 0x1c, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x12, 0x08, 0x00,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x71, 0x72, 0x73, 0x74, 0x75, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3a, 0x3b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5b, 0x5c, 0x5d, 0x5e, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t keycode_s[256] = {	// shift
	0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x12, 0x09, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0xe8, 0x00, 0x00, 0x00,
	0x20, 0x0e, 0x0f, 0x11, 0x0c, 0x1d, 0x1e, 0x1c, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x12, 0x08, 0x00,
	0x30, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2a, 0x2b, 0x3c, 0x2d, 0x2e, 0x3f,
	0x76, 0x77, 0x78, 0x79, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2a, 0x2b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7b, 0x7c, 0x7d, 0x60, 0x00,
	0x00, 0x00, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t keycode_g[256] = {	// graph
	0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe8, 0x00, 0x00, 0x00,
	0x00, 0x0e, 0x0f, 0x11, 0x0b, 0x1d, 0x1e, 0x1c, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x12, 0x08, 0x00,
	0xfa, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x7f, 0x84, 0x82, 0xea, 0xe2, 0xeb, 0xec, 0xed, 0xe7, 0xee, 0xef, 0x8e, 0x86, 0x85, 0xf0,
	0x8d, 0xe0, 0xe3, 0xe9, 0xe4, 0xe6, 0x83, 0xe1, 0x81, 0xe5, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x8f, 0x99, 0x92, 0x98, 0x95, 0x96, 0x94, 0x9a, 0x93, 0x97, 0x9b, 0x9d, 0x87, 0x9c, 0x91, 0x9e,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd, 0x89, 0x87, 0x8c, 0x88, 0xfe,
	0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xfb, 0xe8, 0x8b, 0x00,
	0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t keycode_c[256] = {	// ctrl
	0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x08, 0x09, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00,
	0x1c, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0xe8, 0x00, 0x00, 0x00,
	0x00, 0x0e, 0x0f, 0x11, 0x0b, 0x1d, 0x1e, 0x1c, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x12, 0x08, 0x00,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2a, 0x2b, 0x00, 0x2d, 0x2e, 0x2f,
	0x00, 0x00, 0x00, 0x00, 0x00, 0xec, 0xeb, 0xe2, 0xe1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3a, 0x3b, 0x00, 0x2d, 0x00, 0x00,
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0x1c, 0x1d, 0x1e, 0x00,
	0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t keycode_k[256] = {	// kana
	0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x08, 0x09, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0xe8, 0x00, 0x00, 0x00,
	0x20, 0x0e, 0x0f, 0x11, 0x0b, 0x1d, 0x1e, 0x1c, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x12, 0x08, 0x00,
	0xdc, 0xc7, 0xcc, 0xb1, 0xb3, 0xb4, 0xb5, 0xd4, 0xd5, 0xd6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xc1, 0xba, 0xbf, 0xbc, 0xb2, 0xca, 0xb7, 0xb8, 0xc6, 0xcf, 0xc9, 0xd8, 0xd3, 0xd0, 0xd7,
	0xbe, 0xc0, 0xbd, 0xc4, 0xb6, 0xc5, 0xcb, 0xc3, 0xbb, 0xdd, 0xc2, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2a, 0x2b, 0xc8, 0x2d, 0x2e, 0xd2,
	0x71, 0x72, 0x73, 0x74, 0x75, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb9, 0xda, 0xc8, 0xce, 0xd9, 0xd2,
	0xde, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdf, 0xb0, 0xd1, 0xcd, 0x00,
	0x00, 0x00, 0xdb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t keycode_ks[256] = {	// kana+shift
	0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x12, 0x09, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0xe8, 0x00, 0x00, 0x00,
	0x20, 0x0e, 0x0f, 0x11, 0x0c, 0x1d, 0x1e, 0x1c, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x12, 0x08, 0x00,
	0xa6, 0xc7, 0xcc, 0xa7, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xc1, 0xba, 0xbf, 0xbc, 0xa8, 0xca, 0xb7, 0xb8, 0xc6, 0xcf, 0xc9, 0xd8, 0xd3, 0xd0, 0xd7,
	0xbe, 0xc0, 0xbd, 0xc4, 0xb6, 0xc5, 0xcb, 0xc3, 0xbb, 0xdd, 0xaf, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2a, 0x2b, 0xa4, 0x2d, 0x2e, 0xa5,
	0x76, 0x77, 0x78, 0x79, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb9, 0xda, 0xa4, 0xce, 0xa1, 0xa5,
	0xde, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa2, 0xb0, 0xa3, 0xcd, 0x00,
	0x00, 0x00, 0xdb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t keycode_kb[256] = {	// kana (mode b)
	0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x08, 0x09, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0xe8, 0x00, 0x00, 0x00,
	0x20, 0x0e, 0x0f, 0x11, 0x0b, 0x1d, 0x1e, 0x1c, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x12, 0x08, 0x00,
	0xc9, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xc5, 0xc6, 0xc7, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xbb, 0xc4, 0xc2, 0xbd, 0xb8, 0xbe, 0xbf, 0xcf, 0xcc, 0xd0, 0xd1, 0xd2, 0xd5, 0xd4, 0xcd,
	0xce, 0xb6, 0xb9, 0xbc, 0xba, 0xcb, 0xc3, 0xb7, 0xc1, 0xca, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2a, 0x2b, 0xc8, 0x2d, 0x2e, 0xd2,
	0x71, 0x72, 0x73, 0x74, 0x75, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xde, 0xd3, 0xd6, 0xd7, 0xdc, 0xa6,
	0xda, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdb, 0xd9, 0xdf, 0xd8, 0x00,
	0x00, 0x00, 0xdd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t keycode_ksb[256] = {	// kana+shift (mode b)
	0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x12, 0x09, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0xe8, 0x00, 0x00, 0x00,
	0x20, 0x0e, 0x0f, 0x11, 0x0c, 0x1d, 0x1e, 0x1c, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x12, 0x08, 0x00,
	0xc9, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xc5, 0xc6, 0xc7, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xbb, 0xc4, 0xaf, 0xbd, 0xb8, 0xbe, 0xbf, 0xcf, 0xcc, 0xd0, 0xd1, 0xd2, 0xad, 0xac, 0xcd,
	0xce, 0xb6, 0xb9, 0xbc, 0xba, 0xcb, 0xc3, 0xb7, 0xc1, 0xca, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2a, 0x2b, 0xa4, 0x2d, 0x2e, 0xa5,
	0x76, 0x77, 0x78, 0x79, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0, 0xd3, 0xae, 0xd7, 0xa4, 0xa1,
	0xda, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa2, 0x20, 0xa3, 0xd8, 0x00,
	0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void PSUB::initialize()
{
	key_buf = new FIFO(8);
	key_stat = emu->get_key_buffer();
	
	get_host_time(&cur_time);
	
	// register events
	register_event(this, EVENT_1SEC, 1000000, true, &time_register_id);
	register_event(this, EVENT_DRIVE, 400, true, NULL);
}

void PSUB::release()
{
	key_buf->release();
	delete key_buf;
}

void PSUB::reset()
{
	memset(databuf, 0, sizeof(databuf));
	databuf[0x16][0] = 0xff;
	datap = &databuf[0][0];	// temporary fix
	mode = 0;
	cmdlen = datalen = 0;
	
	play = rec = eot = false;
	
	ibf = true;
	set_ibf(false);
	obf = false;
	set_obf(true);
	
	// key buffer
	key_buf->clear();
	key_prev = key_break = 0;
	key_shift = key_ctrl = key_graph = false;
	key_caps_locked = key_kana_locked = false;
	key_register_id = -1;
	
	// interrupt
	iei = true;
	intr = false;
	
	// break
	d_pio->write_signal(SIG_I8255_PORT_B, 0xff, 1);
}

void PSUB::write_io8(uint32_t addr, uint32_t data)
{
	inbuf = data;
	set_ibf(true);
}

uint32_t PSUB::read_io8(uint32_t addr)
{
	set_obf(true);
	intr = false;
	update_intr();
	return outbuf;
}

void PSUB::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_PSUB_TAPE_REMOTE) {
		if(!(data & mask) && (play || rec)) {
			databuf[0x1a][0] = CMT_STOP;
		}
	} else if(id == SIG_PSUB_TAPE_END) {
		eot = ((data & mask) != 0);
	}
}

void PSUB::set_intr_iei(bool val)
{
	if(iei != val) {
		iei = val;
		update_intr();
	}
}

uint32_t PSUB::get_intr_ack()
{
	return read_io8(0x1900);
}

void PSUB::notify_intr_reti()
{
	// NOTE: some software uses RET, not RETI ???
}

void PSUB::update_intr()
{
	if(intr && iei) {
		d_cpu->set_intr_line(true, true, intr_bit);
	} else {
		d_cpu->set_intr_line(false, true, intr_bit);
	}
}

void PSUB::event_callback(int event_id, int err)
{
	if(event_id == EVENT_1SEC) {
		if(cur_time.initialized) {
			cur_time.increment();
		} else {
			get_host_time(&cur_time);	// resync
			cur_time.initialized = true;
		}
	} else if(event_id == EVENT_DRIVE) {
		// drive sub cpu
		static const int cmdlen_tbl[] = {
			0, 1, 0, 0, 1, 0, 1, 0, 0, 3, 0, 3, 0
		};
		
#ifdef _X1TWIN
		// clear key buffer
		if(vm->is_cart_inserted(0)) {
			// clear key
			key_buf->clear();
		}
#endif
		if(ibf) {
			// sub cpu received data from main cpu
			if(cmdlen) {
				// this is command parameter
				*datap++ = inbuf;
#ifdef DEBUG_COMMAND
				this->out_debug_log(_T(" %2x"), inbuf);
#endif
				cmdlen--;
			} else {
				// this is new command
				mode = inbuf;
#ifdef DEBUG_COMMAND
				this->out_debug_log(_T("X1 PSUB: cmd %2x"), inbuf);
#endif
				if(0xd0 <= mode && mode <= 0xd7) {
					cmdlen = 6;
					datap = &databuf[mode - 0xd0][0]; // receive buffer
				} else if(0xe3 <= mode && mode <= 0xef) {
					cmdlen = cmdlen_tbl[mode - 0xe3];
					datap = &databuf[mode - 0xd0][0]; // receive buffer
				}
			}
			if(cmdlen == 0) {
				// this command has no parameters or all parameters are received,
				// so cpu processes the command
#ifdef DEBUG_COMMAND
				this->out_debug_log(_T("\n"));
#endif
				process_cmd();
			}
			// sub cpu can accept new command or parameter
			set_ibf(false);
			set_obf(true);
			
			// if command is not finished, key irq is not raised
			if(cmdlen || datalen) {
				return;
			}
		}
		if(obf) {
			// sub cpu can send data to main cpu
			if(datalen) {
				// sub cpu sends result data
				outbuf = *datap++;
				set_obf(false);
				datalen--;
			} else if(!key_buf->empty() && databuf[0x14][0] && !intr && iei) {
				// key buffer is not empty and interrupt is not disabled,
				// so sub cpu sends vector and raise irq
				outbuf = databuf[0x14][0];
				set_obf(false);
				intr = true;
				update_intr();
				
				// read key buffer
				mode = 0xe6;
				process_cmd();
			}
		}
	} else if(event_id == EVENT_REPEAT) {
		key_register_id = -1;
		key_down(key_prev, true);
	}
}

// shift, ctrl, graph, caps, kana
#define IS_LOWBYTE_KEY(c) ((c >= 0x10 && c <= 0x12) || c == 0x14 || c == 0x15)

void PSUB::key_down(int code, bool repeat)
{
	// check keycode
	switch(code) {
	case 0x10:
		key_shift = true;
		break;
	case 0x11:
		key_ctrl = true;
		break;
	case 0x12:
		key_graph = true;
		break;
	case 0x14:
		key_caps_locked = !key_caps_locked;
		break;
	case 0x15:
		key_kana_locked = !key_kana_locked;
		break;
	}
	uint16_t lh = get_key(code, repeat);
	if(lh & 0xff00) {
		lh &= ~0x40;
		if(!databuf[0x14][0]) {
			key_buf->clear();
		}
		key_buf->write(lh);
		key_prev = code;
		
		// setup key repeat event
		if(key_register_id != -1) {
			cancel_event(this, key_register_id);
			key_register_id = -1;
		}
		if(!(0x70 <= code && code <= 0x87)) {
			if(repeat) {
				register_event(this, EVENT_REPEAT, 61165, false, &key_register_id);	// 61.165 msec
			} else {
				register_event(this, EVENT_REPEAT, 557085, false, &key_register_id);	// 557.085 msec
			}
		}
		
		// break key is pressed
		if((lh >> 8) == 3) {
			d_pio->write_signal(SIG_I8255_PORT_B, 0, 1);
			key_break = code;
		}
#ifdef _X1TURBO_FEATURE
	} else if(key_prev == 0 && IS_LOWBYTE_KEY(code)) {
		key_buf->write(0xff);
#endif
	}
}

void PSUB::key_up(int code)
{
	// check keycode
	switch(code) {
	case 0x10:
		key_shift = false;
		break;
	case 0x11:
		key_ctrl = false;
		break;
	case 0x12:
		key_graph = false;
		break;
	}
#ifdef _X1TURBO_FEATURE
	if(code == key_prev || (key_prev == 0 && IS_LOWBYTE_KEY(code))) {
#else
	if(code == key_prev) {
#endif
		// last pressed key is released
		if(!databuf[0x14][0]) {
			key_buf->clear();
		}
		key_buf->write(0xff);
		key_prev = 0;
		if(key_register_id != -1) {
			cancel_event(this, key_register_id);
			key_register_id = -1;
		}
	}
	if(code == key_break) {
		// break key is released
		d_pio->write_signal(SIG_I8255_PORT_B, 0xff, 1);
		key_break = 0;
	}
}

void PSUB::play_tape(bool value)
{
	if(value) {
		databuf[0x1a][0] = CMT_STOP;
	} else {
		databuf[0x1a][0] = CMT_EJECT;
	}
	play = value;
	rec = false;
	d_drec->set_remote(false);
}

void PSUB::rec_tape(bool value)
{
	if(value) {
		databuf[0x1a][0] = CMT_STOP;
	} else {
		databuf[0x1a][0] = CMT_EJECT;
	}
	play = false;
	rec = value;
	d_drec->set_remote(false);
}

void PSUB::close_tape()
{
	if(play || rec) {
		databuf[0x1a][0] = CMT_EJECT;
		play = rec = false;
		d_drec->set_remote(false);
	}
}

void PSUB::process_cmd()
{
	uint16_t lh = 0xff;
	
	// preset 
	if(0xd0 <= mode && mode < 0xf0) {
		datap = &databuf[mode - 0xd0][0]; // send buffer
	}
	datalen = 0;
	
	// process command
	switch(mode) {
	case 0x00:
		// reset receive/send buffer
		break;
	case 0xd0:
	case 0xd1:
	case 0xd2:
	case 0xd3:
	case 0xd4:
	case 0xd5:
	case 0xd6:
	case 0xd7:
		// timer set
		break;
	case 0xd8:
	case 0xd9:
	case 0xda:
	case 0xdb:
	case 0xdc:
	case 0xdd:
	case 0xde:
	case 0xdf:
		// timer read
		datap = &databuf[mode - 0xd8][0];	// data buffer for timer set
		datalen = 6;
		break;
#ifdef _X1TURBO_FEATURE
	case 0xe3:
		// game key read (for turbo)
		databuf[0x13][0] = databuf[0x13][1] = databuf[0x13][2] = 0;
		if(config.device_type != 0) {
			databuf[0x13][0] |= key_stat[0x51] ? 0x80 : 0;	// q
			databuf[0x13][0] |= key_stat[0x57] ? 0x40 : 0;	// w
			databuf[0x13][0] |= key_stat[0x45] ? 0x20 : 0;	// e
			databuf[0x13][0] |= key_stat[0x41] ? 0x10 : 0;	// a
			databuf[0x13][0] |= key_stat[0x44] ? 0x08 : 0;	// d
			databuf[0x13][0] |= key_stat[0x5a] ? 0x04 : 0;	// z
			databuf[0x13][0] |= key_stat[0x58] ? 0x02 : 0;	// x
			databuf[0x13][0] |= key_stat[0x43] ? 0x01 : 0;	// c
			databuf[0x13][1] |= key_stat[0x67] ? 0x80 : 0;	// 7
			databuf[0x13][1] |= key_stat[0x64] ? 0x40 : 0;	// 4
			databuf[0x13][1] |= key_stat[0x61] ? 0x20 : 0;	// 1
			databuf[0x13][1] |= key_stat[0x68] ? 0x10 : 0;	// 8
			databuf[0x13][1] |= key_stat[0x62] ? 0x08 : 0;	// 2
			databuf[0x13][1] |= key_stat[0x69] ? 0x04 : 0;	// 9
			databuf[0x13][1] |= key_stat[0x66] ? 0x02 : 0;	// 6
			databuf[0x13][1] |= key_stat[0x63] ? 0x01 : 0;	// 3
			databuf[0x13][2] |= key_stat[0x1b] ? 0x80 : 0;	// esc
			databuf[0x13][2] |= key_stat[0x61] ? 0x40 : 0;	// 1
			databuf[0x13][2] |= key_stat[0x6d] ? 0x20 : 0;	// -
			databuf[0x13][2] |= key_stat[0x6b] ? 0x10 : 0;	// +
			databuf[0x13][2] |= key_stat[0x6a] ? 0x08 : 0;	// *
			databuf[0x13][2] |= key_stat[0x09] ? 0x04 : 0;	// tab
			databuf[0x13][2] |= key_stat[0x20] ? 0x02 : 0;	// sp
			databuf[0x13][2] |= key_stat[0x0d] ? 0x01 : 0;	// ret
		}
		datalen = 3;
		break;
#endif
	case 0xe4:
		// irq vector
//		if(!databuf[0x14][0]) {
			while(!key_buf->empty()) {
				lh = key_buf->read();
				// ???
				databuf[0x16][0] = lh & 0xff;
				databuf[0x16][1] = lh >> 8;
			}
//		}
		intr = false;
		update_intr();
		break;
	case 0xe6:
		// keydata read
		if(!databuf[0x14][0]) {
			// interrupt disabled
			while(!key_buf->empty()) {
				lh = key_buf->read();
				databuf[0x16][0] = lh & 0xff;
				databuf[0x16][1] = lh >> 8;
			}
		} else {
			// interrupt enabed
			if(!key_buf->empty()) {
				lh = key_buf->read();
				databuf[0x16][0] = lh & 0xff;
				databuf[0x16][1] = lh >> 8;
			}
		}
#ifdef _X1TURBO_FEATURE
		databuf[0x16][0] &= ~0x1f;
		databuf[0x16][0] |= get_key_low() & 0x1f;
#endif
#ifdef DEBUG_COMMAND
		this->out_debug_log(_T("X1 PSUB: keycode %2x %2x\n"), databuf[0x16][0], databuf[0x16][1]);
#endif
		datalen = 2;
		break;
	case 0xe7:
		// TV controll
		databuf[0x18][0] = databuf[0x17][0];
		break;
	case 0xe8:
		// TV controll read
		datalen = 1;
		break;
	case 0xe9:
		// CMT controll
		if(databuf[0x1a][0] != databuf[0x19][0]) {
			uint8_t new_status = databuf[0x19][0];
			switch(databuf[0x19][0]) {
			case CMT_EJECT:
				emu->close_tape();
				break;
			case CMT_STOP:
				d_drec->set_remote(false);
				break;
			case CMT_PLAY:
				if(play) {
					d_drec->set_ff_rew(0);
					d_drec->set_remote(true);
				} else if(rec) {
					new_status = CMT_STOP;
				} else {
					new_status = CMT_EJECT;
				}
				break;
			case CMT_FAST_FWD:
				if(play) {
					d_drec->set_ff_rew(1);
					d_drec->set_remote(true);
				} else if(rec) {
					new_status = CMT_STOP;
				} else {
					new_status = CMT_EJECT;
				}
				break;
			case CMT_FAST_REW:
				if(play) {
					d_drec->set_ff_rew(-1);
					d_drec->set_remote(true);
				} else if(rec) {
					new_status = CMT_STOP;
				} else {
					new_status = CMT_EJECT;
				}
				break;
			case CMT_APSS_PLUS:
			case CMT_APSS_MINUS:
				if(play) {
					d_drec->do_apss((databuf[0x19][0] == CMT_APSS_PLUS) ? 1 : -1);
					new_status = CMT_STOP;
				} else if(rec) {
					new_status = CMT_STOP;
				} else {
					new_status = CMT_EJECT;
				}
				break;
			case CMT_REC:
				if(play) {
					new_status = CMT_STOP;
				} else if(rec) {
					d_drec->set_remote(true);
				} else {
					new_status = CMT_EJECT;
				}
				break;
#ifdef DEBUG_COMMAND
			default:
				this->out_debug_log(_T("X1 PSUB: unknown CMT control %2x\n"), databuf[0x19][0]);
				break;
#endif
			}
			databuf[0x1a][0] = new_status;
		}
		break;
	case 0xea:
		// CMT status
		datalen = 1;
		break;
	case 0xeb:
		// CMT sensor (bit2=WP, bit1=SET, bit0=END)
		databuf[0x1b][0] = (play ? 2 : rec ? 6 : 0) | (play && eot ? 1 : 0);
		datalen = 1;
		break;
	case 0xec:
		// set calender
		cur_time.year = FROM_BCD(databuf[0x1c][0]);
		if((databuf[0x1c][1] & 0xf0) != 0) {
			cur_time.month = databuf[0x1c][1] >> 4;
		}
//		cur_time.day_of_week = databuf[0x1c][1] & 7;
		if(databuf[0x1c][2] != 0) {
			cur_time.day = FROM_BCD(databuf[0x1c][2]);
		}
		cur_time.update_year();
		cur_time.update_day_of_week();
		break;
	case 0xed:
		// get calender
		databuf[0x1d][0] = TO_BCD(cur_time.year);
		databuf[0x1d][1] = (cur_time.month << 4) | cur_time.day_of_week;
		databuf[0x1d][2] = TO_BCD(cur_time.day);
		datalen = 3;
		break;
	case 0xee:
		// set time
		cur_time.hour = FROM_BCD(databuf[0x1e][0]);
		cur_time.minute = FROM_BCD(databuf[0x1e][1] & 0x7f);
		cur_time.second = FROM_BCD(databuf[0x1e][2] & 0x7f);
		// restart event
		cancel_event(this, time_register_id);
		register_event(this, EVENT_1SEC, 1000000, true, &time_register_id);
		break;
	case 0xef:
		// get time
		databuf[0x1f][0] = TO_BCD(cur_time.hour);
		databuf[0x1f][1] = TO_BCD(cur_time.minute);
		databuf[0x1f][2] = TO_BCD(cur_time.second);
		datalen = 3;
		break;
#ifdef DEBUG_COMMAND
	default:
		this->out_debug_log(_T("X1 PSUB: unknown cmd %2x\n"), mode);
#endif
	}
}

void PSUB::set_ibf(bool val)
{
	if(ibf != val) {
		d_pio->write_signal(SIG_I8255_PORT_B, val ? 0xff : 0, 0x40);
		ibf = val;
	}
}

void PSUB::set_obf(bool val)
{
	if(obf != val) {
		d_pio->write_signal(SIG_I8255_PORT_B, val ? 0xff : 0, 0x20);
		obf = val;
	}
}

uint8_t PSUB::get_key_low()
{
	uint8_t l = 0xff;
	
	if(key_ctrl) {
		l &= ~0x01;	// ctrl
	}
	if(key_shift) {
		l &= ~0x02;	// shift
	}
	if(key_kana_locked) {
		l &= ~0x04;	// kana
	}
	if(key_caps_locked) {
		l &= ~0x08;	// caps
	}
	if(key_graph) {
		l &= ~0x10;	// graph (alt)
	}
	return l;
}

uint16_t PSUB::get_key(int code, bool repeat)
{
	uint8_t l = get_key_low();
	uint8_t h = 0;
	
	if(repeat) {
		l &= ~0x20;	// repeat
	}
	if(0x60 <= code && code <= 0x74) {
		l &= ~0x80;	// function or numpad
	}
	if(key_kana_locked) {
		if(!(l & 0x02)) {
#ifdef _X1TURBO_FEATURE
			if(config.device_type != 0) {
				h = keycode_ksb[code];	// kana+shift (mode b)
			} else
#endif
			h = keycode_ks[code];	// kana+shift
		} else {
#ifdef _X1TURBO_FEATURE
			if(config.device_type != 0) {
				h = keycode_kb[code];	// kana (mode b)
			} else
#endif
			h = keycode_k[code];	// kana
		}
	} else {
		if(!(l & 0x01)) {
			h = keycode_c[code];	// ctrl
		} else if(!(l & 0x10)) {
			h = keycode_g[code];	// graph
		} else {
			if(!(l & 0x02)) {
				h = keycode_s[code];	// shift
			} else {
				h = keycode[code];	// (none shifted)
			}
			if(key_caps_locked) {
				if(0x41 <= code && code <= 0x5a) {
					h ^= 0x20;	// alphabet
				}
			}
		}
	}
#ifndef _X1TURBO_FEATURE
	if(!h) {
		l = 0xff;
	}
#endif
	return l | (h << 8);
}

#define STATE_VERSION	1

void PSUB::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	cur_time.save_state((void *)state_fio);
	state_fio->FputInt32(time_register_id);
	state_fio->Fwrite(databuf, sizeof(databuf), 1);
	state_fio->FputInt32((int)(datap - &databuf[0][0]));
	state_fio->FputUint8(mode);
	state_fio->FputUint8(inbuf);
	state_fio->FputUint8(outbuf);
	state_fio->FputBool(ibf);
	state_fio->FputBool(obf);
	state_fio->FputInt32(cmdlen);
	state_fio->FputInt32(datalen);
	key_buf->save_state((void *)state_fio);
	state_fio->FputInt32(key_prev);
	state_fio->FputInt32(key_break);
	state_fio->FputBool(key_shift);
	state_fio->FputBool(key_ctrl);
	state_fio->FputBool(key_graph);
	state_fio->FputBool(key_caps_locked);
	state_fio->FputBool(key_kana_locked);
	state_fio->FputInt32(key_register_id);
	state_fio->FputBool(play);
	state_fio->FputBool(rec);
	state_fio->FputBool(eot);
	state_fio->FputBool(iei);
	state_fio->FputBool(intr);
	state_fio->FputUint32(intr_bit);
}

bool PSUB::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	if(!cur_time.load_state((void *)state_fio)) {
		return false;
	}
	time_register_id = state_fio->FgetInt32();
	state_fio->Fread(databuf, sizeof(databuf), 1);
	datap = &databuf[0][0] + state_fio->FgetInt32();
	mode = state_fio->FgetUint8();
	inbuf = state_fio->FgetUint8();
	outbuf = state_fio->FgetUint8();
	ibf = state_fio->FgetBool();
	obf = state_fio->FgetBool();
	cmdlen = state_fio->FgetInt32();
	datalen = state_fio->FgetInt32();
	if(!key_buf->load_state((void *)state_fio)) {
		return false;
	}
	key_prev = state_fio->FgetInt32();
	key_break = state_fio->FgetInt32();
	key_shift = state_fio->FgetBool();
	key_ctrl = state_fio->FgetBool();
	key_graph = state_fio->FgetBool();
	key_caps_locked = state_fio->FgetBool();
	key_kana_locked = state_fio->FgetBool();
	key_register_id = state_fio->FgetInt32();
	play = state_fio->FgetBool();
	rec = state_fio->FgetBool();
	eot = state_fio->FgetBool();
	iei = state_fio->FgetBool();
	intr = state_fio->FgetBool();
	intr_bit = state_fio->FgetUint32();
	return true;
}

