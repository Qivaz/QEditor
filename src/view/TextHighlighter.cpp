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
TextHighlighter::TextHighlighter(const FileType &fileType, QTextDocument *parent, const QString &focused_str,
                                 const QVector<QString> markUpText)
    : QSyntaxHighlighter(parent) {
    if (fileType.IsCpp()) {
        SetupCLang();
    } else if (fileType.IsPython()) {
        SetupPythonLang();
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

void TextHighlighter::SetupSelectedText(const QString &text) {
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
        const auto &text = markTexts[i];
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
        const auto &text = markTexts[i];
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
    // keywordFormat.setFontWeight(QFont::Bold);
    // keywordFormat.setFontItalic(true);
    const QString keywordPatterns[] = {
        Keyword(subgraph),
        Keyword(funcgraph),
        Keyword(attr),
        Keyword(instance),
        Keyword(entry),
        Keyword(IR),
        Keyword(ValueNode),
        Keyword(Parameter),
        Keyword(CNode),
        Keyword(FuncGraph),
        Keyword(Return),
        Keyword(core),
        Keyword(undeterminate),
        Keyword(const),
        Keyword(value),
        Keyword(scope),
        Keyword(call),
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules_.append(rule);
    }

    // Highlight types.
    auto typeColor = QColor(16, 126, 172);
    QTextCharFormat typeFormat;
    typeFormat.setForeground(typeColor);
    const static QString typePatterns[] = {
        Keyword(Tensor),
        Keyword(Primitive),
        Keyword(Tuple),
        Keyword(List),
        Keyword(Dict),
        Keyword(PrimitivePy),
        Keyword(Func),
        Keyword(Float32),
        Keyword(Float16),
        Keyword(Int32),
        Keyword(Int16),
        Keyword(Scalar),
        Keyword(sequence_nodes),
        Keyword(elements_use_flags),
        Keyword(node),
        Keyword(const),
        Keyword(vector),
        Keyword(bool),
        Keyword(value),
        Keyword(scope),
        Keyword(ptr),
        Keyword(Object),
        Keyword(call),
        Keyword(DeadNode),
        Keyword(PolyNode),
        Keyword(false),
        Keyword(true),
        Keyword(unknown),
    };
    for (const QString &pattern : typePatterns) {
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
    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QStringLiteral("#[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules_.append(rule);

    // Highlight the primitive function, check if xxx( exists.
    auto primitiveColor = QColor(67, 201, 176);
    QTextCharFormat primitiveFormat;
    // primitiveFormat.setFontItalic(true);
    primitiveFormat.setForeground(primitiveColor);
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = primitiveFormat;
    highlightingRules_.append(rule);

    // Highlight the mindir function, check if @xx.x( exists.
    auto functionColor = QColor(155, 79, 21);
    QTextCharFormat functionFormat;
    // functionFormat.setFontItalic(true);
    functionFormat.setForeground(functionColor);
    rule.pattern =
        QRegularExpression(QStringLiteral("@[A-Za-z0-9_Φ✓✗↓↰↱@↵↻⇊▲▼▶◀∇]+[\\.]*[A-Za-z0-9_]+\\b(?=(\\(|\\)|:|$)?)"));
    rule.format = functionFormat;
    highlightingRules_.append(rule);
}

void TextHighlighter::SetupPythonLang() {
    HighlightingRule rule;
    // Highlight keywords.
    auto keywordColor = QColor(86, 156, 202);
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(keywordColor);
    // keywordFormat.setFontWeight(QFont::Bold);
    const static QString keywordPatterns[] = {
        // Python CLI: help("keywords")
        Keyword(and),
        Keyword(elif),
        Keyword(if),
        Keyword(print),
        Keyword(as),
        Keyword(else),
        Keyword(import),
        Keyword(raise),
        Keyword(assert),
        Keyword(except),
        Keyword(in),
        Keyword(return),
        Keyword(break),
        Keyword(exec),
        Keyword(is),
        Keyword(try),
        Keyword(class),
        Keyword(finally),
        Keyword(lambda),
        Keyword(while),
        Keyword(continue),
        Keyword(for),
        Keyword(not),
        Keyword(with),
        Keyword(def),
        Keyword(from),
        Keyword(or),
        Keyword(yield),
        Keyword(del),
        Keyword(global),
        Keyword(pass),
        // More keywords
        Keyword(self),
        Keyword(super),
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules_.append(rule);
    }

    // Highlight the string in ""
    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
    rule.format = quotationFormat;
    highlightingRules_.append(rule);

    // Highlight the string in ''
    QTextCharFormat quotation2Format;
    quotation2Format.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QStringLiteral("\'.*\'"));
    rule.format = quotation2Format;
    highlightingRules_.append(rule);

    // Highlight the Python function, check if xxx( exists.
    auto functionColor = QColor(67, 201, 176);
    QTextCharFormat functionFormat;
    // functionFormat.setFontItalic(true);
    functionFormat.setForeground(functionColor);
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = functionFormat;
    highlightingRules_.append(rule);

    // Highlight # comment.
    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QStringLiteral("#[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules_.append(rule);

    // Highlight /*...*/ comment
    commentMultiLineFormat_.setForeground(Qt::darkGreen);
    // Multiple lines start: /*
    commentStartExpressions_.emplaceBack(QRegularExpression(QStringLiteral("'''")));
    commentStartExpressions_.emplaceBack(QRegularExpression(QStringLiteral("\"\"\"")));
    // Multiple lines end: /*
    commentEndExpressions_.emplaceBack(QRegularExpression(QStringLiteral("'''")));
    commentEndExpressions_.emplaceBack(QRegularExpression(QStringLiteral("\"\"\"")));
}

void TextHighlighter::SetupCLang() {
    HighlightingRule rule;
    // Highlight keywords.
    auto keywordColor = QColor(86, 156, 202);
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(keywordColor);
    keywordFormat.setFontWeight(QFont::Bold);
    const static QString keywordPatterns[] = {
        Keyword(signed),
        Keyword(unsigned),
        Keyword(void),
        Keyword(bool),
        Keyword(char),
        Keyword(int),
        Keyword(short),
        Keyword(long),
        Keyword(double),
        Keyword(class),
        Keyword(struct),
        Keyword(template),
        Keyword(typedef),
        Keyword(typename),
        Keyword(union),
        Keyword(const),
        Keyword(constexpr),
        Keyword(enum),
        Keyword(explicit),
        Keyword(friend),
        Keyword(inline),
        Keyword(namespace),
        Keyword(operator),
        Keyword(private),
        Keyword(protected),
        Keyword(public),
        Keyword(signals),
        Keyword(slots),
        Keyword(static),
        Keyword(virtual),
        Keyword(volatile),
        // More keywords.
        Keyword(return),
        Keyword(for),
        Keyword(while),
        Keyword(break),
        Keyword(continue),
        Keyword(if),
        Keyword(else),
        Keyword(ifndef),
        Keyword(ifdef),
        Keyword(elif),
        Keyword(endif),
        Keyword(define),
        Keyword(undef),
        Keyword(include),
        Keyword(error),
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules_.append(rule);
    }

    // // Highlight Qt's class.
    // QTextCharFormat classFormat;
    // classFormat.setFontWeight(QFont::Bold);
    // classFormat.setForeground(Qt::darkMagenta);
    // rule.pattern = QRegularExpression(QStringLiteral("\\bQ[A-Za-z]+\\b"));
    // rule.format = classFormat;
    // highlightingRules_.append(rule);

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
    commentMultiLineFormat_.setForeground(commentColor);
    // Multiple lines start: /*
    commentStartExpressions_.emplaceBack(QRegularExpression(QStringLiteral("/\\*")));
    // Multiple lines end: /*
    commentEndExpressions_.emplaceBack(QRegularExpression(QStringLiteral("\\*/")));
}

void TextHighlighter::highlightBlock(const QString &text) {
    // Handle multiple lines regular firstly.
    if (commentStartExpressions_.size() != commentEndExpressions_.size()) {
        qFatal("Comments start and end expressions list must be equal");
    }
    for (qsizetype i = 0; i < commentStartExpressions_.size(); ++i) {
        const auto &commentStartExpression = commentStartExpressions_[i];
        const auto &commentEndExpression = commentEndExpressions_[i];
        if (commentStartExpression.pattern().isEmpty() || commentEndExpression.pattern().isEmpty()) {
            continue;
        }
        setCurrentBlockState(-1);
        int startIndex = 0;
        int matchedStartLen;
        int currentExprIndex;
        if (previousBlockState() == -1) {
            startIndex = text.indexOf(commentStartExpression);
            matchedStartLen = commentStartExpression.pattern().length();
            currentExprIndex = i;
        } else {
            matchedStartLen = 0;
            currentExprIndex = previousBlockState();
        }
        bool startMultipleLine = false;
        while (startIndex >= 0) {
            const auto &currentEndExpr = commentEndExpressions_[currentExprIndex];
            QRegularExpressionMatch match = currentEndExpr.match(text, startIndex + matchedStartLen);
            int endIndex = match.capturedStart();
            int commentLength = 0;
            if (endIndex == -1) {
                setCurrentBlockState(currentExprIndex);
                commentLength = text.length() - startIndex;
                setFormat(startIndex, commentLength, commentMultiLineFormat_);
                startMultipleLine = true;
                break;
            } else {
                commentLength = endIndex - startIndex + match.capturedLength();
                setFormat(startIndex, commentLength, commentMultiLineFormat_);
                startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
                matchedStartLen = commentStartExpression.pattern().length();
            }
        }
        if (startMultipleLine) {
            break;
        }
    }

    for (const HighlightingRule &rule : qAsConst(highlightingRules_)) {
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
}  // namespace QEditor
