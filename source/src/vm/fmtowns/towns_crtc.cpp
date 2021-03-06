/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ FM-Towns CRTC ]
	History: 2016.12.28 Initial from HD46505 .
*/

#include "towns_crtc.h"

namespace FMTOWNS {
enum {
	EVENT_CRTC_VSTART = 1,
	EVENT_CRTC_VST1   = 2,
	EVENT_CRTC_VST2   = 3,
	EVENT_CRTC_HSTART = 4,
	EVENT_CRTC_HEND   = 5,
	EVENT_CRTC_HSW    = 6,
	EVENT_CRTC_EET    = 7,
	EVENT_CRTC_VDS    = 32,
	EVENT_CRTC_VDE    = 34,
	EVENT_CRTC_HDS    = 36,
	EVENT_CRTC_HDE    = 38,
	
};


void TOWNS_CRTC::initialize()
{
	memset(regs, 0, sizeof(regs));
	memset(regs_written, 0, sizeof(regs_written));

	line_count[0] = line_count[1] = 0;
	for(int i = 0; i < TOWNS_CRTC_MAX_LINES; i++) {
		line_changed[0][i] = true;
		line_changed[1][i] = true;
	}
	event_id_hsync = -1;
	event_id_hsw = -1;
	event_id_vsync = -1;
	event_id_vstart = -1;
	event_id_vst1 = -1;
	event_id_vst2 = -1;
	event_id_vblank = -1;
	
	// register events
	//register_frame_event(this);
	//register_vline_event(this);
	
}

void TOWNS_CRTC::reset()
{
	// initialize
	display = false;
	vblank = vsync = hsync = true;
	
//	memset(regs, 0, sizeof(regs));
	ch = 0;
	
	// initial settings for 1st frame
#ifdef CHARS_PER_LINE
	hz_total = (CHARS_PER_LINE > 54) ? CHARS_PER_LINE : 54;
#else
	hz_total = 54;
#endif
	hz_disp = (hz_total > 80) ? 80 : 40;
	hs_start = hz_disp + 4;
	hs_end = hs_start + 4;
	
	vt_total = LINES_PER_FRAME;
	vt_disp = (SCREEN_HEIGHT > LINES_PER_FRAME) ? (SCREEN_HEIGHT >> 1) : SCREEN_HEIGHT;
	vs_start = vt_disp + 16;
	vs_end = vs_start + 16;
	
	timing_changed = false;
	disp_end_clock = 0;
	
#if defined(TOWNS_CRTC_CHAR_CLOCK)
	char_clock = 0;
	next_char_clock = TOWNS_CRTC_CHAR_CLOCK;
#elif defined(TOWNS_CRTC_HORIZ_FREQ)
	horiz_freq = 0;
	next_horiz_freq = TOWNS_CRTC_HORIZ_FREQ;
#endif

	line_count[0] = line_count[1] = 0;
	for(int i = 0; i < TOWNS_CRTC_MAX_LINES; i++) {
		line_changed[0][i] = true;
		line_rendered[0][i] = false;
		line_changed[1][i] = true;
		line_rendered[1][i] = false;
	}
	if(event_id_hsync  >= 0) cancel_event(this, event_id_hsync);
	if(event_id_hsw    >= 0) cancel_event(this, event_id_hsw);
	if(event_id_vsync  >= 0) cancel_event(this, event_id_vsync);
	if(event_id_vstart >= 0) cancel_event(this, event_id_vstart);
	if(event_id_vst1   >= 0) cancel_event(this, event_id_vst1);
	if(event_id_vst2   >= 0) cancel_event(this, event_id_vst2);
	if(event_id_vblank >= 0) cancel_event(this, event_id_vblank);
	
	event_id_hsw = -1;
	event_id_vsync = -1;
	event_id_vstart = -1;
	event_id_vst1 = -1;
	event_id_vst2 = -1;
	event_id_vblank = -1;

	// Register vstart
	register_event(this, EVENT_CRTC_VSTART, vstart_us, false, &event_id_vstart);
}
// CRTC register #29
void TOWNS_CRTC::set_crtc_clock(uint16_t val)
{
	scsel = (val & 0x0c) >> 2;
	clksel = val & 0x03;
	static const double clocks[] = {
		28.6363e6, 24.5454e6, 25.175e6, 21.0525e6
	};
	if(crtc_clock[clksel] != crtc_clock) {
		crtc_clock = crtc_clock[clksel];
		force_recalc_crtc_param();
	}
}

void TOWNS_CRTC::force_recalc_crtc_param(void)
{
	horiz_width_posi_us = crtc_clock * ((double)(regs[0] & 0x00fe)); // HSW1
	horiz_width_nega_us = crtc_clock * ((double)(regs[1] & 0x00fe)); // HSW2
	horiz_us = crtc_clock * ((double)((regs[4] & 0x07fe) + 1)); // HST
	vsync_pre_us = ((double)(regs[5] & 0x1f)) * horiz_us; // VST1

	double horiz_ref = horiz_us / 2.0;
	
	vst2_us = ((double)(regs[6] & 0x1f)) * horiz_ref; // VST2
	eet_us = ((double)(regs[7] & 0x1f)) * horiz_ref;  // EET
	frame_us = ((double)(regs[8] & 0x07ff)) * horiz_ref; // VST

	for(int layer = 0; layer < 2; layer++) {
		vert_start_us[layer] =  ((double)(regs[(layer << 1) + 13] & 0x07ff)) * horiz_ref;   // VDSx
		vert_end_us[layer] =    ((double)(regs[(layer << 1) + 13 + 1] & 0x07ff)) * horiz_ref; // VDEx
		horiz_start_us[layer] = ((double)(regs[(layer << 1) + 9] & 0x07ff)) * crtc_clock ;   // HDSx
		horiz_end_us[layer] =   ((double)(regs[(layer << 1) + 9 + 1] & 0x07ff)) * crtc_clock ;   // HDEx
	}
		
}


void TONWS_CRTC::write_io8(uint32_t addr, uint32_t data)
{
	if(addr == 0x0440) {
		ch = data & 0x1f;
	} else if(addr == 0x0442) {
		pair16_t rdata;
		rdata.w = regs[ch];
		rdata.l = (uint8_t)data;
		write_io16(addr, rdata.w);
	} else if(addr == 0x0443) {
		pair16_t rdata;
		rdata.w = regs[ch];
		rdata.h = (uint8_t)data;
		write_io16(addr, rdata.w);
	}		
}

void TOWNS_CRTC::write_io16(uint32_t addr, uint32_t data)
{
	if((addr & 0xfffe) == 0x0442) {
		if(ch < 32) {
			if((ch < 0x09) && ((ch >= 0x04) || (ch <= 0x01))) { // HSW1..VST
				if(regs[ch] != (uint16_t)data) {
					force_recalc_crtc_param();
				}
			} else if(ch < 0x11) { // HDS0..VDE1
				uint8_t localch = ((ch  - 0x09) / 2) & 1;
				if(regs[ch] != (uint16_t)data) {
					timing_changed[localch] = true;
					// ToDo: Change render parameter within displaying.
					force_recalc_crtc_param();
				}
			}else if(ch < 0x19) { // FA0..LO1
				uint8_t localch = (ch - 0x11) / 4;
				uint8_t localreg = (ch - 0x11) & 3;
				if(regs[ch] != (uint16_t)data) {
					address_changed[localch] = true;
					switch(localreg) {
					case 0: // FAx
						vstart_addr[localch] = (uint32_t)(data & 0xffff);
						break;
					case 1: // HAJx
						hstart_words[localch] = (uint32_t)(data & 0x07ff);
						break;
					case 2: // FOx
						frame_offet[localch] = (uint32_t)(data & 0xffff);
						break;
					case 3: // LOx
						line_offet[localch] = (uint32_t)(data & 0xffff);
						break;
					}					
				}
			} else  { // All reg
				if(regs[ch] != (uint16_t)data) {
					switch(ch - 0x19) {
					case 0: // EHAJ
					case 1: // EVAJ
						// ToDo: External SYNC.
						break;
					case 2: // ZOOM
						{
							uint8_t zfv[2];
							uint8_t zfh[2];
							pair16_t pd;
							pd.w = (uint16_t)data;
							zfv[0] = ((pd.l & 0xf0) >> 4) + 1;
							zfh[0] = (pd.l & 0x0f) + 1;
							zfv[1] = ((pd.h & 0xf0) >> 4) + 1;
							zfh[1] = (pd.h & 0x0f) + 1;
							if((zfv[0] != zoom_factor_vert[0]) || (zfh[0] != zoom_factor_horiz[0])) {
								timing_changed[0] = true;
								address_changed[0] = true;
								if(zfv[0] != zoom_factor_vert[0]) zoom_count_vert[0] = zfv[0];
							}
							if((zfv[1] != zoom_factor_vert[1]) || (zfh[1] != zoom_factor_horiz[1])) {
								timing_changed[0] = true;
								address_changed[0] = true;
								if(zfv[1] != zoom_factor_vert[1]) zoom_count_vert[1] = zfv[1];
							}
							zoom_factor_vert[0]  = zfv[0];
							zoom_factor_horiz[0] = zfh[0];
							zoom_factor_vert[1]  = zfv[1];
							zoom_factor_horiz[1] = zfh[1];
						}
						break;
					case 3: // CR0
						if(regs[ch] != data) {
							if((data & 0x8000) == 0) {
								// START BIT
								restart_display();
							} else {
								stop_display();
							}
							if((data & 0x4000) == 0) {
								// ESYN BIT
								// EXTERNAL SYNC OFF
							} else {
								// EXTERNAL SYNC ON
							}
							impose_mode[1]  = ((data & 0x0080) == 0);
							impose_mode[0]  = ((data & 0x0040) == 0);
							carry_enable[1] = ((data & 0x0020) != 0);
							carry_enable[0] = ((data & 0x0010) != 0);
							uint8_t dmode[2];
							dmode[0] = data & 0x03;
							dmode[1] = (data & 0x0c) >> 2;
							for(int i = 0; i < 2; i++) {
								if(dmode[0] != display_mode[0]) {
									mode_changed[0] = true;
									notify_mode_changed(i, dmode[i]);
								}
								display_mode[i] = dmode[i];
							}
						}
						break;
					case 4: // CR1
						set_crtc_clock((uint16_t)data);
						break;
					case 5: // Reserved(REG#30)
						break;
					case 6: // CR2
						// ToDo: External trigger.
						break;
					}
				}
			}
			regs[ch] = (uint16_t)data;
		}
	} else if(addr == 0x0440) {
		ch = data & 0x1f;
	}
}

uint16_t TOWNS_CRTC::read_reg30()
{
	uint16_t data = 0x00f0;
	data |= ((frame_in[1])   ?  0x8000 : 0);
	data |= ((frame_in[0])   ?  0x4000 : 0);
	data |= ((hdisp[1])      ?  0x2000 : 0);
	data |= ((hdisp[0])      ?  0x1000 : 0);
	//data |= ((eet)          ?  0x0800 : 0);
	data |= ((vsync)         ?  0x0400 : 0);
	data |= (!(hsync)        ?  0x0200 : 0);
	//data |= ((video_in)     ? 0x0100 : 0);
	//data |= ((half_tone)    ? 0x0008 : 0);
	//data |= ((sync_enable)  ? 0x0004 : 0);
	//data |= ((vcard_enable) ? 0x0002 : 0);
	//data |= ((sub_carry)    ? 0x0001 : 0);
	
}

uint32_t TOWNS_CRTC::read_io16(uint32_t addr)
{
	addr = addr & 0xfffe;
	if(addr == 0x0440) {
		return (uint32_t)ch;
	} else if(addr == 0x0442) {
		if(ch == 30) {
			return (uint32_t)read_reg30();
		} else {
			return regs[ch];
		}
	}
	return 0xffff;
}

uint32_t TOWNS_CRTC::read_io8(uint32_t addr)
{
	if(addr == 0x0440) {
		return (uint32_t)ch;
	} else if(addr == 0x0442) {
		pair16_t d;
		if(ch == 30) {
			d.w = read_reg32();
		} else {
			d.w = regs[ch];
		}
		return (uint32_t)(d.l);
	} else if(addr == 0x0443) {
		pair16_t d;
		if(ch == 30) {
			d.w = read_reg32();
		} else {
			d.w = regs[ch];
		}
		return (uint32_t)(d.h);
	}
	return 0xff;
}

// Note: This entry 
void TOWNS_CRTC::event_line_per_layer(int display_line, int layer)
{
	uint32_t startaddr;
	if(display_line < 0) return;
	
	layer = layer & 1;
	if((one_layer_mode) && (layer != 0)) {
		d_vram->write_signal(SIG_TOWNS_VRAM_DONT_RENDER, (layer << 24) | ((uint32_t)display_line), 0x100fffff);
		return; // Not render.
	}
	zoom_count_vert[layer]--;
	if((zoom_count_vert[layer] == 0) || (display_line == 0)) {
		int vline = vram_line[layer];
		
		startaddr = vstart_addr[layer];
		if(zoom_count_vert[layer] == 0) vram_line[layer]++;
		zoom_count_vert[layer] = zoom_factor_vert[layer];
		
		uint32_t offset_words = (regs[(layer * 4) + 18] & 0x07ff) - (regs[(layer * 2) + 9] & 0x07ff); // HAJx - HDSx
		uint32_t offset_per_line = line_offset[layer];
		uint32_t draw_width  = (regs[0x09 + layer * 2 + 1] & 0x07ff) - (regs[0x09 + layer * 2 + 0] & 0x07ff); // HDEx-HDSx
		
		// ToDo: Interlace mode
		if(one_layer_mode) {
			offset_per_line = offset_per_line * 4 * 2; // 4Words.
			offset_words    = offset_words    * 4 * 2; // 4Words.
		} else {
			offset_per_line = offset_per_line * 2 * 2; // 2Words.
			offset_words    = offset_words    * 2 * 2; // 2Words.
		}

		startaddr = startaddr + offset_per_line * vline;
	
		d_vram->write_signal(SIG_TOWNS_VRAM_SET_PARAM,  layer, 0x1); // Begin modify rendering factor
		d_vram->write_signal(SIG_TOWNS_VRAM_PIX_COUNT,  draw_width, 0xfffffff); // Pixel count
		d_vram->write_signal(SIG_TOWNS_VRAM_START_ADDR, startaddr, 0xffffffff); // VRAM START ADDRESS
		d_vram->write_signal(SIG_TOWNS_VRAM_HZOOM, zoom_factor_horiz[layer], 0x0f); // HZOOM factor
//		d_vram->write_signal(SIG_TOWNS_VRAM_VZOOM, zoom_factor_vert[layer], 0x0f); // OFFSET WORD
		d_vram->write_signal(SIG_TOWNS_VRAM_OFFSET_WORDS, offset_words, 0xffffffff); // OFFSET WORD
	} else {
		d_vram->write_signal(SIG_TOWNS_VRAM_KEEP_PARAM, layer, 0x1); // Begin modify rendering factor
	}	
	if(!(frame_in[layer])) {
		d_vram->write_signal(SIG_TOWNS_VRAM_DONT_RENDER, (layer << 24) | ((uint32_t)display_line), 0x100fffff);
	} else {
		d_vram->write_signal(SIG_TOWNS_VRAM_DO_RENDER, (layer << 24) | ((uint32_t)display_line), 0x100fffff);
	}		
	if(!(carry_enable[layer])) {
		vram_line[layer] = vram_line[layer] & 0x00ff;
	}
}

void TOWNS_CRTC::event_pre_frame()
{
	// ToDo: Resize frame buffer.
}

void TOWNS_CRTC::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	cpu_clocks = new_clocks;
	frames_per_sec = new_frames_per_sec;
	max_lines = new_lines_per_frame;
	max_frame_usec = 1.0e6 / frames_per_sec;
}

