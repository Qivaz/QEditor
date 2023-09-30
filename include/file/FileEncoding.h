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

#ifndef FILEENCODING_H
#define FILEENCODING_H

#include "Logger.h"
#include <QFile>
#include <QMap>
#include <QObject>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QTextCodec>
#else
#include <QtCore5Compat/QTextCodec>
#endif
#include <Toast.h>

namespace QEditor {
//
// https://www.iana.org/assignments/character-sets/character-sets.xml
//
// Check Byte Order Mark.
// BOM Encoding:
//    EF BB BF      UTF-8
//    FE FF         UTF-16 (big-endian)
//    FF FE         UTF-16 (little-endian)
//    00 00 FE FF   UTF-32 (big-endian)
//    FF FE 00 00   UTF-32 (little-endian)
//
// MIB number / name / description:
// 4    ISO-8859-1  latin1
// 106  UTF-8       utf-8
// 1015 UTF-16      utf-16
// 1013 UTF-16BE    utf-16 be
// 1014 UTF-16LE    utf-16 le
// 1017 UTF-32      utf-32
// 1018 UTF-32BE    utf-32 be
// 1019 UTF-32LE    utf-32 le
//

class FileEncoding : public QObject {
    Q_OBJECT
   public:
    FileEncoding() : codec_(QTextCodec::codecForMib(106)), hasBom_(false) {
        qDebug() << "hasBom_:" << hasBom_ << ", codec_: " << codec_ << ", mibEnum: " << mibEnum()
                 << ", name: " << name();
        if (codec_ == nullptr) {
            codec_ = QTextCodec::codecForMib(106);
            Toast::Instance().Show(Toast::kWarning, tr("Use UTF-8 codec instead."));
        }
    }
    FileEncoding(QTextCodec *codec, bool hasBom = false) : codec_(codec), hasBom_(hasBom) {
        qDebug() << "hasBom_:" << hasBom_ << ", codec_: " << codec_ << ", mibEnum: " << mibEnum()
                 << ", name: " << name();
        if (codec_ == nullptr) {
            codec_ = QTextCodec::codecForMib(106);
            Toast::Instance().Show(Toast::kWarning, tr("Use UTF-8 codec instead."));
        }
    }
    FileEncoding(const QString &name)
        : codec_(GetMibByName(name) == 0 ? QTextCodec::codecForLocale() : QTextCodec::codecForMib(GetMibByName(name))),
          hasBom_(name.endsWith(Constants::kCodecMibBom)) {  // ANSI and UTF-8 have no BOM in default.
        qDebug() << "hasBom_:" << hasBom_ << ", codec_: " << codec_ << ", mibEnum: " << mibEnum()
                 << ", name: " << this->name();
        if (codec_ == nullptr) {
            codec_ = QTextCodec::codecForMib(106);
            Toast::Instance().Show(Toast::kWarning, tr("Use UTF-8 codec instead."));
        }
    }
    FileEncoding(int mibEnum)
        : codec_(mibEnum == 0 ? QTextCodec::codecForLocale() : QTextCodec::codecForMib(mibEnum)),
          hasBom_(mibEnum != 106 && mibEnum != 0) {  // ANSI and UTF-8 have no BOM in default.
        qDebug() << "hasBom_:" << hasBom_ << ", codec_: " << codec_ << ", mibEnum: " << mibEnum << ", name: " << name();
        if (codec_ == nullptr) {
            codec_ = QTextCodec::codecForMib(106);
            Toast::Instance().Show(Toast::kWarning, tr("Use UTF-8 codec instead."));
        }
    }
    FileEncoding(QFile &file)
        :  // The max BOM size is 4, and use mibenum 106 i.e. utf-8 in default.
          codec_(QTextCodec::codecForUtfText(file.peek(4), QTextCodec::codecForMib(106))),
          hasBom_(QTextCodec::codecForUtfText(file.peek(4), nullptr) != nullptr) {
        qDebug() << "BOM: " << file.peek(4) << ", hasBom_:" << hasBom_ << ", codec_: " << codec_
                 << ", mibEnum: " << mibEnum() << ", name: " << name();
        if (codec_ == nullptr) {
            codec_ = QTextCodec::codecForMib(106);
            Toast::Instance().Show(Toast::kWarning, tr("Use UTF-8 codec instead."));
        }
    }

    FileEncoding(const FileEncoding &encoding) {
        codec_ = encoding.codec_;
        hasBom_ = encoding.hasBom_;
        if (codec_ == nullptr) {
            codec_ = QTextCodec::codecForMib(106);
            Toast::Instance().Show(Toast::kWarning, tr("Use UTF-8 codec instead."));
        }
    }
    FileEncoding(FileEncoding &&encoding) {
        codec_ = encoding.codec_;
        hasBom_ = encoding.hasBom_;
        if (codec_ == nullptr) {
            codec_ = QTextCodec::codecForMib(106);
            Toast::Instance().Show(Toast::kWarning, tr("Use UTF-8 codec instead."));
        }
    }

    FileEncoding &operator=(const FileEncoding &encoding) {
        codec_ = encoding.codec_;
        hasBom_ = encoding.hasBom_;
        return *this;
    }
    FileEncoding &operator=(FileEncoding &&encoding) {
        codec_ = std::move(encoding.codec_);
        hasBom_ = encoding.hasBom_;
        return *this;
    }

    virtual ~FileEncoding() {}

    bool hasBom() const { return hasBom_; }
    QTextCodec *codec() { return codec_; }
    void setCodec(QTextCodec *codec) { codec_ = codec; }

    int mibEnum() const { return codec_->mibEnum(); }
    QString name() const {
        if (!hasBom()) {
            return codec_->name();
        } else {
            return codec_->name() + " " + Constants::kCodecMibBom;
        }
    }

    // Find proper codec, then change owned codec and return decoded text.
    QString ProcessAnsi(QFile &file);

    static int GetMibByName(const QString &name);
    static QStringList encodingNames();

   private:
    QTextCodec *codec_{nullptr};
    bool hasBom_{false};  // If Byte Order Mark exists.

    static QMap<QString, int> encodingNameToMib_;
    static QStringList encodingNameList_;
};
}  // namespace QEditor

#endif  // FILEENCODING_H
