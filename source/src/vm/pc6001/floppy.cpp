//
// PC-6001/6601 disk I/O
// This file is based on a disk I/O program in C++
// by Mr. Yumitaro and translated into C for Cocoa iP6
// by Koichi NISHIDA 2006
//

/*
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : tanam
	Date   : 2013.12.04-

	[ internal floppy drive ]
*/

#include "floppy.h"
#include "../disk.h"
#if defined(USE_SOUND_FILES)
#define EVENT_SEEK_SOUND 2
#endif

void FLOPPY::event_callback(int event_id, int err)
{
#if defined(USE_SOUND_FILES)
	if((event_id >= EVENT_SEEK_SOUND) && (event_id < (EVENT_SEEK_SOUND + 2))) {
		int drvno = event_id - EVENT_SEEK_SOUND;
		if(cur_trk[drvno] < seek_track_num[drvno]) {
			seek_track_num[drvno] -= 2;
			register_event(this, EVENT_SEEK_SOUND + drvno, 16000, false, &seek_event_id[drvno]);
		} else if(cur_trk[drvno] > seek_track_num[drvno]) {
			seek_track_num[drvno] += 2;
			register_event(this, EVENT_SEEK_SOUND + drvno, 16000, false, &seek_event_id[drvno]);
		} else {
			seek_event_id[drvno] = -1;
		}
		add_sound(FLOPPY_SND_TYPE_SEEK);
	}
#endif
}

