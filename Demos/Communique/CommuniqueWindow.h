/****************************************************************************
**
** Copyright (C) VCreate Logic Private Limited, Bangalore
**
** Use of this file is limited according to the terms specified by
** VCreate Logic Private Limited, Bangalore.  Details of those terms
** are listed in licence.txt included as part of the distribution package
** of this file. This file may not be distributed without including the
** licence.txt file.
**
** Contact info@vcreatelogic.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef COMMUNIQUEWINDOW_H
#define COMMUNIQUEWINDOW_H

#include <QWidget>
#include <GCF3/IpcServerDiscovery>

struct CommuniqueWindowData;
class CommuniqueWindow : public QWidget
{
    Q_OBJECT
    
public:
    explicit CommuniqueWindow(QWidget *parent = 0);
    ~CommuniqueWindow();
    
    // These two methods do all the communication between
    // communique instances.
    Q_SLOT void incomingMessage(const QString &from, const QString &msg);
    Q_SIGNAL void outgoingMessage(const QString &from, const QString &msg);

private slots:
    void onFoundServer(const GCF::IpcServerInfo &info);
    void onServerActivated();
    void onServerDeactivated();
    void onServerCouldNotActivate(const QString &msg);
    void on_cmdSend_clicked();

private:
    CommuniqueWindowData *d;
};

#endif // COMMUNIQUEWINDOW_H
