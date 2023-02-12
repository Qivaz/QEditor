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

#include "RecentFiles.h"

#include <QDir>
#include <QFile>
#include "Utils.h"
#include "Logger.h"

namespace QEditor {
QStringList RecentFiles::files_;

const QString RecentFiles::kAppInternalRecentFilesDirName_ = Constants::kAppInternalRecentFilesDirName;
const QString RecentFiles::kAppInternalRecentFilesFileName_ = Constants::kAppInternalRecentFilesFileName;

void RecentFiles::Clear() {
    files_.clear();
    StoreFiles();
}

void RecentFiles::UpdateFiles(const QString &filePath) {
    auto size = files_.size();
    auto exist = files_.removeOne(filePath);
    if (!exist && size >= kMaxRecentFilesNum_) {
        (void)files_.takeLast();
    }
    qDebug() << "insert: " << filePath;
    files_.push_front(filePath);
    StoreFiles();
}

void RecentFiles::StoreFiles()
{
    qDebug() << "kAppInternalPath_: " << Constants::kAppInternalPath;
    (void)QDir(Constants::kAppInternalPath).remove(kAppInternalRecentFilesDirName_);
    QString autoSavePath = Constants::kAppInternalPath + kAppInternalRecentFilesDirName_ + "/";
    qDebug() << "autoSavePath: " << autoSavePath;
    (void)Utils::mkdir(autoSavePath);

    // Store files information firstly.
    QFile filesInfoFile(autoSavePath + kAppInternalRecentFilesFileName_);
    filesInfoFile.open(QIODevice::WriteOnly);
    QDataStream filesInfoStream(&filesInfoFile);
    filesInfoStream << files_;
}

void RecentFiles::LoadFiles()
{
    if (!QDir(Constants::kAppInternalPath).exists(kAppInternalRecentFilesDirName_)) {
        qDebug() << "," << kAppInternalRecentFilesDirName_ << " not exists, in " << Constants::kAppInternalPath;
        return;
    }
    QString autoSavePath = Constants::kAppInternalPath + kAppInternalRecentFilesDirName_ + "/";

    // Load files information firstly.
    QFile filesInfoFile(autoSavePath + kAppInternalRecentFilesFileName_);
    if (!filesInfoFile.exists()) {
        qDebug() << ", not exists, " << filesInfoFile.fileName();
        return;
    }
    filesInfoFile.open(QIODevice::ReadOnly);
    QDataStream filesInfoStream(&filesInfoFile);
    filesInfoStream >> files_;
}
}  // namespace QEditor
