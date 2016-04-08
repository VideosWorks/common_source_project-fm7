/*
 * Widget: Meta Menu Class.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History: 2015-11-11 Initial.
 */

#include <QDir>

#include "emu.h"
#include "vm.h"
#include "qt_dialogs.h"
#include "menu_metaclass.h"
#include "commonclasses.h"
#include "mainwidget.h"
#include "commonclasses.h"


#include "qt_main.h"
Menu_MetaClass::Menu_MetaClass(EMU *ep, QMenuBar *root_entry, QString desc, QWidget *parent, int drv) : QMenu(root_entry)
{
	QString tmps;
	int ii;
	
	p_wid = parent;
	menu_root = root_entry;
	p_emu = ep;
	media_drive = drv;

	tmps.setNum(drv);
	object_desc = desc;
	object_desc.append(tmps);
	for(ii = 0; ii < using_flags->get_max_d88_banks(); ii++) {
		action_select_media_list[ii] = NULL;
	}
	use_write_protect = true;
	use_d88_menus = false;
	initial_dir = QString::fromUtf8("");
	
	ext_filter.clear();
	history.clear();
	inner_media_list.clear();
	window_title = QString::fromUtf8("");

	icon_insert = QIcon(":/icon_open.png");
	icon_eject = QIcon(":/icon_eject.png");
	icon_write_protected = QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton);
	icon_write_enabled = QIcon();
}

Menu_MetaClass::~Menu_MetaClass()
{
}


// This is virtual function, pls. override.
void Menu_MetaClass::do_set_write_protect(bool f)
{
	write_protect = f;
	if(f) {
		if(use_write_protect) {
			menu_write_protect->setIcon(icon_write_protected);
		}
		action_write_protect_on->setChecked(true);
	} else {
		if(use_write_protect) {
			menu_write_protect->setIcon(icon_write_enabled);
		}
		action_write_protect_off->setChecked(true);
	}		
}

void Menu_MetaClass::do_set_initialize_directory(const char *s)
{
	initial_dir = QString::fromLocal8Bit(s);
}

void Menu_MetaClass::do_open_media(int drv, QString name) {
	//write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit sig_open_media(drv, name);
}

void Menu_MetaClass::do_insert_media(void) {
	//write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit sig_insert_media(media_drive);
}
void Menu_MetaClass::do_eject_media(void) {
	write_protect = false;
	emit sig_eject_media(media_drive);
}

void Menu_MetaClass::do_open_inner_media(int drv, int s_num) {
	emit sig_set_inner_slot(media_drive, s_num);
}

void Menu_MetaClass::do_open_recent_media(int drv, int s_num){
  //   write_protect = false; // Right? On D88, May be writing entry  exists. 
	emit sig_set_recent_media(media_drive, s_num);
}
void Menu_MetaClass::do_write_protect_media(void) {
	write_protect = true;
	{
		if(use_write_protect) {
			menu_write_protect->setIcon(icon_write_protected);
		}
		action_write_protect_on->setChecked(true);
	}		
	emit sig_write_protect_media(media_drive, write_protect);
}

void Menu_MetaClass::do_write_unprotect_media(void) {
	write_protect = false;
	{
		if(use_write_protect) {
			menu_write_protect->setIcon(icon_write_enabled);
		}
		action_write_protect_off->setChecked(true);
	}		
	emit sig_write_protect_media(media_drive, write_protect);
}

void Menu_MetaClass::do_set_window_title(QString s) {
	window_title = s;
}

void Menu_MetaClass::do_add_media_extension(QString ext, QString description)
{
	QString tmps = description;
	QString all = QString::fromUtf8("All Files (*.*)");

	tmps.append(QString::fromUtf8(" ("));
	tmps.append(ext.toLower());
	tmps.append(QString::fromUtf8(" "));
	tmps.append(ext.toUpper());
	tmps.append(QString::fromUtf8(")"));

	ext_filter << tmps;
	ext_filter << all;

	ext_filter.removeDuplicates();
}

