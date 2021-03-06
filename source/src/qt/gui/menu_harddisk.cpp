/*
 * Qt / DIsk Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   LIcense: GPLv2
 *   History: Jan 10, 2015 (MAYBE) : Initial.
 */
#include <QApplication>

#include "commonclasses.h"
#include "mainwidget_base.h"
#include "menu_harddisk.h"

#include "qt_dialogs.h"
//#include "emu.h"


Menu_HDDClass::Menu_HDDClass(QMenuBar *root_entry, QString desc, USING_FLAGS *p, QWidget *parent, int drv, int base_drv) : Menu_MetaClass(root_entry, desc, p, parent, drv, base_drv)
{
}

Menu_HDDClass::~Menu_HDDClass()
{
}

void Menu_HDDClass::create_pulldown_menu_device_sub(void)
{
}


void Menu_HDDClass::connect_menu_device_sub(void)
{
	this->addSeparator();
   
   	connect(this, SIGNAL(sig_open_media(int, QString)), p_wid, SLOT(_open_hard_disk(int, QString)));
	connect(this, SIGNAL(sig_eject_media(int)), p_wid, SLOT(eject_hard_disk(int)));
	connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_hard_disk(int, int)));
}

void Menu_HDDClass::retranslate_pulldown_menu_device_sub(void)
{
	//action_insert->setIcon(icon_floppy);
	action_insert->setText(QApplication::translate("MenuHDD", "Mount", 0));
	action_insert->setToolTip(QApplication::translate("MenuHDD", "Mount virtual hard disk file.", 0));
   
	action_eject->setText(QApplication::translate("MenuHDD", "Unmount", 0));
	action_eject->setToolTip(QApplication::translate("MenuHDD", "Unmount virtual hard disk.", 0));

}
