/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.09.03-

	[ T3444A / T3444M ]
*/

#ifndef _T3444A_H_ 
#define _T3444A_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_T3444A_DRIVE	0
#define SIG_T3444A_TND		1
#define SIG_T3444A_MOTOR	2

// for reading signal
#define SIG_T3444A_DRDY		4
#define SIG_T3444A_CRDY		5
#define SIG_T3444A_RQM		6
#if defined(USE_SOUND_FILES)
#define T3444A_SND_TBL_MAX 256
#ifndef SIG_SOUNDER_MUTE
#define SIG_SOUNDER_MUTE    	(65536 + 0)
#endif
#ifndef SIG_SOUNDER_RELOAD
#define SIG_SOUNDER_RELOAD    	(65536 + 32)
#endif
#ifndef SIG_SOUNDER_ADD
#define SIG_SOUNDER_ADD     	(65536 + 64)
#endif

#define T3444A_SND_TYPE_SEEK 0
#define T3444A_SND_TYPE_HEAD 1
#endif

#ifdef HAS_T3444M
#define SECTORS_IN_TRACK	16
#else
#define SECTORS_IN_TRACK	26
#endif

class DISK;

class T3444A : public DEVICE
{
private:
	// output signals
	outputs_t outputs_rqm;
	
	// drive info
	struct {
		int track;
		int index;
		bool access;
		// timing
		int cur_position;
		int next_trans_position;
		int bytes_before_2nd_rqm;
		int next_sync_position;
		uint32_t prev_clock;
	} fdc[4];
	DISK* disk[4];
	
	// register
	uint8_t status;
	uint8_t cmdreg;
	uint8_t trkreg;
	uint8_t secreg;
	uint8_t datareg;
	uint8_t drvreg;
	uint8_t sidereg;
	bool timerflag;
	uint8_t sector_id[SECTORS_IN_TRACK * 4];
	
	// event
	int register_id[5];
	
	void cancel_my_event(int event);
	void register_my_event(int event, double usec);
	void register_seek_event();
	void register_rqm_event(int bytes);
	void register_lost_event(int bytes);
	
	// status
	bool now_search;
	int seektrk;
	bool rqm;
	bool tnd;
	bool motor_on;
	
	// timing
	uint32_t prev_rqm_clock;
	
	int get_cur_position();
	double get_usec_to_start_trans();
	double get_usec_to_next_trans_pos();
	double get_usec_to_detect_index_hole(int count);
	
	// image handler
	uint8_t search_sector();
	
	// command
	void process_cmd();
	void cmd_seek_zero();
	void cmd_seek();
	void cmd_read_write();
	void cmd_write_id();
	void cmd_sence();
	
	// rqm
	void set_rqm(bool val);
	
#if defined(USE_SOUND_FILES)
	_TCHAR snd_seek_name[512];
	_TCHAR snd_head_name[512];
	int snd_seek_mix_tbl[T3444A_SND_TBL_MAX];
	int snd_head_mix_tbl[T3444A_SND_TBL_MAX];
	int16_t *snd_seek_data; // Read only
	int16_t *snd_head_data; // Read only
	int snd_seek_samples_size;
	int snd_head_samples_size;
	bool snd_mute;
	int snd_level_l, snd_level_r;
	virtual void mix_main(int32_t *dst, int count, int16_t *src, int *table, int samples);
	void add_sound(int type);
#endif
public:
	T3444A(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_rqm);
		tnd = true;
		motor_on = false;
#if defined(USE_SOUND_FILES)
		for(int i = 0; i < T3444A_SND_TBL_MAX; i++) {
			snd_seek_mix_tbl[i] = -1;
			snd_head_mix_tbl[i] = -1;
		}
		snd_seek_data = snd_head_data = NULL;
		snd_seek_samples_size = snd_head_samples_size = 0;
		snd_mute = false;
		snd_level_l = snd_level_r = decibel_to_volume(0);
		memset(snd_seek_name, 0x00, sizeof(snd_seek_name));
		memset(snd_head_name, 0x00, sizeof(snd_head_name));
#endif
		set_device_name(_T("T3444A FDC"));
	}
	~T3444A() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_dma_io8(uint32_t addr, uint32_t data);
	uint32_t read_dma_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int ch);
	void event_callback(int event_id, int err);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_rqm(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_rqm, device, id, mask);
	}
	DISK* get_disk_handler(int drv)
	{
		return disk[drv];
	}
#if defined(USE_SOUND_FILES)
	// Around SOUND. 20161004 K.O
	bool load_sound_data(int type, const _TCHAR *pathname);
	void release_sound_data(int type);
	bool reload_sound_data(int type);
	
	void mix(int32_t *buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
#endif
	void open_disk(int drv, const _TCHAR* file_path, int bank);
	void close_disk(int drv);
	bool is_disk_inserted(int drv);
	void is_disk_protected(int drv, bool value);
	bool is_disk_protected(int drv);
	void set_drive_type(int drv, uint8_t type);
	uint8_t get_drive_type(int drv);
	void set_drive_rpm(int drv, int rpm);
	void set_drive_mfm(int drv, bool mfm);
};

#endif