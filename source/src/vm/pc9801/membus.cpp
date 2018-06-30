/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2017.06.22-

	[ memory bus ]
*/

#include "membus.h"
#include "display.h"

#ifdef _MSC_VER
	// Microsoft Visual C++
	#pragma warning( disable : 4065 )
#endif

/*
	NORMAL PC-9801
		00000h - 9FFFFh: RAM
		A0000h - A1FFFh: TEXT VRAM
		A2000h - A3FFFh: ATTRIBUTE
		A4000h - A4FFFh: CG WINDOW
		A8000h - BFFFFh: VRAM (BRG)
		C0000h - DFFFFh: EXT BIOS
			CC000h - CFFFFh: SOUND BIOS
			D6000h - D6FFFh: 2DD FDD BIOS
			D7000h - D7FFFh: 2HD FDD BIOS
			D7000h - D7FFFh: SASI BIOS
			D8000h - DBFFFh: IDE BIOS
			DC000h - DCFFFh: SCSI BIOS
		E0000h - E7FFFh: VRAM (I)
		E8000h - FFFFFh: BIOS

	HIRESO PC-98XA/XL/XL^2/RL
		00000h - 7FFFFh: RAM
		80000h - BFFFFh: MEMORY WINDOW
		C0000h - DFFFFh: VRAM
		E0000h - E1FFFh: TEXT VRAM
		E2000h - E3FFFh: ATTRIBUTE
		E4000h - E4FFFh: CG WINDOW
		F0000h - FFFFFh: BIOS
*/

void MEMBUS::initialize()
{
	MEMORY::initialize();
	
	// RAM
	memset(ram, 0x00, sizeof(ram));
#if !defined(SUPPORT_HIRESO)
	set_memory_rw(0x00000, 0x9ffff, ram);
#else
	set_memory_rw(0x00000, 0xbffff, ram);
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	if(sizeof(ram) > 0x100000) {
		set_memory_rw(0x100000, sizeof(ram) - 1, ram + 0x100000);
	}
#endif
	
	// VRAM
#if !defined(SUPPORT_HIRESO)
	set_memory_mapped_io_rw(0xa0000, 0xa4fff, d_display);
	set_memory_mapped_io_rw(0xa8000, 0xbffff, d_display);
#if defined(SUPPORT_16_COLORS)
	set_memory_mapped_io_rw(0xe0000, 0xe7fff, d_display);
#endif
#else
	set_memory_mapped_io_rw(0xc0000, 0xe4fff, d_display);
#endif
	
	// BIOS
	memset(bios, 0xff, sizeof(bios));
	if(!read_bios(_T("IPL.ROM"), bios, sizeof(bios))) {
		read_bios(_T("BIOS.ROM"), bios, sizeof(bios));
	}
#if defined(SUPPORT_BIOS_RAM)
	memset(bios_ram, 0x00, sizeof(bios_ram));
#endif
#if defined(SUPPORT_ITF_ROM)
	memset(itf, 0xff, sizeof(itf));
	read_bios(_T("ITF.ROM"), itf, sizeof(itf));
#endif
	
#if !defined(SUPPORT_HIRESO)
	// EXT BIOS
#if defined(_PC9801) || defined(_PC9801E)
	memset(fd_bios_2hd, 0xff, sizeof(fd_bios_2hd));
	read_bios(_T("2HDIF.ROM"), fd_bios_2hd, sizeof(fd_bios_2hd));
	set_memory_r(0xd6000, 0xd6fff, fd_bios_2dd);
	
	memset(fd_bios_2dd, 0xff, sizeof(fd_bios_2dd));
	read_bios(_T("2DDIF.ROM"), fd_bios_2dd, sizeof(fd_bios_2dd));
	set_memory_r(0xd7000, 0xd7fff, fd_bios_2hd);
#endif
	memset(sound_bios, 0xff, sizeof(sound_bios));
//	memset(sound_bios_ram, 0x00, sizeof(sound_bios_ram));
	sound_bios_selected = false;
//	sound_bios_ram_selected = false;
	if(config.sound_type == 0) {
		sound_bios_selected = (read_bios(_T("SOUND.ROM"), sound_bios, sizeof(sound_bios)) != 0);
	} else if(config.sound_type == 2) {
		sound_bios_selected = (read_bios(_T("MUSIC.ROM"), sound_bios, sizeof(sound_bios)) != 0);
	}
	if(sound_bios_selected) {
		d_display->sound_bios_ok();
	}
	update_sound_bios();
#if defined(SUPPORT_SASI_IF)
	memset(sasi_bios, 0xff, sizeof(sasi_bios));
	memset(sasi_bios_ram, 0x00, sizeof(sasi_bios_ram));
	sasi_bios_selected = (read_bios(_T("SASI.ROM"), sasi_bios, sizeof(sasi_bios)) != 0);
	sasi_bios_ram_selected = false;
	update_sasi_bios();
#endif
#if defined(SUPPORT_SCSI_IF)
	memset(scsi_bios, 0xff, sizeof(scsi_bios));
	memset(scsi_bios_ram, 0x00, sizeof(scsi_bios_ram));
	scsi_bios_selected = (read_bios(_T("SCSI.ROM"), scsi_bios, sizeof(scsi_bios)) != 0);
	scsi_bios_ram_selected = false;
	update_scsi_bios();
#endif
#if defined(SUPPORT_IDE_IF)
	memset(ide_bios, 0xff, sizeof(ide_bios));
//	memset(ide_bios_ram, 0x00, sizeof(ide_bios_ram));
	ide_bios_selected = (read_bios(_T("IDE.ROM"), ide_bios, sizeof(ide_bios)) != 0);
//	ide_bios_ram_selected = false;
	update_ide_bios();
#endif
	
	// EMS
#if defined(SUPPORT_NEC_EMS)
	memset(nec_ems, 0, sizeof(nec_ems));
#endif
#endif
}

