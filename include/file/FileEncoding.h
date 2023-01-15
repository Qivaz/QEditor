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

#include <QObject>
#include <QFile>
#include <QMap>
#include <QTextCodec>

#include "Logger.h"

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
extern const QMap<int, QPair<QString, QString>> encodings;

class FileEncoding : public QObject
{
    Q_OBJECT
public:
    FileEncoding()
        : codec_(QTextCodec::codecForMib(106)),
          hasBom_(false),
          mibEnum_(106),
          description_("UTF-8") {
        qDebug() << "hasBom_:" << hasBom_ << ", codec_: " << codec_->name()
                   << ", mibEnum_: " << mibEnum_ << ", description_: " << description_;
    }
    FileEncoding(int mibEnum)
        : codec_(mibEnum == 0 ? QTextCodec::codecForLocale() : QTextCodec::codecForMib(mibEnum)),
          hasBom_(mibEnum != 106 && mibEnum != 0),  // ANSI and UTF-8 have no BOM in default.
          mibEnum_(mibEnum),
          description_(GetDescription(mibEnum)) {
        qDebug() << "hasBom_:" << hasBom_ << ", codec_: " << codec_->name()
                   << ", mibEnum_: " << mibEnum_ << ", description_: " << description_;
    }
    FileEncoding(QFile &file)
        :
          // The max BOM size is 4, and use mibenum 106 i.e. utf-8 in default.
          codec_(QTextCodec::codecForUtfText(file.peek(4), QTextCodec::codecForMib(106))),
          hasBom_(QTextCodec::codecForUtfText(file.peek(4), nullptr) != nullptr),
          mibEnum_(codec_->mibEnum()),
          description_(GetDescription(mibEnum_)) {
        qDebug() << "BOM: " << file.peek(4) << ", hasBom_:" << hasBom_ << ", codec_: " << codec_->name()
                   << ", mibEnum_: " << mibEnum_ << ", description_: " << description_;
    }

    FileEncoding(const FileEncoding &encoding) {
        codec_ = encoding.codec_;
        hasBom_ = encoding.hasBom_;
        mibEnum_ = encoding.mibEnum_;
        description_ = encoding.description_;
    }
    FileEncoding(FileEncoding &&encoding) {
        codec_ = encoding.codec_;
        hasBom_ = encoding.hasBom_;
        mibEnum_ = encoding.mibEnum_;
        description_ = std::move(encoding.description_);
    }

    FileEncoding &operator=(const FileEncoding &encoding) {
        codec_ = encoding.codec_;
        hasBom_ = encoding.hasBom_;
        mibEnum_ = encoding.mibEnum_;
        description_ = encoding.description_;
        return *this;
    }
    FileEncoding &operator=(FileEncoding &&encoding) {
        codec_ = std::move(encoding.codec_);
        hasBom_ = encoding.hasBom_;
        mibEnum_ = encoding.mibEnum_;
        description_ = std::move(encoding.description_);
        return *this;
    }

    virtual ~FileEncoding() {}

    bool hasBom() const { return hasBom_; }
    QTextCodec *codec() { return codec_; }
    void setCodec(QTextCodec *codec) {
        codec_ = codec;
        mibEnum_ = codec->mibEnum();
        description_ = GetDescription(mibEnum_);
    }

    int mibEnum() const { return mibEnum_; }
    const QString &description() const { return description_; }

    QTextCodec *GetCodecByDecription(const QString &codecDesc)
    {
        int mib = GetMibByDescription(codecDesc);
        return QTextCodec::codecForMib(mib);
    }
    int GetMibByDescription(const QString &name);

private:
    QString GetDescription(int mibEnum);
    QString GetMibName(int mibEnum);

    QTextCodec *codec_{nullptr};
    bool hasBom_{false};  // If Byte Order Mark exists.
    int mibEnum_{106};
    QString description_;
};
#endif // FILEENCODING_H
