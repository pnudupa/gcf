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

#ifndef TESTSERVICE_H
#define TESTSERVICE_H

// * - int, bool, double, QString, QStringList, QVariant, QVariantList, QVariantMap, QByteArray, GCF::Result

#include <GCF3/GCFGlobal>

#include <QDate>
#include <QTimer>
#include <QString>
#include <QByteArray>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>
#include <QElapsedTimer>

class TestService : public QObject
{
    Q_OBJECT

public:
    explicit TestService(QObject *parent=0) : QObject(parent) {
        this->initProperties();
    }

    // Valid service methods
    Q_INVOKABLE void noParams() { }
    Q_INVOKABLE int integer(int in) { return in; }
    Q_INVOKABLE bool boolean(bool in) { return in; }
    Q_INVOKABLE double real(double in) { return in; }
    Q_INVOKABLE QString string(const QString &in) { return in; }
    Q_INVOKABLE QStringList stringList(const QStringList &in) { return in; }
    Q_INVOKABLE QVariant variant(const QVariant &in) { return in; }
    Q_INVOKABLE QVariantList variantList(const QVariantList &in) { return in; }
    Q_INVOKABLE QVariantMap variantMap(const QVariantMap &in) { return in; }
    Q_INVOKABLE QByteArray byteArray(const QByteArray &in) { return in; }
    Q_INVOKABLE QVariantList allParams(int intVal, bool boolVal, double doubleVal,
                                        const QString &strVal, const QStringList &strListVal,
                                        const QVariant &varVal, const QVariantList &varList,
                                        const QVariantMap &varMap, const QByteArray &byteArr) {
        QVariantList ret;
        ret << intVal << boolVal << doubleVal << strVal << QVariant(strListVal);
        ret << varVal << QVariant(varList) << QVariant(varMap) << byteArr;
        return ret;
    }

    // Method with unsupported types
    Q_INVOKABLE void unsupportedParameter(const QDate &date) { Q_UNUSED(date); }
    Q_INVOKABLE QDate unsupportedReturn(int d, int m, int y) { return QDate(d,m,y); }

    // For testing call with more parameters
    Q_INVOKABLE void functionWith2Parameters(const QVariantMap &m, const QString &s) {
        Q_UNUSED(m);
        Q_UNUSED(s);
    }

    // For testing GCF::Result function
    Q_INVOKABLE GCF::Result testResultFunction(bool val) {
        if(val)
            return GCF::Result(true, QString(), QString(), QString("Result is good!"));
        return GCF::Result(false, QString("E_BAD_FUNC"), QString("Something went wrong here."), QString("Result is bad!"));
    }

    // Long function
    Q_INVOKABLE void longFunction(int timeout) {
        QElapsedTimer timer;
        timer.start();
        while(!timer.hasExpired(qint64(timeout)))
            qApp->processEvents(QEventLoop::AllEvents|QEventLoop::ExcludeSocketNotifiers);
    }

    // Large payload functions
    Q_INVOKABLE int largeParameter(const QByteArray &bytes) {
        return bytes.size();
    }
    Q_INVOKABLE QByteArray largeReturnType(int size) {
        return QByteArray(size, '@');
    }
    Q_INVOKABLE QByteArray largeParameterAndReturnType(const QByteArray &bytes) {
        return QByteArray(bytes.size(), '@');
    }

    // Special methods
    Q_INVOKABLE void terminate() { QTimer::singleShot(100, qApp, SLOT(quit())); }
    Q_INVOKABLE void terminateNow() { qApp->quit(); }

    // For testing IpcRemoteObject class.
    void initProperties() {
        m_integer = 20;
        m_boolean = false;
        m_real = 123.45;
        m_string = "Hello World";
        m_stringList.clear();
        m_stringList << "A" << "B" << "C" << "D";
        m_variant = QDate::currentDate();
        m_variantList.clear();
        m_variantList << 10 << false << 123.45 << QDate::currentDate();
        m_variantMap.clear();
        m_variantMap["January"] = 31;
        m_variantMap["February"] = 28;
        m_variantMap["March"] = 31;
        m_variantMap["April"] = 30;
        m_variantMap["May"] = 31;
        m_byteArray = QByteArray(1024, 'A');
    }

    Q_PROPERTY(int integer READ integer WRITE setInteger NOTIFY integerSignal)
    int integer() const { return m_integer; }
    Q_SLOT void setInteger(int v) { m_integer = v; emit integerSignal(v); }

