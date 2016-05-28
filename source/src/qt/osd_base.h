/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ Qt dependent ]
*/

#ifndef _QT_OSD_BASE_H_
#define _QT_OSD_BASE_H_


#include <QWidget>
#include <QThread>
#include <QMutex>
#include <QSemaphore>
#include <QPainter>
#include <QElapsedTimer>
#include <QString>
#include <QQueue>

#include <SDL.h>
#include "simd_types.h"

#include <ctime>

//#include "../vm/vm.h"
//#include "../emu.h"
#include "../config.h"
#include "../fileio.h"
#include "../fifo.h"
#if !defined(Q_OS_WIN32)
#include "qt_input.h"
#endif
#define SOCKET_MAX 4
#define SOCKET_BUFFER_MAX 0x100000
#include "osd_types.h"


#define WM_RESIZE  (WM_USER + 1)
#define WM_SOCKET0 (WM_USER + 2)
#define WM_SOCKET1 (WM_USER + 3)
#define WM_SOCKET2 (WM_USER + 4)
#define WM_SOCKET3 (WM_USER + 5)

//#ifdef USE_VIDEO_CAPTURE
#define MAX_CAPTURE_DEVS 8
//#endif

#include "qt_main.h"
//#include "mainwidget.h"
#include "agar_logger.h"

class GLDrawClass;
class EmuThreadClass;
class DrawThreadClass;
class Ui_MainWindow;
class EMU;
class VM;
class FIFO;
class CSP_KeyTables;

QT_BEGIN_NAMESPACE
class OSD_BASE : public QThread
{
	Q_OBJECT
protected:
	EmuThreadClass *parent_thread;
	QSemaphore *VMSemaphore;
	QSemaphore *DebugSemaphore;
	sdl_snddata_t snddata;
//private:
	_TCHAR app_path[_MAX_PATH];
	QElapsedTimer osd_timer;
	bool locked_vm;
	
	// console
	FILE *hStdIn, *hStdOut;
	QString console_cmd_str;
	bool osd_console_opened;
	// input
	void initialize_input();
	void release_input();
	void key_down_sub(int code, bool repeat);
	void key_up_sub(int code);
	CSP_KeyTables *key_table;
	
	scrntype_t *get_buffer(bitmap_t *p, int y);
	bool dinput_key_ok;
//	bool dinput_joy_ok;
	
	uint8_t keycode_conv[256];
	uint8_t key_status[256];	// windows key code mapping
	uint8_t key_dik_prev[256];
	uint8_t key_converted[256];
	bool key_shift_pressed, key_shift_released;


	uint32_t modkey_status;
	bool lost_focus;
	uint32_t joy_status[4];	// joystick #1, #2 (b0 = up, b1 = down, b2 = left, b3 = right, b4- = buttons
	int joy_num;
	uint32_t joy_mask[4];
	
	int mouse_status[3];	// x, y, button (b0 = left, b1 = right)
	bool mouse_enabled;
	int mouse_ptrx;
	int mouse_ptry;
	int mouse_button;
	int mouse_oldx;
	int mouse_oldy;
	Qt::CursorShape mouse_shape;
	
	// printer
	
	// screen
	void initialize_screen();
	void release_screen();
	void initialize_screen_buffer(bitmap_t *buffer, int width, int height, int mode);
	void release_screen_buffer(bitmap_t *buffer);
	void rotate_screen_buffer(bitmap_t *source, bitmap_t *dest);
	
	void stretch_screen_buffer(bitmap_t *source, bitmap_t *dest);
	int add_video_frames();
	
	bitmap_t vm_screen_buffer;
	bitmap_t video_screen_buffer;
	bitmap_t* draw_screen_buffer;
	int vm_window_width, vm_window_height;
	int vm_window_width_aspect, vm_window_height_aspect;
	
	int host_window_width, host_window_height;
	bool host_window_mode;
	int base_window_width, base_window_height;
	int vm_screen_width, vm_screen_height;
	int draw_screen_width, draw_screen_height;
	
	
	_TCHAR video_file_name[_MAX_PATH];
	int rec_video_fps;
	double rec_video_run_frames;
	double rec_video_frames;
	
