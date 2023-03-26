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

#include "RemoteExplorerTreeView.h"
#include "FileType.h"
#include "Logger.h"
#include "MainWindow.h"
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QProcess>
#include <QScreen>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QTreeView>

namespace QEditor {
RemoteExplorerTreeView::RemoteExplorerTreeView(QWidget *parent, const QString &rootPath)
    : QTreeView(parent), rootPath_(rootPath), menu_(new QMenu(parent)) {
    setStyleSheet(
        "QTreeView{color: darkGray; background-color: rgb(28, 28, 28)}"
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
                  QTreeView::item:selected{\
                          background: rgb(9, 71, 113);\
                  }\
                  ");

    verticalScrollBar()->setStyleSheet(
        "QScrollBar {border: none; background-color: rgb(28, 28, 28)}"
        "QScrollBar::add-line:vertical { \
                                            border: none; \
                                            background: none; \
                                        } \
                                        QScrollBar::sub-line:vertical { \
                                            border: none; \
                                            background: none; \
                                        }");
    horizontalScrollBar()->setStyleSheet(
        "QScrollBar {border: none; background-color: rgb(28, 28, 28)}"
        "QScrollBar::add-line:horizontal { \
                                              border: none; \
                                              background: none; \
                                          } \
                                          QScrollBar::sub-line:horizontal { \
                                              border: none; \
                                              background: none; \
                                          }");

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

    model_ = new QSsh::SftpFileSystemModel(this);

    setAnimated(false);
    setIndentation(10);
    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);
    const QSize availableSize = screen()->availableGeometry().size();
    resize(availableSize / 2);
    setColumnWidth(0, width() / 3);

    // Hide the header and the columns except names.
    setHeaderHidden(true);
    hideColumn(1);
    hideColumn(2);
    hideColumn(3);

    connect(this, &QAbstractItemView::clicked, this, &RemoteExplorerTreeView::HandleIndexClick);
    connect(this, &QAbstractItemView::pressed, this, &RemoteExplorerTreeView::HandleIndexPress);
    connect(this, &QTreeView::expanded, this, &RemoteExplorerTreeView::HandleExpanded);
}

void RemoteExplorerTreeView::HandleExpanded(const QModelIndex &index) {
    qDebug() << "index: " << index.row() << index.column() << index.parent().data().toString()
             << index.data().toString();

    QModelIndex modelIndex = index.model()->index(index.row(), index.column(), index.parent());
    qDebug() << "modelIndex: " << modelIndex.row() << modelIndex.column() << index.parent().data().toString()
             << modelIndex.data().toString();
}

void RemoteExplorerTreeView::HandleIndexPress(const QModelIndex &index) {
    if (QApplication::mouseButtons() != Qt::RightButton) {
        return;
    }

    qDebug() << "index: " << index.row() << index.column() << index.parent().data().toString()
             << index.data().toString();
    QModelIndex modelIndex = index.model()->index(index.row(), index.column(), index.parent());
    qDebug() << "modelIndex: " << modelIndex.row() << modelIndex.column() << index.parent().data().toString()
             << modelIndex.data().toString();
}

void RemoteExplorerTreeView::HandleIndexClick(const QModelIndex &index) {
    qCritical() << "index: " << index.row() << index.column() << index.parent().data().toString()
                << index.data().toString();
    QModelIndex modelIndex = index.model()->index(index.row(), index.column(), index.parent());
    qCritical() << "modelIndex: " << modelIndex.row() << modelIndex.column() << index.parent().data().toString()
                << modelIndex.data().toString();
}

void RemoteExplorerTreeView::HandleDirLoaded(const QString &path) {
    qDebug() << "path: " << path;
    if (path == gotoDir_) {
        gotoForLoaded_ = true;
        qDebug() << "Ready to goto " << gotoPath_ << ", since " << path << " loaded.";
    }
}

void RemoteExplorerTreeView::timerEvent(QTimerEvent *event) {}

void RemoteExplorerTreeView::GotoPathPosition(const QString &path) {}
}  // namespace QEditor