    Q_PROPERTY(bool boolean READ boolean WRITE setBoolean NOTIFY booleanSignal)
    bool boolean() const { return m_boolean; }
    Q_SLOT void setBoolean(bool v) { m_boolean = v; emit booleanSignal(v); }

    Q_PROPERTY(double real READ real WRITE setReal NOTIFY realSignal)
    double real() const { return m_real; }
    Q_SLOT void setReal(double v) { m_real = v; emit realSignal(v); }

    Q_PROPERTY(QString string READ string WRITE setString NOTIFY stringSignal)
    QString string() const { return m_string; }
    Q_SLOT void setString(QString v) { m_string = v; emit stringSignal(v); }

    Q_PROPERTY(QStringList stringList READ stringList WRITE setStringList NOTIFY stringListSignal)
    QStringList stringList() const { return m_stringList; }
    Q_SLOT void setStringList(QStringList v) { m_stringList = v; emit stringListSignal(v); }

    Q_PROPERTY(QVariant variant READ variant WRITE setVariant NOTIFY variantSignal)
    QVariant variant() const { return m_variant; }
    Q_SLOT void setVariant(QVariant v) { m_variant = v; emit variantSignal(v); }

    Q_PROPERTY(QVariantList variantList READ variantList WRITE setVariantList NOTIFY variantListSignal)
    QVariantList variantList() const { return m_variantList; }
    Q_SLOT void setVariantList(QVariantList v) { m_variantList = v; emit variantListSignal(v); }

    Q_PROPERTY(QVariantMap variantMap READ variantMap WRITE setVariantMap NOTIFY variantMapSignal)
    QVariantMap variantMap() const { return m_variantMap; }
    Q_SLOT void setVariantMap(QVariantMap v) { m_variantMap = v; emit variantMapSignal(v); }

    Q_PROPERTY(QByteArray byteArray READ byteArray WRITE setByteArray NOTIFY byteArraySignal)
    QByteArray byteArray() const { return m_byteArray; }
    Q_SLOT void setByteArray(QByteArray v) { m_byteArray = v; emit byteArraySignal(v); }

    Q_SLOT void setAll(int intVal, bool boolVal, double doubleVal,
                       const QString &strVal, const QStringList &strListVal,
                       const QVariant &varVal, const QVariantList &varList,
                       const QVariantMap &varMap, const QByteArray &byteArr) {
        m_integer = intVal;
        m_boolean = boolVal;
        m_real = doubleVal;
        m_string = strVal;
        m_stringList = strListVal;
        m_variant = varVal;
        m_variantList = varList;
        m_variantMap = varMap;
        m_byteArray = byteArr;
        this->emitSignals();
    }

    Q_INVOKABLE void emitSignals() {
        emit integerSignal(m_integer);
        emit booleanSignal(m_boolean);
        emit realSignal(m_real);
        emit stringSignal(m_string);
        emit stringListSignal(m_stringList);
        emit variantSignal(m_variant);
        emit variantListSignal(m_variantList);
        emit variantMapSignal(m_variantMap);
        emit byteArraySignal(m_byteArray);
        emit allSignal(m_integer, m_boolean, m_real, m_string,
                       m_stringList, m_variant, m_variantList,
                       m_variantMap, m_byteArray);
    }

signals:
    void integerSignal(int v);
    void booleanSignal(bool v);
    void realSignal(double v);
    void stringSignal(QString v);
    void stringListSignal(QStringList v);
    void variantSignal(QVariant v);
    void variantListSignal(QVariantList v);
    void variantMapSignal(QVariantMap v);
    void byteArraySignal(QByteArray v);
    void allSignal(int intVal, bool boolVal, double doubleVal,
                   const QString &strVal, const QStringList &strListVal,
                   const QVariant &varVal, const QVariantList &varList,
                   const QVariantMap &varMap, const QByteArray &byteArr);

protected:
    Q_INVOKABLE void protectedMethod() { }

private:
    Q_INVOKABLE void privateMethod() { }

private:
    int m_integer;
    bool m_boolean;
    double m_real;
    QString m_string;
    QStringList m_stringList;
    QVariant m_variant;
    QVariantList m_variantList;
    QVariantMap m_variantMap;
    QByteArray m_byteArray;
};

#endif // TESTSERVICE_H
