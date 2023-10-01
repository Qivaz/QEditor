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
ExplorerTreeView::ExplorerTreeView(QWidget *parent, const QString &rootPath)
    : QTreeView(parent), menu_(new QMenu(this)), rootPath_(rootPath) {
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
        "QTreeView::item:selected{color:white; background:rgb(9,71,113);}"
        "QTreeView::item:hover{background:rgb(54,54,54);}");
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
    menu_->setStyleSheet(
        "QMenu{color:lightGray; background-color:rgb(40,40,40); margin:2px 2px;border:none;} "
        "QMenu::item{color:rgb(225,225,225); background-color:rgb(40,40,40); "
        "padding:5px 5px;} QMenu::item:selected{background-color:rgb(9,71,113);}"
        "QMenu::item:pressed{border:1px solid rgb(60,60,60); background-color:rgb(29,91,133);} "
        "QMenu::separator{height:1px; background-color:rgb(80,80,80);}");

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
    connect(this, &QAbstractItemView::pressed, this, &ExplorerTreeView::HandleIndexPress);
    connect(this, &QTreeView::expanded, this, &ExplorerTreeView::HandleExpanded);
    connect(model_, &QFileSystemModel::directoryLoaded, this, &ExplorerTreeView::HandleDirLoaded);
}

void ExplorerTreeView::HandleExpanded(const QModelIndex &index) {
    qDebug() << "index: " << index.row() << index.column() << index.parent().data().toString()
             << index.data().toString();

    QModelIndex modelIndex = index.model()->index(index.row(), index.column(), index.parent());
    qDebug() << "modelIndex: " << modelIndex.row() << modelIndex.column() << index.parent().data().toString()
             << modelIndex.data().toString();
}

void ExplorerTreeView::HandleIndexPress(const QModelIndex &index) {
    if (QApplication::mouseButtons() != Qt::RightButton) {
        return;
    }

    qDebug() << "index: " << index.row() << index.column() << index.parent().data().toString()
             << index.data().toString();
    QModelIndex modelIndex = index.model()->index(index.row(), index.column(), index.parent());
    qDebug() << "modelIndex: " << modelIndex.row() << modelIndex.column() << index.parent().data().toString()
             << modelIndex.data().toString();
    menu_->clear();

    const auto &fileInfo = model_->fileInfo(proxyModel_->mapToSource(index));
    auto filePath = fileInfo.canonicalFilePath();
    QAction *copyPathAction = new QAction(tr("Copy Full Path"));
    menu_->addAction(copyPathAction);
    connect(copyPathAction, &QAction::triggered, this, [filePath]() {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(QDir::toNativeSeparators(filePath));
    });
    //    menu_->popup(QCursor::pos());

    auto fileName = fileInfo.fileName();
    QAction *copyNameAction = new QAction(tr("Copy File Name"));
    menu_->addAction(copyNameAction);
    connect(copyNameAction, &QAction::triggered, this, [fileName]() {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(fileName);
    });
    //    menu_->popup(QCursor::pos());

    menu_->addSeparator();

    auto folderPath = fileInfo.canonicalPath();
#if defined(Q_OS_WIN)
    QAction *openExplorerAction = new QAction(tr("Reveal in File Explorer"));
#else
    QAction *openExplorerAction = new QAction(tr("Open Containing Folder"));
#endif
    menu_->addAction(openExplorerAction);
    connect(openExplorerAction, &QAction::triggered, this, [folderPath, filePath]() {
#if defined(Q_OS_WIN)
        QString cmd = QDir::toNativeSeparators(filePath);
        // Have to add \ before all spaces. Using quotes does not work.
        cmd.replace(QString(" "), QString("\ "));
        QProcess::startDetached("explorer /select," + cmd);
#elif defined(Q_OS_LINUX)
        // If use 'QProcess::startDetached("xdg-open", QStringList(QDir::toNativeSeparators(folderPath)))', can't show the browser with the file selected.
        QString cmd = "dbus-send --session --print-reply --dest=org.freedesktop.FileManager1 --type=method_call /org/freedesktop/FileManager1 org.freedesktop.FileManager1.ShowItems array:string:\"file://%1\" string:\"\"";
        QProcess::startDetached(cmd.arg(filePath));
#else  // Q_OS_OSX
        QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
#endif
    });
    menu_->popup(QCursor::pos());
}

void ExplorerTreeView::HandleIndexClick(const QModelIndex &index) {
    qDebug() << "index: " << index.row() << index.column() << index.parent().data().toString()
             << index.data().toString();
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
    FileType fileType(fileInfo.canonicalFilePath());
    if (!fileType.IsUnknown()) {
        MainWindow::Instance().tabView()->OpenFile(fileInfo.canonicalFilePath());
    }
}

void ExplorerTreeView::HandleDirLoaded(const QString &path) {
    qDebug() << "path: " << path;
    if (path == gotoDir_) {
        gotoForLoaded_ = true;
        qDebug() << "Ready to goto " << gotoPath_ << ", since " << path << " loaded.";
    }
}

void ExplorerTreeView::timerEvent(QTimerEvent *event) {
    if (gotoForLoaded_) {
        gotoForLoaded_ = false;

        QModelIndex modelIndex = proxyModel_->mapFromSource(model_->index(gotoPath_, 0));
        qDebug() << "modelIndex: " << modelIndex.row() << modelIndex.column() << modelIndex.data().toString()
                 << modelIndex.parent().data().toString() << model_->rowCount(modelIndex);
        scrollTo(modelIndex);
    }
    QTreeView::timerEvent(event);
}

void ExplorerTreeView::GotoPathPosition(const QString &path) {
    gotoPath_ = path;
    gotoDir_ = path.section(QDir::separator(), 0, -2);

    QModelIndex modelIndex = proxyModel_->mapFromSource(model_->index(gotoPath_, 0));
    setCurrentIndex(modelIndex);
}

bool CustSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    qDebug() << "source_parent: " << source_parent.row() << source_parent.column() << source_parent.data().toString()
             << source_parent.parent().data().toString() << ", source_row: " << source_row;
    return true;
}

bool CustSortFilterProxyModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const {
    return true;
}

bool CustSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    // If sorting by file names column
    if (sortColumn() == 0) {
        QFileSystemModel *model = qobject_cast<QFileSystemModel *>(sourceModel());
        bool asc = (sortOrder() == Qt::AscendingOrder ? true : false);

        QFileInfo leftFileInfo = model->fileInfo(left);
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

QIcon FileIconProvider::icon(const QFileInfo &info) const {
    if (info.isDir()) {
        auto res = QIcon(":/images/right-outlined.svg");
        res.addFile(":/images/down-outlined.svg", QSize(), QIcon::Mode::Normal, QIcon::State::On);
        return res;
    }
    FileType fileType(info.canonicalFilePath());
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
}  // namespace QEditor
