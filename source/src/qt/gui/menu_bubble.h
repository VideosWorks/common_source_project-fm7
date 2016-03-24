/*
 * Menu_BubbleClass : Defines
 * (C) 2015 by K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Please use this file as templete.
 */


#ifndef _CSP_QT_MENU_BUBBLE_CLASSES_H
#define _CSP_QT_MENU_BUBBLE_CLASSES_H

#include "menu_metaclass.h"

QT_BEGIN_NAMESPACE

class Menu_BubbleClass: public Menu_MetaClass {
	Q_OBJECT
protected:
	//QIcon icon_bubble_casette;
public:
	Menu_BubbleClass(EMU *ep, QMenuBar *root_entry, QString desc, QWidget *parent = 0, int drv = 0);
	~Menu_BubbleClass();
	void create_pulldown_menu_device_sub();
	void connect_menu_device_sub(void);
	void retranslate_pulldown_menu_device_sub(void);
};

QT_END_NAMESPACE

#endif
