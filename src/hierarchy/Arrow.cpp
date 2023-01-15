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

#include "Arrow.h"

#include <QPainter>
#include <QPen>
#include <QtMath>

#include "NodeItem.h"

Arrow::Arrow(NodeItem *startItem, NodeItem *endItem, QGraphicsItem *parent)
    : QGraphicsLineItem(parent), startNode_(startItem), endNode_(endItem)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setPen(QPen(lineColor_, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}

QRectF Arrow::boundingRect() const
{
    qreal extra = (pen().width() + 20) / 2.0;

    return QRectF(line().p1(), QSizeF(line().p2().x() - line().p1().x(),
                                      line().p2().y() - line().p1().y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
}

QPainterPath Arrow::shape() const
{
    QPainterPath path = QGraphicsLineItem::shape();
    path.addPolygon(arrowPolygon_);
    return path;
}

void Arrow::updatePosition()
{
    QLineF line(mapFromItem(startNode_, 0, 0), mapFromItem(endNode_, 0, 0));
    setLine(line);
}

void Arrow::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
                  QWidget *)
{
    if (startNode_->collidesWithItem(endNode_))
        return;

    QPen arrayPen = pen();
    arrayPen.setColor(lineColor_);
    qreal arrowSize = 20;
    painter->setPen(arrayPen);
    painter->setBrush(lineColor_);

    QLineF centerLine(startNode_->pos(), endNode_->pos());
    QPolygonF endPolygon = endNode_->polygon();
    QPointF p1 = endPolygon.first() + endNode_->pos();
    QPointF intersectPoint;
    for (int i = 1; i < endPolygon.count(); ++i) {
        QPointF p2 = endPolygon.at(i) + endNode_->pos();
        QLineF polyLine = QLineF(p1, p2);
        QLineF::IntersectionType intersectionType =
            polyLine.intersects(centerLine, &intersectPoint);
        if (intersectionType == QLineF::BoundedIntersection)
            break;
        p1 = p2;
    }

    setLine(QLineF(intersectPoint, startNode_->pos()));

    double angle = std::atan2(-line().dy(), line().dx());

    QPointF arrowP1 = line().p1() + QPointF(sin(angle + M_PI / 3) * arrowSize,
                                    cos(angle + M_PI / 3) * arrowSize);
    QPointF arrowP2 = line().p1() + QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
                                    cos(angle + M_PI - M_PI / 3) * arrowSize);

    arrowPolygon_.clear();
    arrowPolygon_ << line().p1() << arrowP1 << arrowP2;

    painter->drawLine(line());
    painter->drawPolygon(arrowPolygon_);
    if (isSelected()) {
        painter->setPen(QPen(lineColor_, 1, Qt::DashLine));
        QLineF arrowLine = line();
        arrowLine.translate(0, 4.0);
        painter->drawLine(arrowLine);
        arrowLine.translate(0,-8.0);
        painter->drawLine(arrowLine);
    }
}
