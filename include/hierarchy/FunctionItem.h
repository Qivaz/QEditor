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

#ifndef FUNCTIONITEM_H
#define FUNCTIONITEM_H

#include "NodeItem.h"
#include <QGraphicsPixmapItem>
#include <QVector>

QT_BEGIN_NAMESPACE
class QPixmap;
class QGraphicsSceneContextMenuEvent;
class QMenu;
class QPolygonF;
QT_END_NAMESPACE

namespace QEditor {
class Arrow;
class IParser;

class FunctionItem : public NodeItem {
public:
    FunctionItem(const QString& name, IParser* parser, NodeType diagramType, QMenu* contextMenu,
                 QGraphicsItem* parent = nullptr);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
    IParser* parser_{nullptr};
};
} // namespace QEditor

#endif // FUNCTIONITEM_H
