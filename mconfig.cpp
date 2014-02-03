//
//   Copyright (C) 2003-2010 by Warren Woodford
//   Copyright (C) 2014 by Timothy E. Harris
//   for modifications applicable to the MX-14 project.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//

#include "mconfig.h"
#include <stdio.h>
#include <unistd.h>

MConfig::MConfig(QWidget* parent) : QDialog(parent) {
  setupUi(this);
  setWindowIcon(QApplication::windowIcon());

  proc = new QProcess(this);
  timer = new QTimer(this);

  tabWidget->setCurrentIndex(0);
}

MConfig::~MConfig(){
}

/////////////////////////////////////////////////////////////////////////
// util functions

QString MConfig::getCmdOut(QString cmd) {
  char line[260];
  const char* ret = "";
  FILE* fp = popen(cmd.toAscii(), "r");
  if (fp == NULL) {
    return QString (ret);
  }
  int i;
  if (fgets(line, sizeof line, fp) != NULL) {
    i = strlen(line);
    line[--i] = '\0';
    ret = line;
  }
  pclose(fp);
  return QString (ret);
}

QStringList MConfig::getCmdOuts(QString cmd) {
  char line[260];
  FILE* fp = popen(cmd.toAscii(), "r");
  QStringList results;
  if (fp == NULL) {
    return results;
  }
  int i;
  while (fgets(line, sizeof line, fp) != NULL) {
    i = strlen(line);
    line[--i] = '\0';
    results.append(line);
  }
  pclose(fp);
  return results;
}

QString MConfig::getCmdValue(QString cmd, QString key, QString keydel, QString valdel) {
  const char *ret = "";
  char line[260];

  QStringList strings = getCmdOuts(cmd);
  for (QStringList::Iterator it = strings.begin(); it != strings.end(); ++it) {
    strcpy(line, ((QString)*it).toAscii());
    char* keyptr = strstr(line, key.toAscii());
    if (keyptr != NULL) {
      // key found
      strtok(keyptr, keydel.toAscii());
      const char* val = strtok(NULL, valdel.toAscii());
      if (val != NULL) {
        ret = val;
      }
      break;
    }
  }
  return QString (ret);
}

QStringList MConfig::getCmdValues(QString cmd, QString key, QString keydel, QString valdel) {
  char line[130];
  FILE* fp = popen(cmd.toAscii(), "r");
  QStringList results;
  if (fp == NULL) {
    return results;
  }
  int i;
  while (fgets(line, sizeof line, fp) != NULL) {
    i = strlen(line);
    line[--i] = '\0';
    char* keyptr = strstr(line, key.toAscii());
    if (keyptr != NULL) {
      // key found
      strtok(keyptr, keydel.toAscii());
      const char* val = strtok(NULL, valdel.toAscii());
      if (val != NULL) {
        results.append(val);
      }
    }
  }
  pclose(fp);
  return results;
}

