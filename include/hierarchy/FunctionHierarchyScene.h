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

#ifndef FUNCTIONHIERARCHYSCENE_H
#define FUNCTIONHIERARCHYSCENE_H

#include "FunctionItem.h"
#include "HierarchyScene.h"
#include "IParser.h"

namespace QEditor {
class FunctionHierarchyScene : public HierarchyScene {
    Q_OBJECT
   public:
    explicit FunctionHierarchyScene(IParser *parser, QMenu *itemMenu, QObject *parent = nullptr);

   private:
    std::pair<int, int> PaintFunctionCalls(const QString &funcName, int startX, int startY);
    std::pair<int, int> PaintFunctionCalls(const QString &funcName, int depth);

    std::pair<FunctionItem *, bool> GetNode(const QString &funcName) {
        FunctionItem *startNode;
        bool exist;
        auto iter = nodes_.find(funcName);
        if (iter == nodes_.end()) {
            startNode = new FunctionItem(funcName, itemTextColor_, parser_, itemType_, itemMenu_);
            startNode->setBrush(itemFillColor_);
            startNode->setPen(itemLineColor_);
            addItem(startNode);

            nodes_.insert(funcName, startNode);
            exist = false;
        } else {
            startNode = iter.value();
            exist = true;
        }
        return {startNode, exist};
    }

    IParser *parser_{nullptr};
    QMap<QString, FunctionItem *> nodes_;
};
}  // namespace QEditor

#endif  // FUNCTIONHIERARCHYSCENE_H
