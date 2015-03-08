/*
 * Main memory without MMR for FM-7 [FM7_MAINMEM]
 *  Author: K.Ohta
 *  Date  : 2015.01.01-
 *
 */

#include "fm7_mainmem.h"

int FM7_MAINMEM::window_convert(uint32 addr, uint32 *realaddr)
{
	uint32 raddr = addr;
#ifdef HAS_MMR
	if((addr < 0x8000) && (addr >= 0x7c00) && (window_enabled)) {
		addr &= 0x03ff;
		raddr = ((window_offset << 8) + addr) & 0xffff;
		*realaddr = raddr;
#ifdef _FM77AV_VARIANTS
		return FM7_MAINMEM_MMRBANK_0; // 0x20000 - 0x2ffff
#else // FM77(L4 or others)
		return FM7_MAINMEM_MMRBANK_2; // 0x20000 - 0x2ffff
#endif
	}
	// Window not hit.
#endif
	return -1;
}

int FM7_MAINMEM::mmr_convert(uint32 addr, uint32 *realaddr)
{
	uint32 raddr = 0;
	uint8  mmr_segment;
	uint8  mmr_bank;
	
	if(addr >= 0xfc00) return -1;
	mmr_segment = mainio->read_signal(SIG_FM7MAINIO_MMR_SEGMENT);
	mmr_bank = mainio->read_signal(SIG_FM7MAINIO_MMR_BANK + mmr_segment * 16 + ((addr >> 12) & 0x000f));
	// Out of EXTRAM : 77AV20/40.
	
#if !defined(_FM77AV_VARIANTS)
	mmr_bank &= 0x3f;
#endif
	// Reallocated by MMR
	raddr = addr & 0x0fff;
	// Bank 3x : Standard memories.
	if((mmr_bank < 0x3f) && (mmr_bank >= 0x30)) {
		raddr = (((uint32)mmr_bank & 0x0f) << 12) | raddr;
		return nonmmr_convert(raddr, realaddr);
	}
  
#ifdef _FM77AV_VARIANTS
	if(mmr_bank == 0x3f) {
		if((raddr >= 0xd80) && (raddr <= 0xd97)) { // MMR AREA
			*realaddr = 0;
			return FM7_MAINMEM_NULL;
		}
	}
#else
	if((mmr_bank == 0x3f) && (addr >= 0xc00) && (addr < 0xe00)) {
		if(mainio->read_signal(SIG_FM7MAINIO_IS_BASICROM) != 0) { // BASICROM enabled
			*realaddr = 0;
			return FM7_MAINMEM_ZERO;
		} else {
			*realaddr = addr & 0x1ff;
			return FM7_MAINMEM_SHADOWRAM;
		}
	}
#endif
	
#ifdef _FM77AV_VARIANTS
	if((mmr_bank & 0xf0) == 0x00) { // PAGE 0
		*realaddr = (((uint32)mmr_bank & 0x0f) << 12) | raddr;
		return FM7_MAINMEM_77AV_PAGE0;
	}
	if((mmr_bank & 0xf0) == 0x10) { // PAGE 1
		*realaddr = (((uint32)mmr_bank & 0x0f) << 12) | raddr;
		return FM7_MAIMEM_DIRECTACCESS;
	}
	if((mmr_bank & 0xf0) == 0x20) { // PAGE 2
		uint32 dbank = mainio->read_signal(SIG_FM7_MAINIO_EXTBANK);
		switch(mmr_bank) {
	  		case 0x28:
	  		case 0x29: // Backuped RAM
				if(((dbank & 0x80) != 0) && (dictrom_connected)){ // Battery backuped RAM
					raddr =  raddr & 0x1ff;
					*readladdr = raddr;
					return FM7_MAINMEM_BACKUPED_RAM;
				}
				break;
			case 0x2e:
				if(((dbank & 0x40) != 0) && (dictrom_connected)) { // Dictionary ROM
					dbank = dbank & 0x3f;
					uint32 extrom = mainio->read_signal(SIG_FM7_MAINIO_EXTROM) & 0x80;
					if(extrom == 0) { // Dictionary selected.
						dbank = dbank << 12;
						*realaddr = raddr | dbank;
						return FM7_MAINMEM_77AV40_DICTROM;
					} else if(dbank <= 0x1f) { // KANJI
						*realaddr = (dbank << 12) | raddr;
						return FM7_MAINMEM_KANJI_LEVEL1;
					} else if(dbank <= 0x37) { 
						dbank = dbank << 12;
						*realaddr = (dbank - 0x20000) | raddr;
						return FM7_MAINMEM_77AV40_EXTRAROM;
					} else if(dbank <= 0x3f) {
					  	raddr = ((dbank << 12) - 0x30000) | raddr;
						if((raddr >= 0xffe0) || (raddr < 0xfd00)) { 
							return nonmmr_convert(raddr, realaddr);
						} else if((raddr >= 0xfe00) || (raddr < 0xffe0)) {
							*realaddr = raddr - 0xfe00;
							return FM7_MAINMMEM_BOOTROM_DOS;
						}
						*realaddr = raddr + 0x10000;
						return FM7_MAINMEM_77AV40_EXTRAROM;
					}
				}
				break;
		}
	  	// RAM
		*realaddr = (raddr | (mmr_bank << 12)) & 0x0ffff;
		return FM7_MAINMEM_77AV_PAGE2;
	}
	if(extram_connected) { // PAGE 4-
		if((mmr_bank >> 4) >= (extram_pages + 4)) {
			*realaddr = 0;
			return FM7_MAINMEM_NULL; // $FF
		} else {
			raddr = ((uint32)mmr_bank << 12) | raddr;
			*realaddr = raddr;
			return FM7_MAINMEM_EXTRAM;
		}
	} else {
		if(mmr_bank >= 0x40) {
			*realaddr = 0;
			return FM7_MAINMEM_NULL;
		}
	}
#else // 77
	// page 0 or 1 or 2.
	if(extram_connected) {
		if((mmr_bank >> 4) >= extram_pages) {
			*realaddr = 0;
			return FM7_MAINMEM_NULL;
		} else { // EXTRAM Exists.
			raddr = (((uint32)mmr_bank << 12) & 0x3ffff) | raddr;
			*realaddr = raddr;
			return FM7_MAINMEM_EXTRAM;
		}
	}
#endif
	return -1;
}

