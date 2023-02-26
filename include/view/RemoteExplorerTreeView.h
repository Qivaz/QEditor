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

#ifndef REMOTEEXPLORERTREEVIEW_H
#define REMOTEEXPLORERTREEVIEW_H

#include <QEvent>
#include <QFileIconProvider>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QTreeView>

#include "sftpfilesystemmodel.h"
#include "SshClient.h"

#include "Logger.h"

namespace QEditor {
//class CustSortFilterProxyModel : public QSortFilterProxyModel
//{
//protected:
//    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
//    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
//    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
//};

class RemoteExplorerTreeView : public QTreeView
{
public:
    RemoteExplorerTreeView(QWidget *parent = nullptr, const QString &rootPath = QString());
    ~RemoteExplorerTreeView() = default;

    void HandleIndexClick(const QModelIndex &index);
    void HandleExpanded(const QModelIndex &index);
    void HandleIndexPress(const QModelIndex &index);
    void HandleDirLoaded(const QString &path);

    void GotoPathPosition(const QString &path);

    bool event(QEvent *event) override
    {
        qDebug() << event->type();
        return QTreeView::event(event);
    }

private:
    void timerEvent(QTimerEvent *event) override;

    QSsh::SftpFileSystemModel *model_;
//    CustSortFilterProxyModel *proxyModel_;
    QMenu *menu_;
    QString gotoDir_;
    QString gotoPath_;
    QString rootPath_;
    bool gotoForLoaded_{false};
};

//class FileIconProvider : public QFileIconProvider
//{
//public:
//    QIcon icon(const QFileInfo &info) const override;
//};
}  // namespace QEditor

#endif // REMOTEEXPLORERTREEVIEW_H
