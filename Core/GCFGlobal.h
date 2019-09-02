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

#ifndef GCF_H
#define GCF_H

#include <QUrl>
#include <QtDebug>
#include <QtGlobal>
#include <QTextStream>
#include <QDataStream>
#include <QCoreApplication>

#ifdef GCF_STATIC_BUILD
    #define GCF_EXPORT
#else
    #ifdef GCF_DLL_BUILD
        #define GCF_EXPORT Q_DECL_EXPORT
    #else
        #define GCF_EXPORT Q_DECL_IMPORT
    #endif
#endif

#ifndef GCF_VERSION_MAJOR
#define GCF_VERSION_MAJOR 3
#endif

#ifndef GCF_VERSION_MINOR
#define GCF_VERSION_MINOR 0
#endif

#ifndef GCF_VERSION_REVISION
#define GCF_VERSION_REVISION 0
#endif

#define GCF_INTERFACE_BEGIN
#define GCF_INTERFACE_END

namespace GCF
{

class Version;

class Result
{
public:
    Result() : m_success(false) { }
    Result(bool val) : m_success(val) { }
    Result(bool val, const QString &code, const QString &msg=QString(), const QVariant &data=QVariant())
        : m_success(val), m_code(code), m_message(msg), m_data(data) { }
    Result(const Result &other) : m_success(other.m_success),
        m_code(other.m_code), m_message(other.m_message),
        m_data(other.m_data) { }

    bool isSuccess() const { return m_success; }
    QString code() const {
        if(m_code.isEmpty())
            return m_success ? "SUCCESS" : "FAILURE";
        return m_success ? QString() : m_code;
    }
    QString message() const {
        if(m_message.isEmpty())
            return m_success ? QString() : "unknown cause";
        return m_success ? QString() : m_message;
    }
    QVariant data() const { return m_success ? m_data : QVariant(); }

    operator bool() const { return m_success; }

    Result &operator = (const Result &other) {
        m_success = other.m_success;
        m_code = other.m_code;
        m_message = other.m_message;
        m_data = other.m_data;
        return *this;
    }

    Result &operator = (bool val) {
        m_success = val;
        m_code.clear();
        m_message.clear();
        m_data = QVariant();
        return *this;
    }

    bool operator == (const Result &other) const {
        return m_success == other.m_success &&
               m_code == other.m_code &&
               m_message == other.m_message &&
               m_data == other.m_data;
    }

    bool operator == (bool val) { return m_success == val; }

    QString toString() const {
        return QString("GCF::Result(%1. CODE:\"%2\". MSG:\"%3\") ")
                .arg(m_success ? "true" : "false").arg(m_code).arg(m_message);
    }

private:
    bool m_success;
    QString m_code;
    QString m_message;
    QVariant m_data;
};

GCF_EXPORT const GCF::Version &version();
GCF_EXPORT QString vendor();
GCF_EXPORT QString url();
GCF_EXPORT QString buildTimestamp();

GCF_EXPORT void setContentDirectory(const QString &path);
GCF_EXPORT QString contentDirectory();

GCF_EXPORT void setSettingsDirectory(const QString &path);
GCF_EXPORT QString settingsDirectory();

GCF_EXPORT QString applicationDataDirectoryPath();

GCF_EXPORT QStringList searchPathsForLibs();
GCF_EXPORT QStringList searchPaths();
GCF_EXPORT QString findLibrary(const QString &name);
GCF_EXPORT QString findFile(const QString &name);
GCF_EXPORT QStringList findFiles(const QString &name);

}

inline QDebug operator << (QDebug dbg, const GCF::Result &r)
{
    dbg.nospace() << "GCF::Result(" << r.isSuccess()
                  << ". CODE:" << r.code() << ". MSG:"
                  << r.message() << ")";
    return dbg.space();
}

#include <QMetaType>
Q_DECLARE_METATYPE(GCF::Result)

#define GCF_TRANSLATE(x) // This macro is used for applying translations

#endif // GCF_H
