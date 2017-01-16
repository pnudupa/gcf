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

#ifndef OBJECT_H
#define OBJECT_H

#include <QObject>

class Object : public QObject
{
public:
    static const QObjectList &List();
    static int Count();

    Object(QObject *parent = 0);
    ~Object();
};

class ObjectType1 : public Object
{
    Q_OBJECT

public:
    ObjectType1(QObject *parent=0) : Object(parent) { }
    ~ObjectType1() { }
};

class ObjectType2 : public Object
{
    Q_OBJECT

public:
    ObjectType2(QObject *parent=0) : Object(parent) { }
    ~ObjectType2() { }
};

class ObjectType3 : public Object
{
    Q_OBJECT

public:
    ObjectType3(QObject *parent=0) : Object(parent) { }
    ~ObjectType3() { }
};

class ObjectType4 : public Object
{
    Q_OBJECT

public:
    ObjectType4(QObject *parent=0) : Object(parent) { }
    ~ObjectType4() { }
};

class ObjectType5 : public Object
{
    Q_OBJECT

public:
    ObjectType5(QObject *parent=0) : Object(parent) { }
    ~ObjectType5() { }
};

#endif // OBJECT_H
