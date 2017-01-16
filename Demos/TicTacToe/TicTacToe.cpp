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

#include "TicTacToe.h"

#include <GCF3/QmlApplication>

#include <GCF3/IpcServer>
#include <GCF3/IpcRemoteObject>
#include <GCF3/IpcServerDiscovery>

/*
 * Cross is what this player would mark
 * Dot is what other player would mark
 */

const int NONE = 0;
const int DOT = 1;
const int CROSS = 2;

struct TicTacToeData
{
    TicTacToeData() : otherPlayer(0) {
        this->clear();
    }

    QString status;

    int *tile(const QModelIndex &index) {
        int i = index.row()/3;
        int j = index.row()%3;
        return &(tiles[i][j]);
    }
    void clear() {
        for(int i=0; i<3; i++)
            for(int j=0; j<3; j++)
                tiles[i][j] = NONE;
    }
    int tiles[3][3];
    bool myTurn;

    GCF::IpcServer ipcServer;
    GCF::IpcServerDiscovery discovery;
    GCF::IpcRemoteObject *otherPlayer;
};

TicTacToe::TicTacToe()
{
    d = new TicTacToeData;

#if QT_VERSION < 0x050000
    this->setRoleNames( this->roleNames() );
#endif

    d->status = "Waiting for another player to join";

    QVariantMap info;
    info["allowmetaaccess"] = true;
    new GCF::ObjectTreeNode(gAppService->objectTree()->rootNode(), "TicTacToe", this, info);

    d->ipcServer.listen();

    connect(&d->discovery, SIGNAL(foundServer(GCF::IpcServerInfo)),
            this, SLOT(onFoundServer(GCF::IpcServerInfo)));
    d->discovery.start(50005);
}

TicTacToe::~TicTacToe()
{
    delete d;
}

int TicTacToe::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;
    return 9; // 3 x 3 board
}

int TicTacToe::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;
    return 1;
}

QVariant TicTacToe::data(const QModelIndex &index, int role) const
{
    if(role != Qt::DisplayRole)
        return QVariant();

    int *tile = d->tile(index);
    return *tile;
}

QHash<int,QByteArray> TicTacToe::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[Qt::DisplayRole] = "tile";
    return roles;
}

QModelIndex TicTacToe::tileIndex(int row, int col) const
{
    int i = row*3 + col;
    if(i < 0 || i >= 9)
        return QModelIndex();
    return this->index(i, 0);
}

bool TicTacToe::isEnabled() const
{
    return d->otherPlayer && d->otherPlayer->isActivated();
}

QString TicTacToe::status() const
{
    return d->status;
}

void TicTacToe::cross(int i)
{
    if(d->otherPlayer && d->otherPlayer->isActivated())
    {
        if(!d->myTurn)
            return;

        if(i < 0 || i >= 9)
            return;

        QModelIndex index = this->index(i, 0);
        int *tile = d->tile(index);
        if(*tile == NONE)
            *tile = CROSS;
        else
            return;
        emit dataChanged(index, index);

        d->otherPlayer->call("dot", QVariantList() << i);
        this->changeTurn();
        QMetaObject::invokeMethod(this, "checkIfGameComplete", Qt::QueuedConnection);
    }
}

void TicTacToe::dot(int i)
{
    if(d->otherPlayer && d->otherPlayer->isActivated())
    {
        d->myTurn = false;

        if(i < 0 || i >= 9)
            return;

        QModelIndex index = this->index(i, 0);
        int *tile = d->tile(index);
        if(*tile == NONE)
            *tile = DOT;
        else
            return;
        emit dataChanged(index, index);

        d->otherPlayer->call("cross", QVariantList() << i);
        this->changeTurn();
        QMetaObject::invokeMethod(this, "checkIfGameComplete", Qt::QueuedConnection);
    }
}

void TicTacToe::onFoundServer(const GCF::IpcServerInfo &info)
{
    if(d->otherPlayer)
        return;

    QString msg = QString("Found another player '%1'. Connecting...").arg(info.User);
    this->changeStatus(msg);

    d->otherPlayer = new GCF::IpcRemoteObject(info.Address, info.Port, "Application.TicTacToe");
    d->otherPlayer->setObjectName(info.User);
    connect(d->otherPlayer, SIGNAL(activated()), this, SLOT(onPlayerActivated()));
    connect(d->otherPlayer, SIGNAL(deactivated()), this, SLOT(onPlayerDeactivated()));
    connect(d->otherPlayer, SIGNAL(couldNotActivate(QString)), this, SLOT(onPlayerCouldNotActivate(QString)));
}

void TicTacToe::onPlayerActivated()
{
    QString msg = QString("Player '%1' joined the game. You can start.").arg(d->otherPlayer->objectName());
    d->myTurn = true;
    this->changeStatus(msg);
    emit enabledChanged( this->isEnabled() );
    this->clear();
}

void TicTacToe::onPlayerDeactivated()
{
    this->clear();
    emit enabledChanged( this->isEnabled() );

    QString msg = QString("Player '%1' quit the game. Waiting for another player..")
            .arg(d->otherPlayer->objectName());
    this->changeStatus(msg);

    d->otherPlayer->deleteLater();
    d->otherPlayer = 0;
    d->discovery.releaseAllFoundServers();
}

void TicTacToe::onPlayerCouldNotActivate(const QString &msg)
{
    Q_UNUSED(msg);

    this->clear();
    emit enabledChanged( this->isEnabled() );

    QString statusMsg = QString("Player '%1' could not join the game. Waiting for another player..")
            .arg(d->otherPlayer->objectName());
    this->changeStatus(statusMsg);

    d->otherPlayer->deleteLater();
    d->otherPlayer = 0;
    d->discovery.releaseAllFoundServers();
}

void TicTacToe::clear()
{
    this->beginResetModel();
    d->clear();
    d->myTurn = true;
    this->changeStatus("Your turn");
    this->endResetModel();
}

void TicTacToe::changeTurn()
{
    d->myTurn = !d->myTurn;
    if(d->myTurn)
        this->changeStatus("Your turn");
    else
        this->changeStatus( QString("%1's turn").arg(d->otherPlayer->objectName()) );
}

void TicTacToe::changeStatus(const QString &status)
{
    d->status = status;
    emit statusChanged(d->status);
}

void TicTacToe::checkIfGameComplete()
{
    if( this->checkIfIWon() )
    {
        QString msg = QString("%1 won!").arg(d->discovery.user());
        d->otherPlayer->call("gameComplete", QVariantList() << msg);
        emit gameComplete("WINNER! Congratulations.");
    }

    if( this->checkIfDraw() )
        emit gameComplete("Game Draw");
}

bool TicTacToe::checkIfIWon()
{
    // Check diagonals
    bool success = true;
    for(int i=0; i<3; i++)
        success &= (d->tiles[i][i] == CROSS);
    if(success)
        return true;

    success = true;
    for(int i=0; i<3; i++)
        success &= (d->tiles[2-i][i] == CROSS);
    if(success)
        return true;

    // Check rows
    for(int i=0; i<3; i++)
    {
        success = true;
        for(int j=0; j<3; j++)
            success &= (d->tiles[i][j] == CROSS);
        if(success)
            return true;
    }

    // Check columns
    for(int i=0; i<3; i++)
    {
        success = true;
        for(int j=0; j<3; j++)
            success &= (d->tiles[j][i] == CROSS);
        if(success)
            return true;
    }

    return false;
}

bool TicTacToe::checkIfDraw()
{
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<3; j++)
        {
            if(d->tiles[i][j] == NONE)
                return false;
        }
    }

    return true;
}

