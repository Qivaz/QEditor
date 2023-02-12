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

#include "AnfNodeHierarchyScene.h"

#include <QGraphicsSceneMouseEvent>
#include <QTextCursor>
#include <QDebug>

#include "Arrow.h"
#include "AnfNodeItem.h"
#include "AnfNodeHierarchy.h"
#include "Toast.h"

namespace QEditor {
AnfNodeHierarchyScene::AnfNodeHierarchyScene(const QString &funcName, IParser *parser, QMenu *itemMenu, QObject *parent)
    : HierarchyScene(itemMenu, parent), parser_(parser)
{
    itemMenu_ = itemMenu;
    mode_ = MoveItem;
    itemType_ = AnfNodeItem::Process;
    line_ = nullptr;
    itemColor_ = QColor(106, 106, 106);
    lineColor_ = QColor(0, 122, 204);

    const auto &funcGraphInfo = parser_->GetFuncGraphInfo(funcName);
    const auto &nodesMap = parser_->ParseNodes(funcName);
    constexpr auto startY = 100;
    constexpr auto distanceY = 100;
    constexpr auto startX = 50;
    constexpr auto distanceX = 300;
    xPos_.clear();
    auto [maxX, maxY] = PaintNodeCalls(funcGraphInfo.returnVariable_, nodesMap, 0, distanceY);

    // Handle isolated free variables.
    const auto &nodes = nodesMap.values();
    for (auto iter = nodes.crbegin(); iter != nodes.crend(); ++iter) {
        const auto &nodeInfo = *iter;
        const QString &nodeName = "%" + nodeInfo.variableName_ + "(" + nodeInfo.operatorName_ + ")";
        if (!nodes_.contains(nodeName)) {
            qDebug() << "maxX: " << maxX << ", maxY: " << maxY;
            auto [x, y] = PaintNodeCalls(nodeInfo.variableName_, nodesMap, maxX, distanceY);
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);
        }
    }
}

std::pair<int, int> AnfNodeHierarchyScene::PaintNodeCalls(
    const QString &nodeName, const QMap<QString, NodeInfo> &nodesMap, int startX, int startY)
{
    constexpr auto distanceY = 100;
    constexpr auto distanceX = 250;
    int maxX = 0;
    int maxY = 0;
    const auto &info = nodesMap.value(nodeName);
    if (info.pos_ == -1) {
        return {maxX, maxY};
    }

    auto [startNode, exist] = GetNode(nodeName, info);
    startNode->setPos(QPointF(startX, startY));
    maxX = std::max(maxX, startX) + distanceX;
    maxY = std::max(maxY, startY) + distanceY;

    qDebug() << "nodeName: " << nodeName << ", info.varInputs_: " << info.varInputs_;
    for (int i = 0; i < info.varInputs_.size(); ++i) {
        const auto &input = info.varInputs_[i];
        const auto &inputInfo = nodesMap.value(input);
        qDebug() << "input: " << input << ", pos: " << inputInfo.pos_ << ", x: " << (startX + distanceX * i) << ", y: " << (startY + distanceY);
        if (inputInfo.pos_ == -1) {
            continue;
        }
        auto [endNode, exist] = GetNode(input, inputInfo);
        if (!exist) {
            int newX = startX + distanceX * i;
            int newY = startY + distanceY;
            endNode->setPos(QPointF(newX, newY));
            maxX = std::max(maxX, startX + distanceX * (i + 1));
            maxY = std::max(maxY, newY);
        }

        AnfNodeItem *startItem = qgraphicsitem_cast<AnfNodeItem *>(startNode);
        AnfNodeItem *endItem = qgraphicsitem_cast<AnfNodeItem *>(endNode);
        Arrow *arrow = new Arrow(startItem, endItem);
        arrow->setColor(lineColor_);
        startItem->addArrow(arrow);
        endItem->addArrow(arrow);
        arrow->setZValue(-1000.0);
        addItem(arrow);
        arrow->updatePosition();

        if (!exist) {
            auto [newMaxX, newMaxY] = PaintNodeCalls(input, nodesMap, startX + distanceX * i, startY + distanceY);
            maxX = std::max(maxX, newMaxX);
            maxY = std::max(maxY, newMaxY);
        }
    }
    return {maxX, maxY};
}
}  // namespace QEditor