bool MConfig::replaceStringInFile(QString oldtext, QString newtext, QString filepath) {

QString cmd = QString("sed -i 's/%1/%2/g' %3").arg(oldtext).arg(newtext).arg(filepath);
  if (system(cmd.toAscii()) != 0) {
    return false;
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////
// common

void MConfig::refresh() {
  int i = tabWidget->currentIndex();
  switch (i) {
    case 1:
      refreshDesktop();
      buttonApply->setEnabled(true);
      buttonOk->setEnabled(false);
      break;

    case 2:
      refreshAdd();
      buttonApply->setEnabled(false);
      buttonOk->setEnabled(true);
      break;

    case 3:
      refreshDelete();
      buttonApply->setEnabled(false);
      buttonOk->setEnabled(true);
      break;

    default:
      refreshRestore();
      buttonApply->setEnabled(false);
      buttonOk->setEnabled(true);
      break;
  }
}

/////////////////////////////////////////////////////////////////////////
// special

void MConfig::refreshRestore() {
  char line[130];
  char line2[130];
  char *tok;
  FILE *fp;
  int i;
  // locale
  userComboBox->clear();
  userComboBox->addItem("none");
  userComboBox->addItem("root");
  fp = popen("ls -1 /home", "r");
  if (fp != NULL) {
    while (fgets(line, sizeof line, fp) != NULL) {
      i = strlen(line);
      line[--i] = '\0';
      tok = strtok(line, " ");
      if (tok != NULL && strlen(tok) > 1 && strncmp(tok, "ftp", 3) != 0) {
        sprintf(line2, "grep '^%s' /etc/passwd >/dev/null", tok);
        if (system(line2) == 0) {
          userComboBox->addItem(tok);
        }
      }
    }
    pclose(fp);
  }
  checkGroups->setChecked(false);
  checkQupzilla->setChecked(false);
}

void MConfig::refreshDesktop() {
  char line[130];
  QString cmd;
  fromUserComboBox->clear();
  FILE *fp = popen("ls -1 /home", "r");
  int i;
  char *tok;
  if (fp != NULL) {
    while (fgets(line, sizeof line, fp) != NULL) {
      i = strlen(line);
      line[--i] = '\0';
      tok = strtok(line, " ");
      if (tok != NULL && strlen(tok) > 1 && strncmp(tok, "ftp", 3) != 0) {
        cmd = QString("grep '^%1' /etc/passwd >/dev/null").arg(tok);
	if (system(cmd.toAscii()) == 0) {
          fromUserComboBox->addItem(tok);
	}
      }
    }
    pclose(fp);
  }
  on_fromUserComboBox_activated();
}

void MConfig::refreshAdd() {
  userNameEdit->setText(tr("username"));
  userPasswordEdit->setText("");
  userPassword2Edit->setText("");
}

void MConfig::refreshDelete() {
  char line[130];
  char line2[130];
  char *tok;
  FILE *fp;
  int i;
  // locale
  deleteUserCombo->clear();
  deleteUserCombo->addItem("none");
  fp = popen("ls -1 /home", "r");
  if (fp != NULL) {
    while (fgets(line, sizeof line, fp) != NULL) {
      i = strlen(line);
      line[--i] = '\0';
      tok = strtok(line, " ");
      if (tok != NULL && strlen(tok) > 1 && strncmp(tok, "ftp", 3) != 0) {
        sprintf(line2, "grep '^%s' /etc/passwd >/dev/null", tok);
        if (system(line2) == 0) {
          deleteUserCombo->addItem(tok);
        }
      }
    }
    pclose(fp);
  }
}

// apply but do not close
void MConfig::applyRestore() {
  QString user = userComboBox->currentText();
  if (user.compare("none") == 0) {
    // no user selected
    return;
  }
  QString home = user;
  if (user.compare("root") != 0) {
    home = QString("/home/%1").arg(user);
  }
  QString cmd;
  int ans = QMessageBox::warning(0, QString::null,
    tr("The user configuration will be repaired. Please close all other applications now. When finished, please logout or reboot. Are you sure you want to repair now?"),
        tr("Yes"), tr("No"));
  if (ans != 0) {
    return;
  }
  setCursor(QCursor(Qt::WaitCursor));

  // restore groups
  if (checkGroups->isChecked() && user.compare("root") != 0) {
    cmd = QString("usermod -G audio,cdrom,dialout,dip,disk,fax,floppy,fuse,games,lp,lpadmin,netdev,plugdev,powerdev,scanner,sudo,tape,tty,uml-net,users,uucp,video,voice %1").arg(user);
    system(cmd.toAscii());
  }

  // restore Qupzilla configs
  if (checkQupzilla->isChecked()) {
    cmd = QString("/bin/rm -dfr %1/.config/qupzilla/*/*/*").arg(home);
    system(cmd.toAscii());
  }
  setCursor(QCursor(Qt::ArrowCursor));

  refresh();
}

void MConfig::applyDesktop() {

  if (toUserComboBox->currentText().isEmpty()) {
    QMessageBox::information(0, QString::null,
      tr("You must specify a 'copy to' destination. You can not copy to the desktop you are logged in to."));
    return;
  }
  // verify 
  int ans = QMessageBox::critical(0, QString::null, tr("Before copying, close all other applications. Be sure the copy to destination is large enough to contain the files you are copying. Copying between desktops may overwrite or delete your files or preferences on the destination desktop. Are you sure you want to proceed?"),
    tr("Yes"), tr("No"));
  if (ans != 0) {
    return;
  }

  QString fromDir = QString("/home/%1").arg(fromUserComboBox->currentText());
  QString toDir = QString("/home/%1").arg(toUserComboBox->currentText());
  if (docsRadioButton->isChecked()) {
    fromDir.append("/Documents");
    toDir.append("/Documents");
  } else if (qupRadioButton->isChecked()) {
    fromDir.append("/.config/.qupzilla");
    toDir.append("/.config/.qupzilla");
  } else if (sharedRadioButton->isChecked()) {
    fromDir.append("/Shared");
    toDir.append("/Shared");
  }
  fromDir.append("/");

  setCursor(QCursor(Qt::WaitCursor));
  if (syncRadioButton->isChecked()) {
    syncStatusEdit->setText(tr("Synchronizing desktop..."));
  } else {
    syncStatusEdit->setText(tr("Copying desktop..."));
  }
  disconnect(timer, SIGNAL(timeout()), 0, 0);
  connect(timer, SIGNAL(timeout()), this, SLOT(syncTime()));
  disconnect(proc, SIGNAL(started()), 0, 0);
  connect(proc, SIGNAL(started()), this, SLOT(syncStart()));
  disconnect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), 0, 0);
  connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(syncDone(int, QProcess::ExitStatus)));
  QString cmd = QString("rsync -qa ");
  if (syncRadioButton->isChecked()) {
    cmd.append("--delete-after ");
  }
  cmd.append(fromDir);
  cmd.append(" ");
  cmd.append(toDir);
  proc->start(cmd);
}

