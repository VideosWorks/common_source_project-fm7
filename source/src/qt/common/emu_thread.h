/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -
	License : GPLv2
	History : 2015.11.10 Split from qt_main.h
	[ win32 main ] -> [ Qt main ] -> [Emu Thread]
*/
#ifndef _CSP_QT_EMU_THREAD_H
#define _CSP_QT_EMU_THREAD_H

#include <QThread>
#include <QQueue>
#include <QString>
#include <QElapsedTimer>
#include <QMutexLocker>
#include <QMutex>

#include "common.h"
#include "commonclasses.h"
#include "fileio.h"
#include "emu.h"
#include "vm.h"
#include "menuclasses.h"
#include "mainwidget.h"
#include "commonclasses.h"
#include "config.h"
#include "../gui/emu_thread_tmpl.h"

#ifndef MAX_HISTORY
#define MAX_HISTORY 8
#endif
#define MAX_COMMAND_LEN	64


class META_MainWindow;
class EMU;
class QWaitCondition;
class USING_FLAGS;

QT_BEGIN_NAMESPACE

class EmuThreadClass : public EmuThreadClassBase {
	Q_OBJECT
protected:

	QMutex uiMutex;
	char dbg_prev_command[MAX_COMMAND_LEN];
  
	void button_pressed_mouse_sub(Qt::MouseButton button);
	void button_released_mouse_sub(Qt::MouseButton button);
	void get_qd_string(void);
	void get_fd_string(void);
	void get_tape_string(void);
	void get_cd_string(void);
	void get_bubble_string(void);

	const _TCHAR *get_emu_message(void);
	double get_emu_frame_rate(void);
	int get_message_count(void);
	void dec_message_count(void);
	const _TCHAR *get_device_name(void);
	bool get_power_state(void);
	void resetEmu();
	void specialResetEmu();
	void loadState();
	void saveState();
	
public:
	EmuThreadClass(Ui_MainWindowBase *rootWindow, USING_FLAGS *p, QObject *parent = 0);
	~EmuThreadClass();
	void run() { doWork("");}
	bool now_debugging();
	int get_interval(void);
						  
	int get_d88_file_cur_bank(int drive);
	int get_d88_file_bank_num(int drive);
	QString get_d88_file_disk_name(int drive, int banknum);
	bool is_floppy_disk_protected(int drive);
	void set_floppy_disk_protected(int drive, bool flag);
	QString get_d88_file_path(int drive);

						  
public slots:
	void doWork(const QString &param);
	
	void do_set_display_size(int w, int h, int ww, int wh);
	void moved_mouse(int, int);

	void do_write_protect_disk(int drv, bool flag);
	void do_close_disk(int);
	void do_open_disk(int, QString, int);
	void do_close_hard_disk(int);
	void do_open_hard_disk(int, QString);
	void do_play_tape(int drv, QString name);
	void do_rec_tape(int drv, QString name);
	void do_close_tape(int drv);
	void do_cmt_push_play(int drv);
	void do_cmt_push_stop(int drv);
	void do_cmt_push_fast_forward(int drv);
	void do_cmt_push_fast_rewind(int drv);
	void do_cmt_push_apss_forward(int drv);
	void do_cmt_push_apss_rewind(int drv);
	void do_write_protect_quickdisk(int drv, bool flag);
	void do_close_quickdisk(int drv);
	void do_open_quickdisk(int drv, QString path);
	void do_close_cart(int drv);
	void do_open_cart(int drv, QString path);
	void do_close_laser_disc(int drv);
	void do_open_laser_disc(int drv, QString path);
	void do_eject_cdrom(int drv);
	void do_open_cdrom(int drv, QString path);
	void do_load_binary(int drv, QString path);
	void do_save_binary(int drv, QString path);
	void do_write_protect_bubble_casette(int drv, bool flag);
	void do_close_bubble_casette(int);
	void do_open_bubble_casette(int, QString, int);
	void do_start_auto_key(QString text);
	void do_stop_auto_key(void);
	void set_romakana(bool flag);
	void do_close_debugger(void);

signals:
	int sig_set_draw_fps(double);
	int sig_draw_one_turn(bool);
};

QT_END_NAMESPACE

#endif
