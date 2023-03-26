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

#ifndef FILETYPE_H
#define FILETYPE_H

#include <QFileInfo>
#include <QObject>
#include <QSet>

namespace QEditor {
class FileType : public QObject {
    Q_OBJECT
   public:
    FileType() { fileType_ = kTypeNone; }
    FileType(const QString &path) : fileInfo_(path) { Check(); }
    virtual ~FileType() {}

    void SetPath(const QString &path) {
        fileInfo_.setFile(path);
        Check();
    }

    bool IsUnknown() const { return fileType_ == kTypeNone; }
    bool IsText() const { return fileType_ == kTypeTxt; }
    bool IsCpp() const { return fileType_ == kTypeCpp; }
    bool IsPython() const { return fileType_ == kTypePython; }
    bool IsIr() const { return fileType_ == kTypeIr; }

    bool IsSupported() {
        if (IsUnknown()) {
            return false;
        }
        return true;
    }

    enum Type {
        kTypeNone,  // Not recognition language.
        kTypeTxt,
        kTypeCpp,  // Treat the same for C and C++.
        kTypePython,
        kTypeIr
    };

    Type fileType() const { return fileType_; }

   private:
    void Check() {
        if (txtSuffix_.contains(fileInfo_.suffix())) {
            fileType_ = kTypeTxt;
        } else if (cppSuffix_.contains(fileInfo_.suffix())) {
            fileType_ = kTypeCpp;
        } else if (pythonSuffix_.contains(fileInfo_.suffix())) {
            fileType_ = kTypePython;
        } else if (irSuffix_.contains(fileInfo_.suffix())) {
            fileType_ = kTypeIr;
        } else {
            fileType_ = kTypeNone;
        }
    }

    QFileInfo fileInfo_;
    Type fileType_ = kTypeNone;
    const QSet<QString> cppSuffix_ = {"h", "c", "cc", "cpp"};
    const QSet<QString> pythonSuffix_ = {"py"};
    const QSet<QString> irSuffix_ = {"ir", "dat"};
    const QSet<QString> txtSuffix_ = {"txt"};
};
}  // namespace QEditor

#endif  // FILETYPE_H