void MConfig::applyAdd() {
  //validate data before proceeding
  // see if username is reasonable length
  if (userNameEdit->text().length() < 2) {
    QMessageBox::critical(0, QString::null,
      tr("The user name needs to be at least 2 characters long. Please select a longer name before proceeding."));
    return;
  }
  // see if username contains whitespace
  QString cmd = QString("echo '^%1' | grep ^.*[[:space:]].").arg( userNameEdit->text());
  if  (system(cmd.toAscii()) == 0) {
    QMessageBox::critical(0, QString::null,
      tr("The user name may not contain any spaces. Please select another name."));
    return;
  }
  // check that user name is not already used
  cmd = QString("grep '^%1' /etc/passwd >/dev/null").arg( userNameEdit->text());
  if (system(cmd.toAscii()) == 0) {     
    QMessageBox::critical(0, QString::null,
      tr("Sorry that name is in use. Please select a different name."));
    return;
  }
  if (userPasswordEdit->text().compare(userPassword2Edit->text()) != 0) {
    QMessageBox::critical(0, QString::null,
      tr("The user password entries do not match.  Please try again."));
    return;
  }
  if (userPasswordEdit->text().length() < 2) {
    QMessageBox::critical(0, QString::null,
      tr("The user password needs to be at least 2 characters long. Please select a longer name before proceeding."));
    return;
  }

  cmd = QString("adduser --disabled-login --force-badname --gecos %1 %2").arg( userNameEdit->text()).arg(userNameEdit->text());
  system(cmd.toAscii());
  cmd = QString("passwd %1").arg(userNameEdit->text());
  FILE *fp = popen(cmd.toAscii(), "w");
  bool fpok = true;
  cmd = QString("%1\n").arg(userPasswordEdit->text());
  if (fp != NULL) {
    sleep(1);
    if (fputs(cmd.toAscii(), fp) >= 0) {
      fflush(fp);
      sleep(1);
      if (fputs(cmd.toAscii(), fp) < 0) {
        fpok = false;
      }
    } else {
      fpok = false;
    }
    pclose(fp);
  } else {
    fpok = false;
  }

  if (fpok) {
    QMessageBox::information(0, QString::null,
      tr("The user was added ok."));
  } else {
    QMessageBox::critical(0, QString::null,
      tr("Failed to add the user."));
  }
  refresh();
}

