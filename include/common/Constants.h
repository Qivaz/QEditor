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
constexpr auto kVersionStr = "0.0.4 (V22R10 alpha)";
constexpr auto kOrgName = "Q";

constexpr auto kAppInternalRelativePath = ".Q/data/internal/.files/";
constexpr auto kAppInternalAutoSaveDirName = ".auto_save";
constexpr auto kAppInternalFilesInfoFileName = "files_info";
constexpr auto kAppInternalRecentFilesDirName = ".recent_files";
constexpr auto kAppInternalRecentFilesFileName = "recent_files";
constexpr auto kAppInternalSearchTargetsDirName = ".search_targets";
constexpr auto kAppInternalSearchTargetsFileName = "search_targets";
constexpr auto kAppInternalSingleRunFile = ".single_lock";
constexpr auto kConfigFile = ".config.ini";

constexpr auto kAppInternalLogsRelativePath = "logs/";

constexpr auto kSingleAppHostName = "QEditor::SingleApp";

constexpr auto kMaxExtraSelectionsMarkCount = 2000;

constexpr auto kMaxCharsNumToPairBracket = 20000;
constexpr auto kMaxRecursiveDepthToPairBracket = 10;

constexpr auto kMaxOpenRecentFilesNum = 100;
constexpr auto kMaxSearchTargetsNum = 20;

// File format: new line char.
constexpr auto kFormatNewLineUnix = "Unix";
constexpr auto kFormatNewLineDos = "Dos";
constexpr auto kFormatNewLineMac = "Mac";

extern QString kAppPath;          // Should initialize once when startup.
extern QString kAppInternalPath;  // Should initialize once when startup.

extern qreal kMonoSingleSpace;

constexpr auto kMaxParseFileSize = 15000000;  // ~15M
constexpr auto kMaxParseLineNum = 50000;
constexpr auto kMaxParseCharNum = 9000000;

constexpr auto kMaxHighlightScrollbarFileSize = 5000000;  // ~15M
constexpr auto kMaxHighlightScrollbarLineNum = 10000;
constexpr auto kMaxHighlightScrollbarCharNum = 1000000;
}  // namespace Constants
}  // namespace QEditor

#endif  // CONSTANTS_H