void MEMBUS::reset()
{
	MEMORY::reset();
	
	// BIOS/ITF
#if defined(SUPPORT_BIOS_RAM)
	bios_ram_selected = false;
#endif
#if defined(SUPPORT_ITF_ROM)
	itf_selected = true;
#endif
	update_bios();
	
#if !defined(SUPPORT_HIRESO)
	// EMS
#if defined(SUPPORT_NEC_EMS)
	nec_ems_selected = false;
	update_nec_ems();
#endif
#endif
	
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
#if !defined(SUPPORT_HIRESO)
	dma_access_ctrl = 0xfe; // bit2 = 1, bit0 = 0
	window_80000h = 0x80000;
	window_a0000h = 0xa0000;
#else
	dma_access_ctrl = 0xfb; // bit2 = 0, bit0 = 1
	window_80000h = 0x100000;
	window_a0000h = 0x120000;
#endif
#endif
}

void MEMBUS::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
#if defined(SUPPORT_ITF_ROM)
	case 0x043d:
		switch(data & 0xff) {
		case 0x00:
		case 0x10:
		case 0x18:
			if(!itf_selected) {
				itf_selected = true;
				update_bios();
			}
			break;
		case 0x02:
		case 0x12:
			if(itf_selected) {
				itf_selected = false;
				update_bios();
			}
			break;
		}
		break;
