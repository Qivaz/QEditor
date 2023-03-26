/**
 * Copyright 2023 QEditor QH
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

#include "SearchTargets.h"
#include "Logger.h"
#include "Utils.h"
#include <QDir>
#include <QFile>

namespace QEditor {
QStringList SearchTargets::targets_;

const QString SearchTargets::kAppInternalSearchTargetsDirName_ = Constants::kAppInternalSearchTargetsDirName;
const QString SearchTargets::kAppInternalSearchTargetsFileName_ = Constants::kAppInternalSearchTargetsFileName;

void SearchTargets::Clear() {
    targets_.clear();
    StoreTargets();
}

void SearchTargets::UpdateTargets(const QString &target) {
    auto size = targets_.size();
    auto exist = targets_.removeOne(target);
    if (!exist && size >= kMaxSearchTargetsNum_) {
        (void)targets_.takeLast();
    }
    qDebug() << "insert: " << target;
    targets_.push_front(target);
    StoreTargets();
}

void SearchTargets::StoreTargets() {
    qDebug() << "kAppInternalPath_: " << Constants::kAppInternalPath;
    (void)QDir(Constants::kAppInternalPath).remove(kAppInternalSearchTargetsDirName_);
    QString autoSavePath = Constants::kAppInternalPath + kAppInternalSearchTargetsDirName_ + "/";
    qDebug() << "autoSavePath: " << autoSavePath;
    (void)Utils::mkdir(autoSavePath);

    // Store targets information firstly.
    QFile filesInfoFile(autoSavePath + kAppInternalSearchTargetsFileName_);
    filesInfoFile.open(QIODevice::WriteOnly);
    QDataStream filesInfoStream(&filesInfoFile);
    filesInfoStream << targets_;
}

void SearchTargets::LoadTargets() {
    if (!QDir(Constants::kAppInternalPath).exists(kAppInternalSearchTargetsDirName_)) {
        qDebug() << "," << kAppInternalSearchTargetsDirName_ << " not exists, in " << Constants::kAppInternalPath;
        return;
    }
    QString autoSavePath = Constants::kAppInternalPath + kAppInternalSearchTargetsDirName_ + "/";

    // Load targets information firstly.
    QFile filesInfoFile(autoSavePath + kAppInternalSearchTargetsFileName_);
    if (!filesInfoFile.exists()) {
        qDebug() << ", not exists, " << filesInfoFile.fileName();
        return;
    }
    filesInfoFile.open(QIODevice::ReadOnly);
    QDataStream filesInfoStream(&filesInfoFile);
    filesInfoStream >> targets_;
}
}  // namespace QEditor
