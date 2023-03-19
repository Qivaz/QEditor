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

#ifndef NODEHIERARCHYSCENE_H
#define NODEHIERARCHYSCENE_H

#include "AnfNodeItem.h"
#include "HierarchyScene.h"
#include "IParser.h"

namespace QEditor {
class AnfNodeHierarchyScene : public HierarchyScene {
    Q_OBJECT
public:
    explicit AnfNodeHierarchyScene(const QString& funcName, IParser* parser, QMenu* itemMenu,
                                   QObject* parent = nullptr);

private:
    std::pair<int, int> PaintNodeCalls(const QString& nodeName, const QMap<QString, NodeInfo>& nodesMap, int startX,
                                       int startY);

    std::pair<AnfNodeItem*, bool> GetNode(const QString& inputName, const NodeInfo& info) {
        const QString& nodeName = "%" + inputName + "(" + info.operatorName_ + ")";
        AnfNodeItem* startNode;
        bool exist;
        auto iter = nodes_.find(nodeName);
        if (iter == nodes_.end()) {
            startNode = new AnfNodeItem(nodeName, info, itemType_, itemMenu_);
            startNode->setBrush(itemColor_);
            addItem(startNode);

            nodes_.insert(nodeName, startNode);
            exist = false;
        } else {
            startNode = iter.value();
            exist = true;
        }
        return {startNode, exist};
    }

    IParser* parser_{nullptr};
    QMap<QString, AnfNodeItem*> nodes_;
};
} // namespace QEditor

#endif // NODEHIERARCHYSCENE_H
