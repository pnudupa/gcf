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

#ifndef DRIVEEXPLORERWINDOW_H
#define DRIVEEXPLORERWINDOW_H

#include <QWidget>
#include <GCF3/IGDriveLite>

class QTreeWidgetItem;

struct DriveExplorerWindowData;
class DriveExplorerWindow : public QWidget
{
    Q_OBJECT

public:
    DriveExplorerWindow(GCF::IGDriveLite *gDrive, QWidget *parent = 0);
    ~DriveExplorerWindow();

    Q_INVOKABLE void showFileLocation(const QString &file);

    GCF::IGDriveLite *gDrive() const;

private slots:
    void on_cmdLogout_clicked();
    void on_cmdRefresh_clicked();
    void on_cmdJobs_clicked();
    void on_driveContentTree_clicked(const QModelIndex &index);
    void on_gridView_clicked(const QModelIndex &index);
    void on_contentProperties_itemClicked(QTreeWidgetItem *item, int column);
    void modelDataChanged(const QModelIndex &index);
    void on_cmdRefreshAuthToken_clicked();
    void onGDriveLiteAuthenticationChanged();

protected:
    bool eventFilter(QObject *, QEvent *);
    void showProperties(const QModelIndex &index);
    void handleContextMenu(const QModelIndex &index);

private:
    void clear();
    void download(const QString &id);
    void share(const QString &id);
    void upload(const QString &id);
    void trash(const QString &id);
    void deletePermanently(const QString &id);
    void restore(const QString &id);
    void rename(const QString &id);
    void describe(const QString &id);
    void star(const QString &id);
    void unstar(const QString &id);
    void move(const QString &id);
    void mkdir(const QString &id);

public:
    Q_INVOKABLE void addSharePermission(QObject *object);
    Q_INVOKABLE void removeSharePermission(QObject *object, int index);
    Q_INVOKABLE void updateSharePermisssion(QObject *object, int index);

private:
    DriveExplorerWindowData *d;
};

#endif // DRIVEEXPLORERWINDOW_H
