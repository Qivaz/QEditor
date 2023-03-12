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

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QHelpEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QStyleFactory>
#include <QTextEdit>
#include <QToolTip>

#include "SearchResultItem.h"
#include "MainWindow.h"
#include "MainTabView.h"
#include "Logger.h"

namespace QEditor {
void HtmlDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem itemOption = option;
    initStyleOption(&itemOption, index);

    qDebug() << itemOption.widget << itemOption.widget->style();
    QStyle *style = itemOption.widget ? itemOption.widget->style() : QApplication::style();

    QTextDocument doc;
    doc.setHtml(itemOption.text);

    // Painting item without text
    itemOption.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &itemOption, painter);

    QAbstractTextDocumentLayout::PaintContext ctx;

    // Highlighting text if item is selected
    if (itemOption.state & QStyle::State_Selected) {
        ctx.palette.setColor(QPalette::Text, itemOption.palette.color(QPalette::Active, QPalette::HighlightedText));
    }

    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &itemOption);
    painter->save();
    painter->translate(textRect.topLeft());
    painter->setClipRect(textRect.translated(-textRect.topLeft()));
    doc.documentLayout()->draw(painter, ctx);
    painter->restore();
}

QSize HtmlDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem itemOption = option;
    initStyleOption(&itemOption, index);

    QTextDocument doc;
    doc.setHtml(itemOption.text);
    doc.setTextWidth(itemOption.rect.width());
    return QSize(doc.idealWidth(), doc.size().height());
}

SearchResultList::SearchResultList(TabView *tabView) : QTreeWidget(&MainWindow::Instance()), tabView_(tabView), menu_(new QMenu(this))
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

    setIndentation(15);

    connect(this, &QTreeWidget::itemDoubleClicked, this, &SearchResultList::HandleItemDoubleClicked);
    connect(this, &QTreeWidget::itemClicked, this, &SearchResultList::HandleItemClicked);

    // Create custom delegate
    QEditor::HtmlDelegate *delegate = new HtmlDelegate();
    // Set delegate to the treeview object
    setItemDelegate(delegate);

    menu_->setStyleSheet(
                       "\
                       QMenu {\
                           color: lightGray;\
                           background-color: rgb(40, 40, 40);\
                           margin: 2px 2px;\
                           border: none;\
                       }\
                       QMenu::item {\
                           color: rgb(225, 225, 225);\
                           background-color: rgb(40, 40, 40);\
                           padding: 5px 5px;\
                       }\
                       QMenu::item:selected {\
                           background-color: rgb(9, 71, 113);\
                       }\
                       QMenu::item:pressed {\
                           border: 1px solid rgb(60, 60, 60); \
                           background-color: rgb(29, 91, 133); \
                       }\
                       QMenu::separator {height: 1px; background-color: rgb(80, 80, 80); }\
                      ");
}

void SearchResultList::SetQss()
{
//    QStringList qss;
//    qss.append(QString("QTreeWidget{color: darkGray; background-color: rgb(28, 28, 28)}"));
//    qss.append(QString("QTreeView::branch:selected{background-color: rgb(54, 54, 54)}"));
//    setStyleSheet(qss.join(""));

    setStyleSheet("QTreeView{color: darkGray; background-color: rgb(28, 28, 28)}"
                  "QTreeView::branch:selected{background-color: rgb(9, 71, 113)}"
//                  "QTreeView::branch:has-children:!has-siblings:closed, \
//                  QTreeView::branch:closed:has-children:has-siblings{border-image: none; image: none;} \
//                  QTreeView::branch:open:has-children:!has-siblings, \
//                  QTreeView::branch:open:has-children:has-siblings{border-image: none; image: none)");
                  "QTreeView::branch:has-siblings:!adjoins-item { \
                      border-image: none 0;\
                  }\
                  QTreeView::branch:has-siblings:adjoins-item {\
                      border-image: none 0;\
                  }\
                  QTreeView::branch:!has-children:!has-siblings:adjoins-item {\
                      border-image: none 0;\
                  }\
                  QTreeView::branch:has-children:!has-siblings:closed,\
                  QTreeView::branch:closed:has-children:has-siblings {\
                          border-image: none;\
                          image: none;\
                  }\
                  QTreeView::branch:open:has-children:!has-siblings,\
                  QTreeView::branch:open:has-children:has-siblings {\
                          border-image: none;\
                          image: none;\
                  }\
                  QTreeView::item{\
                          background: rgb(255, 71, 255);\
                  }\
                  QTreeView::item:selected{\
                          background: rgb(9, 71, 113);\
                  }\
                  QTreeView::item:hover{\
                          background: rgb(9, 255, 113);\
                  }\
                  ");
}

void SearchResultList::setTopItem(QTreeWidgetItem *topItem)
{
    topItem_ = topItem;
}

QTreeWidgetItem *SearchResultList::topItem()
{
    if (topItem_ == nullptr) {
        topItem_ = new SearchResultItem();
        addTopLevelItem(topItem_);
    }
    return topItem_;
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

void SearchResultList::HandleItemClicked(QTreeWidgetItem *item, int column)
{
//    item->setBackgroundColor(0, QColor(255, 71, 255));
}

bool SearchResultList::event(QEvent *event)
{
    qDebug() << event->type();
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        auto item = itemAt(helpEvent->pos());
        qDebug() << helpEvent->pos();
        if (item != nullptr) {
            qDebug() << item->toolTip(0);
            QToolTip::showText(helpEvent->globalPos(), item->toolTip(0));
        } else {
            event->ignore();
        }
    }
    return QTreeWidget::event(event);
}

