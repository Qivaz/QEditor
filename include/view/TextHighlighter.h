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

#ifndef TEXTHIGHLIGHTER_H
#define TEXTHIGHLIGHTER_H

#include "FileType.h"
#include <QRegularExpression>
#include <QSyntaxHighlighter>

namespace QEditor {
class TextHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    TextHighlighter(const FileType& fileType, QTextDocument* parent = 0, const QString& focused_str = "",
                    const QVector<QString> markUpText = QVector<QString>());

protected:
    void highlightBlock(const QString& text) override;

private:
    void SetupSelectedText(const QString& text);
    void SetupMakeText(const QVector<QString> markUpText);
    void SetupCLang();
    void SetupMindIRLang();

    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules_;

    QRegularExpression clangCommentStartExpression_;
    QRegularExpression clangcommentEndExpression_;
    QTextCharFormat clangMultiLineCommentFormat_;

    const QVector<QColor> presetMarkColors_ = {
        QColor(250, 128, 114), QColor(255, 215, 0), QColor(192, 255, 62), QColor(127, 255, 212), QColor(255, 99, 71),
        QColor(200, 0, 100),   QColor(0, 255, 127), QColor(255, 0, 255),  QColor(0, 255, 255),   QColor(132, 112, 255)};
};
} // namespace QEditor

#endif // TEXTHIGHLIGHTER_H
