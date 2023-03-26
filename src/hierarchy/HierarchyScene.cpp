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

#include "HierarchyScene.h"
#include "Arrow.h"
#include "NodeItem.h"
#include "Toast.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QTextCursor>

namespace QEditor {
HierarchyScene::HierarchyScene(QMenu *itemMenu, QObject *parent) : QGraphicsScene(parent) { itemMenu_ = itemMenu; }

void HierarchyScene::setLineColor(const QColor &color) {
    lineColor_ = color;
    if (ItemChanged(Arrow::Type)) {
        Arrow *item = qgraphicsitem_cast<Arrow *>(selectedItems().first());
        item->setColor(lineColor_);
        update();
    }
}

void HierarchyScene::setItemColor(const QColor &color) {
    itemColor_ = color;
    if (ItemChanged(AnfNodeItem::Type)) {
        AnfNodeItem *item = qgraphicsitem_cast<AnfNodeItem *>(selectedItems().first());
        item->setBrush(itemColor_);
    }
}

void HierarchyScene::setMode(Mode mode) { mode_ = mode; }

void HierarchyScene::setItemType(AnfNodeItem::NodeType type) { itemType_ = type; }

void HierarchyScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    if (mouseEvent->button() != Qt::LeftButton) return;

    AnfNodeItem *item;
    switch (mode_) {
        case InsertItem:
            item = new AnfNodeItem("Dummy", NodeInfo(), itemType_, itemMenu_);
            item->setBrush(itemColor_);
            addItem(item);
            qWarning() << "scenePos: " << mouseEvent->scenePos();
            item->setPos(mouseEvent->scenePos());
            emit itemInserted(item);
            break;

        case InsertLine:
            line_ = new QGraphicsLineItem(QLineF(mouseEvent->scenePos(), mouseEvent->scenePos()));
            line_->setPen(QPen(lineColor_, 2));
            addItem(line_);
            break;

        default:
            break;
    }
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void HierarchyScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    if (mode_ == InsertLine && line_ != nullptr) {
        QLineF newLine(line_->line().p1(), mouseEvent->scenePos());
        line_->setLine(newLine);
    } else if (mode_ == MoveItem) {
        QGraphicsScene::mouseMoveEvent(mouseEvent);
    }
}

void HierarchyScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    if (line_ != nullptr && mode_ == InsertLine) {
        QList<QGraphicsItem *> startItems = items(line_->line().p1());
        if (startItems.count() && startItems.first() == line_) startItems.removeFirst();
        QList<QGraphicsItem *> endItems = items(line_->line().p2());
        if (endItems.count() && endItems.first() == line_) endItems.removeFirst();

        removeItem(line_);
        delete line_;

        if (startItems.count() > 0 && endItems.count() > 0 && startItems.first()->type() == AnfNodeItem::Type &&
            endItems.first()->type() == AnfNodeItem::Type && startItems.first() != endItems.first()) {
            AnfNodeItem *startItem = qgraphicsitem_cast<AnfNodeItem *>(startItems.first());
            AnfNodeItem *endItem = qgraphicsitem_cast<AnfNodeItem *>(endItems.first());
            Arrow *arrow = new Arrow(startItem, endItem);
            arrow->setColor(lineColor_);
            startItem->addArrow(arrow);
            endItem->addArrow(arrow);
            arrow->setZValue(-1000.0);
            addItem(arrow);
            arrow->updatePosition();
        }
    }
    line_ = nullptr;
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

bool HierarchyScene::ItemChanged(int type) const {
    const QList<QGraphicsItem *> items = selectedItems();
    const auto cb = [type](const QGraphicsItem *item) { return item->type() == type; };
    return std::find_if(items.begin(), items.end(), cb) != items.end();
}
}  // namespace QEditor
