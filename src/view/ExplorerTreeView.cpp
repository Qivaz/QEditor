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

#include "ExplorerTreeView.h"

#include <QScreen>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QTreeView>

#include "FileType.h"
#include "MainWindow.h"
#include "Logger.h"

ExplorerTreeView::ExplorerTreeView(QWidget *parent, const QString &rootPath) : QTreeView(parent), rootPath_(rootPath)
{
    setStyleSheet("QTreeView{color: darkGray; background-color: rgb(28, 28, 28)}"
                  "QTreeView::branch:selected{background-color: rgb(54, 54, 54)}"
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
                  }");

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

//    QStandardItemModel *model = new QStandardItemModel(this);
//    model->setHorizontalHeaderLabels(QStringList()<<"姓名"<<"性别"<<"年龄");
//    setModel(model);

//    QStandardItem *item1 = new QStandardItem("四年级");

//    model->setItem(0,0,item1);
//    QStandardItem *item00 = new QStandardItem("张三");
//    QStandardItem *item10 = new QStandardItem("张四");
//    QStandardItem *item20 = new QStandardItem("张五");

//    QStandardItem *item01 = new QStandardItem("男");
//    QStandardItem *item11 = new QStandardItem("女");
//    QStandardItem *item21 = new QStandardItem("男");

//    QStandardItem *item02 = new QStandardItem("15");
//    QStandardItem *item12 = new QStandardItem("14");
//    QStandardItem *item22 = new QStandardItem("16");

//    model->item(0,0)->setChild(0,0,item00);
//    model->item(0,0)->setChild(1,0,item10);
//    model->item(0,0)->setChild(2,0,item20);

//    model->item(0,0)->setChild(0,1,item01);
//    model->item(0,0)->setChild(1,1,item11);
//    model->item(0,0)->setChild(2,1,item21);

//    model->item(0,0)->setChild(0,2,item02);
//    model->item(0,0)->setChild(1,2,item12);
//    model->item(0,0)->setChild(2,2,item22);

//    QStandardItem *item2 = new QStandardItem("五年级");
//    model->setItem(1,0,item2);

//    QStandardItem *item200 = new QStandardItem("李三");
//    QStandardItem *item210 = new QStandardItem("李四");
//    QStandardItem *item220 = new QStandardItem("李五");

//    QStandardItem *item201 = new QStandardItem("男");
//    QStandardItem *item211 = new QStandardItem("女");
//    QStandardItem *item221 = new QStandardItem("男");

//    QStandardItem *item202 = new QStandardItem("15");
//    QStandardItem *item212 = new QStandardItem("14");
//    QStandardItem *item222 = new QStandardItem("16");

//    model->item(1,0)->setChild(0,0,item200);
//    model->item(1,0)->setChild(1,0,item210);
//    model->item(1,0)->setChild(2,0,item220);

//    model->item(1,0)->setChild(0,1,item201);
//    model->item(1,0)->setChild(1,1,item211);
//    model->item(1,0)->setChild(2,1,item221);

//    model->item(1,0)->setChild(0,2,item202);
//    model->item(1,0)->setChild(1,2,item212);
//    model->item(1,0)->setChild(2,2,item222);


    model_ = new QFileSystemModel(this);
    model_->setRootPath("");
    model_->setIconProvider(new FileIconProvider());
    model_->setOption(QFileSystemModel::DontUseCustomDirectoryIcons);
    model_->setOption(QFileSystemModel::DontWatchForChanges);
    model_->setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
//    model_->setNameFilters(QStringList({"*.txt", "*.ir", "*.dat", "*.h", "*.cpp", "*.cc", "*.py",}));
//    model_->setNameFilters(QStringList({"*.txt"}));
    proxyModel_ = new CustSortFilterProxyModel();
    proxyModel_->setSourceModel(model_);
    proxyModel_->sort(0, Qt::AscendingOrder);
    setModel(proxyModel_);
    if (!rootPath.isEmpty()) {
        const QModelIndex rootIndex = model_->index(QDir::cleanPath(rootPath));
        if (rootIndex.isValid()) {
            setRootIndex(rootIndex);
        }
    }

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

    connect(this, &QAbstractItemView::clicked, this, &ExplorerTreeView::HandleIndexClick);
    connect(this, &QTreeView::expanded, this, &ExplorerTreeView::HandleExpanded);
    connect(model_, &QFileSystemModel::directoryLoaded, this, &ExplorerTreeView::HandleDirLoaded);
}