void TOWNS_CRTC::event_callback(int event_id, int err)
{
	/*
	 * Related CRTC registers:
	 * HST, HSW1, HSW2 : HSYNC
	 * VST, VST1, VST2 : VSYNC
	 * (EET: for interlace : still not implement)
	 * VDS0, VDE0, HDS0, HDE0 : Display timing for Layer0
	 * VDS1, VDE1, HDS1, HDE1 : Display timing for Layer1
	 * FA0, HAJ0, LO0, FO0  : ToDo (For calculating address)
	 * FA1, HAJ1, LO1, FO1  : ToDo (For calculating address)
	 * ZOOM (#27) : ToDo
	 */
	int eid2 = (event_id / 2) * 2;
	if(event_id == EVENT_CRTC_VSTART) {
		d_vram->write_signal(SIG_TOWNS_VRAM_VSTART, 0x01, 0x01);
		line_count[0] = line_count[1] = 0;
		if((horiz_us != next_horiz_us) || (vert_us != next_vert_us)) {
			horiz_us = next_horiz_us;
			vert_us = next_vert_us;
		}
		hsync = false;
		for(int i = 0; i < 2; i++) {
			hdisp[i] = false;
		}
		major_line_count = -1;
		// ToDo: EET
		register_event(this, EVENT_CRTC_VSTART, frame_us, false, &event_id_frame);
		if(vert_sync_pre_us >= 0.0) {
			vsync = false;
			register_event(this, EVENT_CRTC_VST1, vert_sync_pre_us, false, &event_id_vst1); // VST1
		} else {
			vsync = true;
		}
		register_event(this, EVENT_CRTC_VST2, vst2_us, false, &event_id_vst2);
		for(int i = 0; i < 2; i++) {
			frame_in[i] = false;
			if(event_id_vds[i] != -1) {
				cancel_event(this, event_id_vds[i]);
			}
			if(event_id_vde[i] != -1) {
				cancel_event(this, event_id_vde[i]);
			}
			if(vert_start_us[i] > 0.0) {
				register_event(this, EVENT_CRTC_VDS + i, vert_start_us[i], false, &event_id_vds[i]); // VDSx
			} else {
				frame_in[i] = true;
			}
			if(vert_end_us[i] > 0.0) {
				register_event(this, EVENT_CRTC_VDE + i, vert_end_us[i],   false, &event_id_vde[i]); // VDEx
			}
		}
		
		if(event_id_hstart != -1) {
			cancel_event(this, event_id_hstart);
			event_id_hstart = -1;
		}
		if(event_id_hsw != -1) {
			cancel_event(this, event_id_hsw);
			event_id_hsw = -1;
		}
	} else if(event_id == EVENT_CRTC_VST1) { // VSYNC
		vsync = true;
	} else if (event_id == EVENT_CRTC_VST2) {
		vsync = false;
		event_id_vstn = -1;
	} else if(eid2 == EVENT_CRTC_VDS) { // Display start
		int layer = event_id & 1;
		frame_in[layer] = true;
		// DO ofset line?
		event_id_vstart[layer] = -1;
	} else if(eid2 == EVENT_CRTC_VDE) { // Display end
		int layer = event_id & 1;
		frame_in[layer] = false;
		event_id_vend[layer] = -1;
		// DO ofset line?
	} else if(event_id == EVENT_CRTC_HSTART) {
		// Do render
		event_id_hstart = -1;
		major_line_count++;
		hdisp[0] = false;
		hdisp[1] = false;
		if(event_id_hsw != -1) {
			cancel_event(this, event_id_hsw);
			event_id_hsw = -1;
		}
		if(!vsync) {
			hsync = true;
			register_event(this, EVENT_CRTC_HSW, horiz_width_posi_us, false, &event_id_hsw); // VDEx
		} else {
			hsync = true;
			register_event(this, EVENT_CRTC_HSW, horiz_width_nega_us, false, &event_id_hsw); // VDEx
		}
		for(int i = 0; i < 2; i++) {
			if(event_id_hds[i] != -1) {
				cancel_event(this, event_id_hds[i]);
			}
			event_id_hds[i] = -1;
			if(event_id_hde[i] != -1) {
				cancel_event(this, event_id_hde[i]);
			}
			event_id_hde[i] = -1;
			
			if(horiz_start_us[i] > 0.0) {
				register_event(this, EVENT_CRTC_HDS + i, horiz_start_us[i], false, &event_id_hds[i]); // HDS0
			} else {
				hdisp[i] = true;
				if(!vsync) {
					event_line_per_layer(major_line_count, layer);
				}
			}
			if((horiz_end_us[i] > 0.0) && (horiz_end_us[i] > horiz_start_us[i])) {
				register_event(this, EVENT_CRTC_HDE + i, horiz_end_us[i], false, &event_id_hde[i]); // HDS0
			}
		}

		register_event(this, EVENT_CRTC_HSTART, horiz_us, false, &event_id_hstart); // HSTART
	} else if(event_id == EVENT_CRTC_HSW) {
		hsync = false;
		event_id_hsw = -1;
	} else if(eid2 == EVENT_CRTC_HDS) {
		int layer = event_id & 1;
		hdisp[layer] = true;
		if(!vsync) {
			event_line_per_layer(major_line_count, layer);
		}
		if((horiz_end_us[i] <= 0.0) || (horiz_end_us[i] <= horiz_start_us[i])) {
			hdisp[layer] = false;
		}
		event_id_hds[layer] = -1;
	} else if(eid2 == EVENT_CRTC_HDE) {
		int layer = event_id & 1;
		hdisp[layer] = false;	
		event_id_hde[layer] = -1;
	}

}