#endif
#if !defined(SUPPORT_HIRESO)
	case 0x043f:
		switch(data & 0xff) {
		case 0x20:
#if defined(SUPPORT_NEC_EMS)
			if(nec_ems_selected) {
				nec_ems_selected = false;
				update_nec_ems();
			}
#endif
			break;
		case 0x22:
#if defined(SUPPORT_NEC_EMS)
			if(!nec_ems_selected) {
				nec_ems_selected = true;
				update_nec_ems();
			}
#endif
			break;
		case 0xc0:
#if defined(SUPPORT_SASI_IF)
			if(sasi_bios_ram_selected) {
				sasi_bios_ram_selected = false;
				if(sasi_bios_selected) {
					update_sasi_bios();
				}
			}
#endif
#if defined(SUPPORT_SCSI_IF)
			if(scsi_bios_ram_selected) {
				scsi_bios_ram_selected = false;
				if(scsi_bios_selected) {
					update_scsi_bios();
				}
			}
#endif
			break;
		case 0xc2:
#if defined(SUPPORT_SASI_IF)
			if(!sasi_bios_ram_selected) {
				sasi_bios_ram_selected = true;
				if(sasi_bios_selected) {
					update_sasi_bios();
				}
			}
#endif
			break;
		case 0xc4:
#if defined(SUPPORT_SCSI_IF)
			if(!scsi_bios_ram_selected) {
				scsi_bios_ram_selected = true;
				if(scsi_bios_selected) {
					update_scsi_bios();
				}
			}
#endif
			break;
		}
		break;
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
#if !defined(_PC98XA)
	case 0x0439:
		dma_access_ctrl = data;
		break;
#endif
#if !defined(SUPPORT_HIRESO)
	case 0x0461:
#else
	case 0x0091:
#if defined(_PC98XA)
		if(data < 0x10) {
			break;
		}
#endif
#endif
		window_80000h = (data & 0xfe) << 16;
		break;
#if !defined(SUPPORT_HIRESO)
	case 0x0463:
#else
	case 0x0093:
#if defined(_PC98XA)
		if(data < 0x10) {
			break;
		}
#endif
#endif
		window_a0000h = (data & 0xfe) << 16;
		break;
#endif
#if defined(SUPPORT_32BIT_ADDRESS)
	case 0x053d:
#if !defined(SUPPORT_HIRESO)
		if(sound_bios_selected != ((data & 0x80) != 0)) {
			sound_bios_selected = ((data & 0x80) != 0);
			update_sound_bios();
		}
#if defined(SUPPORT_SASI_IF)
		if(sasi_bios_selected != ((data & 0x40) != 0)) {
			sasi_bios_selected = ((data & 0x40) != 0);
			update_sasi_bios();
		}
#endif
#if defined(SUPPORT_SCSI_IF)
		if(scsi_bios_selected != ((data & 0x20) != 0)) {
			scsi_bios_selected = ((data & 0x20) != 0);
			update_scsi_bios();
		}
#endif
#if defined(SUPPORT_IDE_IF)
		if(ide_bios_selected != ((data & 0x10) != 0)) {
			ide_bios_selected = ((data & 0x10) != 0);
			update_ide_bios();
		}
#endif
#endif
#if defined(SUPPORT_BIOS_RAM)
		if(bios_ram_selected != ((data & 0x02) != 0)) {
			bios_ram_selected = ((data & 0x02) != 0);
			update_bios();
		}
#endif
		break;
#endif
	// dummy for no cases
	default:
		break;
	}
}

uint32_t MEMBUS::read_io8(uint32_t addr)
{
	switch(addr) {
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
#if !defined(_PC98XA)
	case 0x0439:
		return dma_access_ctrl;
#endif
#if !defined(SUPPORT_HIRESO)
	case 0x0461:
#else
	case 0x0091:
#endif
		return window_80000h >> 16;
#if !defined(SUPPORT_HIRESO)
	case 0x0463:
#else
	case 0x0093:
#endif
		return window_a0000h >> 16;
	case 0x0567:
		return (uint8_t)(sizeof(ram) >> 17);
#endif
	// dummy for no cases
	default:
		break;
	}
	return 0xff;
}

#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
#if !defined(SUPPORT_HIRESO)
	#define UPPER_MEMORY_24BIT	0x00fa0000
	#define UPPER_MEMORY_32BIT	0xfffa0000
#else
	#define UPPER_MEMORY_24BIT	0x00fc0000
	#define UPPER_MEMORY_32BIT	0xfffc0000
#endif

