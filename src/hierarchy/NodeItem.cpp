/**
 * Copyright 2022 QEditor QH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "NodeItem.h"
#include "Arrow.h"
#include "Logger.h"
#include "MainWindow.h"
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

namespace QEditor {
NodeItem::NodeItem(const QString &name, const QColor &color, NodeType nodeType, QMenu *contextMenu,
                   QGraphicsItem *parent)
    : QGraphicsPolygonItem(parent), name_(name), nodeType_(nodeType), contextMenu_(contextMenu) {
    textItem_ = new QGraphicsTextItem(name, this);
    auto rect = textItem_->boundingRect();
    rect.moveCenter(boundingRect().center());
    textItem_->setPos(rect.topLeft());
    textItem_->setDefaultTextColor(color);
    qDebug() << rect << rect.height() << rect.width();

    switch (nodeType_) {
        case Conditional:
            polygon_ << QPointF(-100, 0) << QPointF(0, 100) << QPointF(100, 0) << QPointF(0, -100) << QPointF(-100, 0);
            break;
        case Process:
            polygon_ << QPointF(-10 - rect.width() / 2, -25) << QPointF(10 + rect.width() / 2, -25)
                     << QPointF(10 + rect.width() / 2, 25) << QPointF(-10 - rect.width() / 2, 25)
                     << QPointF(-10 - rect.width() / 2, -25);
            break;
        default:
            polygon_ << QPointF(-120, -80) << QPointF(-70, 80) << QPointF(120, 80) << QPointF(70, -80)
                     << QPointF(-120, -80);
            break;
    }
    setPolygon(polygon_);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    //    setOpacity(0.8);
}

void NodeItem::removeArrow(Arrow *arrow) { arrows_.removeAll(arrow); }

void NodeItem::removeArrows() {
    // Need a copy here since removeArrow() will
    // modify the arrows container
    const auto arrowsCopy = arrows_;
    for (Arrow *arrow : arrowsCopy) {
        arrow->startItem()->removeArrow(arrow);
        arrow->endItem()->removeArrow(arrow);
        scene()->removeItem(arrow);
        delete arrow;
    }
}

void NodeItem::addArrow(Arrow *arrow) { arrows_.append(arrow); }

QPixmap NodeItem::image() const {
    QPixmap pixmap(250, 250);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 8));
    painter.translate(125, 125);
    painter.drawPolyline(polygon_);

    return pixmap;
}

void NodeItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    scene()->clearSelection();
    setSelected(true);
    if (contextMenu_ != nullptr) {
        contextMenu_->exec(event->screenPos());
    }
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == QGraphicsItem::ItemPositionChange) {
        for (Arrow *arrow : qAsConst(arrows_)) arrow->updatePosition();
    }

    return value;
}
}  // namespace QEditor
