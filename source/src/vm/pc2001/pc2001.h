/*
	NEC PC-2001 Emulator 'ePC-2001'

	Origin : PockEmul
	Author : Takeda.Toshiya
	Date   : 2016.03.18-

	[ virtual machine ]
*/

#ifndef _PC2001_H_
#define _PC2001_H_

#define DEVICE_NAME		"NEC PC-2001"
#define CONFIG_NAME		"pc2001"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		24
#define CPU_CLOCKS		4000000
#define SCREEN_WIDTH		240
#define SCREEN_HEIGHT		24
#define HAS_UPD7907
#define MEMORY_ADDR_MAX		0x10000
#define MEMORY_BANK_SIZE	0x1000

// device informations for win32
#define WINDOW_MODE_BASE	2
#define USE_TAPE
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10
//#define USE_AUTO_KEY_CAPS
//#define USE_SOUND_VOLUME	1
//#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"

//#ifdef USE_SOUND_VOLUME
//static const _TCHAR *sound_device_caption[] = {
//	_T("Beep"),
//};
//#endif

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class MEMORY;
class UPD16434;
class UPD1990A;
class UPD7810;

class IO;

#include "../../fileio.h"

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	MEMORY* memory;
	UPD16434* lcd[4];
	UPD1990A* rtc;
	UPD7810* cpu;
	
	IO* io;
	
	// memory
	uint8_t ram[0xa000];
	uint8_t rom1[0x1000];
	uint8_t rom2[0x4000];
	
public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
	
	VM(EMU* parent_emu);
	~VM();
	
	// ----------------------------------------
	// for emulation class
	// ----------------------------------------
	
	// drive virtual machine
	void reset();
	void run();
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16_t* create_sound(int* extra_frames);
	int get_sound_buffer_ptr();
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r);
#endif
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	
	// user interface
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted();
	bool is_tape_playing();
	bool is_tape_recording();
	int get_tape_position();
	bool is_frame_skippable();
	
	void update_config();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// devices
	DEVICE* get_device(int id);
	DEVICE* dummy;
	DEVICE* first_device;
	DEVICE* last_device;
};

#endif