/*
 * Common source code project -> FM-7 -> Display
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History:
 *  Feb 10, 2015 : Initial.
 */

#ifndef _CSP_FM7_DISPLAY_H
#define _CSP_FM7_DISPLAY_H

#include "../device.h"
#include "../memory.h"
#include "../mc6809.h"
#include "fm7_common.h"


class DEVICE;
class MEMORY;
class MC6809;

class DISPLAY: public MEMORY
{
 protected:
	EMU *p_emu;
	VM *p_vm;

	int nmi_count;
	int irq_count;
	int firq_count;
	int halt_count;
   
	void go_subcpu();
	void halt_subcpu();
   
	void do_nmi(bool);
	void do_irq(bool);
	void do_firq(bool);
	void set_multimode(uint8 val);
	uint8 get_multimode(void);
	uint8 get_cpuaccessmask(void);
	void set_dpalette(uint32 addr, uint8 val);
	uint8 get_dpalette(uint32 addr);
	void enter_display(void);
	void leave_display(void);
	void halt_subsystem(void);
	void restart_subsystem(void);
	void set_crtflag(void);
	void reset_crtflag(void);
	uint8 acknowledge_irq(void);
	uint8 beep(void);
	uint8 attention_irq(void);
	void set_cyclesteal(uint8 val);
	uint8 set_vramaccess(void);
	void reset_vramaccess(void);
	uint8 reset_subbusy(void);
	void set_subbusy(void);
#if defined(_FM77AV_VARIANTS)
	void alu_write_cmdreg(uint8 val);
	void alu_write_logical_color(uint8 val);
	void alu_write_mask_reg(uint8 val);
	void alu_write_mask_reg(int addr, uint8 val);
	void alu_write_disable_reg(uint8 val);
	void alu_write_tilepaint_data(int addr, uint8 val);
	void alu_write_offsetreg_hi(uint8 val);
	void alu_write_offsetreg_lo(uint8 val);
	void alu_write_linepattern_hi(uint8 val);
	void alu_write_linepattern_lo(uint8 val);
	void alu_write_line_position(int addr, uint8 val);
	void select_sub_bank(uint8 val);
	void select_vram_bank_av40(uint8 val);
	uint8 get_miscreg(void);
	void set_miscreg(uint8 val);
	void set_monitor_bank(uint8 var);
	void set_apalette_index_hi(uint8 val);
	void set_apalette_index_lo(uint8 val);
	void calc_apalette(uint32 index);
	void set_apalette_b(uint8 val);
	void set_apalette_r(uint8 val);
	void set_apalette_g(uint8 val);
#endif // _FM77AV_VARIANTS

 private:
	uint32  disp_mode;
	bool vblank;
	bool vsync;
	bool hblank;
	uint32 displine;

	bool subcpu_resetreq;

	DEVICE *ins_led;
	DEVICE *kana_led;
	DEVICE *caps_led;

	// Event handler
	int nmi_event_id;
	int hblank_event_id;
	int hdisp_event_id;
	int vsync_event_id;
	int vstart_event_id;
	int display_mode;

	scrntype dpalette_pixel[8];
	uint8 dpalette_data[8];
#if defined(_FM77AV_VARIANTS)
	int apalette_index;
	uint8 analog_palette_r[4096];
	uint8 analog_palette_g[4096];
	uint8 analog_palette_b[4096];
	scrntype apalette_pixel[4096];
#endif // FM77AV etc...
	int window_low;
	int window_high;
	int window_xbegin;
	int window_xend;
	bool window_opened;

	uint8 multimode_accessmask;
	uint8 multimode_dispmask;
   
	uint32 offset_point;
	uint32 tmp_offset_point;
	bool offset_changed;
	bool offset_77av;
   
#if defined(_FM77AV_VARIANTS)
	uint8 subrom_bank;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	bool monitor_ram;
#endif
#endif	
	uint8 gvram[0xc000];
#if defined(_FM77AV_VARIANTS)
	uint8 gvram_1[0xc000];
# if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
	uint8 gvram_2[0xc000];
# endif
#endif

	uint8 console_ram[0x1000];
	uint8 work_ram[0x380];
	uint8 shared_ram[0x80];

	uint8 subsys_c[0x2800];
#if defined(_FM77AV_VARIANTS)
	
	uint8 subsys_a[0x2000];
#endif

	bool sub_run;
	bool crt_flag;
	bool vram_accessflag;
	bool vram_wait;
	bool is_cyclesteal;
#if defined(_FM77L4) || defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
	uint32 kanji1_addr;
	MEMORY *kanjiclass1;
 #if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)	
	bool kanji_level2;
	uint32 kanji2_addr;
	MEMORY *kanjiclass2;
 #endif
#endif
#if defined(_FM77AV_VARIANTS)
	DEVICE *alu;
#endif	
	DEVICE *mainio;
	DEVICE *subcpu;
	DEVICE *keyboard;
	bool vram_wrote;
	inline int GETVRAM_8_200L(int yoff, scrntype *p, uint32 rgbmask);
 public:
	DISPLAY(VM *parent_vm, EMU *parent_emu);
	~DISPLAY();
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 read_signal(int id); 
	uint32 read_data8(uint32 addr);
	void write_data8(uint32 addr, uint32 data);
	void initialize(void);
	void reset(void);
	void update_config(void);
	
	void draw_screen(void);
	void event_frame(void);
	void event_vline(int v, int clock);


	void set_context_kanjiclass1(MEMORY *p)	{
#if defined(_FM77L4) || defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
		kanji1_addr = 0;
		kanjiclass1 = p;
#endif
	}
	void set_context_kanjiclass2(MEMORY *p)	{
#if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
		kanji2_addr = 0;
		kanjiclass2 = p;
		if(p != NULL) kanji_level2 = true;
#endif
	}
	void set_context_mainio(DEVICE *p) {
		mainio = p;
	}
	void set_context_keyboard(DEVICE *p) {
		keyboard = p;
	}
	void set_context_subcpu(DEVICE *p) {
		subcpu = p;
	}
#if defined(_FM77AV_VARIANTS)
	void set_context_alu(DEVICE *p) {
		alu = p;
	}
#endif
};  
#endif //  _CSP_FM7_DISPLAY_H
