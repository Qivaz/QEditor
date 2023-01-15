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

#include <QTreeWidget>
#include "EditView.h"
#include "MainTabView.h"
#include "Logger.h"

class SearchResultList : public QTreeWidget
{
public:
    SearchResultList(TabView *tabView);
    ~SearchResultList()
    {
        if (topItem_ != nullptr) {
            delete topItem_;
            topItem_ = nullptr;
        }
    }

    QTreeWidgetItem* StartSearchSession(EditView *editView);
    void AddSearchResult(QTreeWidgetItem *sessionItem, const int lineNum, const QString &line, const QTextCursor &cursor);
    void FinishSearchSession(QTreeWidgetItem *sessionItem, const QString &extra_info);

    void UpdateTopTitle(const QString &info);

    void HandleItemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    void SetQss();

    // Used for current edit view temporarily.
    EditView *editView_{nullptr};

    TabView *tabView_{nullptr};
    QTreeWidgetItem *topItem_{nullptr};
};

#endif // SEARCHRESULTLIST_H
