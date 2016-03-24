/*
 * Common Source code project : GUI
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *     License : GPLv2
 *     History:
 *      Jan 14, 2015 : Initial
 *
 * [qt -> gui -> status bar]
 */

#include <QtCore/QVariant>
#include <QtGui>
#include <QSize>
#include <QHBoxLayout>
#include <QPainter>
#include <QBrush>
#include <QGraphicsView>
#include <QTransform>

#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"
#include "vm.h"

extern EMU* emu;

void Ui_MainWindow::initStatusBar(void)
{
	int i;
	int wfactor;
	statusUpdateTimer = new QTimer;
	messagesStatusBar = new QLabel;
	//dummyStatusArea1 = new QWidget;
	QSize size1, size2, size3;
	QString tmpstr;
	QString n_s;
	QString tmps_n;
	//   QHBoxLayout *layout = new QHBoxLayout();
	
	//statusbar->addWidget(layout, 0);
	messagesStatusBar->setFixedWidth(400);
	statusbar->addPermanentWidget(messagesStatusBar, 0);
	messagesStatusBar->setStyleSheet("font: 12pt \"Sans\";");
	dummyStatusArea1 = new QWidget;
	statusbar->addPermanentWidget(dummyStatusArea1, 1);
	
#if defined(USE_FD1) && defined(USE_QD1) && defined(USE_TAPE)
	wfactor = (1280 - 400 - 100 - 100) / (MAX_FD + MAX_QD);
#elif defined(USE_FD1) && defined(USE_TAPE) && defined(USE_BUBBLE1)
	wfactor = (1280 - 400 - 100 - 100 - 100 * MAX_BUBBLE) / MAX_FD;
#elif defined(USE_FD1) && defined(USE_TAPE)
	wfactor = (1280 - 400 - 100 - 100) / MAX_FD;
#elif defined(USE_QD1) && defined(USE_TAPE)
	wfactor = (1280 - 400 - 100 - 100) / MAX_QD;
#elif defined(USE_FD1)
	wfactor = (1280 - 400 - 100) / MAX_FD;
#elif defined(USE_QD1)
	wfactor = (1280 - 400 - 100) / MAX_QD;
#elif defined(USE_QD1) && defined(USE_FD1)
	wfactor = (1280 - 400 - 100) / (MAX_QD + MAX_FD);
#else
	wfactor = 0;
#endif

#ifdef USE_FD1   
	for(i = 0; i < MAX_FD; i++) osd_str_fd[i].clear();
#endif   
#ifdef USE_QD1   
	for(i = 0; i < 2; i++) osd_str_qd[i].clear();
#endif   
#ifdef USE_TAPE
	osd_str_cmt.clear();
#endif
#ifdef USE_COMPACT_DISC
	osd_str_cdrom.clear();
#endif
#ifdef USE_LASER_DISC
	osd_str_laserdisc.clear();
#endif
#ifdef USE_BUBBLE1
	for(i = 0; i < MAX_BUBBLE; i++) osd_str_bubble[i].clear();
#endif	
#ifdef USE_LED_DEVICE
	osd_led_data = 0x00000000;
#endif   

	tmps_n = QString::fromUtf8("font: ");
	n_s.setNum(12);
	tmps_n = tmps_n + n_s + QString::fromUtf8("pt \"Sans\";");
#ifdef USE_FD1
	for(i = 0; i < MAX_FD; i++) { // Will Fix
		fd_StatusBar[i] = new QLabel;
		fd_StatusBar[i]->setStyleSheet(tmps_n);
		fd_StatusBar[i]->setFixedWidth((wfactor > 200) ? 200 : wfactor);
		//      fd_StatusBar[i]->setAlignment(Qt::AlignRight);
		statusbar->addPermanentWidget(fd_StatusBar[i]);
	}
#endif
#ifdef USE_QD1
	for(i = 0; i < MAX_QD; i++) {
		qd_StatusBar[i] = new QLabel;
		qd_StatusBar[i]->setStyleSheet(tmps_n);
		qd_StatusBar[i]->setFixedWidth((wfactor > 150) ? 150 : wfactor);
		//     qd_StatusBar[i]->setAlignment(Qt::AlignRight);
		statusbar->addPermanentWidget(qd_StatusBar[i]);
	}
#endif
#ifdef USE_BUBBLE1
	for(i = 0; i < MAX_BUBBLE; i++) {
		bubble_StatusBar[i] = new QLabel;
		bubble_StatusBar[i]->setFixedWidth(100);
		bubble_StatusBar[i]->setStyleSheet(tmps_n);
		statusbar->addPermanentWidget(bubble_StatusBar[i]);
	}
#endif
#ifdef USE_TAPE
	cmt_StatusBar = new QLabel;
	cmt_StatusBar->setFixedWidth(100);
	cmt_StatusBar->setStyleSheet(tmps_n);;
	statusbar->addPermanentWidget(cmt_StatusBar);
#endif
#ifdef USE_COMPACT_DISC
	cdrom_StatusBar = new QLabel;
	cdrom_StatusBar->setFixedWidth(100);
	cdrom_StatusBar->setStyleSheet(tmps_n);
	statusbar->addPermanentWidget(cdrom_StatusBar);
#endif
#ifdef USE_LASER_DISC
	laserdisc_StatusBar = new QLabel;
	laserdisc_StatusBar->setFixedWidth(100);
	laserdisc_StatusBar->setStyleSheet(tmps_n);
	statusbar->addPermanentWidget(laserdisc_StatusBar);
#endif
	dummyStatusArea2 = new QWidget;
	dummyStatusArea2->setFixedWidth(100);
#ifdef USE_LED_DEVICE
	for(i = 0; i < USE_LED_DEVICE; i++) {
		flags_led[i] = false;
		flags_led_bak[i] = false;
	}
	led_graphicsView = new QGraphicsView(dummyStatusArea2);
	
	led_gScene = new QGraphicsScene(0.0f, 0.0f, (float)dummyStatusArea2->width(), (float)dummyStatusArea2->height());
	QPen pen;
	QBrush bbrush = QBrush(QColor(Qt::black));
	led_graphicsView->setBackgroundBrush(bbrush);
	connect(this, SIGNAL(sig_led_update(QRectF)), led_graphicsView, SLOT(updateSceneRect(QRectF)));
	{
		QBrush rbrush = QBrush(QColor(Qt::red));
		float bitwidth = (float)dummyStatusArea2->width() / ((float)USE_LED_DEVICE * 2.0);
		float start = -(float)dummyStatusArea2->width()  / 2.0f + bitwidth * 3.0f;

		pen.setColor(Qt::black);
		led_gScene->addRect(0, 0, 
				    -(float)dummyStatusArea2->width(),
				    (float)dummyStatusArea2->height(),
				    pen, bbrush);
		for(i = 0; i < USE_LED_DEVICE; i++) {
			led_leds[i] = NULL;
			pen.setColor(Qt::red);
			led_leds[i] = led_gScene->addEllipse(start,
				  (float)dummyStatusArea2->height() / 3.0f,
				   bitwidth - 2.0f, bitwidth - 2.0f,
				   pen, rbrush);
			start = start + bitwidth * 1.5f;
		}
	}
#endif
	statusbar->addPermanentWidget(dummyStatusArea2, 0);
	//   statusbar->addWidget(dummyStatusArea2);
	connect(statusUpdateTimer, SIGNAL(timeout()), this, SLOT(redraw_status_bar()));
	statusUpdateTimer->start(33);
#ifdef USE_LED_DEVICE
	ledUpdateTimer = new QTimer;
	connect(statusUpdateTimer, SIGNAL(timeout()), this, SLOT(redraw_leds()));
	statusUpdateTimer->start(5);
#endif
}