	//LPBITMAPINFO lpDibRec;
	//PAVIFILE pAVIFile;
	//PAVISTREAM pAVIStream;
	//PAVISTREAM pAVICompressed;
	//AVICOMPRESSOPTIONS AVIOpts;
	uint64_t dwAVIFileSize;
	uint64_t lAVIFrames;
	//HANDLE hVideoThread;
	rec_video_thread_param_t rec_video_thread_param;
	
	bool first_draw_screen;
	bool first_invalidate;
	bool self_invalidate;
	
	// sound
	void initialize_sound(int rate, int samples);
	void release_sound();
	static void audio_callback(void *udata, Uint8 *stream, int len);
	int sound_rate, sound_samples;
	bool sound_ok, sound_started, now_mute;
	bool sound_first_half;
	
	_TCHAR sound_file_name[_MAX_PATH];
	FILEIO* rec_sound_fio;
	int rec_sound_bytes;
	int rec_sound_buffer_ptr;
	
	int sound_buffer_size;
	int sound_data_len;
	int sound_data_pos;
	int sound_write_pos;
	bool sound_exit;
	bool sound_debug;
	SDL_sem *snd_apply_sem;
	Sint16 *sound_buf_ptr;
	Uint8 snd_total_volume;
#if defined(USE_SDL2)   
	SDL_AudioDeviceID audio_dev_id;
#else
	int audio_dev_id;
#endif
	SDL_AudioSpec snd_spec_req, snd_spec_presented;
	
	// video device
	void initialize_video();
	void release_video();
	
	//IGraphBuilder *pGraphBuilder;
	//IBaseFilter *pVideoBaseFilter;
	//IBaseFilter *pCaptureBaseFilter;
	//ICaptureGraphBuilder2 *pCaptureGraphBuilder2;
	//ISampleGrabber *pVideoSampleGrabber;
	//IBaseFilter *pSoundBaseFilter;
	//ISampleGrabber *pSoundSampleGrabber;
	//CMySampleGrabberCB *pSoundCallBack;
	//IMediaControl *pMediaControl;
	//IMediaSeeking *pMediaSeeking;
	//IMediaPosition *pMediaPosition;
	//IVideoWindow *pVideoWindow;
	//IBasicVideo *pBasicVideo;
	//IBasicAudio *pBasicAudio;
	//bool bTimeFormatFrame;
	//bool bVerticalReversed;
	
	bitmap_t dshow_screen_buffer;
	int direct_show_width, direct_show_height;
	bool direct_show_mute[2];

	double movie_frame_rate;
	int movie_sound_rate;

	void enum_capture_devs();
	bool connect_capture_dev(int index, bool pin);
	int cur_capture_dev_index;
	int num_capture_devs;
	_TCHAR capture_dev_name[MAX_CAPTURE_DEVS][256];

	_TCHAR prn_file_name[_MAX_PATH];
	FILEIO *prn_fio;
	int prn_data, prn_wait_frames;
	bool prn_strobe;

	// socket
	void initialize_socket();
	void release_socket();
	
	int soc[SOCKET_MAX];
	bool is_tcp[SOCKET_MAX];
	//struct sockaddr_in udpaddr[SOCKET_MAX];
	int socket_delay[SOCKET_MAX];
	char recv_buffer[SOCKET_MAX][SOCKET_BUFFER_MAX];
	int recv_r_ptr[SOCKET_MAX], recv_w_ptr[SOCKET_MAX];

