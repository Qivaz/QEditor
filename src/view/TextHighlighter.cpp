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

#include "TextHighlighter.h"
#include "Logger.h"

namespace QEditor {
TextHighlighter::TextHighlighter(const FileType& fileType, QTextDocument* parent, const QString& focused_str,
                                 const QVector<QString> markUpText)
    : QSyntaxHighlighter(parent) {
    if (fileType.IsCpp()) {
        SetupCLang();
    } else if (fileType.IsPython()) {
        // TODO
    } else if (fileType.IsIr()) {
        SetupMindIRLang();
    }

    // It's too heavy, so we don't use them any more.
    if (!focused_str.isEmpty()) {
        SetupSelectedText(focused_str);
    }
    if (!markUpText.isEmpty()) {
        SetupMakeText(markUpText);
    }
}

void TextHighlighter::SetupSelectedText(const QString& text) {
    // Highlight all texts as focused text.
    HighlightingRule rule;
    rule.pattern = QRegularExpression(/*"\\b" + */ text /* + "\\b"*/);
    QTextCharFormat focus;
    focus.setForeground(Qt::lightGray);
    focus.setBackground(QColor(54, 54, 100));
    rule.format = focus;
    highlightingRules_.append(rule);
}

void TextHighlighter::SetupMakeText(const QVector<QString> markTexts) {
    // Highlight mark up text with preset colors.
    for (int i = 0; i < markTexts.size() && i < presetMarkColors_.size(); ++i) {
        const auto& text = markTexts[i];
        HighlightingRule rule;
        rule.pattern = QRegularExpression(/*"\\b" + */ text /* + "\\b"*/);
        QTextCharFormat markUpFormat;
        markUpFormat.setForeground(Qt::white);
        markUpFormat.setFontWeight(QFont::Bold);
        markUpFormat.setBackground(presetMarkColors_[i]);
        rule.format = markUpFormat;
        highlightingRules_.append(rule);
    }

    // Highlight with more colors.
    int r = 10;
    int g = 20;
    int b = 150;
    for (int i = presetMarkColors_.size(); i < markTexts.size(); ++i) {
        const auto& text = markTexts[i];
        HighlightingRule rule;
        rule.pattern = QRegularExpression("\\b" + text + "\\b");
        QTextCharFormat markUpFormat;
        markUpFormat.setForeground(Qt::white);
        markUpFormat.setFontWeight(QFont::Bold);
        markUpFormat.setBackground(QColor(r, g, b));
        rule.format = markUpFormat;
        highlightingRules_.append(rule);

        r = (r + 50) % 255;
        g = (g + 40) % 255;
        b = (b + 30) % 255;
    }
}

void TextHighlighter::SetupMindIRLang() {
    HighlightingRule rule;
    // Highlight keywords.
    auto keywordColor = QColor(86, 156, 202);
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(keywordColor);
    keywordFormat.setFontWeight(QFont::Bold);
    keywordFormat.setFontItalic(true);
    const QString keywordPatterns[] = {
        QStringLiteral("\\bsubgraph\\b"),      QStringLiteral("\\bfuncgraph\\b"), QStringLiteral("\\battr\\b"),
        QStringLiteral("\\binstance\\b"),      QStringLiteral("\\bentry\\b"),     QStringLiteral("\\bIR\\b"),
        QStringLiteral("\\bValueNode\\b"),     QStringLiteral("\\bParameter\\b"), QStringLiteral("\\bCNode\\b"),
        QStringLiteral("\\bFuncGraph\\b"),     QStringLiteral("\\bReturn\\b"),    QStringLiteral("\\bcore\\b"),
        QStringLiteral("\\bundeterminate\\b"), QStringLiteral("\\bconst\\b"),     QStringLiteral("\\bvalue\\b"),
        QStringLiteral("\\bscope\\b"),         QStringLiteral("\\bcall\\b"),
    };
    for (const QString& pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules_.append(rule);
    }

    // Highlight types.
    auto typeColor = QColor(16, 126, 172);
    QTextCharFormat typeFormat;
    typeFormat.setForeground(typeColor);
    const QString typePatterns[] = {QStringLiteral("\\bTensor\\b"),
                                    QStringLiteral("\\bPrimitive\\b"),
                                    QStringLiteral("\\bTuple\\b"),
                                    QStringLiteral("\\bList\\b"),
                                    QStringLiteral("\\bDict\\b"),
                                    QStringLiteral("\\bPrimitivePy\\b"),
                                    QStringLiteral("\\bFunc\\b"),
                                    QStringLiteral("\\bFloat32\\b"),
                                    QStringLiteral("\\bFloat16\\b"),
                                    QStringLiteral("\\bInt32\\b"),
                                    QStringLiteral("\\bInt16\\b"),
                                    QStringLiteral("\\bScalar\\b"),
                                    QStringLiteral("\\bsequence_nodes\\b"),
                                    QStringLiteral("\\belements_use_flags\\b"),
                                    QStringLiteral("\\bnode\\b"),
                                    QStringLiteral("\\bconst\\b"),
                                    QStringLiteral("\\bvector\\b"),
                                    QStringLiteral("\\bbool\\b"),
                                    QStringLiteral("\\bvalue\\b"),
                                    QStringLiteral("\\bscope\\b"),
                                    QStringLiteral("\\bptr\\b"),
                                    QStringLiteral("\\bObject\\b"),
                                    QStringLiteral("\\bcall\\b"),
                                    QStringLiteral("\\bDeadNode\\b"),
                                    QStringLiteral("\\bPolyNode\\b"),
                                    QStringLiteral("\\bfalse\\b"),
                                    QStringLiteral("\\btrue\\b"),
                                    QStringLiteral("\\bunknown\\b")};
    for (const QString& pattern : typePatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = typeFormat;
        highlightingRules_.append(rule);
    }

    // Highlight the string in ""
    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
    rule.format = quotationFormat;
    highlightingRules_.append(rule);

    // Highlight // comment.
    auto commentColor = QColor(87, 136, 41);
    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setForeground(commentColor);
    rule.pattern = QRegularExpression(QStringLiteral("#[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules_.append(rule);

    // Highlight the primitive function, check if xxx( exists.
    auto primitiveColor = QColor(67, 201, 176);
    QTextCharFormat primitiveFormat;
    //    primitiveFormat.setFontItalic(true);
    primitiveFormat.setForeground(primitiveColor);
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = primitiveFormat;
    highlightingRules_.append(rule);

    // Highlight the mindir function, check if @xx.x( exists.
    auto functionColor = QColor(155, 79, 21);
    QTextCharFormat functionFormat;
    //    functionFormat.setFontItalic(true);
    functionFormat.setForeground(functionColor);
    rule.pattern =
        QRegularExpression(QStringLiteral("@[A-Za-z0-9_Φ✓✗↓↰↱@↵↻⇊▲▼▶◀∇]+[\\.]*[A-Za-z0-9_]+\\b(?=(\\(|\\)|:|$)?)"));
    rule.format = functionFormat;
    highlightingRules_.append(rule);
}

void TextHighlighter::SetupCLang() {
    HighlightingRule rule;
    // Highlight keywords.
    auto keywordColor = QColor(86, 156, 202);
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(keywordColor);
    keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        QStringLiteral("\\bchar\\b"),     QStringLiteral("\\bclass\\b"),     QStringLiteral("\\bconst\\b"),
        QStringLiteral("\\bdouble\\b"),   QStringLiteral("\\benum\\b"),      QStringLiteral("\\bexplicit\\b"),
        QStringLiteral("\\bfriend\\b"),   QStringLiteral("\\binline\\b"),    QStringLiteral("\\bint\\b"),
        QStringLiteral("\\blong\\b"),     QStringLiteral("\\bnamespace\\b"), QStringLiteral("\\boperator\\b"),
        QStringLiteral("\\bprivate\\b"),  QStringLiteral("\\bprotected\\b"), QStringLiteral("\\bpublic\\b"),
        QStringLiteral("\\bshort\\b"),    QStringLiteral("\\bsignals\\b"),   QStringLiteral("\\bsigned\\b"),
        QStringLiteral("\\bslots\\b"),    QStringLiteral("\\bstatic\\b"),    QStringLiteral("\\bstruct\\b"),
        QStringLiteral("\\btemplate\\b"), QStringLiteral("\\btypedef\\b"),   QStringLiteral("\\btypename\\b"),
        QStringLiteral("\\bunion\\b"),    QStringLiteral("\\bunsigned\\b"),  QStringLiteral("\\bvirtual\\b"),
        QStringLiteral("\\bvoid\\b"),     QStringLiteral("\\bvolatile\\b"),  QStringLiteral("\\bbool\\b")};
    for (const QString& pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules_.append(rule);
    }

    //    // Highlight Qt's class.
    //    QTextCharFormat classFormat;
    //    classFormat.setFontWeight(QFont::Bold);
    //    classFormat.setForeground(Qt::darkMagenta);
    //    rule.pattern = QRegularExpression(QStringLiteral("\\bQ[A-Za-z]+\\b"));
    //    rule.format = classFormat;
    //    highlightingRules_.append(rule);

    // Highlight the string in ""
    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
    rule.format = quotationFormat;
    highlightingRules_.append(rule);

    // Highlight the C/C++ function, check if xxx( exists.
    auto functionColor = QColor(67, 201, 176);
    QTextCharFormat functionFormat;
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(functionColor);
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = functionFormat;
    highlightingRules_.append(rule);

    // Highlight // comment.
    auto commentColor = QColor(87, 136, 41);
    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setForeground(commentColor);
    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules_.append(rule);

    // Highlight /*...*/ comment
    clangMultiLineCommentFormat_.setForeground(commentColor);
    // Multiple lines start: /*
    clangCommentStartExpression_ = QRegularExpression(QStringLiteral("/\\*"));
    // Multiple lines end: /*
    clangcommentEndExpression_ = QRegularExpression(QStringLiteral("\\*/"));
}

void TextHighlighter::highlightBlock(const QString& text) {
    // Handle multiple lines regular firstly.
    if (!clangCommentStartExpression_.pattern().isEmpty() && !clangcommentEndExpression_.pattern().isEmpty()) {
        setCurrentBlockState(0);
        int startIndex = 0;
        if (previousBlockState() != 1) {
            startIndex = text.indexOf(clangCommentStartExpression_);
        }
        while (startIndex >= 0) {
            QRegularExpressionMatch match = clangcommentEndExpression_.match(text, startIndex);
            int endIndex = match.capturedStart();
            int commentLength = 0;
            if (endIndex == -1) {
                setCurrentBlockState(1);
                commentLength = text.length() - startIndex;
            } else {
                commentLength = endIndex - startIndex + match.capturedLength();
            }
            setFormat(startIndex, commentLength, clangMultiLineCommentFormat_);
            startIndex = text.indexOf(clangCommentStartExpression_, startIndex + commentLength);
        }
    }

    for (const HighlightingRule& rule : qAsConst(highlightingRules_)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        if (!matchIterator.isValid()) {
            qDebug() << "isValid: " << matchIterator.isValid() << ", text: " << text
                     << ", rule.pattern: " << rule.pattern;
        }
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            qDebug() << "match: " << match << ", text: " << text << ", rule.pattern: " << rule.pattern;
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
} // namespace QEditor
