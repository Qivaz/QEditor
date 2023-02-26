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

#include <QApplication>

#include "SearchResultItem.h"
#include "MainTabView.h"
#include "Logger.h"

namespace QEditor {
void HtmlDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 optionV4 = option;
    initStyleOption(&optionV4, index);

    QStyle *style = optionV4.widget? optionV4.widget->style() : QApplication::style();

    QTextDocument doc;
    doc.setHtml(optionV4.text);

    /// Painting item without text
    optionV4.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter);

    QAbstractTextDocumentLayout::PaintContext ctx;

    // Highlighting text if item is selected
    if (optionV4.state & QStyle::State_Selected)
        ctx.palette.setColor(QPalette::Text, optionV4.palette.color(QPalette::Active, QPalette::HighlightedText));

    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionV4);
    painter->save();
    painter->translate(textRect.topLeft());
    painter->setClipRect(textRect.translated(-textRect.topLeft()));
    doc.documentLayout()->draw(painter, ctx);
    painter->restore();
}

QSize HtmlDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 optionV4 = option;
    initStyleOption(&optionV4, index);

    QTextDocument doc;
    doc.setHtml(optionV4.text);
    doc.setTextWidth(optionV4.rect.width());
    return QSize(doc.idealWidth(), doc.size().height());
}

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

    // Create custom delegate
    QEditor::HtmlDelegate *delegate = new HtmlDelegate();
    // Set delegate to the treeview object
    setItemDelegate(delegate);
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
    topItem_->insertChild(0, sessionItem);
    setItemWidget(sessionItem, 0, new QLabel(title));
    return sessionItem;
}

void SearchResultList::AddSearchResult(QTreeWidgetItem *sessionItem, const int lineNum, const QString &line, const QTextCursor &cursor)
{
    SearchResultItem *searchItem = new SearchResultItem(editView_);
    searchItem->setText(0, line);
    searchItem->setPosition(cursor.selectionStart());
    searchItem->setLen(cursor.selectionEnd() - cursor.selectionStart());
    searchItem->setLine(lineNum);
    sessionItem->addChild(searchItem);
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
