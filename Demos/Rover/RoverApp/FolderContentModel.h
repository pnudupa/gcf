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

#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QAbstractListModel>

class FolderContentModel : public QAbstractListModel
{
    Q_OBJECT

public:
    FolderContentModel(QObject *parent=0):QAbstractListModel(parent) {}
    ~FolderContentModel() {}

    Q_PROPERTY(QString folder READ folder WRITE setFolder NOTIFY folderChanged)
    Q_SLOT void setFolder(const QString &folder) {
        if(m_folder == folder)
            return;

        this->beginResetModel();
        m_folder = folder;
        if( QFileInfo(m_folder).isDir() )
            m_fiList = QDir(m_folder).entryInfoList();
        else
            m_fiList.clear();
        this->endResetModel();

        emit folderChanged(m_folder);
    }
    QString folder() const { return m_folder; }
    Q_SIGNAL void folderChanged(const QString& folder);

    enum Roles
    {
        FileNameRole = Qt::UserRole,
        FilePathRole,
        FileSizeRole,
        ThumbnailRole,
        IsFolderRole
    };

    QHash<int,QByteArray> roleNames() const {
        QHash<int,QByteArray> roles;
        roles[FileNameRole] = "fileName";
        roles[FilePathRole] = "filePath";
        roles[FileSizeRole] = "fileSize";
        roles[ThumbnailRole] = "fileThumbnail";
        roles[IsFolderRole] = "fileIsFolder";
        return roles;
    }

    int rowCount(const QModelIndex &parent) const {
        if(parent.isValid()) return 0;
        return m_fiList.count();
    }

    int columnCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return 1;
    }

    QVariant data(const QModelIndex &index, int role) const {
        QFileInfo fi = m_fiList.at( index.row() );
        switch(role) {
        case FileNameRole: return fi.fileName();
        case FilePathRole: return fi.absoluteFilePath();
        case FileSizeRole: return fi.size();
        case ThumbnailRole: return this->thumbnail(fi);
        case IsFolderRole: return fi.isDir();
        default: break;
        }
        return QVariant();
    }

    QString thumbnail(const QFileInfo &fi) const {
        static QStringList imageExts = QStringList() << "jpg" << "png" << "jpeg" << "bmp";

        if( fi.isDir() )
            return "qrc:///Images/FolderIcon.png";

        if( imageExts.contains(fi.suffix().toLower())) {
            QString imageFileName = QString("%1/%2").arg(fi.absolutePath()).arg(fi.fileName());
            if(QFile(imageFileName).exists()) {
                    return QUrl::fromLocalFile(imageFileName).toString();
           }
        }

        return "qrc:///Images/FileIcon.png";
    }


private:
    QString m_folder;
    QFileInfoList m_fiList;
};

#endif // FILESYSTEMMODEL_H
