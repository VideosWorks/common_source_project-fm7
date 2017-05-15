/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
    Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.10
	History: 2015.11.10 Split from qt_main.cpp
	Note: This class must be compiled per VM, must not integrate shared units.
	[ win32 main ] -> [ Qt main ] -> [Emu Thread] -> [Independed from VMs]
*/

#include <QString>
#include <QTextCodec>
#include <QWaitCondition>

#include <SDL.h>

#include "emu_thread_tmpl.h"

#include "qt_gldraw.h"
#include "../../romakana.h"

//#include "csp_logger.h"
#include "menu_flags.h"

EmuThreadClassBase::EmuThreadClassBase(META_MainWindow *rootWindow, USING_FLAGS *p, QObject *parent) : QThread(parent) {
	MainWindow = rootWindow;
	using_flags = p;
	p_config = p->get_config_ptr();
	
	bRunThread = true;
	prev_skip = false;
	tick_timer.start();
	update_fps_time = tick_timer.elapsed();
	next_time = update_fps_time;
	total_frames = 0;
	draw_frames = 0;
	skip_frames = 0;
	calc_message = true;
	mouse_flag = false;

	drawCond = new QWaitCondition();
	mouse_x = 0;
	mouse_y = 0;
	if(using_flags->is_use_tape() && !using_flags->is_tape_binary_only()) {
		tape_play_flag = false;
		tape_rec_flag = false;
		tape_pos = 0;
	}

	if(using_flags->get_use_sound_volume() > 0) {
		for(int i = 0; i < using_flags->get_use_sound_volume(); i++) {
			bUpdateVolumeReq[i] = true;
			volume_avg[i] = (using_flags->get_config_ptr()->sound_volume_l[i] +
							 using_flags->get_config_ptr()->sound_volume_r[i]) / 2;
			volume_balance[i] = (using_flags->get_config_ptr()->sound_volume_r[i] -
								 using_flags->get_config_ptr()->sound_volume_l[i]) / 2;
		}
	}
	memset(roma_kana_buffer, 0x00, sizeof(roma_kana_buffer));
	roma_kana_ptr = 0;
	roma_kana_queue.clear();
	roma_kana_conv = false;
	romakana_conversion_mode = false;

	key_up_code_fifo = new FIFO(4096);
	key_up_mod_fifo = new FIFO(4096);
	key_up_repeat_fifo = new FIFO(4096);

	key_down_code_fifo = new FIFO(4096);
	key_down_mod_fifo = new FIFO(4096);
	key_down_repeat_fifo = new FIFO(4096);
};

EmuThreadClassBase::~EmuThreadClassBase() {
	delete drawCond;
	
	delete key_up_code_fifo;
	delete key_up_mod_fifo;
	delete key_up_repeat_fifo;
	
	delete key_down_code_fifo;
	delete key_down_mod_fifo;
	delete key_down_repeat_fifo;
};

void EmuThreadClassBase::calc_volume_from_balance(int num, int balance)
{
	int level = volume_avg[num];
	int right;
	int left;
	volume_balance[num] = balance;
	right = level + balance;
	left  = level - balance;
	using_flags->get_config_ptr()->sound_volume_l[num] = left;	
	using_flags->get_config_ptr()->sound_volume_r[num] = right;
}

void EmuThreadClassBase::calc_volume_from_level(int num, int level)
{
	int balance = volume_balance[num];
	int right,left;
	volume_avg[num] = level;
	right = level + balance;
	left  = level - balance;
	using_flags->get_config_ptr()->sound_volume_l[num] = left;	
	using_flags->get_config_ptr()->sound_volume_r[num] = right;
}

void EmuThreadClassBase::doExit(void)
{
	bRunThread = false;
}

