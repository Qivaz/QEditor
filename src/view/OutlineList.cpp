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

#include <QScrollBar>

#include "MainWindow.h"
#include "Logger.h"

namespace QEditor {
OutlineList::OutlineList(IParser *parser) : parser_(parser)
{
    verticalScrollBar()->setStyleSheet("QScrollBar {border: none; background-color: rgb(28, 28, 28)}"
                                       "QScrollBar::add-line:vertical { \
                                            border: none; \
                                            background: none; \
                                        } \
                                        QScrollBar::sub-line:vertical { \
                                            border: none; \
                                            background: none; \
                                        }");
    horizontalScrollBar()->setStyleSheet("QScrollBar {border: none; background-color: rgb(28, 28, 28)}"
                                         "QScrollBar::add-line:horizontal { \
                                              border: none; \
                                              background: none; \
                                          } \
                                          QScrollBar::sub-line:horizontal { \
                                              border: none; \
                                              background: none; \
                                          }");
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

    setStyleSheet("QTreeWidget{color: darkGray; background-color: rgb(28, 28, 28)}"
                  "QTreeView::branch:selected{background-color: rgb(54, 54, 54)}");

    connect(this, &QTreeWidget::itemClicked, this, &OutlineList::HandleItemClicked);

    // TODO: Not work...
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
}

void OutlineList::HandleItemClicked(QTreeWidgetItem *item, int column)
{
    qDebug() << item << column;
    auto editView = MainWindow::Instance().editView();
    if (editView == nullptr) {
        return;
    }
    auto cursor = editView->textCursor();
    auto itemInfo = parser_->funcGraphInfos()[((OverviewItem*)item)->num()];
    cursor.setPosition(itemInfo.pos_, QTextCursor::MoveAnchor);
    editView->GotoCursor(cursor);
}

int OutlineList::GetIndexByCursorPos(int cursorPos)
{
    if (parser_->funcGraphInfos().isEmpty()) {
        return 0;
    }
    return parser_->GetIndexByCursorPosition(cursorPos);
}
}  // namespace QEditor
