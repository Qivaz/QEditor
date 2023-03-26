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

#include "FileRecorder.h"
#include "Logger.h"
#include "TerminalView.h"
#include "Utils.h"
#include <QDir>
#include <QFile>

namespace QEditor {
FileRecorder::FileRecorder(QObject *parent) : QObject(parent) {}

void FileRecorder::StoreFiles() {
    qDebug() << "kAppInternalPath_: " << kAppInternalPath_;
    (void)QDir(kAppInternalPath_).remove(kAppInternalAutoSaveDirName_);
    QString autoSavePath = kAppInternalPath_ + kAppInternalAutoSaveDirName_ + "/";
    qDebug() << "autoSavePath: " << autoSavePath;
    (void)Utils::mkdir(autoSavePath);

    // Store files information firstly.
    QFile filesInfoFile(autoSavePath + kAppInternalFilesInfoFileName_);
    filesInfoFile.open(QIODevice::WriteOnly);
    QDataStream filesInfoStream(&filesInfoFile);
    FileList fileList;
    fileList.pos_ = pos_;
    for (int i = 0; i < editViews_.size(); ++i) {
        auto const &editView = editViews_[i];
        auto terminalView = qobject_cast<TerminalView *>(editView);
        if (terminalView == nullptr) {
            int pos = i;
            // Open file edit, not change, or empty new file edit.
            if (!editView->ShouldSave()) {
                pos = -1;
            }
            FileInfo fileInfo(pos, editView->newFileNum(), editView->filePath());
            fileList.fileInfos_.push_back(fileInfo);
        } else {
            FileInfo fileInfo(terminalView->ip(), terminalView->port(), terminalView->user(), terminalView->pwd());
            fileList.fileInfos_.push_back(fileInfo);
        }
    }
    filesInfoStream << fileList;

    // Store each file.
    for (int i = 0; i < editViews_.size(); ++i) {
        auto const &editView = editViews_[i];
        // Open file edit, not change, or empty new file edit.
        if (!editView->ShouldSave()) {
            continue;
        }
        // New file edit, or open file edit, has change.
        qDebug() << "store plain text: " << editView->toPlainText()
                 << ", mibEnum: " << editView->fileEncoding().mibEnum() << ", for tab " << i;
        FileData fileData(editView->fileEncoding().mibEnum(), editView->toPlainText());

        QFile fileDataFile(autoSavePath + QString::number(i));
        fileDataFile.open(QIODevice::WriteOnly);
        QDataStream fileDataStream(&fileDataFile);
        fileDataStream << fileData;
    }
}

void FileRecorder::LoadFiles() {
    if (!QDir(kAppInternalPath_).exists(kAppInternalAutoSaveDirName_)) {
        qDebug() << "," << kAppInternalAutoSaveDirName_ << " not exists, in " << kAppInternalPath_;
        return;
    }
    QString autoSavePath = kAppInternalPath_ + kAppInternalAutoSaveDirName_ + "/";

    // Load files information firstly.
    QFile filesInfoFile(autoSavePath + kAppInternalFilesInfoFileName_);
    if (!filesInfoFile.exists()) {
        qDebug() << ", not exists, " << filesInfoFile.fileName();
        return;
    }
    filesInfoFile.open(QIODevice::ReadOnly);
    QDataStream filesInfoStream(&filesInfoFile);
    FileList fileList;
    filesInfoStream >> fileList;
    pos_ = fileList.pos_;
    for (int i = 0; i < fileList.fileInfos_.size(); ++i) {
        qDebug() << i << "-> pos_: " << fileList.fileInfos_[i].index_;
        loadedFileInfos_.emplace_back(fileList.fileInfos_[i]);
    }

    // Load each file.
    qDebug() << ", loadedFileInfos_.size: " << loadedFileInfos_.size();
    for (size_t i = 0; i < loadedFileInfos_.size(); ++i) {
        auto const &fileInfo = loadedFileInfos_[i];
        // Terminal view.
        if (fileInfo.IsTerminal()) {
            continue;
        }
        // Open file edit, not change, or empty new file edit.
        if (fileInfo.IsNewFileOrOriginalOpenFile()) {
            texts_.emplace_back(QString(""));
            mibEnums_.emplace_back(106);  // UTF-8 in default.
            continue;
        }
        // New file edit, or open file edit, has change.
        QFile fileDataFile(autoSavePath + QString::number(fileInfo.index_));
        if (!fileDataFile.exists()) {
            qDebug() << ", not exists, " << fileDataFile.fileName();
            return;
        }
        fileDataFile.open(QIODevice::ReadOnly);
        FileData fileData;
        QDataStream fileDataStream(&fileDataFile);
        fileDataStream >> fileData;
        qDebug() << " load plain text: " << fileData.text_ << ", mibEnum: " << fileData.mibEnum_ << ", for tab " << i;
        texts_.emplace_back(std::move(fileData.text_));
        mibEnums_.emplace_back(fileData.mibEnum_);
    }
}
}  // namespace QEditor