void SearchResultList::contextMenuEvent(QContextMenuEvent *event)
{
    menu_->clear();
    QAction *collapseAllAction = new QAction(tr("Collapse All"));
    connect(collapseAllAction, &QAction::triggered, this, [this]() {
        collapseAll();
        topItem()->setExpanded(true);
    });
    menu_->addAction(collapseAllAction);
    QAction *expandAllAction = new QAction(tr("Expand All"));
    connect(expandAllAction, &QAction::triggered, this, [this]() {
        expandAll();
    });
    menu_->addAction(expandAllAction);

    menu_->addSeparator();
    QAction *copySelectedAction = new QAction(tr("Copy Selected"));
    connect(copySelectedAction, &QAction::triggered, this, [this]() {
        QString text;
        const auto items = selectedItems();
        for (const auto &item : items) {
            text += item->toolTip(0) + '\n';
        }
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(text);
    });
    menu_->addAction(copySelectedAction);
    QAction *copyAllAction = new QAction(tr("Copy All"));
    connect(copyAllAction, &QAction::triggered, this, [this]() {
        QString text;
        for (int i = 0; i < topItem()->childCount(); ++i) {
            const auto sessionItem = topItem()->child(i);
            text += sessionItem->toolTip(0) + "\n";
            for (int j = 0; j < sessionItem->childCount(); ++j) {
                const auto item = sessionItem->child(j);
                qDebug() << item->toolTip(0);
                text += "    " + item->toolTip(0) + '\n';
            }
            text += '\n';
        }
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(text);
    });
    menu_->addAction(copyAllAction);

    menu_->addSeparator();
    QAction *clearSelectedItemAction = new QAction(tr("Clear Selected"));
    connect(clearSelectedItemAction, &QAction::triggered, this, [&event, this]() {
        auto item = itemAt(event->pos());
        if (item->parent() != nullptr) {
            item->parent()->removeChild(item);
        }
    });
    menu_->addAction(clearSelectedItemAction);
    QAction *clearSelectedResultAction = new QAction(tr("Clear Containing Result"));
    connect(clearSelectedResultAction, &QAction::triggered, this, [&event, this]() {
        auto item = itemAt(event->pos());
        qDebug() << "item: " << item->toolTip(0);
        while (item->parent() != topItem()) {
            item = item->parent();
            qDebug() << "item: " << item->toolTip(0);
        }
        topItem()->removeChild(item);
    });
    menu_->addAction(clearSelectedResultAction);
    QAction *clearAllAction = new QAction(tr("Clear All"));
    connect(clearAllAction, &QAction::triggered, this, [this]() {
        clear();
        setTopItem(nullptr);
    });
    menu_->addAction(clearAllAction);

    menu_->exec(event->globalPos());
}

void SearchResultList::mousePressEvent(QMouseEvent *event)
{
    if (QApplication::mouseButtons() == Qt::RightButton && selectionMode() != SelectionMode::SingleSelection) {
        return;
    }
    if (QApplication::keyboardModifiers() == Qt::ControlModifier) {  // If Ctrl-Key pressed.
        setSelectionMode(SelectionMode::MultiSelection);
    } else if (QApplication::keyboardModifiers() == Qt::ShiftModifier) {  // If Shift-Key pressed.
        setSelectionMode(SelectionMode::ContiguousSelection);
    } else {
        setSelectionMode(SelectionMode::SingleSelection);
    }
    QTreeWidget::mousePressEvent(event);
}

QTreeWidgetItem* SearchResultList::StartSearchSession(EditView *editView)
{
    editView_ = editView;
    const QString &title = editView->fileName();
    SearchResultItem *sessionItem = new SearchResultItem();
    sessionItem->setText(0, title);
    topItem()->insertChild(0, sessionItem);
    return sessionItem;
}

void SearchResultList::AddSearchResult(QTreeWidgetItem *sessionItem, const int lineNum,
                                       const QString &htmlText, const QString &plainText, const QTextCursor &cursor)
{
    SearchResultItem *searchItem = new SearchResultItem(editView_);
    searchItem->setText(0, htmlText);
    searchItem->setToolTip(0, plainText);
    searchItem->setPosition(cursor.selectionStart());
    searchItem->setLen(cursor.selectionEnd() - cursor.selectionStart());
    searchItem->setLine(lineNum);
    sessionItem->addChild(searchItem);
}

void SearchResultList::FinishSearchSession(QTreeWidgetItem *sessionItem, const QString &target, int matchCount)
{
    // Update current session's search info.
    const auto name = sessionItem->text(0);
    const auto title = name + " " + QString(tr("(Search ")) + "\"" + target + "\": " + QString::number(matchCount) + tr(" hits)");
    const auto html = QString("<div style=\"font-size:14px;font-family:Consolas;color:#E3CEAB\">") + title + QString("</div>");
    sessionItem->setToolTip(0, title);
    sessionItem->setText(0, html);
    expandItem(sessionItem);
    setCurrentItem(sessionItem);

    // Update header's total result count.
    auto info = QString::number(topItem()->childCount()) + tr(" results:");
    auto htmlText = QString("<div style=\"font-size:15px;font-family:Consolas;color:#C3AE8B\">") + info + QString("</div>");
    topItem()->setText(0, htmlText);
}
}  // namespace QEditor
