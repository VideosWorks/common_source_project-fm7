
#ifndef _CSP_QT_MENUCLASSES_H
#define _CSP_QT_MENUCLASSES_H

#include "commonclasses.h"
#include "mainwidget.h"

// This extends class CSP_MainWindow as Ui_MainWindow.
// You may use this as 
QT_BEGIN_NAMESPACE

class USING_FLAGS;
class Object_Menu_Control_98: public Object_Menu_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_98(QObject *parent, USING_FLAGS *p);
	~Object_Menu_Control_98();
signals:
	int sig_sound_device(int);
	int sig_device_type(int);
public slots:
	void do_set_memory_wait(bool);
	void do_set_egc(bool);
	void do_set_gdc_fast(bool);
	void do_set_ram_512k(bool);	
	void do_set_init_memsw(bool);
signals:
	int sig_emu_update_config();
};

class Action_Control_98 : public Action_Control
{
	Q_OBJECT
public:
	Object_Menu_Control_98 *pc98_binds;
	Action_Control_98(QObject *parent, USING_FLAGS *p);
	~Action_Control_98();
};

class QMenu;
class QActionGroup;
class Ui_MainWindow;
class CSP_Logger;
class META_MainWindow : public Ui_MainWindow {
	Q_OBJECT
protected:
	QActionGroup   *actionGroup_SoundDevice;
	QMenu *menu_Emu_SoundDevice;
	Action_Control_98 *actionRAM_512K;
	Action_Control_98 *actionINIT_MEMSW;
	Action_Control_98 *actionGDC_FAST;
#if defined(SUPPORT_EGC)
	Action_Control_98 *actionEGC;
#endif
#if defined(_PC98DO)
	Action_Control_98 *actionMemoryWait;
#endif   
	void setupUI_Emu(void);
	void retranslateUi(void);
public:
	META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent = 0);
	~META_MainWindow();
};

QT_END_NAMESPACE

#endif // END
