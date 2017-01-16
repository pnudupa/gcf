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

#ifndef LOGTHREAD_H
#define LOGTHREAD_H

#include <QTimer>
#include <QObject>
#include <QThread>
#include <QtDebug>
#include <QDateTime>

#include <GCF3/Log>

class Logger : public QObject
{
    Q_OBJECT

public:
    Logger(QObject *parent=0) : QObject(parent),
        m_logCount(0), m_interval(0) { }
    ~Logger() { }

public slots:
    void beginLogging() {
        // Generate a random number between 100 and 150. This
        // would be the ms interval we would use to generate
        // log messages
        m_interval = 100 + ( (unsigned long)QThread::currentThreadId() ) % 50;

        QTimer *timer = new QTimer(this);
        timer->setSingleShot(false);
        connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
        timer->start(m_interval);

        m_logCount = 0;
    }

private slots:
    void onTimeout() {
        QString context = QString("%1: ival(%2) - %3")
                .arg((unsigned long)QThread::currentThreadId()).arg(m_interval).arg(m_logCount);
        switch(m_logCount%5) {
        case 0:
            GCF::Log::instance()->fatal(context, "Fatal Log");
            break;
        case 1:
            GCF::Log::instance()->error(context, "Error Log");
            break;
        case 2:
            GCF::Log::instance()->warning(context, "Warning Log");
            break;
        case 3:
            GCF::Log::instance()->debug(context, "Debug Log");
            break;
        case 4:
            GCF::Log::instance()->info(context, "Info Log");
            break;
        default:
            break;
        }

        ++m_logCount;
        if( m_logCount >= 5 ) {
            QTimer *timer = this->findChild<QTimer*>();
            delete timer;
            QThread::currentThread()->quit();
        }
    }

private:
    int m_logCount;
    int m_interval;
};

#endif // LOGTHREAD_H
