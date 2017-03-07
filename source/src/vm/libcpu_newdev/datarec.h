/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ data recorder ]
*/

#ifndef _DREC_H_
#define _DREC_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_DATAREC_MIC		0
#define SIG_DATAREC_REMOTE	1
#define SIG_DATAREC_TRIG	2

#if defined(USE_SOUND_FILES)
#ifndef SIG_SOUNDER_MUTE
#define SIG_SOUNDER_MUTE    	(65536 + 0)
#endif
#ifndef SIG_SOUNDER_RELOAD
#define SIG_SOUNDER_RELOAD    	(65536 + 32)
#endif
#ifndef SIG_SOUNDER_ADD
#define SIG_SOUNDER_ADD     	(65536 + 64)
#endif
enum {
	DATAREC_SNDFILE_RELAY_ON = 0,
	DATAREC_SNDFILE_RELAY_OFF,
	DATAREC_SNDFILE_PLAY,
	DATAREC_SNDFILE_STOP,
	DATAREC_SNDFILE_EJECT,
	DATAREC_SNDFILE_FF,
	DATAREC_SNDFILE_REW,
	DATAREC_SNDFILE_END,
};
#define DATAREC_SND_TBL_MAX 256
#endif

class FILEIO;
class EMU;
class DATAREC : public DEVICE
{
private:
	// output signals
	outputs_t outputs_ear;
	outputs_t outputs_remote;
	outputs_t outputs_rotate;
	outputs_t outputs_end;
	outputs_t outputs_top;
	outputs_t outputs_apss;
	
	// data recorder
	FILEIO* play_fio;
	FILEIO* rec_fio;
	
	bool play, rec, remote, trigger;
	_TCHAR rec_file_path[_MAX_PATH];
	int ff_rew;
	bool in_signal, out_signal;
	uint32_t prev_clock;
	int positive_clocks, negative_clocks;
	int signal_changed;
	int register_id;
	
	int sample_rate;
	double sample_usec;
	int buffer_ptr, buffer_length;
	uint8_t *buffer, *buffer_bak;
#ifdef DATAREC_SOUND
	int sound_buffer_length;
	int16_t *sound_buffer, sound_sample;
#endif
#if defined(USE_SOUND_FILES)
	_TCHAR snd_file_names[DATAREC_SNDFILE_END][512];
	int snd_mix_tbls[DATAREC_SNDFILE_END][DATAREC_SND_TBL_MAX];
	int16_t *snd_datas[DATAREC_SNDFILE_END];
	int snd_samples[DATAREC_SNDFILE_END];
	bool snd_mute[DATAREC_SNDFILE_END];
	int snd_level_l[DATAREC_SNDFILE_END], snd_level_r[DATAREC_SNDFILE_END];

	virtual void mix_sndfiles(int ch, int32_t *dst, int cnt, int16_t *src, int samples);
	void add_sound(int type);
#endif

	bool is_wav, is_tap;
	double ave_hi_freq;
	
	int apss_buffer_length;
	bool *apss_buffer;
	int apss_ptr, apss_count, apss_remain;
	bool apss_signals;
	
	int pcm_changed;
	uint32_t pcm_prev_clock;
	int pcm_positive_clocks, pcm_negative_clocks;
	int pcm_max_vol;
	int32_t pcm_last_vol_l, pcm_last_vol_r;
	int pcm_volume_l, pcm_volume_r;
#ifdef DATAREC_SOUND
	int32_t sound_last_vol_l, sound_last_vol_r;
	int sound_volume_l, sound_volume_r;
#endif
	
	void update_event();
	void close_file();
	
	int load_wav_image(int offset);
	void save_wav_image();
	int load_t77_image();
	int load_tap_image();
	int load_mzt_image();
	int load_p6_image(bool is_p6t);
	int load_cas_image();
	int load_m5_cas_image();
	int load_msx_cas_image();
	
public:
	DATAREC(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_ear);
		initialize_output_signals(&outputs_remote);
		initialize_output_signals(&outputs_rotate);
		initialize_output_signals(&outputs_end);
		initialize_output_signals(&outputs_top);
		initialize_output_signals(&outputs_apss);
#ifdef DATAREC_PCM_VOLUME
		pcm_max_vol = DATAREC_PCM_VOLUME;
#else
		pcm_max_vol = 8000;
#endif
		pcm_volume_l = pcm_volume_r = 1024;
#ifdef DATAREC_SOUND
		sound_volume_l = sound_volume_r = 1024;
#endif
#if defined(USE_SOUND_FILES)
		for(int j = 0; j < DATAREC_SNDFILE_END; j++) {
			for(int i = 0; i < DATAREC_SND_TBL_MAX; i++) {
				snd_mix_tbls[j][i] = -1;
			}
			snd_datas[j] = NULL;
			snd_samples[j] = 0;
			snd_mute[j] = false;
			snd_level_l[j] = snd_level_r[j] = decibel_to_volume(0);
			memset(snd_file_names[j], 0x00, sizeof(_TCHAR) * 512);
		}
#endif
		set_device_name(_T("DATA RECORDER"));
	}
	~DATAREC() {}
	
	// common functions
	void initialize();
	void reset();
	void release();
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int ch)
	{
		return in_signal ? 1 : 0;
	}
	void event_frame();
	void event_callback(int event_id, int err);
#if defined(USE_SOUND_FILES)
	// Around SOUND. 20161004 K.O
	bool load_sound_data(int type, const _TCHAR *pathname);
	void release_sound_data(int type);
	bool reload_sound_data(int type);
	
#endif
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	// unique functions
	void initialize_sound(int rate, int volume)
	{
		pcm_max_vol = volume;
	}
	void set_context_ear(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_ear, device, id, mask);
	}
	void set_context_remote(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_remote, device, id, mask);
	}
	void set_context_rotate(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_rotate, device, id, mask);
	}
	void set_context_end(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_end, device, id, mask);
	}
	void set_context_top(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_top, device, id, mask);
	}
	void set_context_apss(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_apss, device, id, mask);
	}
	bool play_tape(const _TCHAR* file_path);
	bool rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted()
	{
		return (play || rec);
	}
	bool is_tape_playing()
	{
		return (remote && play);
	}
	bool is_tape_recording()
	{
		return (remote && rec);
	}
	int get_tape_position()
	{
		if(play && buffer_length > 0) {
			if(buffer_ptr >= buffer_length) {
				return 100;
			} else if(buffer_ptr <= 0) {
				return 0;
			} else {
				return (int)(((double)buffer_ptr / (double)buffer_length) * 100.0);
			}
		}
		return 0;
	}
	void set_remote(bool value);
	void set_ff_rew(int value);
	bool do_apss(int value);
	double get_ave_hi_freq();
};

#endif