void Ui_MainWindow::resize_statusbar(int w, int h)
{
	int wfactor;
	QSize nowSize;
	double height, width;
	double scaleFactor;
	int ww;
	int pt;
	int i;
	int qd_width, fd_width;
	int sfactor = 0;;
	QString n_s;
	QString tmps;

	nowSize = messagesStatusBar->size();
	height = (double)(nowSize.height());
	width  = (double)(nowSize.width());
	scaleFactor = (double)w / 1280.0;
   
	statusbar->setFixedWidth(w);
	pt = (int)(14.0 * scaleFactor);
	if(pt < 4) pt = 4;
	sfactor = (int)(400.0 * scaleFactor);
	messagesStatusBar->setFixedWidth((int)(400.0 * scaleFactor));
	
	tmps = QString::fromUtf8("font: ");
	n_s.setNum(pt);
	tmps = tmps + n_s + QString::fromUtf8("pt \"Sans\";");
	messagesStatusBar->setStyleSheet(tmps);
   
#if defined(USE_FD1) && defined(USE_QD1) && defined(USE_TAPE)
	wfactor = (1280 - 400 - 100 - 100) / (MAX_FD + MAX_QD);
#elif defined(USE_FD1) && defined(USE_TAPE) && defined(USE_BUBBLE1)
	wfactor = (1280 - 400 - 100 - 100 - 100 * MAX_BUBBLE) / MAX_FD;
#elif defined(USE_FD1) && defined(USE_TAPE)
	wfactor = (1280 - 400 - 100 - 100) / MAX_FD;
#elif defined(USE_QD1) && defined(USE_TAPE)
	wfactor = (1280 - 400 - 100 - 100) / MAX_QD;
#elif defined(USE_FD1)
	wfactor = (1280 - 400 - 100) / MAX_FD;
#elif defined(USE_QD1)
	wfactor = (1280 - 400 - 100) / MAX_QD;
#elif defined(USE_QD1) && defined(USE_FD1)
	wfactor = (1280 - 400 - 100) / (MAX_QD + MAX_FD);
#else
	wfactor = 100;
#endif
	fd_width = wfactor;
	qd_width = wfactor;
	if(fd_width > 200) fd_width = 200;
	if(fd_width < 50) fd_width = 50;
	if(qd_width > 150) qd_width = 150;
	if(qd_width < 50) qd_width = 50;

#ifdef USE_FD1
	ww = (int)(scaleFactor * (double)fd_width);
	for(i = 0; i < MAX_FD; i++) { // Will Fix
		fd_StatusBar[i]->setStyleSheet(tmps);
		fd_StatusBar[i]->setFixedWidth(ww);
		sfactor += ww;
	}
#endif
#ifdef USE_QD1
	ww = (int)(scaleFactor * (double)fd_width);
	for(i = 0; i < MAX_QD; i++) { // Will Fix
		qd_StatusBar[i]->setStyleSheet(tmps);
		qd_StatusBar[i]->setFixedWidth(ww);
		sfactor += ww;
	}
#endif
#ifdef USE_TAPE
	cmt_StatusBar->setFixedWidth((int)(100.0 * scaleFactor));
	cmt_StatusBar->setStyleSheet(tmps);
	sfactor += (int)(100.0 * scaleFactor);
#endif
#ifdef USE_BUBBLE1
	ww = (int)(scaleFactor * 100.0);
	for(i = 0; i < MAX_BUBBLE; i++) { // Will Fix
		bubble_StatusBar[i]->setStyleSheet(tmps);
		bubble_StatusBar[i]->setFixedWidth(ww);
		sfactor += ww;
	}
#endif
#ifdef USE_LED_DEVICE
	led_graphicsView->setFixedWidth((int)(100.0 * scaleFactor)); 
#endif   
	dummyStatusArea2->setFixedWidth((int)(108.0 * scaleFactor));
	sfactor += (int)(100.0 * scaleFactor);
	sfactor = (int)(1280.0 * scaleFactor) - sfactor;
	if(sfactor > 10) {
		dummyStatusArea1->setVisible(true);
	} else {
		dummyStatusArea1->setVisible(false);
		sfactor = 10;
	}
	dummyStatusArea1->setFixedWidth(sfactor);   
#ifdef USE_LED_DEVICE
	{
		QPen pen;
		QBrush rbrush = QBrush(QColor(Qt::red));
		QBrush bbrush = QBrush(QColor(Qt::black));
		float bitwidth = (float)dummyStatusArea2->width() / ((float)USE_LED_DEVICE * 2.0);
		float start = -(float)dummyStatusArea2->width()  / 2.0f + bitwidth * 3.0f;

		led_gScene->clear();

		pen.setColor(Qt::black);
		led_gScene->addRect(0, 0, 
				    -(float)dummyStatusArea2->width(),
				    (float)dummyStatusArea2->height(),
				    pen, bbrush);
		for(i = 0; i < USE_LED_DEVICE; i++) {
			led_leds[i] = NULL;
			pen.setColor(Qt::red);
			led_leds[i] = led_gScene->addEllipse(start,
				  (float)dummyStatusArea2->height() / 3.0f,
				   bitwidth - 2.0f, bitwidth - 2.0f,
				   pen, rbrush);
			start = start + bitwidth * 1.5f;
		}
		//redraw_leds();
	}
#endif
}