int FM7_MAINMEM::nonmmr_convert(uint32 addr, uint32 *realaddr)
{
	addr &= 0x0ffff;
	if(mainio->read_signal(SIG_FM7_INITROM_ENABLED) != 0) {
		if((addr >= 0x6000) && (addr < 0x8000)) {
			*realaddr = addr - 0x6000;
			return FM7_MAINMEM_INITROM;
		}
		if(addr >= 0xfffe) {
			*realaddr = addr - 0xe000;
			return FM7_MAINMEM_INITROM;
		}
	}
	
	if(addr < 0x8000) {
		*realaddr = addr - 0;
 		return FM7_MAINMEM_OMOTE;
	}
	if(addr < 0xfc00) {
		*realaddr = addr - 0x8000;
		if(mainio->get_rommode_fd0f() == true) return FM7_MAINMEM_BASICROM;
		return FM7_MAINMEM_URA;
	}
	if(addr < 0xfc80) {
		*realaddr = addr - 0xfc00;
		return FM7_MAINMEM_BIOSWORK;
	}
	if(addr < 0xfd00) {
		*realaddr = addr - 0xfc80;
		return FM7_MAINMEM_SHAREDRAM;
	}
	if(addr < 0xfe00) {
		mainio->wait();
		*realaddr = addr - 0xfd00;
		return FM7_MAINMEM_MMIO;
	}
	
	if((addr < 0xffe0) || (addr >= 0xfffe)) {
		if(addr < 0xffe0) mainio->wait();
		*realaddr = addr - 0xfe00;
		//if(mainio->get_boot_romram() != true) return FM7_MAINMEM_BOOTROM_RAM;
		switch(mainio->read_signal(SIG_FM7_GET_BOOTMODE)) {
			case 0:
				return FM7_MAIMEM_BOOTROM_BAS;
				break;
			case 1:
				return FM7_MAIMEM_BOOTROM_DOS;
				break;
			case 2:
				return FM7_MAIMEM_BOOTROM_MMR;
				break;
			case 3:
				return FM7_MAIMEM_BOOTRAM;
				break;
			default:
				return FM7_MAINMEM_BOOTROM_BAS; // Really?
				break;
		}
	}
	if(addr < 0xfffe) { // VECTOR
		*realaddr = addr - 0xffe0;
		return FM7_MAINMEM_VECTOR;
	}
	realaddr = addr;
	return FM7_MAINMEM_NULL;
}
     