uint32_t MEMBUS::read_data8(uint32_t addr)
{
	if(addr < 0x80000) {
		return MEMORY::read_data8(addr);
	} else if(addr < 0xa0000) {
		addr = (addr & 0x1ffff) | window_80000h;
	} else if(addr < 0xc0000) {
		addr = (addr & 0x1ffff) | window_a0000h;
	}
	if(addr < UPPER_MEMORY_24BIT) {
		return MEMORY::read_data8(addr);
#if defined(SUPPORT_24BIT_ADDRESS)
	} else {
#else
	} else if(addr < 0x1000000 || addr >= UPPER_MEMORY_32BIT) {
#endif
		return MEMORY::read_data8(addr & 0xfffff);
	}
	return 0xff;
}

void MEMBUS::write_data8(uint32_t addr, uint32_t data)
{
	if(addr < 0x80000) {
		MEMORY::write_data8(addr, data);
		return;
	} else if(addr < 0xa0000) {
		addr = (addr & 0x1ffff) | window_80000h;
	} else if(addr < 0xc0000) {
		addr = (addr & 0x1ffff) | window_a0000h;
	}
	if(addr < UPPER_MEMORY_24BIT) {
		MEMORY::write_data8(addr, data);
#if defined(SUPPORT_24BIT_ADDRESS)
	} else {
#else
	} else if(addr < 0x1000000 || addr >= UPPER_MEMORY_32BIT) {
#endif
		MEMORY::write_data8(addr & 0xfffff, data);
	}
}

uint32_t MEMBUS::read_dma_data8(uint32_t addr)
{
	if(dma_access_ctrl & 4) {
		addr &= 0x000fffff;
	}
	return MEMBUS::read_data8(addr);
}

void MEMBUS::write_dma_data8(uint32_t addr, uint32_t data)
{
	if(dma_access_ctrl & 4) {
		addr &= 0x000fffff;
	}
	MEMBUS::write_data8(addr, data);
}
#endif

void MEMBUS::update_bios()
{
	unset_memory_rw(0x100000 - sizeof(bios), 0xfffff);
#if defined(SUPPORT_ITF_ROM)
	if(itf_selected) {
		set_memory_r(0x100000 - sizeof(itf), 0xfffff, itf);
	} else {
#endif
#if defined(SUPPORT_BIOS_RAM)
		if(bios_ram_selected) {
			set_memory_rw(0x100000 - sizeof(bios_ram), 0xfffff, bios_ram);
		} else {
#endif
			set_memory_r(0x100000 - sizeof(bios), 0xfffff, bios);
#if defined(SUPPORT_BIOS_RAM)
//			set_memory_w(0x100000 - sizeof(bios_ram), 0xfffff, bios_ram);
		}
#endif
#if defined(SUPPORT_ITF_ROM)
	}
#endif
}

#if !defined(SUPPORT_HIRESO)
void MEMBUS::update_sound_bios()
{
	if(sound_bios_selected) {
//		if(sound_bios_selected) {
//			set_memory_r(0xcc000, 0xcffff, sound_bios_ram);
//		} else {
			set_memory_r(0xcc000, 0xcffff, sound_bios);
			unset_memory_w(0xcc000, 0xcffff);
//		}
	} else {
		unset_memory_rw(0xcc000, 0xcffff);
	}
}

#if defined(SUPPORT_SASI_IF)
void MEMBUS::update_sasi_bios()
{
	if(sasi_bios_selected) {
		if(sasi_bios_ram_selected) {
			set_memory_rw(0xd7000, 0xd7fff, sasi_bios_ram);
		} else {
			set_memory_r(0xd7000, 0xd7fff, sasi_bios);
			unset_memory_w(0xd7000, 0xd7fff);
		}
	} else {
		unset_memory_rw(0xd7000, 0xd7fff);
	}
}
#endif

#if defined(SUPPORT_SCSI_IF)
void MEMBUS::update_scsi_bios()
{
	if(scsi_bios_selected) {
		if(scsi_bios_ram_selected) {
			set_memory_rw(0xdc000, 0xdcfff, scsi_bios_ram);
		} else {
			set_memory_r(0xdc000, 0xdcfff, scsi_bios);
			unset_memory_w(0xdc000, 0xdcfff);
		}
	} else {
		unset_memory_rw(0xdc000, 0xdcfff);
	}
}
#endif