void TOWNS_CRTC::write_signal(int ch, uint32_t data, uint32_t mask)
{
	if(ch == SIG_TONWS_CRTC_SINGLE_LAYER) {
		one_layer_mode = ((data & mask) == 0);
	}
}

#define STATE_VERSION	1

void TOWNS_CRTC::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	state_fio->Fwrite(regs, sizeof(regs), 1);
	state_fio->Fwrite(regs_written, sizeof(regs_written), 1);
	state_fio->FputInt32(ch);
	state_fio->FputBool(timing_changed);
	state_fio->FputInt32(cpu_clocks);
#if defined(TOWNS_CRTC_CHAR_CLOCK)
	state_fio->FputDouble(char_clock);
	state_fio->FputDouble(next_char_clock);
#elif defined(TOWNS_CRTC_HORIZ_FREQ)
	state_fio->FputDouble(horiz_freq);
	state_fio->FputDouble(next_horiz_freq);
#endif
	state_fio->FputDouble(frames_per_sec);
	state_fio->FputInt32(hz_total);
	state_fio->FputInt32(hz_disp);
	state_fio->FputInt32(hs_start);
	state_fio->FputInt32(hs_end);
	state_fio->FputInt32(vt_total);
	state_fio->FputInt32(vt_disp);
	state_fio->FputInt32(vs_start);
	state_fio->FputInt32(vs_end);
	state_fio->FputInt32(disp_end_clock);
	state_fio->FputInt32(hs_start_clock);
	state_fio->FputInt32(hs_end_clock);
	state_fio->FputBool(display);
	state_fio->FputBool(vblank);
	state_fio->FputBool(vsync);
	state_fio->FputBool(hsync);
}