int FM7_MAINMEM::getbank(uint32 addr, uint32 *realaddr)
{
	if(realaddr == NULL) return FM7_MAINMEM_NULL; // Not effect.
	addr = addr & 0xffff;
#ifdef HAS_MMR
	if(window_enabled) {
		int stat;
		uint32 raddr;
		stat = window_convert(addr, &raddr);
		if(stat >= 0) {
			*realaddr = raddr;
			return stat;
		}
	}
	if(mmr_enabled) {
		int stat;
		uint32 raddr;
		stat = mmr_convert(addr, &raddr);
		if(stat >= 0) {
			*realaddr = raddr;
			return stat;
		}
	}
#endif
	// NOT MMR.
	return nonmmr_convert(addr, realaddr);
}

uint32 FM7_MAINMEM::read_data8(uint32 addr)
{
	uint32 ret;
	uint32 realaddr;
	int bank;

	bank = getbank(addr, &realaddr);
	if(bank < 0) return 0xff; // Illegal

        if(bank == FM7_MAINMEM_SHAREDRAM) {
	   	if(display->read_signal(SIG_SUBCPU_HALT) != 0) return 0xff; // Not halt
		return display->read_data8(realaddr + 0xd380); // Okay?
	}
#if defined(_FM77AV_VARIANTS)
	else if(bank == FM7_MAINMEM_77AV_DIRECTACCESS) {
       		if(display->read_signal(SIG_SUBCPU_HALT) != 0) return 0xff; // Not halt
		return display->reat_data8(realaddr); // Okay?
	}
#endif
	if(read_table[bank].dev != NULL) {
		return read_table[bank].dev->read_data8(realaddr);
	} else {
        	if(read_table[bank].memory != NULL) {
	   		return read_table[bank].memory[realaddr];
		}
		return 0xff; // Dummy
	}
}

void FM7_MAINMEM::write_data8(uint32 addr, uint32 data)
{
	uint32 ret;
	uint32 realaddr;
	int bank;
   
	bank = getbank(addr, &realaddr);
	if(bank < 0) return; // Illegal
   
        if(bank == FM7_MAINMEM_SHAREDRAM) {
       		if(display->read_signal(SIG_SUBCPU_HALT) != 0) return; // Not halt
		display->write_data8(realaddr + 0xd380, data); // Okay?
		return;
	}
#if defined(_FM77AV_VARIANTS)
	else if(bank == FM7_MAINMEM_77AV_DIRECTACCESS) {
       		if(display->read_signal(SIG_SUBCPU_HALT) != 0) return; // Not halt
		display->write_data8(realaddr, data); // Okay?
		return;
	}
#endif
	if(write_table[bank].dev != NULL) {
		write_table[bank].dev->write_data8(realaddr, data);
	} else {
        	if(write_table[bank].memory != NULL) {
		  write_table[bank].memory[realaddr] = (uint8)(data & 0x000000ff);
		}
	}
}

// Read / Write data(s) as big endian.
uint32 FM7_MAINMEM::read_data16(uint32 addr)
{
	uint32 hi, lo;
	uint32 val;
   
	hi = read_data8(addr) & 0xff;
	lo = read_data8(addr + 1) & 0xff;
   
	val = hi * 256 + lo;
	return val;
}

