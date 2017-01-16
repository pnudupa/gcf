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

#ifndef GDRIVELITEHELPER_H
#define GDRIVELITEHELPER_H

#include <GCF3/IGDriveLite>
#include <QQuickView>

struct GDriveLiteHelperData;
class GDriveLiteHelper : public QObject
{
    Q_OBJECT

public:
    GDriveLiteHelper(GCF::IGDriveLite *gDrive);
    ~GDriveLiteHelper();

    GCF::IGDriveLite *gDrive() const;

    Q_INVOKABLE void addSharePermission(QObject *object,
                              const QString &emailAddress,
                              const bool &isNotify,
                              const QString &msg,
                              const int &permision);
    Q_INVOKABLE void removeSharePermission(QObject *object, int index);
    Q_INVOKABLE void updateSharePermisssion(QObject *object, int index, const int &permision);
    Q_INVOKABLE void download(const QString &id, QString path);
    Q_INVOKABLE void upload(const QString &id, QString paths);
    Q_INVOKABLE void share(const QString &id);
    Q_INVOKABLE void describe(const QString &id, const QString &text);
    Q_INVOKABLE void showMoveWindow(const QString &id);
    Q_INVOKABLE void closeMoveWindow();
    Q_INVOKABLE void move(const QString &folderName);
    Q_INVOKABLE void mkdir(const QString &id);
    Q_INVOKABLE void star(const QString &id);
    Q_INVOKABLE void unstar(const QString &id);
    Q_INVOKABLE void rename(const QString &id, const QString &name);
    Q_INVOKABLE bool isInMainDrive(const QString &id);
    Q_INVOKABLE bool isInTrash(const QString &id);
    Q_INVOKABLE void trash(const QString &id);
    Q_INVOKABLE void restore(const QString &id);
    Q_INVOKABLE void refresh(const QString &id);
    Q_INVOKABLE void deletePermanently(const QString &id);

private slots:
    void on_cmdJobs_clicked();

private:
    bool isIn(const QModelIndex &index, const QModelIndex &rootIndex) const;
    bool messageBox(const QString &title, const QString &content, bool showCancelButton=true);
    void showWaitCursor();
    void restoreCursor();

private:
    GDriveLiteHelperData *d;
};

#endif // GDRIVELITEHELPER_H
