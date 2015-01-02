/*
 * Main memory without MMR for FM-7 [FM7_MAINMEM]
 *  Author: K.Ohta
 *  Date  : 2015.01.01-
 *
 */

#include "fm7_mainmem.h"

int FM77AV_MAINMEM::getbank(uint32 addr, uint32 *realaddr)
{
   if(realaddr == NULL) return -1; // Not effect.
   
   addr = addr & 0xffff;
      addr = addr & 0x0ffff;

      if(window != NULL) {
	if(window->isenabled() && (addr >= 0x7c00) && (addr < 0x8000)) { // Window open
	    *realaddr = (addr + window->getwindow() * 256) & 0xffff;
	    return FM77AV_MAINMEM_PAGE0; // FM77
	  }
      }
      if(mmr != NULL) {
	uint32 mmr_page = mmr->getpage(addr);
	uint32 mmr_offset;
	if(!mmr->isenabled()) goto _fm7_1;
	if((mmr_page & 0x30) == 0x30) { // 77AV RAM/ROM Page1
	       uint32 a;
	       a = (mmr_page & 0x0f) * 0x1000 + mmr_offset;
	       if((a < 0xfd80) && (a > 0xfd97)) { // Okay.
		  addr = a;
		  goto _fm7_1;
	       }
	       // MMR Area is illegal.
	       *realaddr = 0;
	       return -1;
	    }
	    if(mmr_page < 0x10) { // 77AV RAM PAGE0
	       *realaddr = mmr_page * 0x1000 + mmr_offset;
	       return FM7_MAINMEM_AV_PAGE0;
	    }
	    if(mmr_page < 0x20) { // 77AV VRAM
	       *realaddr = ((mmr_page & 0x0f) * 0x1000) + mmr_offset;
	       return FM7_MAINMEM_AV_SUBMEM;
	    }
	    if(mmr_page < 0x30) { // 77AV Jcard
	       if(jcard != NULL) {
		    if(jcard->isenabled()) {
		       *realaddr = ((mmr_page & 0x0f) * 0x1000) + mmr_offset;
		       return FM7_MAINMEM_AV_JCARD;
		    }
	       }
	       *realaddr = mmr_offset;
	       return -1; // Illegal
	    }
//	    if(use_extram) { // EXTRAM
//	      *realaddr = ((mmr_page - 0x40) * 0x1000) + mmr_offset;
//	      return FM7_MAINMEM_AV_EXTRAM768K;
//	    }
      }
     
_fm7_1:
	if(mainio->is_initiaterom()) { // Initiate rom enabled, force overwrite vector.
	  if(addr >= 0xfffe) {
	    mainio->wait();
	    *realaddr = addr - 0xe000;
	    return FM7_MAINMEM_AV_INITROM;
	  }
	  if((addr >= 0x6000) && (addr < 0x8000)){ // Real addr: $6000 - $7fff
	    *realaddr = addr - 0x6000;
	    return FM7_MAINMEM_AV_INITROM;
	  }
	}
	if((addr < 0x10000) && (addr >= 0xfe00)) {
	  if(addr < 0xffe0) mainio->wait();
	  *realaddr = addr - 0xfe00;
	  return FM7_MAINMEM_BOOTROM_RAM;
	}
	  
	return FM7_MAINMEM::getvram(addr, realaddr);
     // Over
}

      
void FM77AV_MAINMEM::mem_reset(void)
{
  Uint32 bootmode;
  memset(fm7_bootroms[FM7_MAINMEM_BOOTROM_RAM - FM7_MAINMEM_BOOTROM_BAS], 0x00, 0x1e0); // Clear without vectors.
  bootmode = dipsw->getbootmode();

  if((bootmode & FM7_BOOTMODE_RAM) == 0) {
    bootmode &= 1;
    if(bootmode == FM7_BOOTMODE_BASIC) {
      memcpy(fm7_bootroms[FM7_MAINMEM_BOOTROM_RAM - FM7_MAINMEM_BOOTROM_BAS], &initiator_rom[0x1800], 0x1e0);
    } else {
      memcpy(fm7_bootroms[FM7_MAINMEM_BOOTROM_RAM - FM7_MAINMEM_BOOTROM_BAS], &initiator_rom[0x1a00], 0x1e0);
    }
    patch_bootrom();
  }
}

bool FM77AV_MAINMEM::get_loadstat_initrom(void)
{
  return diag_load_initrom;
}