	// wrapper
	virtual void vm_draw_screen(void);
	virtual Sint16* create_sound(int *extra_frames);
	virtual bool get_use_socket(void);
	virtual bool get_support_variable_timing(void);
	virtual bool get_notify_key_down(void);
	virtual bool get_notify_key_down_lr_shift(void);
	virtual bool get_notify_key_down_lr_control(void);
	virtual bool get_notify_key_down_lr_menu(void);
	virtual bool get_use_shift_numpad_key(void);
	virtual bool get_use_auto_key(void);
	virtual bool get_dont_keeep_key_pressed(void);
	virtual bool get_one_board_micro_computer(void);
	virtual bool get_use_screen_rotate(void);
	virtual bool get_use_movie_player(void);
	virtual bool get_use_video_capture(void);
	virtual void vm_key_down(int code, bool flag);
	virtual void vm_key_up(int code);
	virtual void vm_reset(void);
	virtual void update_buttons(bool press_flag, bool release_flag);
	virtual int get_screen_width(void);
	virtual int get_screen_height(void);
	virtual int get_vm_buttons_code(int num);

public:
	OSD_BASE();
	~OSD_BASE();
	
	// common
	VM* vm;
	//EMU* emu;
	class Ui_MainWindow *main_window_handle;
	GLDrawClass *glv;
	int host_cpus;
	bool now_auto_key;
	
	virtual void initialize(int rate, int samples);
	virtual void release();
	virtual void power_off();
	void suspend();
	void restore();
	_TCHAR* application_path();
	_TCHAR* bios_path(const _TCHAR* file_name);
	void get_host_time(cur_time_t* time);
	void sleep(uint32_t ms);
	void create_date_file_name(_TCHAR *name, int length, const _TCHAR *extension);
	_TCHAR  *get_app_path(void);
	// common console
	void open_console(_TCHAR* title);
	void close_console();
	unsigned int get_console_code_page();
	bool is_console_active();
	void set_console_text_attribute(unsigned short attr);
	void write_console(_TCHAR* buffer, unsigned int length);
	int read_console_input(_TCHAR* buffer);
	bool is_console_key_pressed(uint32_t ch);
	
	// common input
	void update_input();
	void key_down(int code, bool repeat);
	void key_up(int code);
	void key_down_native(int code, bool repeat);
	void key_up_native(int code);
	void key_lost_focus();
	void press_button(int num);

# if !defined(Q_OS_WIN) && !defined(Q_OS_CYGWIN)
	uint16_t GetAsyncKeyState(uint32_t vk);  // Win32 GetAsyncKeyState() wrappeer.
# endif
	void key_modifiers(uint32_t mod);
	void enable_mouse();
	void disable_mouse();
	void toggle_mouse();
	bool is_mouse_enabled();
	//QImage *getPseudoVramClass(void) { return pPseudoVram;}
	void set_mouse_pointer(int x, int y);
	void set_mouse_button(int button);
	int get_mouse_button();
	void modify_key_buffer(int code, uint8_t val);
	uint8_t* get_key_buffer();
	uint32_t* get_joy_buffer();
	int* get_mouse_buffer();
	// common printer
	void reset_printer();
	void update_printer();
	void printer_out(uint8_t value);
	void printer_strobe(bool value);
	// printer
	void initialize_printer();
	void release_printer();
	void open_printer_file();
	void close_printer_file();
	
	// common screen
	int get_window_mode_width(int mode);
	int get_window_mode_height(int mode);
	void set_host_window_size(int window_width, int window_height, bool window_mode);
	void set_vm_screen_size(int width, int height, int width_aspect, int height_aspect, int window_width, int window_height);
	int get_vm_window_width();
	int get_vm_window_height();
	int get_vm_window_width_aspect();
	int get_vm_window_height_aspect();
	scrntype_t* get_vm_screen_buffer(int y);
	int draw_screen();
	void reload_bitmap();
	void capture_screen();
	bool start_record_video(int fps);
	void stop_record_video();
	void restart_record_video();
	void add_extra_frames(int extra_frames);
	bool now_record_video;
	bool screen_skip_line;
	// common sound
	void update_sound(int* extra_frames);
	void mute_sound();
	void stop_sound();
	void start_record_sound();
	void stop_record_sound();
	void restart_record_sound();
	bool now_record_sound;
	int get_sound_rate();
	
