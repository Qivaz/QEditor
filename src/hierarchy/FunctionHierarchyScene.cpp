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

#include "FunctionHierarchyScene.h"
#include "Arrow.h"
#include "FunctionHierarchy.h"
#include "FunctionItem.h"
#include "Toast.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QTextCursor>

namespace QEditor {
FunctionHierarchyScene::FunctionHierarchyScene(IParser* parser, QMenu* itemMenu, QObject* parent)
    : HierarchyScene(itemMenu, parent), parser_(parser) {
    itemMenu_ = itemMenu;
    mode_ = MoveItem;
    itemType_ = FunctionItem::Process;
    line_ = nullptr;
    itemColor_ = QColor(106, 106, 106);
    lineColor_ = QColor(0, 122, 204);

    const auto& entryName = parser_->GetEntry();
    constexpr auto startY = 100;
    constexpr auto distanceY = 100;
    constexpr auto startX = 50;
    constexpr auto distanceX = 50;
    xPos_.clear();
    auto [maxX, maxY] = PaintFunctionCalls(entryName, 0);
    setSceneRect(QRectF(0, 0, (maxX + distanceX) * 1.1, (maxY + distanceY) * 2));
}

std::pair<int, int> FunctionHierarchyScene::PaintFunctionCalls(const QString& funcName, int startX, int startY) {
    constexpr auto distanceY = 100;
    constexpr auto distanceX = 300;
    qDebug() << "funcName: " << funcName;
    const auto& info = parser_->GetFuncGraphInfo(funcName);
    if (info.pos_ == -1) {
        return {0, 0};
    }

    int maxX = 0;
    int maxY = 0;
    auto [startNode, exist] = GetNode(funcName);
    startNode->setPos(QPointF(startX, startY));
    maxX = std::max(maxX, startX);
    maxY = std::max(maxY, startY);

    for (int i = 0; i < info.callees_.size(); ++i) {
        const auto& callee = info.callees_[i];
        qDebug() << "callee: " << callee << ", x: " << (startX + distanceX * i) << ", y: " << (startY + distanceY);
        auto [endNode, exist] = GetNode(callee);
        if (!exist) {
            int newX = startX + distanceX * i;
            int newY = startY + distanceY;
            endNode->setPos(QPointF(newX, newY));
            maxX = std::max(maxX, startX + distanceX * (i + 1));
            maxY = std::max(maxY, newY);
        }

        FunctionItem* startItem = qgraphicsitem_cast<FunctionItem*>(startNode);
        FunctionItem* endItem = qgraphicsitem_cast<FunctionItem*>(endNode);
        Arrow* arrow = new Arrow(startItem, endItem);
        arrow->setColor(lineColor_);
        startItem->addArrow(arrow);
        endItem->addArrow(arrow);
        arrow->setZValue(-1000.0);
        addItem(arrow);
        arrow->updatePosition();

        if (!exist) {
            auto [newMaxX, newMaxY] = PaintFunctionCalls(callee, startX + distanceX * i, startY + distanceY);
            maxX = std::max(maxX, newMaxX);
            maxY = std::max(maxY, newMaxY);
        }
    }
    return {maxX, maxY};
}

std::pair<int, int> FunctionHierarchyScene::PaintFunctionCalls(const QString& funcName, int depth) {
    constexpr auto distanceY = 100;
    constexpr auto distanceX = 200;
    qDebug() << "funcName: " << funcName;
    const auto& info = parser_->GetFuncGraphInfo(funcName);
    if (info.pos_ == -1) {
        return {0, 0};
    }

    if (depth == xPos_.size()) {
        xPos_.push_back(distanceX);
    }
    int maxX = 0;
    int maxY = 0;
    auto [startNode, exist] = GetNode(funcName);
    if (depth >= xPos_.size()) {
        Toast::Instance().Show(Toast::kWarning, QString(tr("Wrong call depth.")));
        return {0, 0};
    }
    startNode->setPos(QPointF(xPos_[depth], (depth + 1) * distanceY));
    maxX = std::max(maxX, xPos_[depth]);
    maxY = std::max(maxY, (depth + 1) * distanceY);

    ++depth;
    for (int i = 0; i < info.callees_.size(); ++i) {
        const auto& callee = info.callees_[i];
        auto [endNode, exist] = GetNode(callee);
        if (!exist) {
            if (depth == xPos_.size()) {
                xPos_.push_back(0);
            }
            if (depth >= xPos_.size()) {
                Toast::Instance().Show(Toast::kWarning, QString(tr("Wrong call depth.")));
                return {0, 0};
            }
            int newX = xPos_[depth] + distanceX;
            xPos_[depth] = newX;
            int newY = (depth + 1) * distanceY;
            endNode->setPos(QPointF(newX, newY));
            maxX = std::max(maxX, newX);
            maxY = std::max(maxY, newY);
        }

        FunctionItem* startItem = qgraphicsitem_cast<FunctionItem*>(startNode);
        FunctionItem* endItem = qgraphicsitem_cast<FunctionItem*>(endNode);
        Arrow* arrow = new Arrow(startItem, endItem);
        arrow->setColor(lineColor_);
        startItem->addArrow(arrow);
        endItem->addArrow(arrow);
        arrow->setZValue(-1000.0);
        addItem(arrow);
        arrow->updatePosition();

        if (!exist) {
            auto [newMaxX, newMaxY] = PaintFunctionCalls(callee, depth);
            maxX = std::max(maxX, newMaxX);
            maxY = std::max(maxY, newMaxX);
        }
    }
    return {maxX, maxY};
}
} // namespace QEditor
