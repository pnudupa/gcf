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

#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QtDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>

class RectObject : public QGraphicsObject
{
public:
    RectObject(QGraphicsItem *parent=0) : QGraphicsObject(parent) { }
    ~RectObject() { }

    void setRect(qreal x, qreal y, qreal w, qreal h) {
        this->setRect(QRectF(x,y,w,h));
    }
    void setRect(const QRectF& rect) {
        this->prepareGeometryChange();
        m_rect = rect;
        this->update();
    }
    QRectF rect() const { return m_rect; }

    QPainterPath shape() const {
        QPainterPath path;
        path.addRect(m_rect);
        return path;
    }
    QRectF boundingRect() const { return m_rect; }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
        painter->setPen(Qt::black);
        painter->setBrush(Qt::blue);
        painter->drawRect(this->rect());
    }

#if 0
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) {
        QGraphicsObject::mousePressEvent(event);
        qDebug() << "PRESS" << event->pos();
    }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
        QGraphicsObject::mouseReleaseEvent(event);
        qDebug() << "RELEASE" << event->pos();
    }
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
        QGraphicsObject::mouseMoveEvent(event);
        qDebug() << "MOVE" << event->pos();
    }
#endif

private:
    QRectF m_rect;
};

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    GraphicsView(QWidget *parent=0) : QGraphicsView(parent) {
        this->resize(640, 480);

        QGraphicsScene *gScene = new QGraphicsScene(this);
        this->setScene(gScene);
        gScene->setSceneRect(0, 0, 640, 480);

        QGraphicsTextItem *textItem = new QGraphicsTextItem;
        textItem->setObjectName("Text");
        textItem->setPos(0, 0);
        textItem->setPlainText("Hello World");
        textItem->setFont(QFont("Times New Roman", 32));
        textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
        gScene->addItem(textItem);

        RectObject *rectObject1 = new RectObject;
        rectObject1->setObjectName("Rect1");
        rectObject1->setRect(0, 0, 640, 100);
        rectObject1->setPos(0, 100);
        rectObject1->setFlags(QGraphicsItem::ItemIsSelectable|QGraphicsItem::ItemIsMovable);
        gScene->addItem(rectObject1);

        RectObject *rectObject2 = new RectObject;
        rectObject2->setObjectName("Rect2");
        rectObject2->setRect(0, 0, 640, 100);
        rectObject2->setPos(0, 250);
        rectObject2->setFlags(QGraphicsItem::ItemIsSelectable|QGraphicsItem::ItemIsMovable);
        gScene->addItem(rectObject2);

        m_textItem = textItem;
        m_rectObject1 = rectObject1;
        m_rectObject2 = rectObject2;
    }
    ~GraphicsView() { }

    Q_PROPERTY(QString text READ text)
    QString text() const { return m_textItem->toPlainText(); }

    Q_INVOKABLE bool compareRect1Pos(int x, int y) { return m_rectObject1->pos() == QPointF(x,y); }
    Q_INVOKABLE bool compareRect2Pos(int x, int y) { return m_rectObject2->pos() == QPointF(x,y); }

#if 0
protected:
    void mousePressEvent(QMouseEvent *event) {
        QGraphicsView::mousePressEvent(event);
        qDebug() << "PRESS" << event->pos();
    }
    void mouseReleaseEvent(QMouseEvent *event) {
        QGraphicsView::mouseReleaseEvent(event);
        qDebug() << "RELEASE" << event->pos();
    }
    void mouseMoveEvent(QMouseEvent *event) {
        QGraphicsView::mouseMoveEvent(event);
        qDebug() << "MOVE" << event->pos();
    }
#endif

private:
    QGraphicsTextItem *m_textItem;
    RectObject *m_rectObject1;
    RectObject *m_rectObject2;
};

#endif // GRAPHICSVIEW_H
