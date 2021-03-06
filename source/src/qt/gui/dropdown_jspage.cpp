
#include "dropdown_keyset.h"
#include "dropdown_jsbutton.h"
#include "dropdown_jspage.h"
#include <QApplication>
#include "menu_flags.h"

//extern USING_FLAGS *using_flags;

CSP_DropDownJSPage::CSP_DropDownJSPage(USING_FLAGS *pp, QWidget *parent, QStringList *lst, int jsnum)
{
	int i;
	QString nm;
	char tmps[32];
	p_wid = parent;
	layout = new QGridLayout(this);
	bind_jsnum = jsnum;
	using_flags = pp;
	for(i = 0; i < 4; i++) {
		//label[i] = new QLabel(this);
		combo_js[i] = new CSP_DropDownJSButton(pp, this, lst, jsnum, i);
	}
	label_axis = new QLabel(QApplication::translate("JoystickDialog", "<B>Physical Axis:</B>", 0), this);
	layout->addWidget(label_axis, 0, 0, Qt::AlignLeft);
	// Down
	layout->addWidget(combo_js[1], 1, 1, Qt::AlignRight);
	// Up
	layout->addWidget(combo_js[0], 3, 1, Qt::AlignRight);
	// Left
	layout->addWidget(combo_js[3], 2, 0, Qt::AlignRight);
	// Right
	layout->addWidget(combo_js[2], 2, 2, Qt::AlignRight);
	label_buttons = new QLabel(QApplication::translate("JoystickDialog", "<B>Physical Buttons:</B>", 0), this);
	layout->addWidget(label_buttons, 4, 0, Qt::AlignLeft);

	int joybuttons = using_flags->get_num_joy_button_captions();

	
	for(i = 0; i < 12; i++) {
		if(joybuttons > i) {
			memset(tmps, 0x00, sizeof(char) * 20);
			label_button[i] = new QLabel(this);
			js_button[i] = new CSP_DropDownJSButton(pp, this, lst, jsnum, i + 4);
			if(using_flags->is_use_joy_button_captions()) {
				snprintf(tmps, 32, "<B>%s</B>", using_flags->get_joy_button_captions(i + 4));
			} else {
				snprintf(tmps, 32, "<B>#%02d:</B>", i + 1);
			}
			nm = QString::fromUtf8(tmps);
			label_button[i]->setText(nm);
			layout->addWidget(label_button[i], (i / 4) * 2 + 5 + 0, i % 4, Qt::AlignLeft);
			layout->addWidget(js_button[i], (i / 4) * 2 + 5 + 1, i % 4, Qt::AlignLeft);
		}
	}
	this->setLayout(layout);
	connect(this, SIGNAL(sig_set_js_button(int, int, int)), parent, SLOT(do_set_js_button(int, int, int)));
	connect(this, SIGNAL(sig_set_js_button_idx(int, int, int)), parent, SLOT(do_set_js_button_idx(int, int, int)));
}

CSP_DropDownJSPage::~CSP_DropDownJSPage()
{
}


void CSP_DropDownJSPage::do_select_common(int index, int axes)
{
	if(index < 16) {
		emit sig_set_js_button(bind_jsnum, axes, joystick_define_tbl[index].scan);
	} else if(index < 20) {
		emit sig_set_js_button(bind_jsnum, axes, ((joystick_define_tbl[index - 16].scan) >> 20) | ((index - 16 + 1) << 5));
	} else if(index < 24) {
		emit sig_set_js_button(bind_jsnum, axes, ((joystick_define_tbl[index - 16].scan) >> 24) | ((index - 16 + 1) << 5));
	} else {		
		emit sig_set_js_button_idx(bind_jsnum, axes, -(index - 24));
	}
}
void CSP_DropDownJSPage::do_select_up(int index)
{
	do_select_common(index, 0);
}

void CSP_DropDownJSPage::do_select_down(int index)
{
	do_select_common(index, 1);
}

void CSP_DropDownJSPage::do_select_left(int index)
{
	do_select_common(index, 2);
}

void CSP_DropDownJSPage::do_select_right(int index)
{
	do_select_common(index, 3);
}

void CSP_DropDownJSPage::do_select_js_button(int jsnum, int button, int scan)
{
	//printf("Select: %d %d %d\n", jsnum, button, scan);
	emit sig_set_js_button(jsnum, button, scan);
}

void CSP_DropDownJSPage::do_select_js_button_idx(int jsnum, int button, int scan)
{
	emit sig_set_js_button_idx(jsnum, button, scan);
	//printf("Select_Idx: %d %d %d\n", jsnum, button, scan);
}