int FLOPPY::Seek88(int drvno, int trackno, int sectno)
{
	if(drvno < 2) {
#if defined(USE_SOUND_FILES)
		if(cur_trk[drvno] != trackno) {
			seek_track_num[drvno] = (cur_trk[drvno] & 0xfe);
			if(seek_event_id[drvno] >= 0) {
				cancel_event(this, seek_event_id[drvno]);
			}
			register_event(this, EVENT_SEEK_SOUND + drvno, 16000, false, &seek_event_id[drvno]);
		}
#endif
		cur_trk[drvno] = trackno;
		cur_sct[drvno] = sectno;
		cur_pos[drvno] = 0;
		
		if(disk[drvno]->get_track(trackno >> 1, trackno & 1)) {
			for(int i = 0; i < disk[drvno]->sector_num.sd; i++) {
				if(disk[drvno]->get_sector(trackno >> 1, 0/*trackno & 1*/, i)) {
					if(disk[drvno]->id[2] == sectno) {
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

unsigned char FLOPPY::Getc88(int drvno)
{
	if(drvno < 2 && disk[drvno]->sector != NULL) {
		if(cur_pos[drvno] >= disk[drvno]->sector_size.sd) {
			cur_sct[drvno]++;
			if(!Seek88(drvno, cur_trk[drvno], cur_sct[drvno])) {
//				cur_trk[drvno]++;
				cur_trk[drvno] += 2;
				cur_sct[drvno] = 1;
				if(!Seek88(drvno, cur_trk[drvno], cur_sct[drvno])) {
					return 0xff;
				}
			}
		}
		access[drvno] = true;
		return disk[drvno]->sector[cur_pos[drvno]++];
	}
	return 0xff;
}

int FLOPPY::Putc88(int drvno, unsigned char dat)
{
	if(drvno < 2 && disk[drvno]->sector != NULL) {
		if(cur_pos[drvno] >= disk[drvno]->sector_size.sd) {
			cur_sct[drvno]++;
			if(!Seek88(drvno, cur_trk[drvno], cur_sct[drvno])) {
//				cur_trk[drvno]++;
				cur_trk[drvno] += 2;
				cur_sct[drvno] = 1;
				if(!Seek88(drvno, cur_trk[drvno], cur_sct[drvno])) {
					return 0xff;
				}
			}
		}
		access[drvno] = true;
		disk[drvno]->sector[cur_pos[drvno]++] = dat;
		return 1;
	}
	return 0;
}

// push data to data buffer
void FLOPPY::Push(int part, unsigned char data)
{
	if (part > 3) return;
	
	if(Index[part] < 256) Data[part][Index[part]++] = data;
}

// pop data from data buffer
unsigned char FLOPPY::Pop(int part)
{
	if(part > 3) return 0xff;
	
	if(Index[part] > 0) return Data[part][--Index[part]];
	else                return 0xff;
}

// clear data
void FLOPPY::Clear(int i)
{
	Index[i] = 0;
}

// FDC Status
#define FDC_BUSY			(0x10)
#define FDC_READY			(0x00)
#define FDC_NON_DMA			(0x20)
#define FDC_FD2PC			(0x40)
#define FDC_PC2FD			(0x00)
#define FDC_DATA_READY		(0x80)

// Result Status 0
#define ST0_NOT_READY		(0x08)
#define ST0_EQUIP_CHK		(0x10)
#define ST0_SEEK_END		(0x20)
#define ST0_IC_NT			(0x00)
#define ST0_IC_AT			(0x40)
#define ST0_IC_IC			(0x80)
#define ST0_IC_AI			(0xc0)

// Result Status 1
#define ST1_NOT_WRITABLE	(0x02)

// Result Status 2

// Result Status 3
#define ST3_TRACK0			(0x10)
#define ST3_READY			(0x20)
#define ST3_WRITE_PROTECT	(0x40)
#define ST3_FAULT			(0x80)

// initialise
int FLOPPY::DiskInit66(void)
{
	memset( &CmdIn,  0, sizeof( CmdBuffer ) );
	memset( &CmdOut, 0, sizeof( CmdBuffer ) );
	SeekST0 = 0;
	LastCylinder = 0;
	SeekEnd = 0;
	SendSectors  = 0;
	Status = FDC_DATA_READY | FDC_READY | FDC_PC2FD;
	return 1;
}

// push data to status buffer
void FLOPPY::PushStatus(int data)
{
	CmdOut.Data[CmdOut.Index++] = data;
}

// pop data from status buffer
unsigned char FLOPPY::PopStatus()
{
	return CmdOut.Data[--CmdOut.Index];
}

// write to FDC
void FLOPPY::OutFDC(unsigned char data)
{
	const int CmdLength[] = { 0,0,0,3,2,9,9,2,1,0,0,0,0,6,0,3 };
	
	CmdIn.Data[CmdIn.Index++] = data;
	if (CmdLength[CmdIn.Data[0]&0xf] == CmdIn.Index) Exec();
}

// read from FDC
unsigned char FLOPPY::InFDC()
{
	if (CmdOut.Index == 1) Status = FDC_DATA_READY | FDC_PC2FD;
	return PopStatus();
}

// read
void FLOPPY::Read()
{
	int Drv, C, H, R, N;
	int i, j;
	
	Drv = CmdIn.Data[1]&3;		// drive number No.(0-3)
	C   = CmdIn.Data[2];		// cylinder
	H   = CmdIn.Data[3];		// head address
	R   = CmdIn.Data[4];		// sector No.
	N   = CmdIn.Data[5] ? CmdIn.Data[5]*256 : 256;	// sector size
	
	if (disk[Drv]->inserted) {
		// seek
		// double track number(1D->2D)
		Seek88(Drv, C*2+H, R);
		for (i = 0; i < SendSectors; i++) {
			Clear(i);
			for(j=0; j<N; j++)
				Push(i, Getc88(Drv));
		}
	}
	PushStatus(N);	// N
	PushStatus(R);	// R
	PushStatus(H);	// H
	PushStatus(C);	// C
	PushStatus(0);	// st2
	PushStatus(0);	// st1
	PushStatus(disk[Drv]->inserted ? 0 : ST0_NOT_READY);	// st0  bit3 : media not ready
	Status = FDC_DATA_READY | FDC_FD2PC;
}

// Write
void FLOPPY::Write(void)
{
	int Drv, C, H, R, N;
	int i, j;
	
	Drv = CmdIn.Data[1]&3;		// drive No.(0-3)
	C   = CmdIn.Data[2];		// cylinder
	H   = CmdIn.Data[3];		// head address
	R   = CmdIn.Data[4];		// sector No.
	N   = CmdIn.Data[5] ? CmdIn.Data[5]*256 : 256;	// sector size
	
	if (disk[Drv]->inserted) {
		// seek
		// double track number(1D->2D)
		Seek88(Drv, C*2+H, R);
		for (i=0; i<SendSectors; i++) {
			for(j=0; j<0x100; j++)
				Putc88(Drv, Pop(i));	// write data
		}
	}
	
	PushStatus(N);	// N
	PushStatus(R);	// R
	PushStatus(H);	// H
	PushStatus(C);	// C
	PushStatus(0);	// st2
	PushStatus(0);	// st1
	
	PushStatus(disk[Drv]->inserted ? 0 : ST0_NOT_READY);	// st0  bit3 : media not ready
	
	Status = FDC_DATA_READY | FDC_FD2PC;
}

// seek
void FLOPPY::Seek(void)
{
	int Drv,C,H;
	
	Drv = CmdIn.Data[1]&3;		// drive No.(0-3)
	C   = CmdIn.Data[2];		// cylinder
	H   = CmdIn.Data[3];		// head address
	
	if (!disk[Drv]->inserted) {	// disk unmounted ?
		SeekST0      = ST0_IC_AT | ST0_SEEK_END | ST0_NOT_READY | Drv;
		SeekEnd      = 0;
		LastCylinder = 0;
	} else { // seek
		// double number(1D->2D)
		Seek88(Drv, C*2+H, 1);
		SeekST0      = ST0_IC_NT | ST0_SEEK_END | Drv;
		SeekEnd      = 1;
		LastCylinder = C;
	}
}

// sense interrupt status
void FLOPPY::SenseInterruptStatus(void)
{
	if (SeekEnd) {
		SeekEnd = 0;
		PushStatus(LastCylinder);
		PushStatus(SeekST0);
	} else {
		PushStatus(0);
		PushStatus(ST0_IC_IC);
	}
	
	Status = FDC_DATA_READY | FDC_FD2PC;
}

// execute FDC command
void FLOPPY::Exec()
{
	CmdOut.Index = 0;
	switch (CmdIn.Data[0] & 0xf) {
	case 0x03:	// Specify
		break;
	case 0x05:	// Write Data
		Write();
		break;
	case 0x06:	// Read Data
		Read();
		break;
	case 0x08:	// Sense Interrupt Status
		SenseInterruptStatus();
		break;
	case 0x0d:	// Write ID
		// Format is Not Implimented
		break;
	case 0x07:	// Recalibrate
		CmdIn.Data[2] = 0;	// Seek to TRACK0
	case 0x0f:	// Seek
		Seek();
		break;
	default: ;	// Invalid
	}
	CmdIn.Index = 0;
}

// I/O access functions
void FLOPPY::OutB1H_66(unsigned char data) { DIO = data&2 ? 1 : 0; }			// FD mode
void FLOPPY::OutB2H_66(unsigned char data) {}									// FDC INT?
void FLOPPY::OutB3H_66(unsigned char data) {}									// in out of PortB2h
void FLOPPY::OutD0H_66(unsigned char data) { Push(0, data); }					// Buffer
void FLOPPY::OutD1H_66(unsigned char data) { Push(1, data); }					// Buffer
void FLOPPY::OutD2H_66(unsigned char data) { Push(2, data); }					// Buffer
void FLOPPY::OutD3H_66(unsigned char data) { Push(3, data); }					// Buffer
void FLOPPY::OutD6H_66(unsigned char data) {}									// select drive
void FLOPPY::OutD8H_66(unsigned char data) {}									//
void FLOPPY::OutDAH_66(unsigned char data) { SendSectors = ~(data - 0x10); }	// set transfer amount
void FLOPPY::OutDDH_66(unsigned char data) { OutFDC(data); }					// FDC data register
void FLOPPY::OutDEH_66(unsigned char data) {}									// ?
	
unsigned char FLOPPY::InB2H_66() { return 3; }									// FDC INT
unsigned char FLOPPY::InD0H_66() { return Pop(0); }								// Buffer
unsigned char FLOPPY::InD1H_66() { return Pop(1); }								// Buffer
unsigned char FLOPPY::InD2H_66() { return Pop(2); }								// Buffer
unsigned char FLOPPY::InD3H_66() { return Pop(3); }								// Buffer
unsigned char FLOPPY::InD4H_66() { return 0; }									// Mortor(on 0/off 1)
unsigned char FLOPPY::InDCH_66() { return Status; }								// FDC status register
unsigned char FLOPPY::InDDH_66() { return InFDC(); }							// FDC data register

void FLOPPY::initialize()
{
	for(int i = 0; i < 2; i++) {
		disk[i] = new DISK(emu);
	}
	DiskInit66();
}

void FLOPPY::release()
{
	for(int i = 0; i < 2; i++) {
		if(disk[i]) {
			disk[i]->close();
			delete disk[i];
		}
	}
}

void FLOPPY::reset()
{
	io_B1H = 0;
	memset(Index, 0, sizeof(Index));
}

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	// disk I/O
	uint16_t port=(addr & 0x00ff);
	uint8_t Value=(data & 0xff);
	
	switch(port)
	{
	// disk I/O
	case 0xB1:
	case 0xB5:
		io_B1H = Value;
		break;
	case 0xB2:
	case 0xB6:
		OutB2H_66(Value);
		break;
	case 0xB3:
	case 0xB7:
		OutB3H_66(Value);
		break;
	case 0xD0:
		if(io_B1H & 4) {
			d_ext->write_io8(0, data);
		} else {
			OutD0H_66(Value);
		}
		break;
	case 0xD1:
		if(io_B1H & 4) {
			d_ext->write_io8(1, data);
		} else {
			OutD1H_66(Value);
		}
		break;
	case 0xD2:
		if(io_B1H & 4) {
			d_ext->write_io8(2, data);
		} else {
			OutD2H_66(Value);
		}
		break;
	case 0xD3:
		if(io_B1H & 4) {
			d_ext->write_io8(3, data);
		} else {
			OutD3H_66(Value);
		}
		break;
	case 0xD4:
		if(io_B1H & 4) {
			d_ext->write_io8(0, data);
		}
		break;
	case 0xD5:
		if(io_B1H & 4) {
			d_ext->write_io8(1, data);
		}
		break;
	case 0xD6:
		if(io_B1H & 4) {
			d_ext->write_io8(2, data);
		} else {
			OutD6H_66(Value);
		}
		break;
	case 0xD7:
		if(io_B1H & 4) {
			d_ext->write_io8(3, data);
		}
		break;
	case 0xD8:
		OutD8H_66(Value);
		break;
	case 0xDA:
		OutDAH_66(Value);
		break;
	case 0xDD:
		OutDDH_66(Value);
		break;
	case 0xDE:
		OutDEH_66(Value);
		break;
	}
	return;
}

uint32_t FLOPPY::read_io8(uint32_t addr)
{
	// disk I/O
	uint16_t port=(addr & 0x00ff);
	uint8_t Value=0xff;
	
	switch(addr & 0xff) {
	case 0xB2:
	case 0xB6:
		Value=InB2H_66();
		break;
	case 0xD0:
		if(io_B1H & 4) {
			Value=d_ext->read_io8(0);
		} else {
			Value=InD0H_66();
		}
		break;
	case 0xD1:
		if(io_B1H & 4) {
			Value=d_ext->read_io8(1);
		} else {
			Value=InD1H_66();
		}
		break;
	case 0xD2:
		if(io_B1H & 4) {
			Value=d_ext->read_io8(2);
		} else {
			Value=InD2H_66();
		}
		break;
	case 0xD3:
		if(io_B1H & 4) {
			Value=d_ext->read_io8(3);
		} else {
			Value=InD3H_66();
		}
		break;
	case 0xD4:
		if(io_B1H & 4) {
			Value=d_ext->read_io8(0);
		} else {
			Value=InD4H_66();
		}
		break;
	case 0xD5:
		if(io_B1H & 4) {
			Value=d_ext->read_io8(1);
		}
		break;
	case 0xD6:
		if(io_B1H & 4) {
			Value=d_ext->read_io8(2);
		}
		break;
	case 0xD7:
		if(io_B1H & 4) {
			Value=d_ext->read_io8(3);
		}
		break;
	case 0xDC:
		Value=InDCH_66();
		break;
	case 0xDD:
		Value=InDDH_66();
		break;
	}
	return(Value);
}

uint32_t FLOPPY::read_signal(int ch)
{
	// get access status
	uint32_t stat = 0;
	for(int drv = 0; drv < 2; drv++) {
		if(access[drv]) {
			stat |= 1 << drv;
		}
		access[drv] = false;
	}
	return stat;
}

void FLOPPY::open_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < 2) {
		disk[drv]->open(file_path, bank);
		Seek88(drv, 0, 1);
	}
}

void FLOPPY::close_disk(int drv)
{
	if(drv < 2 && disk[drv]->inserted) {
		disk[drv]->close();
	}
}

bool FLOPPY::is_disk_inserted(int drv)
{
	if(drv < 2) {
		return disk[drv]->inserted;
	}
	return false;
}

void FLOPPY::is_disk_protected(int drv, bool value)
{
	if(drv < 2) {
		disk[drv]->write_protected = value;
	}
}

bool FLOPPY::is_disk_protected(int drv)
{
	if(drv < 2) {
		return disk[drv]->write_protected;
	}
	return false;
}

#if defined(USE_SOUND_FILES)
void FLOPPY::add_sound(int type)
{
	int *p;
	if(type == FLOPPY_SND_TYPE_SEEK) {
		p = snd_seek_mix_tbl;
	} else if(type == FLOPPY_SND_TYPE_HEAD) {
		p = snd_head_mix_tbl;
	} else {
		return;
	}
	for(int i = 0; i < FLOPPY_SND_TBL_MAX; i++) {
		if(p[i] < 0) {
			p[i] = 0;
			break;
		}
	}
}

bool FLOPPY::load_sound_data(int type, const _TCHAR *pathname)
{
	if((type < 0) || (type > 1)) return false;
	int16_t *data = NULL;
	int dst_size = 0;
	int id = (this_device_id << 8) + type;
	const _TCHAR *sp;
	sp = create_local_path(pathname);
	emu->load_sound_file(id, sp, &data, &dst_size);
	if((dst_size <= 0) || (data == NULL)) { // Failed
		this->out_debug_log("ID=%d : Failed to load SOUND FILE for %s:%s", id, (type == 0) ? _T("SEEK") : _T("HEAD") ,pathname);
		return false;
	} else {
		int utl_size = dst_size * 2 * sizeof(int16_t);
		int alloc_size = utl_size + 64;
		switch(type) {
		case FLOPPY_SND_TYPE_SEEK: // SEEK
			snd_seek_data = (int16_t *)malloc(alloc_size);
			memcpy(snd_seek_data, data, utl_size);
			strncpy(snd_seek_name, pathname, 511);
			snd_seek_samples_size = dst_size;
			break;
		case FLOPPY_SND_TYPE_HEAD: // HEAD
			snd_seek_data = (int16_t *)malloc(alloc_size);
			memcpy(snd_head_data, data, utl_size);
			strncpy(snd_head_name, pathname, 511);
			snd_head_samples_size = dst_size;
			break;
		default:
			this->out_debug_log("ID=%d : Illegal type (%d): 0 (SEEK SOUND) or 1 (HEAD SOUND) is available.",
								id, type);
			return false;
		}
		this->out_debug_log("ID=%d : Success to load SOUND FILE for %s:%s",
							id, (type == 0) ? _T("SEEK") : _T("HEAD") ,
							pathname);
	}
	return true;
}

void FLOPPY::release_sound_data(int type)
{
	switch(type) {
	case FLOPPY_SND_TYPE_SEEK: // SEEK
		if(snd_seek_data != NULL) free(snd_seek_data);
		memset(snd_seek_name, 0x00, sizeof(snd_seek_name));
		snd_seek_data = NULL;
		break;
	case FLOPPY_SND_TYPE_HEAD: // HEAD
		if(snd_head_data != NULL) free(snd_head_data);
		memset(snd_head_name, 0x00, sizeof(snd_head_name));
		snd_head_data = NULL;
			break;
	default:
		break;
	}
}

bool FLOPPY::reload_sound_data(int type)
{
	switch(type) {
	case FLOPPY_SND_TYPE_SEEK: // SEEK
		if(snd_seek_data != NULL) free(snd_seek_data);
		break;
	case FLOPPY_SND_TYPE_HEAD:
		if(snd_seek_data != NULL) free(snd_seek_data);
		break;
	default:
		return false;
		break;
	}
	_TCHAR *p = (type == FLOPPY_SND_TYPE_SEEK) ? snd_seek_name : snd_head_name;
    _TCHAR tmps[512];
	strncpy(tmps, p, 511);
	return load_sound_data(type, tmps);
}

void FLOPPY::mix_main(int32_t *dst, int count, int16_t *src, int *table, int samples)
{
	int ptr, pp;
	int i, j, k;
	int32_t data[2];
	int32_t *dst_tmp;
	for(i=0; i < FLOPPY_SND_TBL_MAX; i++) {
		ptr = table[i];
		if(ptr >= 0) {
			if(ptr < samples) {
				if(!snd_mute) {
					pp = ptr << 1;
					dst_tmp = dst;
					k = 0;
					for(j = 0; j < count; j++) {
						if(ptr >= samples) {
							break;
						}
						data[0] = (int32_t)src[pp + 0];
						data[1] = (int32_t)src[pp + 1];
						dst_tmp[k + 0] += apply_volume((int32_t)data[0], snd_level_l);
						dst_tmp[k + 1] += apply_volume((int32_t)data[1], snd_level_r);
						k += 2;
						pp += 2;
						ptr++;
					}
				} else {
					ptr += count;
				}
			}
			if(ptr >= samples) {
				table[i] = -1;
			} else {
				table[i] = ptr;
			}
		}
	}
}

void FLOPPY::mix(int32_t *buffer, int cnt)
{
	if(snd_seek_data != NULL) mix_main(buffer, cnt, snd_seek_data, snd_seek_mix_tbl, snd_seek_samples_size);
	if(snd_head_data != NULL) mix_main(buffer, cnt, snd_head_data, snd_head_mix_tbl, snd_head_samples_size);
}

void FLOPPY::set_volume(int ch, int decibel_l, int decibel_r)
{
	snd_level_l = decibel_to_volume(decibel_l);
	snd_level_r = decibel_to_volume(decibel_r);
}
#endif

#define STATE_VERSION	2

void FLOPPY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(io_B1H);
	for(int i = 0; i < 2; i++) {
		disk[i]->save_state(state_fio);
	}
	state_fio->Fwrite(cur_trk, sizeof(cur_trk), 1);
	state_fio->Fwrite(cur_sct, sizeof(cur_sct), 1);
	state_fio->Fwrite(cur_pos, sizeof(cur_pos), 1);
	state_fio->Fwrite(access, sizeof(access), 1);
	state_fio->Fwrite(Data, sizeof(Data), 1);
	state_fio->Fwrite(Index, sizeof(Index), 1);
	state_fio->Fwrite(&CmdIn, sizeof(CmdBuffer), 1);
	state_fio->Fwrite(&CmdOut, sizeof(CmdBuffer), 1);

	state_fio->FputUint8(SeekST0);
	state_fio->FputUint8(LastCylinder);
	state_fio->FputInt32(SeekEnd);
	state_fio->FputUint8(SendSectors);
	state_fio->FputInt32(DIO);
	state_fio->FputUint8(Status);
#if defined(USE_SOUND_FILES)
	for(int i = 0; i < 2; i++) {
		state_fio->FputInt32(seek_event_id[i]);
		state_fio->FputInt32(seek_track_num[i]);
	}
	state_fio->Fwrite(snd_seek_name, sizeof(snd_seek_name), 1);
	state_fio->Fwrite(snd_head_name, sizeof(snd_head_name), 1);
	for(int i = 0; i < FLOPPY_SND_TBL_MAX; i++) {
		state_fio->FputInt32(snd_seek_mix_tbl[i]);
	}
	for(int i = 0; i < FLOPPY_SND_TBL_MAX; i++) {
		state_fio->FputInt32(snd_head_mix_tbl[i]);
	}
	state_fio->FputBool(snd_mute);
	state_fio->FputInt32(snd_level_l);
	state_fio->FputInt32(snd_level_r);
#endif
}

bool FLOPPY::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	io_B1H = state_fio->FgetUint8();
	for(int i = 0; i < 2; i++) {
		if(!disk[i]->load_state(state_fio)) {
			return false;
		}
	}
	state_fio->Fread(cur_trk, sizeof(cur_trk), 1);
	state_fio->Fread(cur_sct, sizeof(cur_sct), 1);
	state_fio->Fread(cur_pos, sizeof(cur_pos), 1);
	state_fio->Fread(access, sizeof(access), 1);
	state_fio->Fread(Data, sizeof(Data), 1);
	state_fio->Fread(Index, sizeof(Index), 1);
	state_fio->Fread(&CmdIn, sizeof(CmdBuffer), 1);
	state_fio->Fread(&CmdOut, sizeof(CmdBuffer), 1);
	SeekST0 = state_fio->FgetUint8();
	LastCylinder = state_fio->FgetUint8();
	SeekEnd = state_fio->FgetInt32();
	SendSectors = state_fio->FgetUint8();
	DIO = state_fio->FgetInt32();
	Status = state_fio->FgetUint8();
#if defined(USE_SOUND_FILES)
	for(int i = 0; i < 2; i++) {
		seek_event_id[i] = state_fio->FgetInt32();
		seek_track_num[i] = state_fio->FgetInt32();
	}
	state_fio->Fread(snd_seek_name, sizeof(snd_seek_name), 1);
	state_fio->Fread(snd_head_name, sizeof(snd_head_name), 1);
	for(int i = 0; i < FLOPPY_SND_TBL_MAX; i++) {
		snd_seek_mix_tbl[i] = state_fio->FgetInt32();
	}
	for(int i = 0; i < FLOPPY_SND_TBL_MAX; i++) {
		snd_head_mix_tbl[i] = state_fio->FgetInt32();
	}
	snd_mute = state_fio->FgetBool();
	snd_level_l = state_fio->FgetInt32();
	snd_level_r = state_fio->FgetInt32();
	if(snd_seek_data != NULL) free(snd_seek_data);
	if(snd_head_data != NULL) free(snd_head_data);
	if(strlen(snd_seek_name) > 0) {
		_TCHAR tmps[512];
		strncpy(tmps, snd_seek_name, 511);
		load_sound_data(FLOPPY_SND_TYPE_SEEK, (const _TCHAR *)tmps);
	}
	if(strlen(snd_head_name) > 0) {
		_TCHAR tmps[512];
		strncpy(tmps, snd_head_name, 511);
		load_sound_data(FLOPPY_SND_TYPE_HEAD, (const _TCHAR *)tmps);
	}
#endif
	return true;
}

