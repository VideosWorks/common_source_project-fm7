/*
	Systems Formulate BUBCOM80 Emulator 'eBUBCOM80'

	Author : Takeda.Toshiya
	Date   : 2018.05.09-

	[ cmt ]
*/

#include "cmt.h"
#include "../mc6850.h"

namespace BUBCOM80 {
void CMT::initialize()
{
	fio = new FILEIO();
	play = rec = remote = false;
}

void CMT::release()
{
	release_tape();
	delete fio;
}

void CMT::reset()
{
	close_tape();
	play = rec = remote = false;
}

void CMT::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_CMT_REMOTE) {
		remote = ((data & mask) != 0);
	} else if(id == SIG_CMT_OUT) {
		if(rec && remote) {
			// recv from sio
			buffer[bufcnt++] = data & mask;
			if(bufcnt >= BUFFER_SIZE) {
				fio->Fwrite(buffer, bufcnt, 1);
				bufcnt = 0;
			}
		}
	}
}

void CMT::play_tape(const _TCHAR* file_path)
{
	close_tape();
	
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fseek(0, FILEIO_SEEK_END);
		int size = (fio->Ftell() + 9) & (BUFFER_SIZE - 1);
		fio->Fseek(0, FILEIO_SEEK_SET);
		memset(buffer, 0, sizeof(buffer));
		fio->Fread(buffer, sizeof(buffer), 1);
		fio->Fclose();
		
		// send data to sio
		// this implement does not care the sio buffer size... :-(
		for(int i = 0; i < size; i++) {
			d_sio->write_signal(SIG_MC6850_RECV, buffer[i], 0xff);
		}
		play = true;
	}
}

void CMT::rec_tape(const _TCHAR* file_path)
{
	close_tape();
	
	if(fio->Fopen(file_path, FILEIO_READ_WRITE_NEW_BINARY)) {
		my_tcscpy_s(rec_file_path, _MAX_PATH, file_path);
		bufcnt = 0;
		rec = true;
	}
}

void CMT::close_tape()
{
	// close file
	release_tape();
	
	// clear sio buffer
	d_sio->write_signal(SIG_MC6850_CLEAR, 0, 0);
}

void CMT::release_tape()
{
	// close file
	if(fio->IsOpened()) {
		if(rec && bufcnt) {
			fio->Fwrite(buffer, bufcnt, 1);
		}
		fio->Fclose();
	}
	play = rec = false;
}

#define STATE_VERSION	3

bool CMT::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateBool(play);
	state_fio->StateBool(rec);
	state_fio->StateBool(remote);
	state_fio->StateBuffer(rec_file_path, sizeof(rec_file_path), 1);
	if(loading) {
		int length_tmp = state_fio->FgetInt32_LE();
		if(rec) {
			fio->Fopen(rec_file_path, FILEIO_READ_WRITE_NEW_BINARY);
			while(length_tmp != 0) {
				uint8_t buffer_tmp[1024];
				int length_rw = min(length_tmp, (int)sizeof(buffer_tmp));
				state_fio->Fread(buffer_tmp, length_rw, 1);
				if(fio->IsOpened()) {
					fio->Fwrite(buffer_tmp, length_rw, 1);
				}
				length_tmp -= length_rw;
			}
		}
	} else {
		if(rec && fio->IsOpened()) {
			int length_tmp = (int)fio->Ftell();
			fio->Fseek(0, FILEIO_SEEK_SET);
			state_fio->FputInt32_LE(length_tmp);
			while(length_tmp != 0) {
				uint8_t buffer_tmp[1024];
				int length_rw = min(length_tmp, (int)sizeof(buffer_tmp));
				fio->Fread(buffer_tmp, length_rw, 1);
				state_fio->Fwrite(buffer_tmp, length_rw, 1);
				length_tmp -= length_rw;
			}
		} else {
			state_fio->FputInt32_LE(0);
		}
	}
	state_fio->StateInt32(bufcnt);
	state_fio->StateBuffer(buffer, sizeof(buffer), 1);
	return true;
}
}
