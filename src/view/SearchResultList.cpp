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

#include "SearchResultList.h"

#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QStyleFactory>
#include <QTextEdit>

#include "SearchResultItem.h"
#include "MainTabView.h"
#include "Logger.h"

namespace QEditor {
SearchResultList::SearchResultList(TabView *tabView) : tabView_(tabView)
{
//    setHeaderLabel("Result...");
//    setStyle(QStyleFactory::create("windows"));
    SetQss();
//    setRootIsDecorated(false);
    setHeaderHidden(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QHeaderView *headerView = header();
//    headerView->setSectionResizeMode(QHeaderView::ResizeToContents);
//    headerView->setStretchLastSection(false);
    headerView->setSectionResizeMode(QHeaderView::Stretch);

    clear();

    topItem_ = new SearchResultItem();
    addTopLevelItem(topItem_);
    expandAll();

    setIndentation(15);

    connect(this, &QTreeWidget::itemDoubleClicked, this, &SearchResultList::HandleItemDoubleClicked);
}

void SearchResultList::SetQss()
{
    QStringList qss;
    qss.append(QString("QTreeWidget{color: darkGray; background-color: rgb(28, 28, 28)}"));
    qss.append(QString("QTreeView::branch:selected{background-color: rgb(54, 54, 54)}"));

//    qss.append(QString("QTreeView::item{color: darkGray; background-color: rgb(28, 28, 28)}"));
//    qss.append(QString("QTreeView::item:hover{background-color: rgb(0,255,0,50)}"));
//    qss.append(QString("QTreeView::item:selected{background-color: rgb(54, 54, 54)}"));

    setStyleSheet(qss.join(""));
}

void SearchResultList::HandleItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    qDebug() << ", " << item << ", col: " << column;
    auto searchItem = (SearchResultItem*)item;
    auto editView = searchItem->editView();
    if (editView == nullptr) {
        qDebug() << "editView_ is null";
        return;
    }
    tabView_->setCurrentWidget(editView);
    auto cursor = editView->textCursor();
    // Move to the position and select the searched text.
    cursor.setPosition(searchItem->position());
    for (int i = 0; i < searchItem->len(); ++i) {
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    }
    editView->GotoCursor(cursor);
}

QTreeWidgetItem* SearchResultList::StartSearchSession(EditView *editView)
{
    editView_ = editView;
    const QString &title = editView->fileName();
    SearchResultItem *sessionItem = new SearchResultItem();
    topItem_->addChild(sessionItem);
    setItemWidget(sessionItem, 0, new QLabel(title));
    return sessionItem;
}

void SearchResultList::AddSearchResult(QTreeWidgetItem *sessionItem, const int lineNum, const QString &line, const QTextCursor &cursor)
{
    SearchResultItem *searchItem = new SearchResultItem(editView_);
    searchItem->setPosition(cursor.selectionStart());
    searchItem->setLen(cursor.selectionEnd() - cursor.selectionStart());
    searchItem->setLine(lineNum);
    sessionItem->addChild(searchItem);

    auto lineText = new QLabel();
    lineText->setTextFormat(Qt::RichText);
    lineText->setWordWrap(true);
    lineText->setStyleSheet("QLabel { background: transparent; color: darkGray; }");
    lineText->setFont(QFont("Consolas", 11));
    setItemWidget(searchItem, 0, lineText);
    lineText->setText(line);
    qDebug() << "editView_ size: " << editView_->sizeHint() << ", " << editView_->size();
    qDebug() << "lineText size: " << lineText->sizeHint();
    // TODO:
    // The label height would be abnormal if not set a width for it.
    // But the horizonal scroll bar will not show when window resize.
    lineText->setMinimumWidth(editView_->size().width());
    lineText->adjustSize();
    searchItem->treeWidget()->setStyleSheet("QTreeView { background: transparent; color: darkGray; }");

    resizeColumnToContents(0);
}

void SearchResultList::FinishSearchSession(QTreeWidgetItem *sessionItem, const QString &extra_info)
{
    auto widget = itemWidget(sessionItem, 0);
    auto title = dynamic_cast<QLabel*>(widget);
    if (title == nullptr) {
        qCritical() << "Expect a label, but got others";
        return;
    }
    title->setFont(QFont("Consolas", 11));
    title->setTextFormat(Qt::RichText);
    title->setText(QString("<font color=#E3CEAB>") + title->text() + " " + extra_info + "</font>");
    expandItem(sessionItem);
    setCurrentItem(sessionItem);
}

void SearchResultList::UpdateTopTitle(const QString &info)
{
    auto title = new QLabel(info);
    title->setFont(QFont("Consolas", 11));
    setItemWidget(topItem_, 0, title);
}
}  // namespace QEditor
