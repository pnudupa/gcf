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

#ifndef TICTACTOE_H
#define TICTACTOE_H

#include <QAbstractListModel>

namespace GCF { struct IpcServerInfo; }

struct TicTacToeData;
class TicTacToe : public QAbstractListModel
{
    Q_OBJECT

public:
    TicTacToe();
    ~TicTacToe();

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int,QByteArray> roleNames() const;

    QModelIndex tileIndex(int row, int col) const;

    Q_PROPERTY(bool enabled READ isEnabled NOTIFY enabledChanged)
    bool isEnabled() const;
    Q_SIGNAL void enabledChanged(bool enabled);

    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    QString status() const;
    Q_SIGNAL void statusChanged(const QString &status);

    Q_INVOKABLE void cross(int index);
    Q_INVOKABLE void dot(int index);
    Q_SIGNAL void gameComplete(const QString &msg);
    Q_INVOKABLE void clear();

private slots:
    void onFoundServer(const GCF::IpcServerInfo &info);
    void onPlayerActivated();
    void onPlayerDeactivated();
    void onPlayerCouldNotActivate(const QString &msg);

private:
    void changeTurn();
    void changeStatus(const QString &status);
    Q_INVOKABLE void checkIfGameComplete();
    bool checkIfIWon();
    bool checkIfDraw();

private:
    TicTacToeData *d;
};

#endif // TICTACTOE_H