void MConfig::applyDelete() {
  QString cmd = QString(tr("This action can not be undone. Are you sure you want to delete user %1?")).arg(deleteUserCombo->currentText());
  int ans = QMessageBox::warning(this, QString::null, cmd,
          tr("Yes"), tr("No"));
  if (ans == 0) {
    if (deleteHomeCheckBox->isChecked()) {
      cmd = QString("deluser --remove-home %1").arg( deleteUserCombo->currentText());
    } else {
      cmd = QString("deluser %1").arg(deleteUserCombo->currentText());
    }
    system(cmd.toAscii());
    refresh();
  }
}

/////////////////////////////////////////////////////////////////////////
// sync process events

void MConfig::syncStart() {
  timer->start(100);
}

void MConfig::syncTime() {
  int i = syncProgressBar->value() + 1;
  if (i > 100) {
    i = 0;
  }
  syncProgressBar->setValue(i);
}

void MConfig::syncDone(int exitCode, QProcess::ExitStatus exitStatus) {
  if (exitStatus == QProcess::NormalExit) {
    QString fromDir = QString("/home/%1").arg(fromUserComboBox->currentText());
    QString toDir = QString("/home/%1").arg(toUserComboBox->currentText());
/*    if (docsRadioButton->isChecked()) {
      toDir.append("/Documents");
    } else if (qupRadioButton->isChecked()) {
      toDir.append("/.qupzilla");
    } else if (sharedRadioButton->isChecked()) {
      toDir.append("/Shared");
    }
*/    // fix owner
    QString cmd = QString("chown -R %1:users %2").arg(toUserComboBox->currentText()).arg(toDir);
    system(cmd.toAscii());

    // fix files
    if (entireRadioButton->isChecked() || qupRadioButton->isChecked()) {
      // fix qupzilla tree
      cmd = QString("rm -f %1/.confg/.qupzilla/*/*/").arg(toDir);
      system(cmd.toAscii());
      cmd = QString("find %1/,config/.qupzilla -type f -exec sed -i 's|home/%2|home/%3|g' '{}' \\;").arg(toDir).arg(fromUserComboBox->currentText()).arg(toUserComboBox->currentText());
      system(cmd.toAscii()); 
    }

    if (entireRadioButton->isChecked()) {
      //delete some files
      cmd = QString("rm -f %1/.recently-used").arg(toDir);
      system(cmd.toAscii());
      cmd = QString("rm -f %1/.openoffice.org/*/.lock").arg(toDir);
      system(cmd.toAscii());
      cmd = QString("find %1/.openoffice.org -type f -exec sed -i 's|home/%2|home/%3|g' '{}' \\;").arg(toDir).arg(fromUserComboBox->currentText()).arg(toUserComboBox->currentText());
      system(cmd.toAscii());
      cmd = QString("find %1/.thunderbird -type f -exec sed -i 's|home/%2|home/%3|g' '{}' \\;").arg(toDir).arg(fromUserComboBox->currentText()).arg(toUserComboBox->currentText());
      system(cmd.toAscii());
      cmd = QString("find %1/.adobe -type f -exec sed -i 's|home/%2|home/%3|g' '{}' \\;").arg(toDir).arg(fromUserComboBox->currentText()).arg(toUserComboBox->currentText());
      system(cmd.toAscii());
      cmd = QString("find %1/.gimp-2.4 -type f -exec sed -i 's|home/%2|home/%3|g' '{}' \\;").arg(toDir).arg(fromUserComboBox->currentText()).arg(toUserComboBox->currentText());
      system(cmd.toAscii());
      cmd = QString("find %1/.xine -type f -exec sed -i 's|home/%2|home/%3|g' '{}' \\;").arg(toDir).arg(fromUserComboBox->currentText()).arg(toUserComboBox->currentText());
      system(cmd.toAscii());
    }
    if (syncRadioButton->isChecked()) {
      syncStatusEdit->setText(tr("Synchronizing desktop...ok"));
    } else {
      syncStatusEdit->setText(tr("Copying desktop...ok"));
    }
  } else {
    if (syncRadioButton->isChecked()) {
      syncStatusEdit->setText(tr("Synchronizing desktop...failed"));
    } else {
      syncStatusEdit->setText(tr("Copying desktop...failed"));
    }
  }
  timer->stop();
  syncProgressBar->setValue(0);
  setCursor(QCursor(Qt::ArrowCursor));
}