bool TOWNS_CRTC::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(regs, sizeof(regs), 1);
	state_fio->Fread(regs_written, sizeof(regs_written), 1);
	ch = state_fio->FgetInt32();
	timing_changed = state_fio->FgetBool();
	cpu_clocks = state_fio->FgetInt32();
#if defined(TOWNS_CRTC_CHAR_CLOCK)
	char_clock = state_fio->FgetDouble();
	next_char_clock = state_fio->FgetDouble();
#elif defined(TOWNS_CRTC_HORIZ_FREQ)
	horiz_freq = state_fio->FgetDouble();
	next_horiz_freq = state_fio->FgetDouble();
#endif
	frames_per_sec = state_fio->FgetDouble();
	hz_total = state_fio->FgetInt32();
	hz_disp = state_fio->FgetInt32();
	hs_start = state_fio->FgetInt32();
	hs_end = state_fio->FgetInt32();
	vt_total = state_fio->FgetInt32();
	vt_disp = state_fio->FgetInt32();
	vs_start = state_fio->FgetInt32();
	vs_end = state_fio->FgetInt32();
	disp_end_clock = state_fio->FgetInt32();
	hs_start_clock = state_fio->FgetInt32();
	hs_end_clock = state_fio->FgetInt32();
	display = state_fio->FgetBool();
	vblank = state_fio->FgetBool();
	vsync = state_fio->FgetBool();
	hsync = state_fio->FgetBool();

	return true;
}

}
