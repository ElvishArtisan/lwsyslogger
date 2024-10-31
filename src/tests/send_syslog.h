// send_syslog.h
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

#ifndef SEND_SYSLOG_H
#define SEND_SYSLOG_H

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QUdpSocket>
#include <QWidget>

//
// Global Definitions
//
#define SEND_SYSLOG_USAGE "\n\n"

class MainWidget : public QWidget
{
  Q_OBJECT
 public:
  MainWidget(QWidget *parent=0);
  QSize sizeHint() const;

 private slots:
  void modeActivatedData(int n);
  void facilityActivatedData(int n);
  void messageTextChangedData();
  void sendData();

 protected:
 void resizeEvent(QResizeEvent *e);
  
 private:
  void SendSyslog() const;
  void SendUdp();
  void CheckSendState();
  QLabel *d_mode_label;
  QComboBox *d_mode_box;
  QLabel *d_address_label;
  QLineEdit *d_address_edit;
  QLabel *d_port_label;

  QSpinBox *d_port_spin;
  QLabel *d_facility_label;
  QComboBox *d_facility_box;
  QLabel *d_severity_label;
  QComboBox *d_severity_box;
  QLabel *d_repeats_label;
  QSpinBox *d_repeats_spin;
  QPushButton *d_send_button;
  QLabel *d_message_label;
  QTextEdit *d_message_edit;

  QUdpSocket *d_udp_socket;
};


#endif  // SEND_SYSLOG_H
