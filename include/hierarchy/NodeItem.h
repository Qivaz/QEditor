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

#ifndef NODEITEM_H
#define NODEITEM_H

#include <QGraphicsPixmapItem>
#include <QVector>

QT_BEGIN_NAMESPACE
class QPixmap;
class QGraphicsSceneContextMenuEvent;
class QMenu;
class QPolygonF;
QT_END_NAMESPACE

class Arrow;
class IParser;

class NodeItem : public QGraphicsPolygonItem
{
public:
    enum { Type = UserType + 15 };
    enum NodeType { Process, Conditional, Io };

    NodeItem(const QString &name, const QColor &color, NodeType diagramType,
             QMenu *contextMenu, QGraphicsItem *parent = nullptr);

    void removeArrow(Arrow *arrow);
    void removeArrows();
    NodeType diagramType() const { return nodeType_; }
    QPolygonF polygon() const { return polygon_; }
    void addArrow(Arrow *arrow);
    QPixmap image() const;
    int type() const override { return Type; }

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    QString name_;

private:
    NodeType nodeType_;
    QPolygonF polygon_;
    QMenu *contextMenu_;
    QVector<Arrow *> arrows_;

    QGraphicsTextItem *textItem_;
};
#endif // NODEITEM_H
