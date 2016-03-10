/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2016.03.01-

	[ SCSI base device ]
*/

#include "scsi_dev.h"
#include "../fifo.h"

#define EVENT_SEL	0
#define EVENT_PHASE	1
#define EVENT_REQ	2

void SCSI_DEV::initialize()
{
	buffer = new FIFO(SCSI_BUFFER_SIZE);
	phase = SCSI_PHASE_BUS_FREE;
}

void SCSI_DEV::release()
{
	buffer->release();
	delete buffer;
}

void SCSI_DEV::reset()
{
	data_bus = 0;
	sel_status = atn_status = ack_status = rst_status = false;
	selected = atn_pending = false;
	
	event_sel = event_phase = event_req = -1;
	set_phase(SCSI_PHASE_BUS_FREE);
}

inline int get_command_length(int value)
{
	switch((value >> 5) & 7) {
	case 0:
	case 3:
	case 6:
	case 7:
		return 6;
	case 1:
	case 2:
		return 10;
	case 5:
		return 12;
	}
	return 6;
}

void SCSI_DEV::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_SCSI_DAT:
		data_bus = data & mask;
		break;
		
	case SIG_SCSI_SEL:
		{
			bool prev_status = sel_status;
			sel_status = ((data & mask) != 0);
			
			if(!prev_status && sel_status) {
				// L -> H
				if(phase == SCSI_PHASE_BUS_FREE) {
#if 0
					event_callback(EVENT_SEL, 0);
#else
					if(event_sel != -1) {
						cancel_event(this, event_sel);
					}
					register_event(this, EVENT_SEL, 20.0, false, &event_sel);
#endif
				}
			} else if(prev_status && !sel_status) {
				// H -> L
				if(event_sel != -1) {
					cancel_event(this, event_sel);
					event_sel = -1;
				}
				if(selected) {
					if(atn_status) {
						// change to message out phase
						buffer->clear();
						set_phase_delay(SCSI_PHASE_MESSAGE_OUT, 10.0);
//						set_phase(SCSI_PHASE_MESSAGE_OUT);
					} else {
						// change to command phase
						memset(command, 0, sizeof(command));
						command_index = 0;
						set_phase_delay(SCSI_PHASE_COMMAND, 10.0);
//						set_phase(SCSI_PHASE_COMMAND);
					}
				}
			}
		}
		break;
		
	case SIG_SCSI_ATN:
		{
			bool prev_status = atn_status;
			atn_status = ((data & mask) != 0);
			
			if(phase == SCSI_PHASE_BUS_FREE) {
				// this device is not selected
			} else if(!prev_status && atn_status) {
				// L -> H
				#ifdef _SCSI_DEBUG_LOG
					emu->out_debug_log(_T("[SCSI_DEV:ID=%d] ATN signal raised\n"), scsi_id);
				#endif
				if(ack_status) {
					// wait until ack=off
					atn_pending = true;
				} else {
					// change to message out phase
					atn_pending = false;
					buffer->clear();
					set_phase_delay(SCSI_PHASE_MESSAGE_OUT, 10.0);
				}
			}
		}
		break;
		
	case SIG_SCSI_ACK:
		{
/*
			initiator --->	device

			wait req=on
					set req=on
					wait ack=on
			write data
			set ack=on
			wait req=off
					read data
					set req=off
					wait ack=off
			set ack=off

			initiator <---	device

			wait req=on
					write data
					set req=on
					wait ack=on
			read data
			set ack=on
			wait req=off
					set req=off
					wait ack=off
			set ack=off
*/
			bool prev_status = ack_status;
			ack_status = ((data & mask) != 0);
			
			if(phase == SCSI_PHASE_BUS_FREE) {
				// this device is not selected
			} else if(!prev_status & ack_status) {
				// L -> H
				switch(phase) {
				case SCSI_PHASE_DATA_OUT:
					buffer->write(data_bus);
					
					// check defect list length in format unit data
					if(command[0] == SCSI_CMD_FORMAT) {
						if(buffer->count() == 4) {
							remain += buffer->read_not_remove(2) * 256 + buffer->read_not_remove(3);
						}
					}
					break;
					
				case SCSI_PHASE_COMMAND:
					command[command_index++] = data_bus;
					break;
					
				case SCSI_PHASE_MESSAGE_OUT:
					buffer->write(data_bus);
					break;
				}
				set_req(0);
			} else if(prev_status && !ack_status) {
				// H -> L
				if(atn_pending) {
					// change to message out phase
					atn_pending = false;
					buffer->clear();
					set_phase_delay(SCSI_PHASE_MESSAGE_OUT, 10.0);
				} else {
					switch(phase) {
					case SCSI_PHASE_DATA_OUT:
						if(--remain > 0) {
							// flush buffer
							if(buffer->full()) {
								write_buffer(buffer->count());
							}
							// request to write next data
							switch(command[0]) {
							case SCSI_CMD_WRITE6:
							case SCSI_CMD_WRITE10:
							case SCSI_CMD_WRITE12:
								set_req_delay(1, 1000000.0 / bytes_per_sec);
								break;
							default:
								set_req_delay(1, 1.0);
								break;
							}
						} else {
							// flush buffer
							if(!buffer->empty()) {
								write_buffer(buffer->count());
							}
							// change to status phase
							set_dat(SCSI_STATUS_GOOD);
							set_phase_delay(SCSI_PHASE_STATUS, 10.0);
						}
						break;
						
					case SCSI_PHASE_DATA_IN:
						if(--remain > 0) {
							// update buffer
							if(buffer->count() == 0) {
								if(remain > SCSI_BUFFER_SIZE) {
									read_buffer(SCSI_BUFFER_SIZE);
								} else {
									read_buffer((int)remain);
								}
							}
							// request to read next data
							set_dat(buffer->read());
							switch(command[0]) {
							case SCSI_CMD_READ6:
							case SCSI_CMD_READ10:
							case SCSI_CMD_READ12:
								set_req_delay(1, 1000000.0 / bytes_per_sec);
								break;
							default:
								set_req_delay(1, 1.0);
								break;
							}
						} else {
							// change to status phase
							set_dat(SCSI_STATUS_GOOD);
							set_phase_delay(SCSI_PHASE_STATUS, 10.0);
						}
						break;
						
					case SCSI_PHASE_COMMAND:
						if(command_index < get_command_length(command[0])) {
							// request next command
							set_req_delay(1, 1.0);
						} else {
							// start command
							start_command();
						}
						break;
						
					case SCSI_PHASE_STATUS:
						// FIXME: message length is always 1byte?
						// change to message in phase
						set_dat(0x00); // command complete message
						set_phase_delay(SCSI_PHASE_MESSAGE_IN, 10.0);
						break;
						
					case SCSI_PHASE_MESSAGE_OUT:
						if((buffer->read() & 0xb8) == 0x80) {
							// identify, change to command phase
							memset(command, 0, sizeof(command));
							command_index = 0;
							set_phase_delay(SCSI_PHASE_COMMAND, 10.0);
						} else {
							// abort, change to bus free phase
							set_phase_delay(SCSI_PHASE_BUS_FREE, 10.0);
						}
						break;
						
					case SCSI_PHASE_MESSAGE_IN:
						// change to bus free phase
						set_phase_delay(SCSI_PHASE_BUS_FREE, 10.0);
						break;
					}
				}
			}
		}
		break;
		
	case SIG_SCSI_RST:
		{
			bool prev_status = rst_status;
			rst_status = ((data & mask) != 0);
			
			if(!prev_status & rst_status) {
				// L -> H
				#ifdef _SCSI_DEBUG_LOG
					emu->out_debug_log(_T("[SCSI_DEV:ID=%d] RST signal raised\n"), scsi_id);
				#endif
				set_phase(SCSI_PHASE_BUS_FREE);
			}
		}
		break;
	}
}

