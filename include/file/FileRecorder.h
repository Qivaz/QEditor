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

#ifndef FILERECORD_H
#define FILERECORD_H

#include <QObject>
#include <QDataStream>

#include "Constants.h"
#include "EditView.h"

class FileRecorder : public QObject
{
    Q_OBJECT
public:
    explicit FileRecorder(QObject *parent = nullptr);

    class FileInfo
    {
    public:
        FileInfo() = default;
        FileInfo(int pos, int num, const QString &path)
            : pos_(pos), num_(num), path_(std::move(path)) {}

        // Store data.
        friend QDataStream &operator<<(QDataStream &stream, const FileInfo &data)
        {
            stream << data.pos_;
            stream << data.num_;
            stream << data.path_;
            return stream;
        }
        // Load into data.
        friend QDataStream &operator>>(QDataStream &stream, FileInfo &data)
        {
            stream >> data.pos_;
            stream >> data.num_;
            stream >> data.path_;
            return stream;
        }

        ///////////////////////////////////////////
        int pos_;               // Previous tab index, used as file name for storing data.
                                // -1 if no need to store or load. Such as: Open file edit but not change, or empty new file edit.

        int num_;               // New file edit, "* new xxx".

        QString path_;          // Open file edit.
                                // Empty if new file.
        ///////////////////////////////////////////

        bool IsNewFile()
        {
            // Notice that, pos_ == -1 maybe empty new file, should not be used as new file checking.
            return path_.isEmpty();
        }
        bool IsOpenFile()
        {
            return !path_.isEmpty();
        }
        bool IsChangedOpenFile()
        {
            return !path_.isEmpty() && pos_ != -1;
        }
        bool IsOriginalOpenFile()
        {
            return !path_.isEmpty() && pos_ == -1;
        }
    };

    class FileList
    {
    public:
        FileList() = default;

        // Store data.
        friend QDataStream &operator<<(QDataStream &stream, const FileList &data)
        {
            stream << data.pos_ << data.fileInfos_;
            return stream;
        }
        // Load into data.
        friend QDataStream &operator>>(QDataStream &stream, FileList &data)
        {
            stream >> data.pos_ >> data.fileInfos_;
            return stream;
        }

        int pos_;
        QVector<FileInfo> fileInfos_;
    };

    class FileData
    {
    public:
        FileData() = default;
        FileData(int mibEnum, const QString &text) : mibEnum_(mibEnum), text_(std::move(text)) {}

        // Store data.
        friend QDataStream &operator<<(QDataStream &stream, const FileData &data)
        {
            stream << data.mibEnum_ << data.text_;
            return stream;
        }
        // Load into data.
        friend QDataStream &operator>>(QDataStream &stream, FileData &data)
        {
            stream >> data.mibEnum_ >> data.text_;
            return stream;
        }

        int mibEnum_;
        QString text_;
    };

    // Set EditViews before store.
    // To call before StoreFiles().
    void SetPos(int pos) { pos_ = pos; }
    void SetEditViews(QVector<EditView*> editViews) { editViews_ = std::move(editViews); }

    // Get loaded text for each tab.
    // To call after LoadFiles().
    int GetPos() { return pos_; }
    const QString GetText(int index)
    {
        if ((size_t)index >= texts_.size()) {
            qCritical() << " index: " << index << ", text size: " << texts_.size();
            return "";
        }
        return texts_[index];
    }
    int GetMibEnum(int index)
    {
        if ((size_t)index >= mibEnums_.size()) {
            qCritical() << " index: " << index << ", mib enum size: " << mibEnums_.size();
            return 106;  // UTF-8 in default.
        }
        return mibEnums_[index];
    }
    const FileInfo GetFileInfo(int index) { return loadedFileInfos_[index]; }
    int GetFileCount() { return loadedFileInfos_.size(); }

    void StoreFiles();
    void LoadFiles();

private:
    QDataStream fileListStream_;
    QDataStream fileDataStream_;

    int pos_;  // Focused edit view.
    QVector<EditView*> editViews_;

    std::vector<QString> texts_;
    std::vector<int> mibEnums_;

    std::vector<FileInfo> loadedFileInfos_;

    const QString kAppInternalPath_ = Constants::kAppInternalPath;
    const QString kAppInternalAutoSaveDirName_ = Constants::kAppInternalAutoSaveDirName;
    const QString kAppInternalFilesInfoFileName_ = Constants::kAppInternalFilesInfoFileName;
};

#endif // FILERECORD_H