	// common video device
	void get_video_buffer();
	void mute_video_dev(bool l, bool r);
	bool open_movie_file(const _TCHAR* file_path);
	void close_movie_file();
	void play_movie();
	void stop_movie();
	void pause_movie();
	double get_movie_frame_rate();
	int get_movie_sound_rate();
	void set_cur_movie_frame(int frame, bool relative);
	uint32_t get_cur_movie_frame();
	bool now_movie_play, now_movie_pause;
	int get_cur_capture_dev_index();
	int get_num_capture_devs();
	_TCHAR* get_capture_dev_name(int index);
	void open_capture_dev(int index, bool pin);
	void close_capture_dev();
	void show_capture_dev_filter();
	void show_capture_dev_pin();
	void show_capture_dev_source();
	void set_capture_dev_channel(int ch);
	
	// common printer
	void create_bitmap(bitmap_t *bitmap, int width, int height);
	void release_bitmap(bitmap_t *bitmap);
	void create_font(font_t *font, const _TCHAR *family, int width, int height, int rotate, bool bold, bool italic);
	void release_font(font_t *font);
	void create_pen(pen_t *pen, int width, uint8_t r, uint8_t g, uint8_t b);
	void release_pen(pen_t *pen);

	void clear_bitmap(bitmap_t *bitmap, uint8_t r, uint8_t g, uint8_t b);
	int get_text_width(bitmap_t *bitmap, font_t *font, const char *text);
	
	void draw_text_to_bitmap(bitmap_t *bitmap, font_t *font, int x, int y, const _TCHAR *text, uint8_t r, uint8_t g, uint8_t b);
	void draw_line_to_bitmap(bitmap_t *bitmap, pen_t *pen, int sx, int sy, int ex, int ey);
	void draw_rectangle_to_bitmap(bitmap_t *bitmap, int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b);
	void draw_point_to_bitmap(bitmap_t *bitmap, int x, int y, uint8_t r, uint8_t g, uint8_t b);

	void stretch_bitmap(bitmap_t *dest, int dest_x, int dest_y, int dest_width, int dest_height, bitmap_t *source, int source_x, int source_y, int source_width, int source_height);
	void write_bitmap_to_file(bitmap_t *bitmap, const _TCHAR *file_path);

	// common socket
	int get_socket(int ch);
	void notify_socket_connected(int ch);
	void notify_socket_disconnected(int ch);
	void update_socket();
	bool initialize_socket_tcp(int ch);
	bool initialize_socket_udp(int ch);
	bool connect_socket(int ch, uint32_t ipaddr, int port);
	void disconnect_socket(int ch);
	bool listen_socket(int ch);
	void send_socket_data_tcp(int ch);
	void send_socket_data_udp(int ch, uint32_t ipaddr, int port);
	void send_socket_data(int ch);
	void recv_socket_data(int ch);

	// win32 dependent
	void update_screen();
	void set_parent_thread(EmuThreadClass *parent);
	EmuThreadClass *get_parent_handler();

	_TCHAR *console_input_string(void);
	void clear_console_input_string(void);
	// Wrapper
	virtual void lock_vm(void);
	virtual void unlock_vm(void);
	virtual void force_unlock_vm(void);
	virtual bool is_vm_locked(void);
	virtual void set_draw_thread(DrawThreadClass *handler);
	virtual QString get_vm_config_name(void);
	virtual double vm_frame_rate(void);
	
public slots:
	void do_write_inputdata(QString s);
	void do_set_input_string(QString s);
	void close_debugger_console();
	void do_close_debugger_thread();
	void do_assign_js_setting(int jsnum, int axis_idx, int assigned_value);
	
signals:
	int sig_update_screen(bitmap_t *);
	int sig_save_screen(const char *);
	int sig_close_window(void);
	int sig_resize_vm_screen(QImage *, int, int);
	int sig_put_string_debugger(QString);
	int sig_console_input_string(QString);
	int sig_enqueue_video(QByteArray *data, int width, int height); 
	int sig_enqueue_audio(QByteArray *data); 
	int sig_debugger_finished();
};
QT_END_NAMESPACE

#endif