/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for X1TurboZ .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QApplication>
#include <QVariant>
#include <QtGui>
#include <QMenu>

#include "emu.h"
#include "commonclasses.h"
#include "menuclasses.h"
#include "qt_main.h"

extern config_t config;

//QT_BEGIN_NAMESPACE
Action_Control_HC20::Action_Control_HC20(QObject *parent, USING_FLAGS *p) : Action_Control(parent, p)
{
	hc20_binds = new Object_Menu_Control_HC20(parent, p);
}

Action_Control_HC20::~Action_Control_HC20(){
	delete hc20_binds;
}

Object_Menu_Control_HC20::Object_Menu_Control_HC20(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
{
}

Object_Menu_Control_HC20::~Object_Menu_Control_HC20(){
}

void Object_Menu_Control_HC20::set_dipsw(bool flag)
{
	emit sig_dipsw(getValue1(), flag);
}


void META_MainWindow::setupUI_Emu(void)
{
	int i;
	QString tmps;
	menu_Emu_DipSw = new QMenu(menuMachine);
	menu_Emu_DipSw->setObjectName(QString::fromUtf8("menu_DipSw"));

	actionGroup_DipSw = new QActionGroup(this);
	actionGroup_DipSw->setExclusive(false);
	menuMachine->addAction(menu_Emu_DipSw->menuAction());
	for(i = 0; i < 4; i++) {
      	action_Emu_DipSw[i] = new Action_Control_HC20(this, using_flags);
        action_Emu_DipSw[i]->setCheckable(true);
        action_Emu_DipSw[i]->hc20_binds->setValue1(i);
        tmps.number(i + 1);
        tmps = QString::fromUtf8("actionEmu_DipSw") + tmps;
        action_Emu_DipSw[i]->setObjectName(tmps);
		
        if(((1 << i) & config.dipswitch) != 0) action_Emu_DipSw[i]->setChecked(true);
		
		menu_Emu_DipSw->addAction(action_Emu_DipSw[i]);
		actionGroup_DipSw->addAction(action_Emu_DipSw[i]);
		connect(action_Emu_DipSw[i], SIGNAL(toggled(bool)),
				action_Emu_DipSw[i]->hc20_binds, SLOT(set_dipsw(bool)));
		connect(action_Emu_DipSw[i]->hc20_binds, SIGNAL(sig_dipsw(int, bool)),
				this, SLOT(set_dipsw(int, bool)));
		
	}
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu(" ",  false);
   // Set Labels
	menu_Emu_DipSw->setTitle(QApplication::translate("Machine", "DIP Switches", 0));
	action_Emu_DipSw[0]->setText(QApplication::translate("Machine", "Dip Switch 1", 0));
	action_Emu_DipSw[1]->setText(QApplication::translate("Machine", "Dip Switch 2", 0));
	action_Emu_DipSw[2]->setText(QApplication::translate("Machine", "Dip Switch 3", 0));
	action_Emu_DipSw[3]->setText(QApplication::translate("MainWindow", "Dip Switch 4", 0));

#ifdef USE_DEBUGGER
	actionDebugger[1]->setText(QApplication::translate("MainWindow", "TF-20 CPU", 0));
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(true);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
} // retranslateUi


META_MainWindow::META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