#if defined(SUPPORT_IDE_IF)
void MEMBUS::update_ide_bios()
{
	if(ide_bios_selected) {
//		if(ide_bios_selected) {
//			set_memory_r(0xd8000, 0xdbfff, ide_bios_ram);
//		} else {
			set_memory_r(0xd8000, 0xdbfff, ide_bios);
			unset_memory_w(0xd8000, 0xdbfff);
//		}
	} else {
		unset_memory_rw(0xd8000, 0xdbfff);
	}
}
#endif

#if defined(SUPPORT_NEC_EMS)
void MEMBUS::update_nec_ems()
{
	if (nec_ems_selected) {
		unset_memory_rw(0xb0000, 0xbffff);
		set_memory_rw(0xb0000, 0xbffff, nec_ems);
	} else {
		unset_memory_rw(0xb0000, 0xbffff);
		set_memory_mapped_io_rw(0xb0000, 0xbffff, d_display);
	}
}
#endif
#endif

#define STATE_VERSION	3

#include "../statesub.h"

void MEMBUS::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_1D_ARRAY(ram, sizeof(ram));
#if defined(SUPPORT_BIOS_RAM)
	DECL_STATE_ENTRY_1D_ARRAY(bios_ram, sizeof(bios_ram));
	DECL_STATE_ENTRY_BOOL(bios_ram_selected);
#endif
#if defined(SUPPORT_ITF_ROM)
	DECL_STATE_ENTRY_BOOL(itf_selected);
#endif
#if !defined(SUPPORT_HIRESO)
//	DECL_STATE_ENTRY_1D_ARRAY(sound_bios_ram, sizeof(sound_bios_ram), 1);
	DECL_STATE_ENTRY_BOOL(sound_bios_selected);
//	DECL_STATE_ENTRY_BOOL(sound_bios_ram_selected);
#if defined(SUPPORT_SASI_IF)
	DECL_STATE_ENTRY_1D_ARRAY(sasi_bios_ram, sizeof(sasi_bios_ram));
	DECL_STATE_ENTRY_BOOL(sasi_bios_selected);
	DECL_STATE_ENTRY_BOOL(sasi_bios_ram_selected);
#endif
#if defined(SUPPORT_SCSI_IF)
	DECL_STATE_ENTRY_1D_ARRAY(scsi_bios_ram, sizeof(scsi_bios_ram));
	DECL_STATE_ENTRY_BOOL(scsi_bios_selected);
	DECL_STATE_ENTRY_BOOL(scsi_bios_ram_selected);
#endif
#if defined(SUPPORT_IDE_IF)
//	DECL_STATE_ENTRY_1D_ARRAY(ide_bios_ram, sizeof(ide_bios_ram), 1);
	DECL_STATE_ENTRY_BOOL(ide_bios_selected);
//	DECL_STATE_ENTRY_BOOL(ide_bios_ram_selected);
#endif
#if defined(SUPPORT_NEC_EMS)
	DECL_STATE_ENTRY_1D_ARRAY(nec_ems, sizeof(nec_ems));
	DECL_STATE_ENTRY_BOOL(nec_ems_selected);
#endif
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	DECL_STATE_ENTRY_UINT8(dma_access_ctrl);
	DECL_STATE_ENTRY_INT32(window_80000h);
	DECL_STATE_ENTRY_INT32(window_a0000h);
#endif
	leave_decl_state();

	// ToDo: Helper.
//	MEMORY::decl_state();
}

