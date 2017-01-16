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

#ifndef QMLDIALOG_H
#define QMLDIALOG_H

#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QEventLoop>

class QmlDialog : public QQuickView
{
    Q_OBJECT

public:
    QmlDialog(QWindow *parent=0) : QQuickView(parent) {
        this->m_retCode = -1;
    }

    Q_INVOKABLE void ok() {
        m_retCode = 1;
        m_eventLoop.quit();
    }

    Q_INVOKABLE void cancel() {
        m_retCode = 0;
        m_eventLoop.quit();
    }

    // 1 on Ok, 0 on Cancel.
    int exec() {
        this->show();
        m_eventLoop.exec();
        return m_retCode;
    }

private:
    int m_retCode;
    QEventLoop m_eventLoop;
};

class QmlMessageBox : public QmlDialog
{
    Q_OBJECT

public:
    QmlMessageBox(const QString &text, bool showCancelButton, QWindow *parent=0) : QmlDialog(parent) {
        this->engine()->rootContext()->setContextProperty("messageBoxText", text);
        this->engine()->rootContext()->setContextProperty("isOkButtonVisible", true);
        this->engine()->rootContext()->setContextProperty("isCancelButtonVisible", showCancelButton);
        this->engine()->rootContext()->setContextProperty("window", this);
        this->setSource(QUrl("qrc:///Qml/QmlMessageBox.qml"));
    }
};

class QmlInputDialog : public QmlDialog
{
    Q_OBJECT

public:
    QmlInputDialog(const QString &text, bool showCancelButton, bool inputEnabled, QWindow *parent=0) : QmlDialog(parent) {
        this->engine()->rootContext()->setContextProperty("messageBoxText", text);
        this->engine()->rootContext()->setContextProperty("isOkButtonVisible", true);
        this->engine()->rootContext()->setContextProperty("isInputEnabled", inputEnabled);
        this->engine()->rootContext()->setContextProperty("isCancelButtonVisible", showCancelButton);
        this->engine()->rootContext()->setContextProperty("window", this);
        this->setSource(QUrl("qrc:///Qml/QmlInputDialog.qml"));
    }

    Q_INVOKABLE void ok(QString inputText) {
        m_inputValue = inputText;
        QmlDialog::ok();
    }

    QString inputValue() { return m_inputValue; }

private:
    QString m_inputValue;
};

#endif // QMLDIALOG_H
