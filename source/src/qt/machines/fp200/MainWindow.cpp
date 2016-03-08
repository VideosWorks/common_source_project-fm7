/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for X1TurboZ .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QVariant>
#include <QtGui>
#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE




void META_MainWindow::retranslateUi(void)
{

	retranslateControlMenu("", false);
	retranslateCMTMenu();
	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateMachineMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();
	
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
	menuBootMode->setTitle("BOOT Mode");
	actionBootMode[0]->setText(QString::fromUtf8("BASIC"));
	actionBootMode[1]->setText(QString::fromUtf8("CETL"));	
	
	//	actionStart_Record_Movie->setText(QApplication::translate("MainWindow", "Start Record Movie", 0));
	//      actionStop_Record_Movie->setText(QApplication::translate("MainWindow", "Stop Record Movie", 0));
	// 
	// FP1100 Specified
	
	// Set Labels
	
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
	menuBootMode = new QMenu(menuMachine);
	menuBootMode->setObjectName(QString::fromUtf8("menuControl_BootMode"));
	menuMachine->addAction(menuBootMode->menuAction());
	
	ConfigCPUBootMode(2);
}


META_MainWindow::META_MainWindow(QWidget *parent) : Ui_MainWindow(parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



