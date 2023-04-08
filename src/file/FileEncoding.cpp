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
#include <QList>

namespace QEditor {
QStringList FileEncoding::encodingNameList_ = {
    "System",
    "UTF-8",  // Move
    "UTF-8 BOM",
    "UTF-16BE",  // Move
    "UTF-16LE",  // Move
    "UTF-16",    // Move
    "UTF-32",    // Move
    "UTF-32BE",  // Move
    "UTF-32LE",  // Move
    "US-ASCII",
    "ISO-8859-1",
    "ISO-8859-2",
    "ISO-8859-3",
    "ISO-8859-4",
    "ISO-8859-5",
    "ISO-8859-6",
    "ISO-8859-7",
    "ISO-8859-8",
    "ISO-8859-9",
    "ISO-8859-10",
    "ISO-2022-JP-1",
    "Shift_JIS",
    "EUC-JP",
    // "US-ASCII",
    "EUC-KR",
    "ISO-2022-KR",
    "EUC-KR",
    "ISO-2022-JP",
    "ISO-2022-JP-2",
    "GB_2312-80",
    "ISO-8859-6",
    "ISO-8859-6",
    "ISO-8859-8",
    "ISO-8859-8",
    "ISO-2022-CN",
    "ISO-2022-CN-EXT",
    // "UTF-8",
    "ISO-8859-13",
    "ISO-8859-14",
    "ISO-8859-15",
    "GBK",
    "GB18030",
    "UCS-2",
    "UCS-4",
    "SCSU",
    "UTF-7",
    // "UTF-16BE",
    // "UTF-16LE",
    // "UTF-16",
    "CESU-8",
    // "UTF-32",
    // "UTF-32BE",
    // "UTF-32LE",
    "BOCU-1",
    "hp-roman8",
    "Adobe-Standard-Encoding",
    "IBM850",
    "IBM862",
    "IBM-Thai",
    "Shift_JIS",
    "GB2312",
    "Big5",
    "macintosh",
    "IBM037",
    "IBM273",
    "IBM277",
    "IBM278",
    "IBM280",
    "IBM284",
    "IBM285",
    "IBM290",
    "IBM297",
    "IBM420",
    "IBM424",
    "IBM437",
    "IBM500",
    "cp851",
    "IBM852",
    "IBM855",
    "IBM857",
    "IBM860",
    "IBM861",
    "IBM863",
    "IBM864",
    "IBM865",
    "IBM868",
    "IBM869",
    "IBM870",
    "IBM871",
    "IBM918",
    "IBM1026",
    "KOI8-R",
    "HZ-GB-2312",
    "IBM866",
    "IBM775",
    "KOI8-U",
    "IBM00858",
    "IBM01140",
    "IBM01141",
    "IBM01142",
    "IBM01143",
    "IBM01144",
    "IBM01145",
    "IBM01146",
    "IBM01147",
    "IBM01148",
    "IBM01149",
    "Big5-HKSCS",
    "IBM1047",
    "windows-1250",
    "windows-1251",
    "windows-1252",
    "windows-1253",
    "windows-1254",
    "windows-1255",
    "windows-1256",
    "windows-1257",
    "windows-1258",
    "TIS-620",
};

// {name, MIB number}
QMap<QString, int> FileEncoding::encodingNameToMib_ = {
    {"System", 0},
    {"UTF-8", 106},  // Move
    {"UTF-8 BOM", 106},
    {"UTF-16BE", 1013},  // Move
    {"UTF-16LE", 1014},  // Move
    {"UTF-16", 1015},    // Move
    {"UTF-32", 1017},    // Move
    {"UTF-32BE", 1018},  // Move
    {"UTF-32LE", 1019},  // Move
    {"US-ASCII", 3},
    {"ISO-8859-1", 4},
    {"ISO-8859-2", 5},
    {"ISO-8859-3", 6},
    {"ISO-8859-4", 7},
    {"ISO-8859-5", 8},
    {"ISO-8859-6", 9},
    {"ISO-8859-7", 10},
    {"ISO-8859-8", 11},
    {"ISO-8859-9", 12},
    {"ISO-8859-10", 13},
    {"ISO-2022-JP-1", 16},
    {"Shift_JIS", 17},
    {"EUC-JP", 18},
    // {"US-ASCII",	30},
    {"EUC-KR", 36},
    {"ISO-2022-KR", 37},
    {"EUC-KR", 38},
    {"ISO-2022-JP", 39},
    {"ISO-2022-JP-2", 40},
    {"GB_2312-80", 57},
    {"ISO-8859-6", 81},
    {"ISO-8859-6", 82},
    {"ISO-8859-8", 84},
    {"ISO-8859-8", 85},
    {"ISO-2022-CN", 104},
    {"ISO-2022-CN-EXT", 105},
    // {"UTF-8", 106},
    {"ISO-8859-13", 109},
    {"ISO-8859-14", 110},
    {"ISO-8859-15", 111},
    {"GBK", 113},
    {"GB18030", 114},
    {"UCS-2", 1000},
    {"UCS-4", 1001},
    {"SCSU", 1011},
    {"UTF-7", 1012},
    // {"UTF-16BE", 1013},
    // {"UTF-16LE", 1014},
    // {"UTF-16", 1015},
    {"CESU-8", 1016},
    // {"UTF-32", 1017},
    // {"UTF-32BE", 1018},
    // {"UTF-32LE", 1019},
    {"BOCU-1", 1020},
    {"hp-roman8", 2004},
    {"Adobe-Standard-Encoding", 2005},
    {"IBM850", 2009},
    {"IBM862", 2013},
    {"IBM-Thai", 2016},
    {"Shift_JIS", 2024},
    {"GB2312", 2025},
    {"Big5", 2026},
    {"macintosh", 2027},
    {"IBM037", 2028},
    {"IBM273", 2030},
    {"IBM277", 2033},
    {"IBM278", 2034},
    {"IBM280", 2035},
    {"IBM284", 2037},
    {"IBM285", 2038},
    {"IBM290", 2039},
    {"IBM297", 2040},
    {"IBM420", 2041},
    {"IBM424", 2043},
    {"IBM437", 2011},
    {"IBM500", 2044},
    {"cp851", 2045},
    {"IBM852", 2010},
    {"IBM855", 2046},
    {"IBM857", 2047},
    {"IBM860", 2048},
    {"IBM861", 2049},
    {"IBM863", 2050},
    {"IBM864", 2051},
    {"IBM865", 2052},
    {"IBM868", 2053},
    {"IBM869", 2054},
    {"IBM870", 2055},
    {"IBM871", 2056},
    {"IBM918", 2062},
    {"IBM1026", 2063},
    {"KOI8-R", 2084},
    {"HZ-GB-2312", 2085},
    {"IBM866", 2086},
    {"IBM775", 2087},
    {"KOI8-U", 2088},
    {"IBM00858", 2089},
    {"IBM01140", 2091},
    {"IBM01141", 2092},
    {"IBM01142", 2093},
    {"IBM01143", 2094},
    {"IBM01144", 2095},
    {"IBM01145", 2096},
    {"IBM01146", 2097},
    {"IBM01147", 2098},
    {"IBM01148", 2099},
    {"IBM01149", 2100},
    {"Big5-HKSCS", 2101},
    {"IBM1047", 2102},
    {"windows-1250", 2250},
    {"windows-1251", 2251},
    {"windows-1252", 2252},
    {"windows-1253", 2253},
    {"windows-1254", 2254},
    {"windows-1255", 2255},
    {"windows-1256", 2256},
    {"windows-1257", 2257},
    {"windows-1258", 2258},
    {"TIS-620", 2259},
};

QStringList FileEncoding::encodingNames() { return encodingNameList_; }

QString FileEncoding::ProcessAnsi(QFile &file) {
    // If no BOM, we try to decode by UTF8 firstly,
    // then decode by other codeces if failed.
    const QByteArray &data = file.readAll();
    QTextCodec::ConverterState state;
    // FileEncoding.codec is UTF8 in default.
    QTextCodec *utf8Codec = QTextCodec::codecForMib(106);
    QString ansiText = utf8Codec->toUnicode(data.constData(), data.size(), &state);
    qDebug() << "codec: " << utf8Codec->name() << ", invalidChars: " << state.invalidChars
             << ", text: " << ansiText.mid(0, 15) << "..., bytearry: " << data.mid(0, 15).data() << "...";
    if (state.invalidChars == 0) {
        setCodec(utf8Codec);
        return ansiText;
    }

    // Use System.
    auto systemCodec = QTextCodec::codecForLocale();                // QTextCodec::codecForName("System"); //
                                                                    // QTextCodec::codecForMib(0); // Use system codec.
    if (systemCodec != nullptr && systemCodec->mibEnum() != 106) {  // Not UTF-8.
        qDebug() << "systemCodec: " << systemCodec << ", " << systemCodec->name() << ", " << systemCodec->mibEnum();
        setCodec(systemCodec);
        const auto &ansiText = systemCodec->toUnicode(data.constData());
        return ansiText;
    }

    // Use GBK.
    auto gbkCodec = QTextCodec::codecForMib(113);  // GBK
    if (gbkCodec != nullptr) {
        qDebug() << "gbkCodec: " << gbkCodec << ", " << gbkCodec->name() << ", " << gbkCodec->mibEnum();
        setCodec(gbkCodec);
        const auto &ansiText = gbkCodec->toUnicode(data.constData());
        return ansiText;
    }

    // If no BOM, and not UTF-8..., we try to find proper codec.
    QTextCodec *bestCodec;
    int minInvalidCharsNum = data.size();
    for (const auto &name : encodingNames()) {
        auto codec = QTextCodec::codecForName(name.toLocal8Bit().data());
        if (codec == nullptr) {
            continue;
        }
        QTextCodec::ConverterState check_state;
        ansiText = codec->toUnicode(data.constData(), data.size(), &check_state);
        qDebug() << "codec: " << codec->name() << ", invalidChars: " << check_state.invalidChars
                 << ", minInvalidCharsNum: " << minInvalidCharsNum << ", text: " << ansiText.mid(0, 15)
                 << "..., bytearry: " << data.mid(0, 15).data() << "...";
        if (check_state.invalidChars == 0) {
            bestCodec = codec;
            break;
        }

        if (check_state.invalidChars < minInvalidCharsNum) {
            bestCodec = codec;
            minInvalidCharsNum = check_state.invalidChars;
        }
    }
    setCodec(bestCodec);
    return ansiText;
}

int FileEncoding::GetMibByName(const QString &name) {
    auto iter = encodingNameToMib_.find(name);
    if (iter != encodingNameToMib_.end()) {
        return iter.value();
    }
    return -1;
}
}  // namespace QEditor