void SCSI_DEV::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_SEL:
		event_sel = -1;
		if((data_bus & 0x7f) == (1 << scsi_id)) {
			// this device is selected!
			#ifdef _SCSI_DEBUG_LOG
				emu->out_debug_log(_T("[SCSI_DEV:ID=%d] This device is selected\n"), scsi_id);
			#endif
			set_bsy(true);
			selected = true;
		}
		break;
		
	case EVENT_PHASE:
		event_phase = -1;
		set_phase(next_phase);
		break;
		
	case EVENT_REQ:
		event_req = -1;
		set_req(next_req);
		break;
	}
}

void SCSI_DEV::set_phase(int value)
{
	#ifdef _SCSI_DEBUG_LOG
		emu->out_debug_log(_T("[SCSI_DEV:ID=%d] Phase %s -> %s\n"), scsi_id, scsi_phase_name[phase], scsi_phase_name[value]);
	#endif
	if(event_phase != -1) {
		cancel_event(this, event_phase);
		event_phase = -1;
	}
	set_io (value & 1);
	set_msg(value & 2);
	set_cd (value & 4);
	
	if(value == SCSI_PHASE_BUS_FREE) {
		set_bsy(false);
		set_req(0);
		selected = false;
	} else {
//		set_bsy(true);
		set_req_delay(1, 10.0);
	}
	phase = value;
}

