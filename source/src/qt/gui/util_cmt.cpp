/*
 * UI->Qt->MainWindow : CMT Utils.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 *
 * History:
 * Jan 24, 2014 : Moved from some files.
 */


#include "menuclasses.h"
#include "emu_utils.h"
#include "qt_dialogs.h"
#include "emu.h"
#include "agar_logger.h"

//QT_BEGIN_NAMESPACE
#ifdef USE_TAPE
int Ui_MainWindow::set_recent_cmt(int num) 
{
    QString s_path;
    char path_shadow[PATH_MAX];
    int i;
    if((num < 0) || (num >= MAX_HISTORY)) return -1;
    
    s_path = QString::fromUtf8(config.recent_tape_path[num]);
    strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
    UPDATE_HISTORY(path_shadow, config.recent_tape_path);
    strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
   
    get_parent_dir(path_shadow);
    strcpy(config.initial_tape_dir, path_shadow);
    strncpy(path_shadow, s_path.toUtf8().constData(), PATH_MAX);
//   for(int i = num; i > 0; i--) {
 //      strcpy(config.recent_tape_path[i], config.recent_tape_path[i - 1]);
 //    }
//    strcpy(config.recent_tape_path[0], path.c_str());
    if(emu) {
      AGAR_DebugLog(AGAR_LOG_DEBUG, "Tape: Open READ");
      emu->LockVM();
      emu->play_tape(path_shadow); // Play Readonly, to safety.
      emu->UnlockVM();
    }
    for(i = 0; i < MAX_HISTORY; i++) {
       if(action_Recent_List_CMT[i] != NULL) { 
	  action_Recent_List_CMT[i]->setText(QString::fromUtf8(config.recent_tape_path[i]));
	  //emit action_Recent_List_FD[drive][i]->changed();
       }
    }
   return 0;
}

void Ui_MainWindow::do_write_protect_cmt(bool flag)
{
   write_protect = flag;
}


#ifdef USE_TAPE_BUTTON
void Ui_MainWindow::do_push_play_tape(void)
{
  // Do notify?
  if(emu) {
    emu->LockVM();
    emu->push_play();
    emu->UnlockVM();
  }
  actionPlay_Start->setChecked(true);
}

void Ui_MainWindow::do_push_stop_tape(void)
{
  // Do notify?
  if(emu) {
    emu->LockVM();
    emu->push_stop();
    emu->UnlockVM();
  }
  actionPlay_Stop->setChecked(true);
}

void Ui_MainWindow::do_display_tape_play(bool flag)
{
  if(flag) {
    actionPlay_Start->setChecked(true);
  } else {
    actionPlay_Stop->setChecked(true);
  }
}
 
#endif
#endif

#ifdef USE_TAPE
void Ui_MainWindow::set_wave_shaper(bool f)
{
  if(f) {
    config.wave_shaper = 1;
  } else {
    config.wave_shaper = 0;
  }
}

bool Ui_MainWindow::get_wave_shaper(void)
{
  if(config.wave_shaper == 0) return false;
  return true;
}
#endif // USE_TAPE

#ifdef USE_TAPE
void Ui_MainWindow::set_direct_load_from_mzt(bool f)
{

  if(f) {
    config.direct_load_mzt = 1;
  } else {
    config.direct_load_mzt = 0;
  }
}

bool Ui_MainWindow::get_direct_load_mzt(void)
{
  if(config.direct_load_mzt == 0) return false;
  return true;
}
#endif // USE_TAPE

void Ui_MainWindow::_open_cmt(bool mode, const QString path)
{
  char path_shadow[PATH_MAX];
  int play;
  int i;
   
  play = (mode == false)? 0 : 1;
#ifdef USE_TAPE
  if(path.length() <= 0) return;
  strncpy(path_shadow, path.toUtf8().constData(), PATH_MAX);
  UPDATE_HISTORY(path_shadow, config.recent_tape_path);
  get_parent_dir(path_shadow);
  strcpy(config.initial_tape_dir, path_shadow);
  // Copy filename again.
  strncpy(path_shadow, path.toUtf8().constData(), PATH_MAX);
  if(emu) {
     emu->LockVM();
     if((play != false) || (write_protect != false)) {
	AGAR_DebugLog(AGAR_LOG_DEBUG, "Tape: Open READ : filename = %s", path_shadow);
	emu->play_tape(path_shadow);
     } else {
	AGAR_DebugLog(AGAR_LOG_DEBUG, "Tape: Open Write : filename = %s", path_shadow);
	emu->rec_tape(path_shadow);
     }
     emu->UnlockVM();
  }
    for(i = 0; i < MAX_HISTORY; i++) {
       if(action_Recent_List_CMT[i] != NULL) { 
	  action_Recent_List_CMT[i]->setText(QString::fromUtf8(config.recent_tape_path[i]));
	  //emit action_Recent_List_FD[drive][i]->changed();
       }
    }
   
#endif
}

void Ui_MainWindow::eject_cmt(void) 
{
#ifdef USE_TAPE
  if(emu) {
     emu->LockVM();
     emu->close_tape();
     emu->UnlockVM();
  }
#endif
}

//QT_END_NAMESPACE