void Menu_MetaClass::do_select_inner_media(int num)
{
	if(use_d88_menus && (num < using_flags->get_max_d88_banks())) {
		if(action_select_media_list[num] != NULL) {
			action_select_media_list[num]->setChecked(true);
		}
	}
}


void Menu_MetaClass::do_open_dialog()
{
	CSP_DiskDialog dlg;
	
	if(initial_dir.isEmpty()) { 
		QDir dir;
		char app[PATH_MAX];
		initial_dir = dir.currentPath();
		strncpy(app, initial_dir.toLocal8Bit().constData(), PATH_MAX);
		initial_dir = QString::fromLocal8Bit(get_parent_dir(app));
	}
	dlg.setOption(QFileDialog::ReadOnly, false);
	dlg.setOption(QFileDialog::DontUseNativeDialog, true);
	//dlg.setAcceptMode(QFileDialog::AcceptSave);
	dlg.setFileMode(QFileDialog::AnyFile);
	//dlg.setLabelText(QFileDialog::Accept, QApplication::translate("MainWindow", "Open File", 0));

	dlg.param->setDrive(media_drive);
	dlg.param->setPlay(true);
	dlg.setDirectory(initial_dir);
	dlg.setNameFilters(ext_filter);

	QString tmps;
	tmps = QApplication::translate("MainWindow", "Open", 0);
	if(!window_title.isEmpty()) {
		tmps = tmps + QString::fromUtf8(" ") + window_title;
	} else {
		tmps = tmps + QString::fromUtf8(" ") + this->title();
	}
	dlg.setWindowTitle(tmps);
	
	QObject::connect(&dlg, SIGNAL(fileSelected(QString)),
					 dlg.param, SLOT(_open_disk(QString))); 
	QObject::connect(dlg.param, SIGNAL(do_open_disk(int, QString)),
					 this, SLOT(do_open_media(int, QString)));

	dlg.show();
	dlg.exec();
	return;
}

void Menu_MetaClass::do_update_histories(QStringList lst)
{
	int ii;
	QString tmps;
	
	history.clear();
	for(ii = 0; ii < MAX_HISTORY; ii++) {
		tmps = QString::fromUtf8("");
		if(ii < lst.size()) tmps = lst.value(ii);
		history << tmps;
		action_recent_list[ii]->setText(tmps);
		if(!tmps.isEmpty()) {
			action_recent_list[ii]->setVisible(true);
		} else {
			action_recent_list[ii]->setVisible(false);
		}			
	}
}

void Menu_MetaClass::do_clear_inner_media(void)
{
	int ii;
	inner_media_list.clear();
	if(use_d88_menus) {
		for(ii = 0; ii < using_flags->get_max_d88_banks(); ii++) {
			if(action_select_media_list[ii] != NULL) {
				action_select_media_list[ii]->setText(QString::fromUtf8(""));
				action_select_media_list[ii]->setVisible(false);
			}
		}
	}
}

void Menu_MetaClass::do_update_inner_media(QStringList lst, int num)
{
	QString tmps;
	int ii;
	inner_media_list.clear();
#if defined(USE_FD1)	
	if(use_d88_menus) {
		for(ii = 0; ii < using_flags->get_max_d88_banks(); ii++) {
			if(ii < p_emu->d88_file[media_drive].bank_num) {
				inner_media_list << lst.value(ii);
				action_select_media_list[ii]->setText(lst.value(ii));
				action_select_media_list[ii]->setVisible(true);
				if(ii == num) action_select_media_list[ii]->setChecked(true);
			} else {
				if(action_select_media_list[ii] != NULL) {
					action_select_media_list[ii]->setText(QString::fromUtf8(""));
					action_select_media_list[ii]->setVisible(false);
				}
			}
		}
	}
#endif	
}

void Menu_MetaClass::do_update_inner_media_bubble(QStringList lst, int num)
{
	QString tmps;
	int ii;
	inner_media_list.clear();
#if defined(USE_BUBBLE1)	
	if(use_d88_menus) {
		for(ii = 0; ii < using_flags->get_max_b77_banks(); ii++) {
			if(ii < p_emu->b77_file[media_drive].bank_num) {
				inner_media_list << lst.value(ii);
				action_select_media_list[ii]->setText(lst.value(ii));
				action_select_media_list[ii]->setVisible(true);
				if(ii == num) action_select_media_list[ii]->setChecked(true);
			} else {
				if(action_select_media_list[ii] != NULL) {
					action_select_media_list[ii]->setText(QString::fromUtf8(""));
					action_select_media_list[ii]->setVisible(false);
				}
			}
		}
	}
#endif	
}

