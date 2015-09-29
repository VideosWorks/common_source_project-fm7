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

void META_MainWindow::retranslateUi(void)
{
	retranslateControlMenu(" ", false);
	retranslateFloppyMenu(0, 1);
	retranslateFloppyMenu(1, 2);
#if defined(USE_FD3)	
	retranslateFloppyMenu(2, 3);
#endif
#if defined(USE_FD4)	
	retranslateFloppyMenu(3, 4);
#endif
	
	retranslateSoundMenu();
	retranslateScreenMenu();
   
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
  
	actionCapture_Screen->setText(QApplication::translate("MainWindow", "Capture Screen", 0));
  
	menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0));
	menuStretch_Mode->setTitle(QApplication::translate("MainWindow", "Stretch Mode", 0));
	
// End.
// 
//        menuRecord->setTitle(QApplication::translate("MainWindow", "Record", 0));
//        menuRecoad_as_movie->setTitle(QApplication::translate("MainWindow", "Recoad as movie", 0));
	
	menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0));
	menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0));
  
	menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0));
	actionHelp_AboutQt->setText(QApplication::translate("MainWindow", "About Qt", 0));
	actionAbout->setText(QApplication::translate("MainWindow", "About...", 0));
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{


}


META_MainWindow::META_MainWindow(QWidget *parent) : Ui_MainWindow(parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

