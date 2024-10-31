// send_syslog.cpp
//
// Simple syslog logger for Livewire sites
//
//   (C) Copyright 2024 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <syslog.h>
#include <unistd.h>

#include <QApplication>
#include <QDateTime>
#include <QHostAddress>
#include <QMessageBox>

#include "cmdswitch.h"
#include "send_syslog.h"

const char *global_ident={"send_syslog"};

MainWidget::MainWidget(QWidget *parent)
  : QWidget(parent)
{
  new CmdSwitch("send_syslog",VERSION,SEND_SYSLOG_USAGE);

  d_udp_socket=new QUdpSocket(this);
  
  QFont label_font(font().family(),font().pixelSize(),QFont::Bold);
  
  //
  // Mode Dropdown
  //
  d_mode_label=new QLabel(tr("Transmission Mode")+":",this);
  d_mode_label->setFont(label_font);
  d_mode_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  d_mode_box=new QComboBox(this);
  connect(d_mode_box,SIGNAL(activated(int)),this,SLOT(modeActivatedData(int)));
  d_mode_box->insertItem(d_mode_box->count(),tr("Use syslog(3)"));
  d_mode_box->insertItem(d_mode_box->count(),tr("UDP"));

  //
  // IP Address
  //
  d_address_label=new QLabel(tr("IP Address")+":",this);
  d_address_label->setFont(label_font);
  d_address_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  d_address_edit=new QLineEdit(this);
  d_address_edit->setText("127.0.0.1");

  //
  // Port
  //
  d_port_label=new QLabel(tr("Port")+":",this);
  d_port_label->setFont(label_font);
  d_port_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  d_port_spin=new QSpinBox(this);
  d_port_spin->setRange(1,0xFFFF);
  d_port_spin->setValue(514);
  
  //
  // Facility Dropdown
  //
  d_facility_label=new QLabel(tr("Facility")+":",this);
  d_facility_label->setFont(label_font);
  d_facility_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  d_facility_box=new QComboBox(this);
  connect(d_facility_box,SIGNAL(activated(int)),
	  this,SLOT(facilityActivatedData(int)));
  d_facility_box->insertItem(d_facility_box->count(),"AUTH",LOG_AUTH);
  d_facility_box->insertItem(d_facility_box->count(),"CRON",LOG_CRON);
  d_facility_box->insertItem(d_facility_box->count(),"DAEMON",LOG_DAEMON);
  d_facility_box->insertItem(d_facility_box->count(),"LPR",LOG_LPR);
  d_facility_box->insertItem(d_facility_box->count(),"MAIL",LOG_MAIL);
  d_facility_box->insertItem(d_facility_box->count(),"NEWS",LOG_NEWS);
  d_facility_box->insertItem(d_facility_box->count(),"SYSLOG",LOG_SYSLOG);
  d_facility_box->insertItem(d_facility_box->count(),"USER",LOG_USER);
  d_facility_box->insertItem(d_facility_box->count(),"UUCP",LOG_UUCP);
  d_facility_box->insertItem(d_facility_box->count(),"LOCAL0",LOG_LOCAL0);
  d_facility_box->insertItem(d_facility_box->count(),"LOCAL1",LOG_LOCAL1);
  d_facility_box->insertItem(d_facility_box->count(),"LOCAL2",LOG_LOCAL2);
  d_facility_box->insertItem(d_facility_box->count(),"LOCAL3",LOG_LOCAL3);
  d_facility_box->insertItem(d_facility_box->count(),"LOCAL4",LOG_LOCAL4);
  d_facility_box->insertItem(d_facility_box->count(),"LOCAL5",LOG_LOCAL5);
  d_facility_box->insertItem(d_facility_box->count(),"LOCAL6",LOG_LOCAL6);
  d_facility_box->insertItem(d_facility_box->count(),"LOCAL7",LOG_LOCAL7);
  d_facility_box->setCurrentText("USER");
  
  //
  // Severity Dropdown
  //
  d_severity_label=new QLabel(tr("Severity")+":",this);
  d_severity_label->setFont(label_font);
  d_severity_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  d_severity_box=new QComboBox(this);
  d_severity_box->insertItem(d_severity_box->count(),"EMERG",LOG_EMERG);
  d_severity_box->insertItem(d_severity_box->count(),"ALERT",LOG_ALERT);
  d_severity_box->insertItem(d_severity_box->count(),"CRIT",LOG_CRIT);
  d_severity_box->insertItem(d_severity_box->count(),"ERR",LOG_ERR);
  d_severity_box->insertItem(d_severity_box->count(),"WARNING",LOG_WARNING);
  d_severity_box->insertItem(d_severity_box->count(),"NOTICE",LOG_NOTICE);
  d_severity_box->insertItem(d_severity_box->count(),"INFO",LOG_INFO);
  d_severity_box->insertItem(d_severity_box->count(),"DEBUG",LOG_DEBUG);
  d_severity_box->setCurrentText("DEBUG");

  //
  // Repeats
  //
  d_repeats_label=new QLabel(tr("Repeats")+":",this);
  d_repeats_label->setFont(label_font);
  d_repeats_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  d_repeats_spin=new QSpinBox(this);
  d_repeats_spin->setRange(1,100);

  //
  // Text Area
  //
  d_message_edit=new QTextEdit(this);
  connect(d_message_edit,SIGNAL(textChanged()),
	  this,SLOT(messageTextChangedData()));
  
  //
  // Send Button
  //
  d_send_button=new QPushButton(tr("Send"),this);
  d_send_button->setFont(label_font);
  connect(d_send_button,SIGNAL(clicked()),this,SLOT(sendData()));

  modeActivatedData(d_mode_box->currentIndex());
  facilityActivatedData(d_facility_box->currentIndex());
  CheckSendState();
}