void Menu_MetaClass::create_pulldown_menu_sub(void)
{
	action_insert = new Action_Control(p_wid);
	action_insert->setObjectName(QString::fromUtf8("action_insert_") + object_desc);
	action_insert->binds->setDrive(media_drive);
	connect(action_insert, SIGNAL(triggered()), this, SLOT(do_open_dialog()));
	action_insert->setIcon(icon_insert);
	
	action_eject = new Action_Control(p_wid);
	action_eject->setObjectName(QString::fromUtf8("action_eject_") + object_desc);
	action_eject->binds->setDrive(media_drive);
	connect(action_eject, SIGNAL(triggered()), this, SLOT(do_eject_media()));
	action_eject->setIcon(icon_eject);

	
	{
		QString tmps;
		int ii;
		action_group_recent = new QActionGroup(p_wid);
		action_group_recent->setExclusive(true);
		
		for(ii = 0; ii < MAX_HISTORY; ii++) {
			tmps = history.value(ii, "");
			action_recent_list[ii] = new Action_Control(p_wid);
			action_recent_list[ii]->binds->setDrive(media_drive);
			action_recent_list[ii]->binds->setNumber(ii);
			
			action_recent_list[ii]->setText(tmps);
			action_group_recent->addAction(action_recent_list[ii]);
			if(!tmps.isEmpty()) {
				action_recent_list[ii]->setVisible(true);
			} else {
				action_recent_list[ii]->setVisible(false);
			}			
		}
	}
	if(use_d88_menus) {
		int ii;
		QString tmps;
		action_group_inner_media = new QActionGroup(p_wid);
		action_group_inner_media->setExclusive(true);
		
		for(ii = 0; ii < using_flags->get_max_d88_banks(); ii++) {
			tmps = history.value(ii, "");
			action_select_media_list[ii] = new Action_Control(p_wid);
			action_select_media_list[ii]->binds->setDrive(media_drive);
			action_select_media_list[ii]->binds->setNumber(ii);
			action_select_media_list[ii]->setText(tmps);
			action_select_media_list[ii]->setCheckable(true);
			if(ii == 0) action_select_media_list[ii]->setChecked(true);
			action_group_inner_media->addAction(action_select_media_list[ii]);
			if(!tmps.isEmpty()) {
				action_select_media_list[ii]->setVisible(true);
			} else {
				action_select_media_list[ii]->setVisible(false);
			}			
		}
	}
	if(use_write_protect) {
		action_group_protect = new QActionGroup(p_wid);
		action_group_protect->setExclusive(true);

		action_write_protect_on = new Action_Control(p_wid);
		action_write_protect_on->setObjectName(QString::fromUtf8("action_write_protect_on_") + object_desc);
		action_write_protect_on->setCheckable(true);
		action_write_protect_on->setChecked(true);
		action_write_protect_on->binds->setDrive(media_drive);
		action_write_protect_on->binds->setNumber(0);
		
		action_write_protect_off = new Action_Control(p_wid);
		action_write_protect_off->setObjectName(QString::fromUtf8("action_write_protect_off_") + object_desc);
		action_write_protect_off->setCheckable(true);
		action_write_protect_off->binds->setDrive(media_drive);
		action_write_protect_off->binds->setNumber(0);
		
		action_group_protect->addAction(action_write_protect_on);
		action_group_protect->addAction(action_write_protect_off);
		connect(action_write_protect_on, SIGNAL(triggered()), this, SLOT(do_write_protect_media()));
		connect(action_write_protect_off, SIGNAL(triggered()), this, SLOT(do_write_unprotect_media()));
	}
}

// This is virtual function, pls.apply
void Menu_MetaClass::create_pulldown_menu_device_sub(void)
{
	// Create device specific entries.
}