void ExplorerTreeView::HandleExpanded(const QModelIndex &index)
{
    qDebug() << "index: " << index.row() << index.column() << index.parent().data().toString() << index.data().toString();

    QModelIndex modelIndex = index.model()->index(index.row(), index.column(), index.parent());
    qDebug() << "modelIndex: " << modelIndex.row() << modelIndex.column() << index.parent().data().toString()
                << modelIndex.data().toString();
}

void ExplorerTreeView::HandleIndexClick(const QModelIndex &index)
{
    qDebug() << "index: " << index.row() << index.column() << index.parent().data().toString() << index.data().toString();

    QModelIndex modelIndex = index.model()->index(index.row(), index.column(), index.parent());
    qDebug() << "modelIndex: " << modelIndex.row() << modelIndex.column() << index.parent().data().toString()
                << modelIndex.data().toString();
    const auto &fileInfo = model_->fileInfo(proxyModel_->mapToSource(index));
    if (fileInfo.isDir()) {
        if (isExpanded(index)) {
            collapse(index);
        } else {
            expand(index);
        }
        return;
    }
    FileType fileType(fileInfo.filePath());
    if (!fileType.IsUnknown()) {
        MainWindow::Instance().tabView()->OpenFile(fileInfo.filePath());
    }
}

void ExplorerTreeView::HandleDirLoaded(const QString &path)
{
    qDebug() << "path: " << path;
    if (path == gotoDir_) {
        gotoForLoaded_ = true;
        qDebug() << "Ready to goto " << gotoPath_ << ", since " << path << " loaded.";
    }
}

void ExplorerTreeView::timerEvent(QTimerEvent *event)
{
    if (gotoForLoaded_) {
        gotoForLoaded_ = false;

        QModelIndex modelIndex = proxyModel_->mapFromSource(model_->index(gotoPath_, 0));
        qDebug() << "modelIndex: " << modelIndex.row() << modelIndex.column() << modelIndex.data().toString()
                 << modelIndex.parent().data().toString() << model_->rowCount(modelIndex);
        scrollTo(modelIndex);
    }
    QTreeView::timerEvent(event);
}

void ExplorerTreeView::GotoPathPosition(const QString &path)
{
    gotoPath_ = path;
    gotoDir_ = path.section(QDir::separator(), 0, -2);

    QModelIndex modelIndex = proxyModel_->mapFromSource(model_->index(gotoPath_, 0));
    setCurrentIndex(modelIndex);
}

bool CustSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    qDebug() << "source_parent: " << source_parent.row() << source_parent.column() << source_parent.data().toString()
             << source_parent.parent().data().toString() << ", source_row: " << source_row;
    return true;
}

bool CustSortFilterProxyModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    return true;
}

bool CustSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    // If sorting by file names column
    if (sortColumn() == 0) {
        QFileSystemModel *model = qobject_cast<QFileSystemModel*>(sourceModel());
        bool asc = (sortOrder() == Qt::AscendingOrder ? true : false);

        QFileInfo leftFileInfo  = model->fileInfo(left);
        QFileInfo rightFileInfo = model->fileInfo(right);

        // If DotAndDot move in the beginning
        if (sourceModel()->data(left).toString() == "..") {
            return asc;
        }
        if (sourceModel()->data(right).toString() == "..") {
            return !asc;
        }

        // Move dirs upper
        if (!leftFileInfo.isDir() && rightFileInfo.isDir()) {
            return !asc;
        }
        if (leftFileInfo.isDir() && !rightFileInfo.isDir()) {
            return asc;
        }
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

QIcon FileIconProvider::icon(const QFileInfo &info) const
{
    if (info.isDir()) {
        auto res = QIcon(":/images/right-outlined.svg");
        res.addFile(":/images/down-outlined.svg", QSize(), QIcon::Mode::Normal, QIcon::State::On);
        return res;
    }
    FileType fileType(info.filePath());
    if (fileType.IsIr()) {
        return QIcon(":/images/file-type-pipeline.svg");
    } else if (fileType.IsPython()) {
        return QIcon(":/images/file-type-python.svg");
    } else if (fileType.IsCpp()) {
        return QIcon(":/images/file-type-cpp2.svg");
    } else if (fileType.IsText()) {
        return QIcon(":images/file-type-text.svg");
    }
    return QFileIconProvider::icon(info);
}
