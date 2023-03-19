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

#ifndef UTILS_H
#define UTILS_H

#include "Logger.h"
#include <QDir>

namespace QEditor {
namespace Utils {
    static inline QString mkdir(const QString& path) {
        QDir dir(path);
        if (dir.exists(path)) {
            return path;
        }
        auto pos = path.lastIndexOf('/');
        if (pos == -1) {
            qDebug() << "Can not create dir: " << path;
            qFatal("Create dir failed.");
            return path;
        }
        auto parentPath = mkdir(path.mid(0, pos));
        auto currentDirName = path.mid(path.lastIndexOf('/') + 1);
        if (!currentDirName.isEmpty()) {
            QDir parentDir(parentPath);
            parentDir.mkpath(currentDirName);
        }
        return path;
    }
} // namespace Utils
} // namespace QEditor

#endif // UTILS_H
