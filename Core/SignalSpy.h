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

#ifndef SIGNALSPY_H
#define SIGNALSPY_H

#include "GCFGlobal.h"

#include <QList>
#include <QVector>
#include <QObject>
#include <QPointer>
#include <QEventLoop>
#include <QBasicTimer>
#include <QMetaMethod>
#include <QMetaObject>
#include <QVariantList>

namespace GCF
{

class GCF_EXPORT SignalSpyBase : public QObject
{
    Q_OBJECT

signals:
    void caughtSignal(const QVariantList &args);

protected:
    SignalSpyBase(QObject *parent) : QObject(parent) { }
};

class GCF_EXPORT WaitEventLoop : public QObject
{
public:
    WaitEventLoop(QObject *parent=0) : QObject(parent), m_loop(0), m_timerId(-1) { }

    bool inLoop() const { return m_loop != 0; }

    int enter(int ms) {
        Q_ASSERT(m_loop == 0);
        QEventLoop loop;
        m_loop = &loop;
        m_timerId = this->startTimer(ms);
        int retVal = loop.exec();
        m_loop = 0;
        return retVal;
    }

    void exitLoop(int code=0) {
        if(m_loop) {
            if(m_timerId > 0)
                killTimer(m_timerId);
            m_timerId = -1;
            m_loop->exit(code);
        }
    }

private:
    void timerEvent(QTimerEvent *te) {
        if(te->timerId() == m_timerId)
            exitLoop(-1);
    }

private:
    QEventLoop *m_loop;
    int m_timerId;
};

class GCF_EXPORT SignalSpy : public SignalSpyBase
{
public:
    SignalSpy(QObject *sender, const char *signal, QObject *parent = 0)
        : SignalSpyBase(parent), m_sender(sender), m_valid(false) {
        // First find out if the signal is valid in sender
        if(sender && signal) {
            QByteArray signalMember(signal+1);
            int signalIndex = sender->metaObject()->indexOfSignal(signalMember);
            if(signalIndex >= 0) {
                QMetaMethod method = sender->metaObject()->method(signalIndex);
#if QT_VERSION >= 0x050000
                m_signal = method.methodSignature();
#else
                m_signal = QByteArray(method.signature());
#endif

                // Initialize arg types list
                QList<QByteArray> paramTypes = method.parameterTypes();
                for(int i=0; i<paramTypes.count(); i++) {
                    int tp = QMetaType::type(paramTypes.at(i));
#if QT_VERSION >= 0x050000
                    if (tp == QMetaType::UnknownType) {
                        void *argv[] = { &tp, &i };
                        QMetaObject::metacall(const_cast<QObject*>(sender),
                                              QMetaObject::RegisterMethodArgumentMetaType,
                                              method.methodIndex(), argv);
                        if (tp == -1)
                            tp = QMetaType::UnknownType;
                    }
                    if (tp == QMetaType::UnknownType) {
                        Q_ASSERT(tp != QMetaType::Void); // void parameter => metaobject is corrupt
                        qWarning("Don't know how to handle '%s', use qRegisterMetaType to register it.",
                                 paramTypes.at(i).constData());
                        break;
                    }
#else
                    if(tp == QMetaType::Void) {
                        qWarning("Don't know how to handle '%s', use qRegisterMetaType to register it.",
                                 paramTypes.at(i).constData());
                        break;
                    }
#endif
                    m_argTypes << tp;
                }

                // Perform signal/slot connection
                if(m_argTypes.count() == paramTypes.count()) {
                    int slotIndex = SignalSpyBase::staticMetaObject.methodCount();
                    QMetaObject::connect(sender, signalIndex, this, slotIndex);
                    m_valid = true;
                } else {
                    m_valid = false;
                    m_argTypes.clear();
                }
            }
        }
    }
    ~SignalSpy() { }

    bool isValid() const { return m_sender.data() && m_valid; }
    QObject *sender() const { return m_sender.data(); }
    QByteArray signal() const { return m_signal; }

    bool wait(int timeout=5000) {
        return m_waitLoop.enter(timeout) >= 0;
    }

    void clear() { m_emissions.clear(); }
    int count() const { return m_emissions.count(); }
    bool isEmpty() const { return m_emissions.isEmpty(); }
    const QVariantList &first() const { return m_emissions.first(); }
    const QVariantList &last() const { return m_emissions.last(); }
    QVariantList takeFirst() { return m_emissions.takeFirst(); }
    QVariantList takeLast() { return m_emissions.takeLast(); }
    QVariantList takeAt(int index) { return m_emissions.takeAt(index); }
    QVariantList at(int index) const { return m_emissions.at(index); }
    void removeAt(int index) { m_emissions.removeAt(index); }
    const QList< QVariantList > &emissions() const { return m_emissions; }

protected:
#if QT_VERSION >= 0x050000
    int qt_metacall(QMetaObject::Call call, int methodId, void **a) Q_DECL_OVERRIDE {
#else
    int qt_metacall(QMetaObject::Call call, int methodId, void **a) {
#endif
        methodId = SignalSpyBase::qt_metacall(call, methodId, a);
        if(methodId < 0)
            return methodId;
        if(call == QMetaObject::InvokeMetaMethod) {
            if(methodId == 0) {
                QVariantList args;
                for(int i=0; i<m_argTypes.count(); i++) {
                    if(m_argTypes.at(i) == QMetaType::QVariant)
                        args.append( *reinterpret_cast<QVariant*>(a[i+1]) );
                    else
                        args.append( QVariant(m_argTypes.at(i), a[i+1]) );
                }
                m_emissions.append(args);
                this->handleSignalEmission(args);
                m_waitLoop.exitLoop();
            }
            --methodId;
        }
        return methodId;
    }

    virtual void handleSignalEmission(const QVariantList &args) {
        emit caughtSignal(args);
    }

private:
    QPointer<QObject> m_sender;
    QByteArray m_signal;
    QList<int> m_argTypes;
    bool m_valid;
    WaitEventLoop m_waitLoop;
    QList< QVariantList > m_emissions;
};

}

#endif // SIGNALSPY_H
