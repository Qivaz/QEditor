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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace QEditor {
namespace Constants {
constexpr auto kAppName = "QEditor";
constexpr auto kVersionStr = "0.0.2 (V22R10 alpha)";
constexpr auto kOrgName = "Q";

constexpr auto kAppInternalRelativePath = ".Q/data/internal/.files/";
constexpr auto kAppInternalAutoSaveDirName = ".auto_save";
constexpr auto kAppInternalFilesInfoFileName = "files_info";
constexpr auto kAppInternalRecentFilesDirName = ".recent_files";
constexpr auto kAppInternalRecentFilesFileName = "recent_files";
constexpr auto kAppInternalSingleRunFile = ".single_lock";
constexpr auto kConfigFile = ".config.ini";

constexpr auto kAppInternalLogsRelativePath = "logs/";

constexpr auto kSingleAppHostName = "QEditor::SingleApp";

constexpr auto kMaxExtraSelectionsMarkCount = 2000;

constexpr auto kMaxCharsNumToPairBracket = 20000;
constexpr auto kMaxRecursiveDepthToPairBracket = 10;

constexpr auto kMaxOpenRecentFilesNum = 100;

// File format: new line char.
constexpr auto kFormatNewLineUnix = "Unix";
constexpr auto kFormatNewLineDos = "Dos";
constexpr auto kFormatNewLineMac = "Mac";

// File encoding.
constexpr auto kEncodingSystem  = "ANSI";
constexpr auto kEncodingUtf8    = "UTF-8";
constexpr auto kEncodingUtf8Bom = "UTF-8 BOM";
constexpr auto kEncodingUtf16   = "UTF-16";
constexpr auto kEncodingUtf16Be = "UTF-16 BE";
constexpr auto kEncodingUtf16Le = "UTF-16 LE";
constexpr auto kEncodingUtf32   = "UTF-32";
constexpr auto kEncodingUtf32Be = "UTF-32 BE";
constexpr auto kEncodingUtf32Le = "UTF-32 LE";

extern QString kAppPath;           // Should initialize once when startup.
extern QString kAppInternalPath;   // Should initialize once when startup.
}  // namespace
}  // namespace QEditor

#endif // CONSTANTS_H
