/*
	FUJITSU FM16pi Emulator 'eFM16pi'

	Author : Takeda.Toshiya
	Date   : 2010.12.25-

	[ virtual machine ]
*/

#ifndef _FM16PI_H_
#define _FM16PI_H_

#define DEVICE_NAME		"FUJITSU FM16pi"
#define CONFIG_NAME		"fm16pi"

// device informations for virtual machine

// TODO: check refresh rate
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	220
#define CPU_CLOCKS		4915200
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		200
#define MAX_DRIVE		4
#define HAS_I86
#define I8259_MAX_CHIPS		1
#define MEMORY_ADDR_MAX		0x100000
#define MEMORY_BANK_SIZE	0x4000
#define IO_ADDR_MAX		0x10000

// device informations for win32
#define USE_FD1
#define USE_FD2
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_NOTIFY_POWER_OFF
#define USE_ACCESS_LAMP
#define USE_SOUND_VOLUME	1
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class I8251;
class I8253;
class I8255;
class I8259;
class I286;
class IO;
class MB8877;
class MEMORY;
class MSM58321;
class NOT;
class PCM1BIT;

class SUB;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	I8251* sio;
	I8253* pit;
	I8255* pio;
	I8259* pic;
	I286* cpu;
	IO* io;
	MB8877* fdc;
	MEMORY* memory;
	MSM58321* rtc;
	NOT* not_pit;
	PCM1BIT* pcm;
	
	SUB* sub;
	
	// memory
	uint8 ram[0x80000];
	uint8 kanji[0x40000];
	uint8 cart[0x40000];
	
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
	void notify_power_off();
	void run();
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
	int get_access_lamp_status();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int* extra_frames);
	int get_sound_buffer_ptr();
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r);
#endif
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	
	// user interface
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
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
