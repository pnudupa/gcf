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

#ifndef TESTCLIENT_H
#define TESTCLIENT_H

#include <QObject>
#include <QVariant>

class TestClient : public QObject
{
    Q_OBJECT

public:
    TestClient(const QString &fn, quint16 port, QObject *parent=0)
        : QObject(parent) {
        this->testCall(fn, port);
    }
    ~TestClient() { }

private slots:
    void testCall(const QString &fn, quint16 port);
    void onCallDone(bool success);

private:
    bool m_expectedSuccess;
    QVariant m_expectedResult;
    QString m_expectedErrorMessage;
};

#endif // TESTCLIENT_H
