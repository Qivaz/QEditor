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

#ifndef OUTLINELIST_H
#define OUTLINELIST_H

#include <QTreeWidget>

#include "IParser.h"

class OverviewItem : public QTreeWidgetItem
{
public:
    OverviewItem(int num) : num_(num) {}
    int num() { return num_; }

private:
    int num_;
};

class OutlineList : public QTreeWidget
{
public:
    OutlineList(IParser *parser);

    void HandleItemClicked(QTreeWidgetItem *item, int column);

    int GetIndexByCursorPos(int cursorPos);

private:
    IParser *parser_{nullptr};
};

#endif // OUTLINELIST_H
