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

#include "AnfNodeItem.h"

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

#include "Arrow.h"
#include "Logger.h"
#include "MainWindow.h"

AnfNodeItem::AnfNodeItem(const QString &name, const NodeInfo &info, NodeType nodeType, QMenu *contextMenu, QGraphicsItem *parent)
    : NodeItem(name, QColor(Qt::white), nodeType, contextMenu, parent), nodeInfo_(info) {}

void AnfNodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    auto editView = MainWindow::Instance().editView();
    if (editView == nullptr) {
        return;
    }
    auto cursor = editView->textCursor();
    cursor.setPosition(nodeInfo_.pos_, QTextCursor::MoveAnchor);
    editView->GotoCursor(cursor);
    QGraphicsItem::mouseDoubleClickEvent(event);
}

