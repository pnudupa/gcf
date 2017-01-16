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

#ifndef FILEBROWSERCOMPONENT_H
#define FILEBROWSERCOMPONENT_H

#include <GCF3/GuiComponent>
#include <QWidget>

struct FileBrowserComponentData;
class FileBrowserComponent : public GCF::GuiComponent
{
    Q_OBJECT

public:
    FileBrowserComponent(QObject *parent=0);

signals:
    void fileDoubleClicked(const QString &fileName);

protected:
    // Destructor is protected, because we want components
    // to be destroyed only via finalize() method and
    // we dont want instanes of this component to be created
    // on the stack.
    ~FileBrowserComponent();

    void contentLoadEvent(GCF::ContentLoadEvent *e);
    QWidget *loadWidget(const QString &name, const QVariantMap &info);

protected slots:
    void onTreeViewActivated(const QModelIndex &index);

private:
    FileBrowserComponentData *d;
};

#endif // FILEBROWSERCOMPONENT_H
