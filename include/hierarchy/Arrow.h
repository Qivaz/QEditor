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

#ifndef ARROW_H
#define ARROW_H

#include <QGraphicsLineItem>

namespace QEditor {
class NodeItem;

class Arrow : public QGraphicsLineItem {
   public:
    enum { Type = UserType + 4 };

    Arrow(NodeItem *startItem, NodeItem *endItem, QGraphicsItem *parent = nullptr);

    int type() const override { return Type; }
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void setColor(const QColor &color) { lineColor_ = color; }
    NodeItem *startItem() const { return startNode_; }
    NodeItem *endItem() const { return endNode_; }

    void updatePosition();

   protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

   private:
    NodeItem *startNode_;
    NodeItem *endNode_;
    QPolygonF arrowPolygon_;
    QColor lineColor_ = Qt::blue;
};
}  // namespace QEditor

#endif  // ARROW_H