void MEMBUS::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->Fwrite(ram, sizeof(ram), 1);
//#if defined(SUPPORT_BIOS_RAM)
//	state_fio->Fwrite(bios_ram, sizeof(bios_ram), 1);
//	state_fio->FputBool(bios_ram_selected);
//#endif
//#if defined(SUPPORT_ITF_ROM)
//	state_fio->FputBool(itf_selected);
//#endif
//#if !defined(SUPPORT_HIRESO)
////	state_fio->Fwrite(sound_bios_ram, sizeof(sound_bios_ram), 1);
//	state_fio->FputBool(sound_bios_selected);
////	state_fio->FputBool(sound_bios_ram_selected);
//#if defined(SUPPORT_SASI_IF)
//	state_fio->Fwrite(sasi_bios_ram, sizeof(sasi_bios_ram), 1);
//	state_fio->FputBool(sasi_bios_selected);
//	state_fio->FputBool(sasi_bios_ram_selected);
//#endif
//#if defined(SUPPORT_SCSI_IF)
//	state_fio->Fwrite(scsi_bios_ram, sizeof(scsi_bios_ram), 1);
//	state_fio->FputBool(scsi_bios_selected);
//	state_fio->FputBool(scsi_bios_ram_selected);
//#endif
//#if defined(SUPPORT_IDE_IF)
//	state_fio->Fwrite(ide_bios_ram, sizeof(ide_bios_ram), 1);
//	state_fio->FputBool(ide_bios_selected);
//	state_fio->FputBool(ide_bios_ram_selected);
//#endif
//#if defined(SUPPORT_NEC_EMS)
//	state_fio->Fwrite(nec_ems, sizeof(nec_ems), 1);
//	state_fio->FputBool(nec_ems_selected);
//#endif
//#endif
//#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
//	state_fio->FputUint8(dma_access_ctrl);
//	state_fio->FputInt32(window_80000h);
//	state_fio->FputInt32(window_a0000h);
//#endif
	
//	MEMORY::save_state(state_fio);
}

bool MEMBUS::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) return false;
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	state_fio->Fread(ram, sizeof(ram), 1);
//#if defined(SUPPORT_BIOS_RAM)
//	state_fio->Fwrite(bios_ram, sizeof(bios_ram), 1);
//	bios_ram_selected = state_fio->FgetBool();
//#endif
//#if defined(SUPPORT_ITF_ROM)
//	itf_selected = state_fio->FgetBool();
//#endif
//#if !defined(SUPPORT_HIRESO)
////	state_fio->Fread(sound_bios_ram, sizeof(sound_bios_ram), 1);
//	sound_bios_selected = state_fio->FgetBool();
////	sound_bios_ram_selected = state_fio->FgetBool();
//#if defined(SUPPORT_SASI_IF)
//	state_fio->Fread(sasi_bios_ram, sizeof(sasi_bios_ram), 1);
//	sasi_bios_selected = state_fio->FgetBool();
//	sasi_bios_ram_selected = state_fio->FgetBool();
//#endif
//#if defined(SUPPORT_SCSI_IF)
//	state_fio->Fread(scsi_bios_ram, sizeof(scsi_bios_ram), 1);
//	scsi_bios_selected = state_fio->FgetBool();
//	scsi_bios_ram_selected = state_fio->FgetBool();
//#endif
//#if defined(SUPPORT_IDE_IF)
////	state_fio->Fread(ide_bios_ram, sizeof(ide_bios_ram), 1);
//	ide_bios_selected = state_fio->FgetBool();
////	ide_bios_ram_selected = state_fio->FgetBool();
//#endif
//#if defined(SUPPORT_NEC_EMS)
//	state_fio->Fread(nec_ems, sizeof(nec_ems), 1);
//	nec_ems_selected = state_fio->FgetBool();
//#endif
//#endif
//#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
//	dma_access_ctrl = state_fio->FgetUint8();
//	window_80000h = state_fio->FgetUint32();
//	window_a0000h = state_fio->FgetUint32();
//#endif
	
	// post process
	update_bios();
#if !defined(SUPPORT_HIRESO)
	update_sound_bios();
#if defined(SUPPORT_SASI_IF)
	update_sasi_bios();
#endif
#if defined(SUPPORT_SCSI_IF)
	update_scsi_bios();
#endif
#if defined(SUPPORT_IDE_IF)
	update_ide_bios();
#endif
#if defined(SUPPORT_EMS)
	update_nec_ems();
#endif
#endif
//	return MEMORY::load_state(state_fio);
	return true;
}
