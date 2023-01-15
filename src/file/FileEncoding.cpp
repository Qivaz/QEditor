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

#include "FileEncoding.h"

const QMap<int, QPair<QString, QString>> encodings = {
    // {MIB number, {name, description}
    // {4,     {"ISO-8859-1",  "Latin1"}},
    // {0,     {"System",      Constants::kEncodingUtf8}},
    {106,   {"UTF-8",       Constants::kEncodingUtf8Bom}},
    {1015,  {"UTF-16",      Constants::kEncodingUtf16}},
    {1013,  {"UTF-16BE",    Constants::kEncodingUtf16Be}},
    {1014,  {"UTF-16LE",    Constants::kEncodingUtf16Le}},
    {1017,  {"UTF-32",      Constants::kEncodingUtf32}},
    {1018,  {"UTF-32BE",    Constants::kEncodingUtf32Be}},
    {1019,  {"UTF-32LE",    Constants::kEncodingUtf32Le}},
};

const QMap<QString, int> encodingDescriptionToMib = {
    // {name, MIB number}
    {Constants::kEncodingSystem,    0   },
    {Constants::kEncodingUtf8,      106 },
    {Constants::kEncodingUtf8Bom,   106 },
    {Constants::kEncodingUtf16,     1015},
    {Constants::kEncodingUtf16Be,   1013},
    {Constants::kEncodingUtf16Le,   1014},
    {Constants::kEncodingUtf32,     1017},
    {Constants::kEncodingUtf32Be,   1018},
    {Constants::kEncodingUtf32Le,   1019},
};

QString FileEncoding::GetDescription(int mibEnum)
{
    if (!hasBom_) {
        if (mibEnum == 0) {
            return Constants::kEncodingSystem;
        } else {  // 106
            return Constants::kEncodingUtf8;
        }
    }
    auto iter = encodings.find(mibEnum);
    if (iter != encodings.end()) {
        return iter.value().second;
    }
    // Not found preset mibenum, use utf-8.
    return "UTF-8";
}

QString FileEncoding::GetMibName(int mibEnum)
{
    auto iter = encodings.find(mibEnum);
    if (iter != encodings.end()) {
        return iter.value().first;
    }
    // Not found preset mibenum, use utf-8.
    return "UTF-8";
}

int FileEncoding::GetMibByDescription(const QString &name)
{
    auto iter = encodingDescriptionToMib.find(name);
    if (iter != encodingDescriptionToMib.end()) {
        return iter.value();
    }
    // Not found preset mibenum, use utf-8.
    return 106;
}