void SCSI_DEV::set_phase_delay(int value, double usec)
{
	if(usec <= 0.0) {
		set_phase(value);
	} else {
		if(event_phase != -1) {
			cancel_event(this, event_phase);
			event_phase = -1;
		}
		register_event(this, EVENT_PHASE, usec, false, &event_phase);
		next_phase = value;
	}
}

void SCSI_DEV::set_dat(int value)
{
	#ifdef _SCSI_DEBUG_LOG
//		emu->force_out_debug_log(_T("[SCSI_DEV:ID=%d] DATA = %02x\n"), scsi_id, value);
	#endif
	write_signals(&outputs_dat, value);
}

void SCSI_DEV::set_bsy(int value)
{
	#ifdef _SCSI_DEBUG_LOG
//		emu->out_debug_log(_T("[SCSI_DEV:ID=%d] BUSY = %d\n"), scsi_id, value ? 1 : 0);
	#endif
	write_signals(&outputs_bsy, value ? 0xffffffff : 0);
}

void SCSI_DEV::set_cd(int value)
{
	#ifdef _SCSI_DEBUG_LOG
//		emu->out_debug_log(_T("[SCSI_DEV:ID=%d] C/D = %d\n"), scsi_id, value ? 1 : 0);
	#endif
	write_signals(&outputs_cd,  value ? 0xffffffff : 0);
}

void SCSI_DEV::set_io(int value)
{
	#ifdef _SCSI_DEBUG_LOG
//		emu->out_debug_log(_T("[SCSI_DEV:ID=%d] I/O = %d\n"), scsi_id, value ? 1 : 0);
	#endif
	write_signals(&outputs_io,  value ? 0xffffffff : 0);
}

void SCSI_DEV::set_msg(int value)
{
	#ifdef _SCSI_DEBUG_LOG
//		emu->out_debug_log(_T("[SCSI_DEV:ID=%d] MSG = %d\n"), scsi_id, value ? 1 : 0);
	#endif
	write_signals(&outputs_msg, value ? 0xffffffff : 0);
}

void SCSI_DEV::set_req(int value)
{
	#ifdef _SCSI_DEBUG_LOG
//		emu->out_debug_log(_T("[SCSI_DEV:ID=%d] REQ = %d\n"), scsi_id, value ? 1 : 0);
	#endif
	if(event_req != -1) {
		cancel_event(this, event_req);
		event_req = -1;
	}
	write_signals(&outputs_req, value ? 0xffffffff : 0);
}

void SCSI_DEV::set_req_delay(int value, double usec)
{
	if(usec <= 0.0) {
		set_req(value);
	} else {
		if(event_req != -1) {
			cancel_event(this, event_req);
			event_req = -1;
		}
		register_event(this, EVENT_REQ, usec, false, &event_req);
		next_req = value;
	}
}