#ifdef USE_LED_DEVICE
void Ui_MainWindow::do_recv_data_led(quint32 d)
{
	osd_led_data = (uint32_t)d;
}

void Ui_MainWindow::redraw_leds(void)
{
		uint32_t drawflags;
		int i;
		float bitwidth = (float)dummyStatusArea2->width() / ((float)USE_LED_DEVICE * 2.0);
		float start = -(float)dummyStatusArea2->width() + bitwidth * 4.0f;
		drawflags = osd_led_data;
		
		for(i = 0; i < USE_LED_DEVICE; i++) {
			flags_led[i] = ((drawflags & (1 << i)) != 0);
			if(led_leds[i] != NULL) {
				if(flags_led[i]) {
					led_leds[i]->setVisible(true);
				} else {
					led_leds[i]->setVisible(false);
				}
			}
			emit sig_led_update(QRectF(start,
					   0.0f, bitwidth * 2.0f, bitwidth * 2.0f));
			start = start + bitwidth * 1.5f;
		}
		led_graphicsView->setScene(led_gScene);
}	
#endif	

#if defined(USE_QD1)
void Ui_MainWindow::do_change_osd_qd(int drv, QString tmpstr)
{
	if((drv < 0) || (drv > 1)) return;
	osd_str_qd[drv] = tmpstr;
}
#endif

