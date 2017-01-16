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

#ifndef RECEIVER_H
#define RECEIVER_H

#include <QObject>

class Receiver : public QObject
{
    Q_OBJECT

public:
    Receiver(QObject *parent=0) : QObject(parent) { }
    ~Receiver() { }

    QString lastSlot() {
        QString ret = m_lastSlot;
        m_lastSlot.clear();
        return ret;
    }

public slots:
    void emptySlot() { m_lastSlot = "emptySlot()"; }
    void stringSlot(const QString &val) { m_lastSlot = QString("stringSlot(%1)").arg(val); }
    void integerSlot(int val) { m_lastSlot = QString("integerSlot(%1)").arg(val); }

private:
    QString m_lastSlot;
};

#endif // RECEIVER_H
