/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for N5200 .
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

	

void META_MainWindow::setupUI_Emu(void)
{
	menuMachine->setVisible(false);
}

void META_MainWindow::retranslateUi(void)
{
	retranslateControlMenu(" ",  false);
	retranslateFloppyMenu(0, 1);
	retranslateFloppyMenu(1, 2);

	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateUI_Help();
   
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
  
	actionReset->setText(QApplication::translate("MainWindow", "Reset", 0));
	actionCapture_Screen->setText(QApplication::translate("MainWindow", "Capture Screen", 0));
  
	actionAbout->setText(QApplication::translate("MainWindow", "About...", 0));
  

	menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0));
	menuStretch_Mode->setTitle(QApplication::translate("MainWindow", "Stretch Mode", 0));
	
	menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0));
  
	menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0));
	actionHelp_AboutQt->setText(QApplication::translate("MainWindow", "About Qt", 0));
	// Set Labels
} // retranslateUi



META_MainWindow::META_MainWindow(QWidget *parent) : Ui_MainWindow(parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE


