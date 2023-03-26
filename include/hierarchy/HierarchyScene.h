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

#ifndef HIERARCHYSCENE_H
#define HIERARCHYSCENE_H

#include "AnfNodeItem.h"
#include "IParser.h"
#include <QGraphicsScene>

QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
class QMenu;
class QPointF;
class QGraphicsLineItem;
class QFont;
class QGraphicsTextItem;
class QColor;
QT_END_NAMESPACE

namespace QEditor {
class HierarchyScene : public QGraphicsScene {
    Q_OBJECT
   public:
    enum Mode { InsertItem, InsertLine, InsertText, MoveItem };

    explicit HierarchyScene(QMenu *itemMenu, QObject *parent = nullptr);
    QColor itemColor() const { return itemColor_; }
    QColor lineColor() const { return lineColor_; }
    void setLineColor(const QColor &color);
    void setItemColor(const QColor &color);

   public slots:
    void setMode(Mode mode);
    void setItemType(AnfNodeItem::NodeType type);

   signals:
    void itemInserted(AnfNodeItem *item);
    void itemSelected(QGraphicsItem *item);

   protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

    bool ItemChanged(int type) const;

    NodeItem::NodeType itemType_;
    QMenu *itemMenu_;
    Mode mode_;
    bool leftButtonDown_;
    QPointF startPoint_;
    QGraphicsLineItem *line_;
    QColor itemColor_;
    QColor lineColor_;
    QVector<int> xPos_;
};
}  // namespace QEditor

#endif  // HIERARCHYSCENE_H
