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

#ifndef SEARCHRESULTLIST_H
#define SEARCHRESULTLIST_H

#include "EditView.h"
#include "Logger.h"
#include "MainTabView.h"
#include <QStyledItemDelegate>
#include <QTreeWidget>

namespace QEditor {
// https://stackoverflow.com/questions/1956542/how-to-make-item-view-render-rich-html-text-in-qt
class HtmlDelegate : public QStyledItemDelegate {
   public:
    HtmlDelegate() = default;
    ~HtmlDelegate() = default;

   protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class SearchResultList : public QTreeWidget {
    Q_OBJECT
   public:
    SearchResultList(TabView *tabView);
    ~SearchResultList() {
        if (topItem_ != nullptr) {
            delete topItem_;
            topItem_ = nullptr;
        }
    }

    QTreeWidgetItem *StartSearchSession(EditView *editView);
    void AddSearchResult(QTreeWidgetItem *sessionItem, const int lineNum, const QString &htmlText,
                         const QString &plainText, const QTextCursor &cursor);
    void FinishSearchSession(QTreeWidgetItem *sessionItem, const QString &target, int matchCount);

    void HandleItemDoubleClicked(QTreeWidgetItem *item, int column);
    void HandleItemClicked(QTreeWidgetItem *item, int column);

    QTreeWidgetItem *topItem();

    void setTopItem(QTreeWidgetItem *topItem);

   protected:
    bool event(QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

   private:
    void SetQss();

    // Used for current edit view temporarily.
    EditView *editView_{nullptr};

    TabView *tabView_{nullptr};
    QTreeWidgetItem *topItem_{nullptr};

    QMenu *menu_;
};
}  // namespace QEditor

#endif  // SEARCHRESULTLIST_H
