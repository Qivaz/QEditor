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

#ifndef RECENTFILES_H
#define RECENTFILES_H

#include "Constants.h"
#include <QDataStream>
#include <QObject>

namespace QEditor {
class RecentFiles : public QObject {
    Q_OBJECT
   public:
    static void UpdateFiles(const QString &filePath);
    static void Clear();

    static void StoreFiles();
    static void LoadFiles();

    static const QStringList &files() { return files_; }

   private:
    static QStringList files_;

    static const QString kAppInternalRecentFilesDirName_;
    static const QString kAppInternalRecentFilesFileName_;

    static const int kMaxRecentFilesNum_ = Constants::kMaxOpenRecentFilesNum;
};
}  // namespace QEditor

#endif  // RECENTFILES_H