void EmuThreadClassBase::button_pressed_mouse(Qt::MouseButton button)
{
	if(using_flags->is_use_mouse()) {
		button_pressed_mouse_sub(button);
	} else {		
		if(using_flags->get_max_button() > 0) {
			button_desc_t *vm_buttons_d = using_flags->get_vm_buttons();
			if(vm_buttons_d == NULL) return;
			switch(button) {
			case Qt::LeftButton:
			case Qt::RightButton:
				for(int i = 0; i < using_flags->get_max_button(); i++) {
					if((mouse_x >= vm_buttons_d[i].x) &&
					   (mouse_x < (vm_buttons_d[i].x + vm_buttons_d[i].width))) {
						if((mouse_y >= vm_buttons_d[i].y) &&
						   (mouse_y < (vm_buttons_d[i].y + vm_buttons_d[i].height))) {
							if(vm_buttons_d[i].code != 0x00) {
								key_queue_t sp;
								sp.code = vm_buttons_d[i].code;
								sp.mod = key_mod;
								sp.repeat = false;
								enqueue_key_down(sp);
							} else {
								bResetReq = true;
							}
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}
}

void EmuThreadClassBase::button_released_mouse(Qt::MouseButton button)
{
	if(using_flags->is_use_mouse()) {
		button_released_mouse_sub(button);
	} else {
		if(using_flags->get_max_button() > 0) {
			button_desc_t *vm_buttons_d = using_flags->get_vm_buttons();
			if(vm_buttons_d == NULL) return;
			
			switch(button) {
			case Qt::LeftButton:
			case Qt::RightButton:
				for(int i = 0; i < using_flags->get_max_button(); i++) {
					if((mouse_x >= vm_buttons_d[i].x) &&
					   (mouse_x < (vm_buttons_d[i].x + vm_buttons_d[i].width))) {
						if((mouse_y >= vm_buttons_d[i].y) &&
						   (mouse_y < (vm_buttons_d[i].y + vm_buttons_d[i].height))) {
							if(vm_buttons_d[i].code != 0x00) {
								key_queue_t sp;
								sp.code = vm_buttons_d[i].code;
								sp.mod = key_mod;
								sp.repeat = false;
								enqueue_key_up(sp);
							}
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}
}


void EmuThreadClassBase::do_key_down(uint32_t vk, uint32_t mod, bool repeat)
{
	key_queue_t sp;
	sp.code = vk;
	sp.mod = mod;
	sp.repeat = repeat;
	//key_changed = true;
	if(using_flags->is_use_roma_kana_conversion()) {
		if(p_config->roma_kana_conversion) {
			if((vk == VK_F12)) {
				romakana_conversion_mode = !romakana_conversion_mode;
				//emit sig_romakana_mode(romakana_conversion_mode);
				return;
			}
			if(roma_kana_ptr < (int)(sizeof(roma_kana_buffer) / sizeof(_TCHAR)) &&
			   (((vk >= 'A') && (vk <= 'z')) ||
				(vk == VK_OEM_4) || (vk == VK_OEM_6) ||
				(vk == VK_OEM_2) || (vk == VK_OEM_COMMA) ||
				(vk == VK_OEM_PERIOD) || (vk == VK_OEM_MINUS)) &&
				romakana_conversion_mode) {
				return;
			}
		}
	}
	enqueue_key_down(sp);
	key_mod = mod;
}

void EmuThreadClassBase::do_key_up(uint32_t vk, uint32_t mod)
{
	key_queue_t sp;
	sp.code = vk;
	sp.mod = mod;
	sp.repeat = false;

	if(using_flags->is_use_roma_kana_conversion()) {
		if(p_config->roma_kana_conversion && romakana_conversion_mode) {
			if(roma_kana_ptr < (int)(sizeof(roma_kana_buffer) / sizeof(_TCHAR)) &&
			   (((vk >= 'A') && (vk <= 'z')) ||
				(vk == VK_OEM_4) || (vk == VK_OEM_6) ||
				(vk == VK_OEM_2) || (vk == VK_OEM_COMMA) ||
				(vk == VK_OEM_PERIOD) || (vk == VK_OEM_MINUS))) {
				wchar_t kana_buffer[12];
				int kana_len = 10;
				memset(kana_buffer, 0x00, sizeof(kana_buffer));
				uint32_t vvk = vk;
				switch(vk) {
				case VK_OEM_MINUS:
					vvk = '-';
					break;
				case VK_OEM_4:
					vvk = '[';
					break;
				case VK_OEM_6:
					vvk = ']';
					break;
				case VK_OEM_COMMA:
					vvk = ',';
					break;
				case VK_OEM_PERIOD:
					vvk = '.';
					break;
				case VK_OEM_2:
					vvk = '/';
					break;
				}
				roma_kana_shadow[roma_kana_ptr] = vk;
				roma_kana_buffer[roma_kana_ptr++] = (_TCHAR)vvk;
				if(alphabet_to_kana((const _TCHAR *)roma_kana_buffer, kana_buffer, &kana_len) > 0) {
					if(kana_len > 0) {
						QString kana_str = QString::fromWCharArray((const wchar_t *)kana_buffer, kana_len);
						roma_kana_queue.enqueue(kana_str);
						//printf("%s\n", kana_str.toUtf8().constData());
					}
					roma_kana_ptr = 0;
					memset(roma_kana_buffer, 0x00, sizeof(roma_kana_buffer));
					memset(roma_kana_shadow, 0x00, sizeof(roma_kana_shadow));
					roma_kana_conv = true;
				}
				return;
			} else {
				if(roma_kana_ptr > 0) {
					for(int i = 0; i < roma_kana_ptr; i++) {
						key_queue_t sss;
						sss.code = roma_kana_shadow[i];
						sss.mod = mod;
						sss.repeat = false;
						enqueue_key_down(sss);
						enqueue_key_up(sss);
					}
					memset(roma_kana_buffer, 0x00, sizeof(roma_kana_buffer));
					memset(roma_kana_shadow, 0x00, sizeof(roma_kana_shadow));
					roma_kana_ptr = 0;
				}
			}
		}
	}
	enqueue_key_up(sp);
	key_mod = mod;
}

void EmuThreadClassBase::set_tape_play(bool flag)
{
	tape_play_flag = flag;
}

void EmuThreadClassBase::resize_screen(int screen_width, int screen_height, int stretched_width, int stretched_height)
{
	emit sig_resize_screen(screen_width, screen_height);
}

void EmuThreadClassBase::sample_access_drv(void)
{
	if(using_flags->is_use_qd()) get_qd_string();
	if(using_flags->is_use_fd()) get_fd_string();
	if(using_flags->is_use_tape() && !using_flags->is_tape_binary_only()) get_tape_string();
	if(using_flags->is_use_compact_disc()) get_cd_string();
	if(using_flags->is_use_bubble()) get_bubble_string();
}

void EmuThreadClassBase::do_update_config()
{
	bUpdateConfigReq = true;
}

void EmuThreadClassBase::do_start_record_sound()
{
	bStartRecordSoundReq = true;
}

void EmuThreadClassBase::do_stop_record_sound()
{
	bStopRecordSoundReq = true;
}

void EmuThreadClassBase::do_reset()
{
	bResetReq = true;
}

void EmuThreadClassBase::do_special_reset()
{
	bSpecialResetReq = true;
}

void EmuThreadClassBase::do_load_state(QString s)
{
	sStateFile = s;
	bLoadStateReq = true;
}

void EmuThreadClassBase::do_save_state(QString s)
{
	sStateFile = s;
	bSaveStateReq = true;
}

void EmuThreadClassBase::do_start_record_video(int fps)
{
	record_fps = fps;
	bStartRecordMovieReq = true;
}

void EmuThreadClassBase::do_stop_record_video()
{
	bStartRecordMovieReq = false;
}

void EmuThreadClassBase::do_update_volume_level(int num, int level)
{
	if(using_flags->get_use_sound_volume() > 0) {
		if((num < using_flags->get_use_sound_volume()) && (num >= 0)) {
			calc_volume_from_level(num, level);
			bUpdateVolumeReq[num] = true;
		}
	}
}

void EmuThreadClassBase::do_update_volume_balance(int num, int level)
{
	if(using_flags->get_use_sound_volume() > 0) {
		if((num < using_flags->get_use_sound_volume()) && (num >= 0)) {
			calc_volume_from_balance(num, level);
			bUpdateVolumeReq[num] = true;
		}
	}
}