#if defined(USE_FD1)
void Ui_MainWindow::do_change_osd_fd(int drv, QString tmpstr)
{
	if((drv < 0) || (drv >= MAX_FD)) return;
	osd_str_fd[drv] = tmpstr;
}
#endif
#if defined(USE_COMPACT_DISC)
void Ui_MainWindow::do_change_osd_cdrom(QString tmpstr)
{
	osd_str_cdrom = tmpstr;
	//printf("%s\n", tmpstr.toLocal8Bit().constData());
}
#endif
#if defined(USE_TAPE)
void Ui_MainWindow::do_change_osd_cmt(QString tmpstr)
{
	osd_str_cmt = tmpstr;
}
#endif
#if defined(USE_BUBBLE1)
void Ui_MainWindow::do_change_osd_bubble(int drv, QString tmpstr)
{
	if(drv < MAX_BUBBLE) osd_str_bubble[drv] = tmpstr;
}
#endif

void Ui_MainWindow::redraw_status_bar(void)
{
	int access_drv;
	int tape_counter;
	int i;
#ifdef USE_FD1   
	for(i = 0; i < MAX_FD; i++) {	   
		if(osd_str_fd[i] != fd_StatusBar[i]->text()) fd_StatusBar[i]->setText(osd_str_fd[i]);
	}
#endif   
#ifdef USE_QD1   
	for(i = 0; i < MAX_QD; i++) {	   
		if(osd_str_qd[i] != qd_StatusBar[i]->text()) qd_StatusBar[i]->setText(osd_str_qd[i]);
	}
#endif   
#ifdef USE_TAPE
	if(osd_str_cmt != cmt_StatusBar->text()) cmt_StatusBar->setText(osd_str_cmt);
#endif
#ifdef USE_COMPACT_DISC
	if(osd_str_cdrom != cdrom_StatusBar->text()) cdrom_StatusBar->setText(osd_str_cdrom);
#endif
#ifdef USE_LASER_DISC
	if(osd_str_laserdisc != laserdisc_StatusBar->text()) laserdisc_StatusBar->setText(osd_str_laserdisc);
#endif
#ifdef USE_BUBBLE1
	for(i = 0; i < MAX_BUBBLE; i++) {
		if(osd_str_bubble[i] != bubble_StatusBar[i]->text()) bubble_StatusBar[i]->setText(osd_str_bubble[i]);
	}
#endif
}


void Ui_MainWindow::message_status_bar(QString str)
{
	//QString tmpstr;
	if(messagesStatusBar == NULL) return;
	if(str != messagesStatusBar->text()) messagesStatusBar->setText(str);
}