QSize MainWidget::sizeHint() const
{
  return QSize(750,300);
}


void MainWidget::modeActivatedData(int n)
{
  d_address_label->setEnabled(n>0);
  d_address_edit->setEnabled(n>0);
  d_port_label->setEnabled(n>0);
  d_port_spin->setEnabled(n>0);
  CheckSendState();
}


void MainWidget::facilityActivatedData(int n)
{
  int facility=d_facility_box->itemData(n).toInt();

  closelog();
  openlog(global_ident,LOG_PID,facility);
}


void MainWidget::messageTextChangedData()
{
  CheckSendState();
}


void MainWidget::sendData()
{
  if(d_mode_box->currentIndex()==0) {
    SendSyslog();
  }
  else {
    SendUdp();
  }
}


void MainWidget::resizeEvent(QResizeEvent *e)
{
  int w=size().width();
  int h=size().height();

  d_mode_label->setGeometry(10,3,150,22);
  d_mode_box->setGeometry(165,3,110,22);

  d_address_label->setGeometry(280,3,100,22);
  d_address_edit->setGeometry(385,3,110,22);

  d_port_label->setGeometry(500,3,60,22);
  d_port_spin->setGeometry(565,3,80,22);
  
  d_facility_label->setGeometry(10,33,80,22);
  d_facility_box->setGeometry(95,33,100,22);

  d_severity_label->setGeometry(210,33,80,22);
  d_severity_box->setGeometry(295,33,100,22);

  d_repeats_label->setGeometry(400,33,80,22);
  d_repeats_spin->setGeometry(485,33,60,22);

  d_send_button->setGeometry(w-90,5,80,50);
  
  d_message_edit->setGeometry(10,60,w-20,h-70);
}


void MainWidget::SendSyslog() const
{
  int facility=d_facility_box->itemData(d_facility_box->currentIndex()).toInt();
  int severity=d_severity_box->itemData(d_severity_box->currentIndex()).toInt();

  for(int i=0;i<d_repeats_spin->value();i++) {
    syslog(facility|severity,"%s",
	   d_message_edit->toPlainText().toUtf8().trimmed().constData());
  }
}


void MainWidget::SendUdp()
{
  int facility=d_facility_box->itemData(d_facility_box->currentIndex()).toInt();
  int severity=d_severity_box->itemData(d_severity_box->currentIndex()).toInt();
  QString msg;
  char hostname[PATH_MAX];
  memset(hostname,0,PATH_MAX);
  gethostname(hostname,PATH_MAX-1);
  QHostAddress addr(d_address_edit->text());
  if(addr.isNull()) {
    QMessageBox::warning(this,"send_syslog - Error","Invalid IP Address!");
    return;
  }
  msg+=QString::asprintf("<%d>",facility|severity);  // Priority
  msg+=QDateTime::currentDateTime().toString("MMM dd hh:mm:ss ");  // Timestamp
  msg+=hostname;
  msg+=QString::asprintf(" send_syslog[%u]: ",getpid());
  msg+=d_message_edit->toPlainText();

  for(int i=0;i<d_repeats_spin->value();i++) {
    d_udp_socket->writeDatagram(msg.toUtf8(),addr,d_port_spin->value());
  }
}


void MainWidget::CheckSendState()
{
  if(d_mode_box->currentIndex()==0) {  // Use syslog(3)
    d_send_button->
      setDisabled(d_message_edit->toPlainText().trimmed().isEmpty());
  }
  else {  // UDP
    QHostAddress addr(d_address_edit->text());
    d_send_button->
      setDisabled(d_message_edit->toPlainText().trimmed().isEmpty()||
		  addr.isNull());
  }
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv);

  MainWidget *w=new MainWidget();
  w->show();

  return a.exec();
}
