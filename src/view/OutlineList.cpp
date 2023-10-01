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

#include "OutlineList.h"
#include "Logger.h"
#include "MainWindow.h"
#include <QScrollBar>

namespace QEditor {
OutlineList::OutlineList(IParser *parser) : parser_(parser) {
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

    setHeaderHidden(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    int num = 0;
    for (const auto &info : parser->funcGraphInfos()) {
        auto top = new OverviewItem(num);
        ++num;
        auto resizeFont = font();
        resizeFont.setPointSize(10);
        top->setFont(0, resizeFont);
        //        top->setFont(0, QFont("Consolas", 10));
        top->setIcon(0, QIcon(":/images/function.svg"));
        top->setText(0, info.name_ + "() -> " + info.returnValue_);
        addTopLevelItem(top);
    }

    expandAll();

    setIndentation(15);

    setStyleSheet(
        "QTreeView{color:darkGray; background-color:rgb(28,28,28);}"
        "QTreeView::branch:selected{background-color:rgb(9,71,113);}"
        "QTreeView::branch:hover{background:rgb(54,54,54);}"
        "QTreeView::branch:has-siblings:!adjoins-item{border-image:none0;}"
        "QTreeView::branch:has-siblings:adjoins-item{border-image:none0;}"
        "QTreeView::branch:!has-children:!has-siblings:adjoins-item{border-image:none0;}"
        "QTreeView::branch:has-children:!has-siblings:closed,QTreeView::branch:closed:has-children:has-siblings{border-image:none;image:none;}"
        "QTreeView::branch:open:has-children:!has-siblings,QTreeView::branch:open:has-children:has-siblings{border-image:none;image:none;}"
        "QTreeView::item{background:rgb(28,28,28);}"
        "QTreeView::item:selected{color: rgb(0, 215, 210); background:rgb(9,71,113);}"
        "QTreeView::item:hover{background:rgb(54,54,54);}");

    connect(this, &QTreeWidget::itemClicked, this, &OutlineList::HandleItemClicked);

    // TODO: Not work...
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
}

void OutlineList::HandleItemClicked(QTreeWidgetItem *item, int column) {
    qDebug() << item << column;
    auto editView = MainWindow::Instance().editView();
    if (editView == nullptr) {
        return;
    }
    auto cursor = editView->textCursor();
    auto itemInfo = parser_->funcGraphInfos()[((OverviewItem *)item)->num()];
    cursor.setPosition(itemInfo.pos_, QTextCursor::MoveAnchor);
    editView->GotoCursor(cursor);
}

int OutlineList::GetIndexByCursorPos(int cursorPos) {
    if (parser_->funcGraphInfos().isEmpty()) {
        return 0;
    }
    const auto res = parser_->GetIndexByCursorPosition(cursorPos);
    if (res != -1) {
        return res;
    }
    return 0;
}
}  // namespace QEditor
