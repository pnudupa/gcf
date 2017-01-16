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

#ifndef VERSION_H
#define VERSION_H

#include "GCFGlobal.h"
#include <QString>
#include <QStringList>
#include <QVariant>

namespace GCF
{

class Version
{
public:
    Version(quint16 major=0, quint16 minor=0, quint16 revision=0)
        : m_major(major), m_minor(minor), m_revision(revision) { }
    Version(const Version &other) : m_major(other.major()),
        m_minor(other.minor()), m_revision(other.revision()) { }
    explicit Version(const QString &versionStr) {
        QStringList comps = versionStr.split(".", QString::KeepEmptyParts);
        bool ok = true;
        unsigned int compVal = 0;
        m_major = m_minor = m_revision = 0;
        if(comps.count() <= 3) {
            if(ok && comps.count() >= 1) {
                compVal = comps.at(0).toUInt(&ok);
                ok &= !comps.at(0).isEmpty();
                ok &= compVal <= 0xffff;
                m_major = (quint16)compVal;
            }
            if(ok && comps.count() >= 2) {
                compVal = comps.at(1).toUInt(&ok);
                ok &= !comps.at(1).isEmpty();
                ok &= compVal <= 0xffff;
                m_minor = (quint16)compVal;
            }
            if(ok && comps.count() >= 3) {
                compVal = comps.at(2).toUInt(&ok);
                ok &= !comps.at(2).isEmpty();
                ok &= compVal <= 0xffff;
                m_revision = (quint16)compVal;
            }
        } else
            ok = false;
        if(!ok)
            m_major = m_minor = m_revision = 0;
    }

    ~Version() { }

    bool operator == (const Version &other) const {
        return m_major == other.major() &&
                m_minor == other.minor() &&
                m_revision == other.revision();
    }
    Version &operator = (const Version &other) {
        m_major = other.m_major;
        m_minor = other.m_minor;
        m_revision = other.m_revision;
        return *this;
    }

    bool isValid() const { return m_major > 0; }
    bool isBaseVersion() const {
        return m_major == 1 && m_minor == 0 && m_revision == 0;
    }

    bool operator < (const Version &other) const {
        if(*this == other) return false;
        if(m_major < other.major()) return true;
        if(m_major == other.major() && m_minor < other.minor()) return true;
        if(m_major == other.major() && m_minor == other.minor() && m_revision < other.revision()) return true;
        return false;
    }
    bool operator > (const Version &other) const {
        if(*this == other) return false;
        if(m_major > other.major()) return true;
        if(m_major == other.major() && m_minor > other.minor()) return true;
        if(m_major == other.major() && m_minor == other.minor() && m_revision > other.revision()) return true;
        return false;
    }
    bool operator <= (const Version &other) const {
        return (*this == other) || (*this < other);
    }
    bool operator >= (const Version &other) const {
        return (*this == other) || (*this > other);
    }
    bool operator != (const Version &other) const {
        return !(*this == other);
    }

    quint16 major() const { return m_major; }
    quint16 minor() const { return m_minor; }
    quint16 revision() const { return m_revision; }
    operator QString() const { return this->toString(); }
    QString toString() const {
        if(this->isValid())
            return QString("%1.%2.%3").arg(m_major).arg(m_minor).arg(m_revision);
        return QString();
    }

private:
    quint16 m_major;
    quint16 m_minor;
    quint16 m_revision;
};

}

#include <QMetaType>
Q_DECLARE_METATYPE(GCF::Version)

#endif // VERSION_H
