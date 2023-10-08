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

#include "FunctionItem.h"
#include "Arrow.h"
#include "Logger.h"
#include "MainWindow.h"
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

namespace QEditor {
FunctionItem::FunctionItem(const QString &name, const QColor &textColor, IParser *parser, NodeType nodeType,
                           QMenu *contextMenu, QGraphicsItem *parent)
    : NodeItem(name, textColor, nodeType, contextMenu, parent), parser_(parser) {}

void FunctionItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    auto editView = MainWindow::Instance().editView();
    if (editView == nullptr) {
        return;
    }
    const auto &itemInfo = parser_->GetFuncGraphInfo(name_);
    if (itemInfo.pos_ == -1) {
        return;
    }
    auto cursor = editView->textCursor();
    cursor.setPosition(itemInfo.pos_, QTextCursor::MoveAnchor);
    editView->GotoCursor(cursor);

    MainWindow::Instance().UpdateNodeHierarchyDockView(new AnfNodeHierarchy(name_, parser_));

    QGraphicsItem::mouseDoubleClickEvent(event);
}
}  // namespace QEditor