uint32 FM7_MAINMEM::read_data32(uint32 addr)
{
	uint32 ah, a2, a3, al;
	uint32 val;
   
	ah = read_data8(addr) & 0xff;
	a2 = read_data8(addr + 1) & 0xff;
	a3 = read_data8(addr + 2) & 0xff;
	al = read_data8(addr + 3) & 0xff;
   
	val = ah * (65536 * 256) + a2 * 65536 + a3 * 256 + al;
	return val;
}

void FM7_MAINMEM::write_data16(uint32 addr, uint32 data)
{
	uint32 d = data;
   
	write_data8(addr + 1, d & 0xff);
	d = d / 256;
	write_data8(addr + 0, d & 0xff);
}

void FM7_MAINMEM::write_data32(uint32 addr, uint32 data)
{
	uint32 d = data;
   
	write_data8(addr + 3, d & 0xff);
	d = d / 256;
	write_data8(addr + 2, d & 0xff);
	d = d / 256;
	write_data8(addr + 1, d & 0xff);
	d = d / 256;
	write_data8(addr + 0, d & 0xff);
}


bool FM7_MAINMEM::get_loadstat_basicrom(void)
{
	return diag_load_basicrom;
}

bool FM7_MAINMEM::get_loadstat_bootrom_bas(void)
{
	return diag_load_bootrom_bas;
}

bool FM7_MAINMEM::get_loadstat_bootrom_dos(void)
{
	return diag_load_bootrom_dos;
}

uint32 FM7_MAINMEM::read_bios(const char *name, uint8 *ptr, uint32 size)
{
	FILEIO fio;
	uint32 blocks;
	_TCHAR *s;
  
	if((name == NULL) || (ptr == NULL))  return 0;
	s = bios_path(name);
	if(s == NULL) return 0;
  
	if(!fio.Fopen(s, FILEIO_READ_BINARY)) return 0;
	blocks = fio.Fread(s, size, 1);
	fio.Fclose();

	return blocks * size;
}

FM7_MAINMEM::FM7_MAINMEM(VM* parent_vm, EMU* parent_emu) : MEMORY(parent_vm, parent_emu)
{
	int i;
	p_vm = parent_vm;
	p_emu = parent_emu;
	for(i = 0; i < 4; i++) fm7_bootroms[i - FM7_MAINMEM_BOOTROM_BAS] = malloc(0x200);
}

FM7_MAINMEM::~FM7_MAINMEM()
{
	int i;
	for(i = 0; i < 4; i++) {
		if(fm7_bootroms[i] != NULL) free(fm7_bootroms[i]);
		fm7_bootroms[i] = NULL;
	}
	delete mainio;
}