/////////////////////////////////////////////////////////////////////////
// slots

void MConfig::show() {
  QDialog::show();
  refresh();
}

void MConfig::on_fromUserComboBox_activated() {
  char line[130];
  QString cmd;

  toUserComboBox->clear();
  FILE *fp = popen("ls -1 /home", "r");
  int i;
  char *tok;
  if (fp != NULL) {
    while (fgets(line, sizeof line, fp) != NULL) {
      i = strlen(line);
      line[--i] = '\0';
      tok = strtok(line, " ");
      if (tok != NULL && strlen(tok) > 1 && strncmp(tok, "ftp", 3) != 0) {
        cmd= QString("grep '^%1' /etc/passwd >/dev/null").arg(tok);
	if (system(cmd.toAscii()) == 0 && fromUserComboBox->currentText().compare(tok) != 0) {
	  cmd = QString("who | grep '%1'").arg(tok);
          if (system(cmd.toAscii()) != 0) {
            toUserComboBox->addItem(tok);
	  }
        }
      }
    }
    pclose(fp);
  }
}

void MConfig::on_userComboBox_activated() {
  buttonApply->setEnabled(true);
}

void MConfig::on_deleteUserCombo_activated() {
  buttonApply->setEnabled(true);
}

void MConfig::on_userNameEdit_textEdited() {
  buttonApply->setEnabled(true);
}

// apply but do not close
void MConfig::on_buttonApply_clicked() {
  if (!buttonApply->isEnabled()) {
    return;
  }

  int i = tabWidget->currentIndex();
  switch (i) {
    case 1:
      applyDesktop();
      buttonApply->setEnabled(false);
      break;

    case 2:
      setCursor(QCursor(Qt::WaitCursor));
      applyAdd();
      setCursor(QCursor(Qt::ArrowCursor));
      buttonApply->setEnabled(false);
      break;

    case 3:
      setCursor(QCursor(Qt::WaitCursor));
      applyDelete();
      setCursor(QCursor(Qt::ArrowCursor));
      buttonApply->setEnabled(false);
      break;

    default:
      setCursor(QCursor(Qt::WaitCursor));
      applyRestore();
      setCursor(QCursor(Qt::ArrowCursor));
      buttonApply->setEnabled(false);
      break;
  }
}

void MConfig::on_tabWidget_currentChanged() {
  refresh();
}


// close but do not apply
void MConfig::on_buttonCancel_clicked() {
  close();
}

// apply then close
void MConfig::on_buttonOk_clicked() {
  on_buttonApply_clicked();
  close();
}

bool MConfig::hasInternetConnection() 
{
   bool internetConnection  = false;
   // Query network interface status
   QStringList interfaceList  = getCmdOuts("ifconfig -a -s");
   int i=1;
   while (i<interfaceList.size()) {
      QString interface = interfaceList.at(i);
      interface = interface.left(interface.indexOf(" "));
      if ((interface != "lo") && (interface != "wmaster0") && (interface != "wifi0")) {
         QStringList ifStatus  = getCmdOuts(QString("ifconfig %1").arg(interface));
         QString unwrappedList = ifStatus.join(" ");
         if (unwrappedList.indexOf("UP ") != -1) {
            if (unwrappedList.indexOf(" RUNNING ") != -1) {
               internetConnection  = true;
            }
         }
      }
      ++i;
   }
   return internetConnection;
}

void MConfig::executeChild(const char* cmd, const char* param)
{
   pid_t childId;
   childId = fork();
   if (childId >= 0)
      {
      if (childId == 0)
         {
         execl(cmd, cmd, param, (char *) 0);

         //system(cmd);
         }
      }
}

// show about
void MConfig::on_buttonAbout_clicked() {
  QMessageBox::about(0, tr("About"),
      tr("<p><b>MX User Assistant</b></p>"));
}