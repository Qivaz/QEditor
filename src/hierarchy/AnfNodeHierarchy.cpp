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

#include "AnfNodeHierarchy.h"
#include "AnfNodeHierarchyScene.h"
#include <QScrollBar>

namespace QEditor {
AnfNodeHierarchy::AnfNodeHierarchy(const QString &funcName, IParser *parser, QWidget *parent)
    : QGraphicsView(parent), funcName_(funcName) {
    setStyleSheet(
        "color:darkGray; background-color:rgb(28,28,28); selection-color:lightGray; selection-background-color:rgb(9,71,113); border:none;");
    verticalScrollBar()->setStyleSheet(
        "QScrollBar{background:rgb(28,28,28); border:none; width:10px;}"
        "QScrollBar::handle{background:rgb(54,54,54); border:none;}"
        "QScrollBar::add-line:vertical{border:none; background:none;}"
        "QScrollBar::sub-line:vertical{border:none; background:none;}");
    horizontalScrollBar()->setStyleSheet(
        "QScrollBar{background:rgb(28,28,28); border:none; height:10px;}"
        "QScrollBar::handle{background:rgb(54,54,54); border:none;}"
        "QScrollBar::add-line:horizontal{border:none;background:none;}"
        "QScrollBar::sub-line:horizontal{border:none;background:none;}");

    scene_ = new AnfNodeHierarchyScene(funcName, parser, nullptr, this);
    setScene(scene_);
    scrollContentsBy(0, 0);
}
}  // namespace QEditor