void SCSI_DEV::start_command()
{
	switch(command[0]) {
	case SCSI_CMD_TST_U_RDY:
		// XXX: this code invites the device (for example the hard disk drive) is always ready,
		// so I need to implement this command in SCSI_CDROM::start_command() to consider the disc is not inserted
		#ifdef _SCSI_DEBUG_LOG
			emu->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: Test Unit Ready\n"), scsi_id);
		#endif
		// this device is always ready, change to status phase
		set_dat(SCSI_STATUS_GOOD);
		set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		break;
		
	case SCSI_CMD_REQ_SENSE:
		// XXX: this code invites the device (for example the hard disk drive) is always ready,
		// so I need to implement this command in SCSI_CDROM::start_command() to consider the disc is not inserted
		#ifdef _SCSI_DEBUG_LOG
			emu->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: Request Sense\n"), scsi_id);
		#endif
		// start position
		position = ((command[1] & 0x1f) * 0x10000 + command[2] * 0x100 + command[3]) * logical_block_size;
		// transfer length
		remain = 16;
		// create sense data table
		buffer->clear();
		buffer->write(SCSI_SERROR_CURRENT);
		buffer->write(0x00);
		buffer->write(SCSI_KEY_NOSENSE); // this device is always ready, no sense
		buffer->write(0x00);
		buffer->write(0x00);
		buffer->write(0x00);
		buffer->write(0x00);
		buffer->write(0x08);
		buffer->write(0x00);
		buffer->write(0x00);
		buffer->write(0x00);
		buffer->write(0x00);
		buffer->write(0x00);
		buffer->write(0x00);
		buffer->write(0x00);
		buffer->write(0x00);
		// set first data
		set_dat(buffer->read());
		// change to data in phase
		set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
		break;
		
	case SCSI_CMD_FORMAT:
		#ifdef _SCSI_DEBUG_LOG
			emu->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: Format Unit\n"), scsi_id);
		#endif
		if(command[1] & 0x10) {
			// change to data out phase for extra bytes
			remain = 4;
			set_phase_delay(SCSI_PHASE_DATA_OUT, 10.0);
		} else {
			// no extra bytes, change to status phase
			set_dat(SCSI_STATUS_GOOD);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		break;
		
	case SCSI_CMD_RD_DEFECT:
		#ifdef _SCSI_DEBUG_LOG
			emu->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: Read Defect Data\n"), scsi_id);
		#endif
		// transfer length
		remain = 4;
		// create detect data table
		buffer->clear();
		buffer->write(0x00);
		buffer->write(command[2]);
		buffer->write(0x00); // msb of defect list length
		buffer->write(0x00); // lsb of defect list length
		// set first data
		set_dat(buffer->read());
		// change to data in phase
		set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
		break;
		
	case SCSI_CMD_READ6:
		#ifdef _SCSI_DEBUG_LOG
			emu->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: Read 6-byte\n"), scsi_id);
		#endif
		// start position
		position = ((command[1] & 0x1f) * 0x10000 + command[2] * 0x100 + command[3]) * logical_block_size;
		// transfer length
		if((remain = command[4] * logical_block_size) != 0) {
			// set first data
			buffer->clear();
			if(remain > SCSI_BUFFER_SIZE) {
				read_buffer(SCSI_BUFFER_SIZE);
			} else {
				read_buffer((int)remain);
			}
			set_dat(buffer->read());
			// change to data in phase
			set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
		} else {
			// transfer length is zero, change to status phase
			set_dat(SCSI_STATUS_GOOD);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		break;
		
	case SCSI_CMD_WRITE6:
		#ifdef _SCSI_DEBUG_LOG
			emu->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: Write 6-Byte\n"), scsi_id);
		#endif
		// start position
		position = ((command[1] & 0x1f) * 65536 + command[2] * 256 + command[3]) * logical_block_size;
		// transfer length
		if((remain = command[4] * logical_block_size) != 0) {
			// clear data buffer
			buffer->clear();
			// change to data in phase
			set_phase_delay(SCSI_PHASE_DATA_OUT, 10.0);
		} else {
			// transfer length is zero, change to status phase
			set_dat(SCSI_STATUS_GOOD);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		break;
		
	case SCSI_CMD_READ10:
		#ifdef _SCSI_DEBUG_LOG
			emu->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: Read 10-byte\n"), scsi_id);
		#endif
		// start position
		position = (command[2] * 0x1000000 + command[3] * 0x10000 + command[4] * 0x100 + command[5]) * logical_block_size;
		// transfer length
		if((remain = (command[7] * 0x100 + command[8]) * logical_block_size) != 0) {
			// set first data
			buffer->clear();
			if(remain > SCSI_BUFFER_SIZE) {
				read_buffer(SCSI_BUFFER_SIZE);
			} else {
				read_buffer((int)remain);
			}
			set_dat(buffer->read());
			// change to data in phase
			set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
		} else {
			// transfer length is zero, change to status phase
			set_dat(SCSI_STATUS_GOOD);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		break;
		
	case SCSI_CMD_WRITE10:
		#ifdef _SCSI_DEBUG_LOG
			emu->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: Write 10-Byte\n"), scsi_id);
		#endif
		goto WRITE10;
	case SCSI_CMD_WRT_VERIFY:
		#ifdef _SCSI_DEBUG_LOG
			emu->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: Write and Verify\n"), scsi_id);
		#endif
	WRITE10:
		// start position
		position = (command[2] * 0x1000000 + command[3] * 0x10000 + command[4] * 0x100 + command[5]) * logical_block_size;
		// transfer length
		if((remain = (command[7] * 0x100 + command[8]) * logical_block_size) != 0) {
			// clear data buffer
			buffer->clear();
			// change to data in phase
			set_phase_delay(SCSI_PHASE_DATA_OUT, 10.0);
		} else {
			// transfer length is zero, change to status phase
			set_dat(SCSI_STATUS_GOOD);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		break;
		
	case SCSI_CMD_READ12:
		#ifdef _SCSI_DEBUG_LOG
			emu->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: Read 12-byte\n"), scsi_id);
		#endif
		// start position
		position = (command[2] * 0x1000000 + command[3] * 0x10000 + command[4] * 0x100 + command[5]) * logical_block_size;
		// transfer length
		if((remain = (command[6] * 0x1000000 + command[7] * 0x10000 + command[8] * 0x100 + command[9]) * logical_block_size) != 0) {
			// set first data
			buffer->clear();
			if(remain > SCSI_BUFFER_SIZE) {
				read_buffer(SCSI_BUFFER_SIZE);
			} else {
				read_buffer((int)remain);
			}
			set_dat(buffer->read());
			// change to data in phase
			set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
		} else {
			// transfer length is zero, change to status phase
			set_dat(SCSI_STATUS_GOOD);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		break;
		
	case SCSI_CMD_WRITE12:
		#ifdef _SCSI_DEBUG_LOG
			emu->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: Write 12-Byte\n"), scsi_id);
		#endif
		// start position
		position = (command[2] * 0x1000000 + command[3] * 0x10000 + command[4] * 0x100 + command[5]) * logical_block_size;
		// transfer length
		if((remain = (command[6] * 0x1000000 + command[7] * 0x10000 + command[8] * 0x100 + command[9]) * logical_block_size) != 0) {
			// clear data buffer
			buffer->clear();
			// change to data in phase
			set_phase_delay(SCSI_PHASE_DATA_OUT, 10.0);
		} else {
			// transfer length is zero, change to status phase
			set_dat(SCSI_STATUS_GOOD);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		break;
		
	default:
		#ifdef _SCSI_DEBUG_LOG
			emu->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: Unknown %02X\n"), scsi_id, command[0]);
		#endif
		set_dat(SCSI_STATUS_GOOD);
		set_phase_delay(SCSI_PHASE_STATUS, 10.0);
	}
}

void SCSI_DEV::read_buffer(int length)
{
	for(int i = 0; i < length; i++) {
		buffer->write(0);
		position++;
	}
}

void SCSI_DEV::write_buffer(int length)
{
	for(int i = 0; i < length; i++) {
		buffer->read();
		position++;
	}
}

#define STATE_VERSION	1

void SCSI_DEV::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint32(data_bus);
	state_fio->FputBool(sel_status);
	state_fio->FputBool(atn_status);
	state_fio->FputBool(ack_status);
	state_fio->FputBool(rst_status);
	state_fio->FputBool(selected);
	state_fio->FputBool(atn_pending);
	state_fio->FputInt32(phase);
	state_fio->FputInt32(next_phase);
	state_fio->FputInt32(next_req);
	state_fio->FputInt32(event_sel);
	state_fio->FputInt32(event_phase);
	state_fio->FputInt32(event_req);
	state_fio->Fwrite(command, sizeof(command), 1);
	state_fio->FputInt32(command_index);
	buffer->save_state((void *)state_fio);
	state_fio->FputUint64(position);
	state_fio->FputUint64(remain);
}

bool SCSI_DEV::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	data_bus = state_fio->FgetUint32();
	sel_status = state_fio->FgetBool();
	atn_status = state_fio->FgetBool();
	ack_status = state_fio->FgetBool();
	rst_status = state_fio->FgetBool();
	selected = state_fio->FgetBool();
	atn_pending = state_fio->FgetBool();
	phase = state_fio->FgetInt32();
	next_phase = state_fio->FgetInt32();
	next_req = state_fio->FgetInt32();
	event_sel = state_fio->FgetInt32();
	event_phase = state_fio->FgetInt32();
	event_req = state_fio->FgetInt32();
	state_fio->Fread(command, sizeof(command), 1);
	command_index = state_fio->FgetInt32();
	if(!buffer->load_state((void *)state_fio)) {
		return false;
	}
	position = state_fio->FgetUint64();
	remain = state_fio->FgetUint64();
	return true;
}