// This is virtual function, pls.apply
void Menu_MetaClass::connect_menu_device_sub(void)
{

}

//void Menu_MetaClass::setTitle(QString s)
//{
//	QMenu::setTitle(s);
//}

//QAction *Menu_MetaClass::menuAction()
//{
//	return QMenu::menuAction();
//}

void Menu_MetaClass::create_pulldown_menu(void)
{
	int ii;
	// Example:: Disk.
	create_pulldown_menu_sub();
	create_pulldown_menu_device_sub();
	// Create 

	menu_history = new QMenu(this);
	menu_history->setObjectName(QString::fromUtf8("menu_history_") + object_desc);

	if(use_d88_menus) {
		menu_inner_media = new QMenu(this);
		menu_inner_media->setObjectName(QString::fromUtf8("menu_inner_media_") + object_desc);
		for(ii = 0; ii < using_flags->get_max_d88_banks(); ii++) menu_inner_media->addAction(action_select_media_list[ii]);
		this->addAction(menu_inner_media->menuAction());
	}
	{
		menu_history = new QMenu(this);
		menu_history->setObjectName(QString::fromUtf8("menu_history_") + object_desc);
		for(ii = 0; ii < MAX_HISTORY; ii++) menu_history->addAction(action_recent_list[ii]);
		this->addAction(menu_history->menuAction());
	}
	if(use_write_protect) {
		this->addSeparator();
		menu_write_protect = new QMenu(this);
	}
	// Belows are setup of menu.
	this->addAction(action_insert);
	this->addAction(action_eject);

	// Connect extra menus to this.
	connect_menu_device_sub();
	this->addSeparator();
	
	// More actions
	this->addAction(menu_history->menuAction());
	if(use_d88_menus) {
		this->addAction(menu_inner_media->menuAction());
	}
	if(use_write_protect) {
		this->addSeparator();
		this->addAction(menu_write_protect->menuAction());
		menu_write_protect->addAction(action_write_protect_on);
		menu_write_protect->addAction(action_write_protect_off);
	}
	// Do connect!
	
	for(ii = 0; ii < MAX_HISTORY; ii++) {
		connect(action_recent_list[ii], SIGNAL(triggered()),
				action_recent_list[ii]->binds, SLOT(on_recent_disk()));
		connect(action_recent_list[ii]->binds, SIGNAL(set_recent_disk(int, int)),
				this, SLOT(do_open_recent_media(int, int)));
	}
	if(use_d88_menus) {
		for(ii = 0; ii < using_flags->get_max_d88_banks(); ii++) {
			connect(action_select_media_list[ii], SIGNAL(triggered()),
					action_select_media_list[ii]->binds, SLOT(on_d88_slot()));
			connect(action_select_media_list[ii]->binds, SIGNAL(set_d88_slot(int, int)),
					this, SLOT(do_open_inner_media(int, int)));
		}
	}
}

void Menu_MetaClass::retranslate_pulldown_menu_sub(void)
{
	action_insert->setText(QApplication::translate("MainWindow", "Insert", 0));
	action_eject->setText(QApplication::translate("MainWindow", "Eject", 0));
	if(use_write_protect) {
		menu_write_protect->setTitle(QApplication::translate("MainWindow", "Write Protection", 0));
		action_write_protect_on->setText(QApplication::translate("MainWindow", "On", 0));
		action_write_protect_off->setText(QApplication::translate("MainWindow", "Off", 0));
	}
	
	if(use_d88_menus) {
		menu_inner_media->setTitle(QApplication::translate("MainWindow", "Select D88 Image", 0));
	} else {
		//menu_inner_media->setVisible(false);
	}		
	menu_history->setTitle(QApplication::translate("MainWindow", "Recent opened", 0));

}
void Menu_MetaClass::retranslate_pulldown_menu_device_sub(void)
{
}

void Menu_MetaClass::retranslateUi(void)
{
	retranslate_pulldown_menu_sub();
	retranslate_pulldown_menu_device_sub();
}

void Menu_MetaClass::setEmu(EMU *p)
{
	p_emu = p;
}