void FM7_MAINMEM::initialize(void)
{
	int i;
	// Initialize table
	// $0000-$7FFF
	i = FM7_MAINMEM_OMOTE;
	memset(fm7_maimem_omote, 0x00, 0x8000 * sizeof(uint8));
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_omote;
	write_table[i].dev = NULL;
	write_table[i].memory = fm7_mainmem_omote;
	

	// $8000-$FBFF
	i = FM7_MAINMEM_URA
	memset(fm7_maimem_ura, 0x00, 0x7c00 * sizeof(uint8));
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_ura;
	write_table[i].dev = NULL;
	write_table[i].memory = fm7_mainmem_ura;

#if defined(_FM77AV_VARIANTS)
	i = FM7_MAINMEM_MMRBANK_0;
	memset(fm7_maimem_mmrbank_0, 0xff, 0x10000 * sizeof(uint8));
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_mmrbank_0;
	write_table[i].dev = NULL;
	write_table[i].memory = fm7_mainmem_mmrbank_0;
	
	i = FM7_MAINMEM_MMRBANK_2;
	memset(fm7_maimem_mmrbank_2, 0xff, 0x10000 * sizeof(uint8));
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_mmrbank_0;
	write_table[i].dev = NULL;
	write_table[i].memory = fm7_mainmem_mmrbank_0;
	
	i = FM7_MAINMEM_VECTOR;
	memset(fm7_mainmem_bootrom_vector, 0x00, 0x1e);
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_bootrom_vector;
	write_table[i].dev = NULL;
	write_table[i].memory = fm7_mainmem_bootrom_vector;
	
	i = FM7_MAINMEM_VECTOR;
	memset(fm7_mainmem_bootrom_vector, 0x00, 0x1e);
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_bootrom_vector;
	write_table[i].dev = NULL;
	write_table[i].memory = fm7_mainmem_bootrom_vector;
	
	
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || defined(_FM77AV20)
	i = FM7_MAINMEM_77AV40_DICTROM;
	memset(fm7_maimem_extrarom, 0xff, 0x40000 * sizeof(uint8));
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_dictrom;
	write_table[i].dev = NULL;
	write_table[i].memory = write_dummy;
	if(read_bios("DICROM.ROM", read_table[i].memory, 0x40000) == 0x40000) diag_load_dictrom = true;
	
	i = FM7_MAINMEM_77AV40_EXTRAROM;
	memset(fm7_maimem_extrarom, 0xff, 0x20000 * sizeof(uint8));
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_extrarom;
	write_table[i].dev = NULL;
	write_table[i].memory = write_dummy;
	if(read_bios("EXTSUB.ROM", read_table[i].memory, 0xc000) >= 0xc000) diag_load_extrarom = true;
	
	if(config.extram_pages > 0) {
		i = FM7_MAINMEM_EXTRAM;
		extram_pages = config.extram_pages;
		if(extram_pages >= 12) extram_pages = 12;
		fm7_mainmem_extram = malloc(extram_pages * 0x10000);
		if(fm7_maimem_extram != NULL) {
			memset(fm7_maimem_extram, 0x00, extram_pages * 0x10000);
			read_table[i].dev = NULL;
			read_table[i].memory = fm7_mainmem_extram;
			write_table[i].dev = NULL;
			write_table[i].memory = fm7_mainmem_extram;
		}
	}
# else
	// 77AV Only
# endif
	// Both 77AV and AV40
	for(i = FM7_MAINMEM_BOOTROM_BAS; i <= FM7_MAINMEM_BOOTROM_RAM; i++) {
		read_table[i].dev = NULL;
		read_table[i].memory = fm7_bootroms[i - FM7_MAINMEM_BOOTROM_BAS];
		write_table[i].dev = NULL;
		write_table[i].memory = write_dummy;
	}
	memcpy(fm7_bootroms[0], &fm7_mainmem_initiate[0x1800], 0x200 * sizeof(uint8));
	memcpy(fm7_bootroms[1], &fm7_mainmem_initiate[0x1a00], 0x200 * sizeof(uint8));
	read_table[FM7_MAINMEM_BOOTROM_MMR].memory = NULL; // Not connected.
	
	memset(fm7_bootroms[3], 0x00, 0x200 * sizeof(uint8)); // RAM
	if(diag_load_extrom) diag_load_bootrom_bas = true;
	if(diag_load_extrom) diag_load_bootrom_dos = true;
	write_table[FM7_BAINMEM_BOOTRAM].memory = read_table[FM7_BAINMEM_BOOTROM_RAM].memory; // Write enabled on BOOTRAM.
#else
	// FM-7/77
	if(read_bios("BOOT_BAS.ROM", fm7_bootroms[0], 0x200) >= 0x1e0) {
		diag_load_bootrom_bas = true;
	} else {
		diag_load_bootrom_bas = false;
		memset(fm7_bootroms[0], 0xff, 0x200);
	}
	if(read_bios("BOOT_DOS.ROM", fm7_bootroms[1], 0x200) >= 0x1e0) {
		diag_load_bootrom_dos = true;
	} else {
		diag_load_bootrom_dos = false;
		memset(fm7_bootroms[1], 0xff, 0x200);
	}
	
# if defined(_FM77) || defined(_FM77L2) || defined(_FM77L4)
	
	if(read_bios("BOOT_MMR.ROM", fm7_bootroms[2], 0x200) >= 0x1e0) {
		diag_load_bootrom_mmr = true;
	} else {
		diag_load_bootrom_mmr = false;
		memset(fm7_bootroms[2], 0xff, 0x200);
	}
	
	if(config.extram_pages > 0) {
		i = FM7_MAINMEM_EXTRAM;
		extram_pages = config.extram_pages;
		if(extram_pages >= 3) pages = 3;
		fm7_mainmem_extram = malloc(extram_pages * 0x10000);
		if(fm7_maimem_extram != NULL) {
			memset(fm7_maimem_extram, 0x00, extram_pages * 0x10000);
			read_table[i].dev = NULL;
			read_table[i].memory = fm7_mainmem_extram;
			write_table[i].dev = NULL;
			write_table[i].memory = fm7_mainmem_extram;
		}
	}
#else
	// FM-7
	diag_load_bootrom_mmr = false;
	memset(fm7_bootroms[2], 0xff, 0x200);
#endif

#if defined(_FM77) || defined(_FM77L2) || defined(_FM77L4)
	for(i = FM7_MAINMEM_BOOTROM_BAS; i <= FM7_MAINMEM_BOOTRAM; i++) {
	    read_table[i].dev = NULL;
	    read_table[i].memory = fm7_bootroms[i - FM7_MAINMEM_BOOTROM_BAS];
	    write_table[i].dev = NULL;
	    write_table[i].memory = write_dummy;
	}
	memset(fm7_bootroms[3], 0x00, 0x200 * sizeof(uint8)); // RAM
	write_table[FM7_BAINMEM_BOOTRAM].memory = read_table[FM7_BAINMEM_BOOTROM_RAM].memory; // Write enabled on BOOTRAM.
#else	
	// FM-7/8
	for(i = FM7_MAINMEM_BOOTROM_BAS; i <= FM7_MAINMEM_BOOTROM_DOS; i++) {
		read_table[i].dev = NULL;
		read_table[i].memory = fm7_bootroms[i - FM7_MAINMEM_BOOTROM_BAS];
		write_table[i].dev = NULL;
		write_table[i].memory = write_dummy;
	}
#endif
	
	i = FM7_MAINMEM_VECTOR;
	memset(fm7_mainmem_bootrom_vector, 0x00, 0x1e);
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_bootrom_vector;
	write_table[i].dev = NULL;
	write_table[i].memory = fm7_mainmem_bootrom_vector;
#endif 
	for(i = 0; i <= 3; i++) {
		uint8 *p = fm7_bootroms[i];
		p[0x1fe] = 0xfe; // Set reset vector.
		p[0x1ff] = 0x00; //
	}
	
	i = FM7_MAINMEM_BASICROM;
	memset(fm7_maimem_basicrom, 0xff, 0x7c00 * sizeof(uint8));
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_basicrom;
	write_table[i].dev = NULL;
	write_table[i].memory = write_dummy;
	if(read_bios("FBASIC30.ROM", read_table[i].memory, 0x7c00) == 0x7c00) diag_load_basicrom = true;
	
	i = FM7_MAINMEM_BIOSWORK;
	memset(fm7_maimem_bioswork, 0x00, 0x80 * sizeof(uint8));
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_bioswork;
	write_table[i].dev = NULL;
	write_table[i].memory = fm7_mainmem_bioswork;
	
	i = FM7_MAINMEM_SHAREDRAM;
	read_table[i].dev = display;
	read_table[i].memory = NULL;
	write_table[i].dev = display;
	write_table[i].memory = NULL;
	
#if defined(_FM77AV_VARIANTS)
	i = FM7_MAINMEM_DIRECTACCESS;
	read_table[i].dev = display;
	read_table[i].memory = NULL;
	write_table[i].dev = display;
	write_table[i].memory = NULL;
#endif
}
