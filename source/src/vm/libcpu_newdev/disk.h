/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.16-

	[ d88 image handler ]
*/

#ifndef _DISK_H_
#define _DISK_H_

#ifndef _ANY2D88
#include "vm.h"
#include "../emu.h"
#else
#include "../common.h"
#endif
#if defined(_USE_QT)
#include "../qt/gui/csp_logger.h"
#endif

#include <stdarg.h>

// d88 media type
#define MEDIA_TYPE_2D	0x00
#define MEDIA_TYPE_2DD	0x10
#define MEDIA_TYPE_2HD	0x20
#define MEDIA_TYPE_144	0x30
#define MEDIA_TYPE_UNK	0xff

#define DRIVE_TYPE_2D	MEDIA_TYPE_2D
#define DRIVE_TYPE_2DD	MEDIA_TYPE_2DD
#define DRIVE_TYPE_2HD	MEDIA_TYPE_2HD
#define DRIVE_TYPE_144	MEDIA_TYPE_144
#define DRIVE_TYPE_UNK	MEDIA_TYPE_UNK

// this value will be stored to the state file,
// so don't change these definitions
#define SPECIAL_DISK_X1TURBO_ALPHA	  1
#define SPECIAL_DISK_X1_BATTEN		  2
#define SPECIAL_DISK_FM7_GAMBLER	  11
#define SPECIAL_DISK_FM7_DEATHFORCE   12
#define SPECIAL_DISK_FM77AV_PSYOBLADE 13
#define SPECIAL_DISK_FM7_TAIYOU1 14
#define SPECIAL_DISK_FM7_TAIYOU2 15
#define SPECIAL_DISK_FM7_XANADU2_D 16
#define SPECIAL_DISK_FM7_RIGLAS 17

// d88 constant
#define DISK_BUFFER_SIZE	0x380000	// 3.5MB
#define TRACK_BUFFER_SIZE	0x080000	// 0.5MB

class FILEIO;

class DISK
{
#ifndef _ANY2D88
protected:
	EMU* emu;
#endif
private:
	uint8_t buffer[DISK_BUFFER_SIZE + TRACK_BUFFER_SIZE];
	_TCHAR orig_path[_MAX_PATH];
	_TCHAR dest_path[_MAX_PATH];
	pair_t file_size;
	int file_bank;
	uint32_t crc32;
	bool trim_required;
	
	bool is_1dd_image;
	bool is_solid_image;
	bool is_fdi_image;
	uint8_t fdi_header[4096];
	_TCHAR this_device_name[128];
	
	int solid_ncyl, solid_nside, solid_nsec, solid_size;
	bool solid_mfm;
	
	void set_sector_info(uint8_t *t);
	void trim_buffer();
	
	// teledisk image decoder (td0)
	bool teledisk_to_d88(FILEIO *fio);
	
	// imagedisk image decoder (imd)
	bool imagedisk_to_d88(FILEIO *fio);
	
	// cpdread image decoder (dsk)
	bool cpdread_to_d88(FILEIO *fio);
	
	// solid image decoder (fdi/tfd/2d/img/sf7)
	bool solid_to_d88(FILEIO *fio, int type, int ncyl, int nside, int nsec, int size, bool mfm);
public:
#ifndef _ANY2D88
	DISK(EMU* parent_emu) : emu(parent_emu)
#else
	DISK()
#endif
	{
		inserted = ejected = write_protected = changed = false;
		file_size.d = 0;
		sector_size.sd = sector_num.sd = 0;
		sector = NULL;
		drive_type = DRIVE_TYPE_UNK;
		drive_rpm = 0;
		drive_mfm = true;
		static int num = 0;
		drive_num = num++;
		_TCHAR strb[16];
		snprintf(strb, 16, "DISK%d", drive_num);
		set_device_name((const _TCHAR *)strb);
	}
	~DISK()
	{
#ifndef _ANY2D88
		if(inserted) {
			close();
		}
#endif
	}
	
	void open(const _TCHAR* file_path, int bank);
	void close();
#ifdef _ANY2D88
	bool open_as_1dd;
	bool open_as_256;
	void save_as_d88(const _TCHAR* file_path);
#endif
	bool get_track(int trk, int side);
	bool make_track(int trk, int side);
	bool get_sector(int trk, int side, int index);
	void set_deleted(bool value);
	void set_data_crc_error(bool value);
	void set_data_mark_missing();
	
	bool format_track(int trk, int side);
	void insert_sector(uint8_t c, uint8_t h, uint8_t r, uint8_t n, bool deleted, bool data_crc_error, uint8_t fill_data, int length);
	void sync_buffer();
	
	int get_rpm();
	int get_track_size();
	double get_usec_per_track();
	double get_usec_per_bytes(int bytes);
	int get_bytes_per_usec(double usec);
	bool check_media_type();
	
	bool inserted;
	bool ejected;
	bool write_protected;
	bool changed;
	uint8_t media_type;
	int is_special_disk;
	
	// track
	uint8_t track[TRACK_BUFFER_SIZE];
	pair_t sector_num;
	bool track_mfm;
	bool invalid_format;
	//bool no_skew;
	int cur_track, cur_side;
	
	int sync_position[512];
	int am1_position[512];
	int id_position[512];
	int data_position[512];
//	int gap3_size;
	
	// sector
	uint8_t* sector;
	pair_t sector_size;
	uint8_t id[6];
	uint8_t density;
	bool deleted;
	bool addr_crc_error;
	bool data_crc_error;
	
	// drive
	uint8_t drive_type;
	int drive_rpm;
	bool drive_mfm;
	int drive_num;
	bool correct_timing()
	{
#ifndef _ANY2D88
#if defined(_FM7) || defined(_FM8) || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
		if((is_special_disk == SPECIAL_DISK_FM7_TAIYOU1) || (is_special_disk == SPECIAL_DISK_FM7_TAIYOU2)) {
		   return true;
		}
#endif		
		if(drive_num < (int)array_length(config.correct_disk_timing)) {
			return config.correct_disk_timing[drive_num];
		}
#endif
		return false;
	}
	bool ignore_crc()
	{
#ifndef _ANY2D88
		if(drive_num < (int)array_length(config.ignore_disk_crc)) {
			return config.ignore_disk_crc[drive_num];
		}
#endif
		return false;
	}
	void set_device_name(const _TCHAR *p)
	{
		if(p == NULL) return;
		strncpy(this_device_name, p, 128);
		
	}
	void out_debug_log(const char *fmt, ...)
	{
		char strbuf[4096];
		va_list ap;
		
		va_start(ap, fmt);
		vsnprintf(strbuf, 4095, fmt, ap);
#if defined(_USE_QT)
		csp_logger->debug_log(CSP_LOG_DEBUG, -1, "[%s] %s", this_device_name, strbuf);
#else
		char strbuf2[4096];
		snprintf(strbuf2, 4095, "[%s] %s", this_device_name, strbuf);
		emu->out_debug_log("%s", strbuf2);
#endif
		va_end(ap);
	}
	// state
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif
