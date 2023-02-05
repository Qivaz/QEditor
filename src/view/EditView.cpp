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

#include "EditView.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QSaveFile>
#include <QScrollBar>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextLayout>

#include "Constants.h"
#include "FunctionHierarchy.h"
#include "IrParser.h"
#include "MainWindow.h"
#include "MainTabView.h"
#include "OutlineList.h"
#include "Toast.h"
#include "Logger.h"
#include "diff_match_patch.h"

QVector<bool> NewFileNum::numbers_use_status_;

// Open file edit.
EditView::EditView(const QFileInfo &fileInfo, QWidget *parent)
    : tabView_((TabView*)parent),
      fileName_(fileInfo.fileName()),
      filePath_(fileInfo.canonicalFilePath()),
      fileType_(filePath_)
{
    if (!fileType_.IsUnknown()) {
        highlighter_ = new TextHighlighter(fileType_, document(), "");
    }

    Init();
}

// New file edit.
EditView::EditView(const QString &fileName, QWidget *parent)
    : tabView_((TabView*)parent),
      fileName_(fileName),
      filePath_("")
{
    Init();
}

EditView::EditView(QWidget *parent)
{
    EditView("", parent);
}

void EditView::Init()
{
    setBackgroundVisible(false);
    setCenterOnScroll(true);
    setStyleSheet("color: darkGray;"
                  "background-color: rgb(28, 28, 28);"
                  "selection-color: lightGray;"
                  "selection-background-color: rgb(76, 76, 167);"
                  "border: none;"
                  /*"font-family: '文泉驿等宽正黑';"*/);

    verticalScrollBar()->setStyleSheet("QScrollBar {border: none;}"
                                       "QScrollBar::add-line:vertical { \
                                          border: none; \
                                          background: none; \
                                        } \
                                        QScrollBar::sub-line:vertical { \
                                          border: none; \
                                          background: none; \
                                        }");
    horizontalScrollBar()->setStyleSheet("QScrollBar {border: none;}"
                                         "QScrollBar::add-line:horizontal { \
                                            border: none; \
                                            background: none; \
                                          } \
                                          QScrollBar::sub-line:horizontal { \
                                            border: none; \
                                            background: none; \
                                          }");

    setFont(QFont("Consolas", 11));
    currentFontSize_ = font().pointSize();

    lineNumberArea_ = new LineNumberArea(this);

    connect(this, &EditView::blockCountChanged, this, &EditView::HandleBlockCountChanged);
    connect(this, &EditView::updateRequest, this, &EditView::HandleUpdateRequest);
    connect(this, &EditView::cursorPositionChanged, this, &EditView::HandleCursorPositionChanged);
    connect(this, &EditView::selectionChanged, this, &EditView::HandleSelectionChanged);
    connect(this, &EditView::textChanged, this, &EditView::HandleTextChanged);
    connect(this->document(), &QTextDocument::contentsChanged, this, &EditView::HandleContentsChanged);
    connect(this->document(), &QTextDocument::contentsChange, this, &EditView::HandleContentsChange);

    connect(this, &QPlainTextEdit::copyAvailable, this, &EditView::HandleCopyAvailable);
    connect(this, &QPlainTextEdit::undoAvailable, this, &EditView::HandleUndoAvailable);
    connect(this, &QPlainTextEdit::redoAvailable, this, &EditView::HandleRedoAvailable);

    // TODO: Cause transparent window...
    // ApplyWrapTextState();
    // ApplySpecialCharsVisible();

    HandleBlockCountChanged(0);
    UnderpaintCurrentBlock();

//    UpdateStatusBarWithCursor();

    setFocus();
}

void EditView::ApplyWrapTextState()
{
    QPlainTextEdit::LineWrapMode mode;
    if (MainWindow::Instance().shouldWrapText()) {
        mode = QPlainTextEdit::LineWrapMode::WidgetWidth;
    } else {
        mode = QPlainTextEdit::LineWrapMode::NoWrap;
    }
    setLineWrapMode(mode);
}

void EditView::ApplySpecialCharsVisible()
{
    QTextOption textOption;
    if (MainWindow::Instance().specialCharsVisible()) {
        textOption.setFlags(QTextOption::ShowTabsAndSpaces |
                            QTextOption::ShowLineAndParagraphSeparators |
                            QTextOption::ShowDocumentTerminator);
    } else {
        textOption.setFlags(0);
    }
    document()->setDefaultTextOption(textOption);
}

void EditView::ApplyTabCharNum()
{
    int num = MainWindow::Instance().tabCharNum();
    qreal monoSingleSpace = QFontMetricsF(font()).horizontalAdvance(QLatin1Char('9'));
    setTabStopDistance(monoSingleSpace * num);
}

void EditView::setFileEncoding(FileEncoding &&fileEncoding)
{
    fileEncoding_ = std::move(fileEncoding);

    // Update status bar info. if file encoding changes.
    MainWindow::Instance().UpdateStatusBarRareInfo("Unix", fileEncoding_.description(), 0);
}

void EditView::ChangeFileEncoding(FileEncoding &&fileEncoding)
{
    if (fileEncoding.hasBom() == fileEncoding_.hasBom() &&
        fileEncoding.mibEnum() == fileEncoding_.mibEnum()) {
        return;
    }
#if 0
    qDebug() << fileEncoding.hasBom() << fileEncoding_.hasBom() << fileEncoding_.codec() <<
                  fileEncoding.mibEnum() << fileEncoding_.mibEnum();
    const auto &text = document()->toPlainText();
    const QByteArray &oldData = fileEncoding_.codec()->fromUnicode(text);
    qDebug() << "text: " << text << ", " << text.toUtf8() << ", bytearry: " << oldData.data();

    auto oldFileEncoding = std::move(fileEncoding_);
    fileEncoding_ = std::move(fileEncoding);

    QTextCodec::ConverterState state;
    // FileEncoding.codec is UTF8 in default.
    const auto &newText = fileEncoding_.codec()->toUnicode(oldData.constData(), oldData.size(), &state);
    qDebug() << "newText: " << newText << ", " << newText.toUtf8() << ", new bytearry: " << fileEncoding_.codec()->fromUnicode(newText).data();

    if (state.invalidChars > 0) {
        qDebug() << "state.invalidChars: " << state.invalidChars;
    }
    setPlainText(newText);
    qDebug() << "Change codec from " << oldFileEncoding.codec()->name() << " to " << fileEncoding_.codec()->name();
#else
    if (!tabView()->LoadFile(this, filePath_, std::move(fileEncoding), true)) {
        Toast::Instance().Show(Toast::kError, "Load file failed, can't change encoding.");
        return;
    }
    SetModified(false);
#endif
}

// Jump to the position of text cursor.
void EditView::GotoCursor(const QTextCursor &cursor)
{
    setTextCursor(cursor);
    setFocus();
}

// Jump to the position of block number, and return its line count.
// block number starts from 0.
int EditView::GotoBlock(int blockNumber)
{
    auto block = document()->findBlockByNumber(blockNumber);
    int position = block.position();
    QTextCursor cursor = textCursor();
    cursor.setPosition(position, QTextCursor::MoveAnchor);
    setTextCursor(cursor);
    setFocus();
    return block.lineCount();
}

// Select the lines of current block.
void EditView::SelectCurrentBlock(const int lineCount)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    for (int i = 0; i < lineCount - 1; ++i) {
        cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor);
    }
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    // Including \n
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
}

void EditView::AddMarkText(const QString &str) {
    if (str == '.' || str == '*' || str == '+' || str == ',' ||
            str == '^' || str == '&' || str == '?') {
        return;
    }
    if (str == '\\') {
        markTexts_.push_back("\\\\");
    }
    markTexts_.push_back(str);
}

bool EditView::RemoveMarkText(const QString &str) {
    if (str == '\\') {
        return markTexts_.removeOne("\\\\");
    }
    return markTexts_.removeOne(str);
}

QColor EditView::GetMarkTextBackground(int i)
{
    static QVector<QColor> presetMarkColors = {
        QColor(255, 99, 71),
        QColor(255, 0, 255),
        QColor(200, 0, 100),
        QColor(0, 255, 127),
        QColor(0, 255, 255),
        QColor(250, 128, 114),
        QColor(132, 112, 255),
        QColor(255, 215, 0),
        QColor(192, 255, 62),
        QColor(127, 255, 212)
    };
    // Highlight mark up text with preset colors.
    if (i < presetMarkColors.size()) {
        return presetMarkColors[i];
    }

    // Highlight with more random colors.
    int r = QRandomGenerator::global()->bounded(0, 255);
    int g = QRandomGenerator::global()->bounded(0, 255);
    int b = QRandomGenerator::global()->bounded(0, 255);
    auto newColor = QColor(r, g, b);
    presetMarkColors.push_back(newColor);
    return newColor;
}

void EditView::HighlightMarkTexts() {
    for (int i = 0; i < markTexts_.size(); ++i) {
        const auto &text = markTexts_[i];
        HighlightVisibleChars(text, QColor(Qt::white), GetMarkTextBackground(i));
    }
}

int EditView::GetLineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    constexpr qreal padding = 15;
    qreal monoSingleSpace = QFontMetricsF(font()).horizontalAdvance(QLatin1Char('9'));
    qDebug() << "EditView::lineNumberAreaWidth, singleSpace: " << monoSingleSpace << ", font: " << font();
    const static qreal minSpace = 10 + monoSingleSpace + padding;
    qreal space = monoSingleSpace * digits + padding;
    if (space < minSpace) {
        space = minSpace;
    }
    return space;
}

void EditView::HandleBlockCountChanged(int newBlockCount)
{
    qDebug() << "newBlockCount: " << newBlockCount;
    setViewportMargins(GetLineNumberAreaWidth(), 0, 0, 0);
}

void EditView::HandleUpdateRequest(const QRect &rect, int dy)
{
    qDebug() << "rect: " << rect;
    UpdateLineNumberArea(rect, dy);
}

void EditView::UpdateLineNumberArea(const QRect &rect, int dy)
{
    qDebug() << "EditView::updateLineNumberArea";
    if (dy) {
        lineNumberArea_->scroll(0, dy);
    } else {
        lineNumberArea_->update(0, rect.y(), lineNumberArea_->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        HandleBlockCountChanged(0);
    }
}

void EditView::SetCurrentFile(const QString &filePath)
{
    filePath_ = filePath;
    document()->setModified(false);

    QString shownName = filePath_;
    if (filePath_.isEmpty()) {
        shownName = "untitled";
    }
    setWindowFilePath(shownName);
}

bool EditView::SaveFile(const QString &filePath)
{
    QString errorMessage;

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QSaveFile file(filePath);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << toPlainText();
        if (!file.commit()) {
            errorMessage = tr("Cannot write file %1:\n%2.")
                           .arg(QDir::toNativeSeparators(filePath), file.errorString());
        }
    } else {
        errorMessage = tr("Cannot open file %1 for writing:\n%2.")
                       .arg(QDir::toNativeSeparators(filePath), file.errorString());
    }
    QGuiApplication::restoreOverrideCursor();

    if (!errorMessage.isEmpty()) {
        QMessageBox::warning(this, tr(Constants::kAppName), errorMessage);
        return false;
    }

    SetModified(false);
    setFilePath(filePath);
    QFileInfo fileInfo = QFileInfo(filePath);
    auto fileName = fileInfo.fileName();
    setFileName(fileName);
    tabView()->ChangeTabDescription(fileInfo, tabView()->indexOf(this));
    tabView()->UpdateWindowTitle();

    MainWindow::Instance().statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

bool EditView::Save()
{
    if (filePath_.isEmpty()) {  // New file.
        if (document()->isEmpty()) {
            return true;
        }
        return SaveAs();
    } else {  // Open file.
        return MaybeSave();
    }
}

bool EditView::SaveAs()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }
    return SaveFile(dialog.selectedFiles().first());
}

// Return true if save or discard, otherwise false.
bool EditView::MaybeSave()
{
    if (!ShouldSave()) {  // Not use document()->isModified() any more.
        return true;
    }
    const QMessageBox::StandardButton res
        = QMessageBox::warning(this, tr(Constants::kAppName),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (res) {
    case QMessageBox::Save:
        return SaveFile(filePath_);
    case QMessageBox::Discard:
        return true;
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void EditView::SetModifiedIcon(bool modified)
{
    if (modified) {
        qDebug() << "Change tab " << tabView()->indexOf(this) << " as modified icon.";
        tabView()->setTabIcon(tabView()->indexOf(this),
                              QIcon::fromTheme("modification-yes", QIcon(":/images/asterisk_blue.svg")));
    } else {
        qDebug() << "Change tab " << tabView()->indexOf(this) << " as no-modified icon.";
        tabView()->setTabIcon(tabView()->indexOf(this),
                              QIcon::fromTheme("modification-no", QIcon(":/images/pencil.svg")));
    }
}

void EditView::SetModified(bool modified) {
    SetShouldSave(modified);
    SetModifiedIcon(modified);
    setWindowModified(modified);
    tabView()->UpdateWindowTitle();
}

void EditView::HandleSelectionChanged()
{
    qDebug();
    auto text = textCursor().selectedText();
    if (text != selectedText_) {
        selectedText_ = std::move(text);
        highlighterInvalid_ = true;
    }
    qDebug() << ", selectedText_: " << selectedText_
               << ", highlighterInvalid_: " << highlighterInvalid_;
}

void EditView::HighlightFocusChars()
{
    HighlightVisibleChars(selectedText_);
}

// Hightlight the 'text' only in the visible region.
void EditView::HighlightVisibleChars(const QString &text, const QColor &foreground, const QColor &background)
{
    if (text.isEmpty()) {
        return;
    }

    QPoint bottomRight(viewport()->width() -1, viewport()->height() - 1);
    QTextCursor visibleBottomCursor = cursorForPosition(bottomRight);
    int visibleBottomPos = visibleBottomCursor.position();

    auto block = firstVisibleBlock();
#define BLOCK_POS_SEARCH
#ifdef BLOCK_POS_SEARCH
    if (!block.isValid() || !block.isVisible()) {
        return;
    }
    int start = block.position();
    // Suppose all lines have the same height.
    auto lineHeight = block.layout()->lineAt(0).height();
    auto offset = contentOffset();
    if (offset.ry() < 0) {
        auto offsetLine = (-offset.ry()) / lineHeight;
        // TODO: Why lineStart is small after Ctrl+End.
        auto lineStart = block.layout()->lineAt(offsetLine).textStart();
        start = std::max(start, lineStart);
    }
    QTextCursor startCursor = textCursor();
    startCursor.setPosition(start);
    while (startCursor.position() < visibleBottomPos) {
        qDebug() << "start: " << startCursor.position() << ", visibleBottomPos: " << visibleBottomPos
                 << ", block pos: " << block.position();
#ifdef DEBUG_BLOCK_LINE
        qDebug() << "Line " << block.blockNumber() << ": block: " << block.text() << ", " << block.layout()->lineCount();
        for (int i = 0; i < block.layout()->lineCount(); ++i) {
            qDebug() << "lineAt[" << i << "]: start: " << block.layout()->lineAt(i).textStart() << ", len: " << block.layout()->lineAt(i).textLength();
            qDebug() << "lineAt[" << i << "]: position: " << block.layout()->lineAt(i).position() << ", rect: " << block.layout()->lineAt(i).rect();
        }
#endif  // DEBUG_BLOCK_LINE

        startCursor = document()->find(text, startCursor);
        if (startCursor.isNull()) {
            break;
        }
        auto posInText = startCursor.selectionStart();
        qDebug() << "visibleBottomPos: " << visibleBottomPos << ", posInText: " << posInText;

        // Break searching if the left lines of the block is out of visible.
        if (posInText >= visibleBottomPos) {
            qDebug() << "Touch the bottom, " << posInText << " >= " << visibleBottomPos;
            return;
        }
        if (!HighlightChars(posInText, text.size(), foreground, background)) {
            Toast::Instance().Show(Toast::kError,
                                   QString("Hightlight selected text failed. The texts count to mark exceed %1").arg(
                                       Constants::kMaxExtraSelectionsMarkCount));
            return;
        }
    }
#else
    int start = 0;
    while (block.isValid() && block.isVisible() && block.position() < visibleBottomPos) {
        qDebug() << "visibleBottomPos: " << visibleBottomPos << ", pos: " << block.position();
#ifdef DEBUG_BLOCK_LINE
        qDebug() << "Line " << block.blockNumber() << ": block: " << block.text() << ", " << block.layout()->lineCount();
        for (int i = 0; i < block.layout()->lineCount(); ++i) {
            qDebug() << "lineAt[" << i << "]: start: " << block.layout()->lineAt(i).textStart() << ", len: " << block.layout()->lineAt(i).textLength();
            qDebug() << "lineAt[" << i << "]: position: " << block.layout()->lineAt(i).position() << ", rect: " << block.layout()->lineAt(i).rect();
        }
#endif  // DEBUG_BLOCK_LINE
        // Check the line, not the block.
        start = block.text().indexOf(text, 0);
        while (start != -1) {
            auto posInText = block.position() + start;
            qDebug() << "visibleBottomPos: " << visibleBottomPos << ", posInText: " << posInText;
            // Break searching if the left lines of the block is out of visible.
            if (posInText >= visibleBottomPos) {
                qDebug() << "Touch the bottom, " << posInText << " >= " << visibleBottomPos;
                return;
            }
            if (!HighlightChars(posInText, text.size(), foreground, background)) {
                Toast::Instance().Show(Toast::ERROR,
                                       QString("Hightlight selected text failed. The texts count to mark exceed %1").arg(
                                           Constants::kMaxExtraSelectionsMarkCount));
                return;
            }
            start += text.size();
            start = block.text().indexOf(text, start);
        }
        // Continue the next block.
        block = block.next();
    }
#endif
}

void EditView::UpdateStatusBarWithCursor()
{
    // Update status bar with cursor.
    const QTextCursor &cursor = textCursor();
    QString posOrSel;
    if (cursor.hasSelection()) {
        posOrSel = "Sel: " + QString::number(cursor.selectedText().length());
    } else {
        posOrSel = "Pos: " + QString::number(cursor.position() + 1);
    }
    MainWindow::Instance().UpdateStatusBarFrequentInfo(
                "Ln: " + QString::number(cursor.blockNumber() + 1),
                "Col: " + QString::number(cursor.columnNumber() + 1),
                posOrSel,
                "Lines: " + QString::number(blockCount()),
                "Length: " + QString::number(document()->characterCount()));

    // Update the rarely change information.
    MainWindow::Instance().UpdateStatusBarRareInfo("Unix", fileEncoding_.description(), 0);
}

void EditView::HandleCursorPositionChanged()
{
    qDebug();
    currentBlockNumber_ = textCursor().blockNumber();
    highlighterInvalid_ = true;

    UpdateStatusBarWithCursor();

    // Ignore the event before load finish.
    if (!fileLoaded_) {
        return;
    }

    auto pos = textCursor().position();
    if (MainWindow::Instance().IsOutlineDockViewShowing()) {
        MainWindow::Instance().SetOutlineDockViewPosition(pos);
    }

    tabView()->RecordStep(this, pos);
}

void EditView::HandleTextChanged()
{
    qDebug();
}

void EditView::HandleContentsChanged()
{
    qDebug();;
}

void EditView::HandleContentsChange(int from, int charsRemoved, int charsAdded)
{
    if (charsAdded > 50 || charsRemoved > 50) {
        TrigerParser();
    }
    // Ignore the event before load finish.
    if (!fileLoaded_) {
        return;
    }
    SetModified(true);
    highlighterInvalid_ = true;
}

void EditView::HandleCopyAvailable(bool avail)
{
    qDebug();
    copyAvail_ = avail;
    MainWindow::Instance().SetCopyAvailable(avail);
}

void EditView::HandleUndoAvailable(bool avail)
{
    qDebug();
    undoAvail_ = avail;
    MainWindow::Instance().SetUndoAvailable(avail);
}

void EditView::HandleRedoAvailable(bool avail)
{
    qDebug();
    redoAvail_ = avail;
    MainWindow::Instance().SetRedoAvailable(avail);
}

void EditView::TrigerParser()
{
    // TODO: Add more lang.
    if (!fileType_.IsIr()) {
        if (parser_ == nullptr) {
            parser_ = new DummyParser(this);
            overviewList_ = new OutlineList(parser_);
        }
        MainWindow::Instance().UpdateOutlineDockView(overviewList_);

        MainWindow::Instance().HideHierarchyDockView();
        return;
    }

    // If change.
    if (true) {
        if (parser_ != nullptr) {
            delete parser_;
        }
        parser_ = new IrParser(this, this);

        if (overviewList_ != nullptr) {
            delete overviewList_;
        }
        overviewList_ = new OutlineList(parser_);
        MainWindow::Instance().UpdateOutlineDockView(overviewList_);

        if (hierarchy_ != nullptr) {
            delete hierarchy_;
        }
        hierarchy_ = new FunctionHierarchy(parser_);
        MainWindow::Instance().UpdateHierarchyDockView(hierarchy_);
    } else {
        MainWindow::Instance().UpdateOutlineDockView(overviewList_);
        MainWindow::Instance().UpdateHierarchyDockView(hierarchy_);
    }
}

void EditView::Hover(QTextCursor &cursor)
{
    auto hoverPos = cursor.position();
    if (hoverPos == hoverPos_) {
        return;
    }
    hoverPos_ = hoverPos;

    if (jumpAvailable_) {
        HighlightFocus();  // Clear the chars marked before.
    }
    // If ctrl not pressed, ignore.
    if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
        JumpHint(cursor);
        jumpAvailable_ = true;
    } else {
        jumpAvailable_ = false;
    }
}

QString EditView::GetCursorText()
{
    auto cursor = textCursor();
    return GetCursorText(cursor);
}

QString EditView::GetCursorText(QTextCursor &textCursor)
{
    if (!textCursor.hasSelection()) {
        textCursor.select(QTextCursor::WordUnderCursor);
    }
    auto text = textCursor.selectedText();
    qDebug() << ", selectedText: " << text;
    return text;
}

void EditView::JumpHint(QTextCursor &cursor)
{
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    const auto &text = cursor.selectedText();
    funcGraphInfo_ = parser_->GetFuncGraphInfo(text);
    if (funcGraphInfo_.pos_ != -1) {
        HighlightChars(cursor.selectionStart(), cursor.selectionEnd() - cursor.selectionStart(),
                       QColor(16, 126, 172), QColor(28, 28, 28), true);
    }
}

void EditView::Jump()
{
    if (funcGraphInfo_.pos_ == -1) {
        return;
    }
    auto cursor = textCursor();
    cursor.setPosition(funcGraphInfo_.pos_);
    setTextCursor(cursor);
}

std::pair<QTextCursor, bool> EditView::FindPairingBracketCursor(
        QTextCursor cursor, QTextCursor::MoveOperation direct,
        const QChar &startBracketChar, const QChar &endBracketChar)
{
    // Always start at outside of bracket, so jump one firstly.
    if (direct == QTextCursor::Left) {
        cursor.movePosition(direct, QTextCursor::MoveAnchor);
    }

    int moveCount = 0;
    int recursiveDepth = 0;
    while (cursor.movePosition(direct, QTextCursor::MoveAnchor)) {
        // Exceed the max recursive depth or max charactors.
        if (recursiveDepth > Constants::kMaxRecursiveDepthToPairBracket) {
            qCritical() << "Too much recursive depth, recursiveDepth: " << recursiveDepth;
            return std::make_pair(cursor, false);
        }
        ++moveCount;
        if (moveCount > Constants::kMaxCharsNumToPairBracket) {
            qCritical() << "Too much distance to pairing, moveCount: " << moveCount;
            return std::make_pair(cursor, false);
        }

        auto doc = document();
        auto charactor = doc->characterAt(cursor.position());  // Get the char on the right hand of position.
        qDebug() << "current char: " << charactor;
        if (charactor == startBracketChar) {
            qDebug() << "Match more start bracket char: " << startBracketChar;
            ++recursiveDepth;
        }
        if (charactor == endBracketChar) {
            if (recursiveDepth == 0) {
                qDebug() << "Finally match end bracket char: " << endBracketChar << " for " << startBracketChar;
                if (direct == QTextCursor::Right) {
                    cursor.movePosition(direct, QTextCursor::MoveAnchor);
                }
                return std::make_pair(cursor, true);
            }
            --recursiveDepth;
        }
    }
    return std::make_pair(cursor, false);
}

bool EditView::HighlightChars(int startPos, int count, const QColor &foreground, const QColor &background, bool underline)
{
    QList<QTextEdit::ExtraSelection> markExtraSelections = this->extraSelections();
    qDebug() << ", startPos: " << startPos << ", count: " << count << ", char: " << document()->characterAt(startPos) << ", extras: " << markExtraSelections.size() << ", " << (markExtraSelections.size() > Constants::kMaxExtraSelectionsMarkCount);
    if (markExtraSelections.size() > Constants::kMaxExtraSelectionsMarkCount) {
        return false;
    }

    QTextCursor startCursor = textCursor();
    startCursor.setPosition(startPos, QTextCursor::MoveAnchor);
    for (int i = 0; i < count; ++i) {
        bool res = startCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        if (!res) {
            break;
        }
    }
    qDebug() << "To mark chars: " << startCursor.selectedText() << ", at line: " << startCursor.blockNumber();
    QTextEdit::ExtraSelection selection;
    selection.format.setForeground(foreground);
    selection.format.setBackground(background);  // QColor(255, 54, 100)
    selection.format.setFontUnderline(underline);
    selection.cursor = startCursor;
    markExtraSelections.append(selection);

    setExtraSelections(markExtraSelections);
    return true;
}

void EditView::HighlightBrackets(const QTextCursor &leftCursor, const QTextCursor &rightCursor)
{
    qDebug() << "left char: " << document()->characterAt(leftCursor.position());
    qDebug() << "right char: " << document()->characterAt(rightCursor.position());

    QList<QTextEdit::ExtraSelection> markExtraSelections = this->extraSelections();
    if (markExtraSelections.size() > Constants::kMaxExtraSelectionsMarkCount) {
        Toast::Instance().Show(Toast::kError,
                               QString("Highlight brackets failed. The texts count to mark exceed %1").arg(
                                   Constants::kMaxExtraSelectionsMarkCount));
        return;
    }

    // Mark all chars excluding brackets.
    auto startCursor = leftCursor;
    while(true) {
        bool res = startCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        if (!res) {
            break;
        }
        qDebug() << "next char: " << document()->characterAt(startCursor.position());
        if (startCursor == rightCursor) {
            break;
        }
    }
    QTextEdit::ExtraSelection selection;
    QColor markColor = QColor(60, 60, 90);
    selection.format.setBackground(markColor);
    // TODO: Why setFontItalic() not work, but setFontUnderline works ok?
    // selection.format.setFontItalic(true);
    // selection.format.setFontUnderline(true);
    selection.cursor = startCursor;
    markExtraSelections.append(selection);

    qDebug() << "markExtraSelections.size: " << markExtraSelections.size();

    // Mark brackets separately.
    startCursor = leftCursor;
    bool res = startCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    if (res) {
        QTextEdit::ExtraSelection selection;
        selection.format.setForeground(QColor(255, 99, 71));
        // TODO: Why not work?
        // selection.format.setFontItalic(true);
        selection.cursor = startCursor;
        markExtraSelections.append(selection);
    }
    auto endCursor = rightCursor;
    res = endCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    if (res) {
        QTextEdit::ExtraSelection selection;
        selection.format.setForeground(QColor(255, 99, 71));
        // TODO: Why not work?
        // selection.format.setFontItalic(true);
        selection.cursor = endCursor;
        markExtraSelections.append(selection);
    }
    setExtraSelections(markExtraSelections);
}

void EditView::HighlightFocus()
{
    // Must call MarkFocusNearBracket() after UnderpaintCurrentBlock() and other operations,
    // since the former clears ExtraSelections, and the latters reuses ExtraSelections.
    UnderpaintCurrentBlock();
    HighlightFocusNearBracket();
    HighlightMarkTexts();
    HighlightVisibleChars("�", QColor(Qt::lightGray), QColor(255, 54, 54));  // Mark the unrecognized char.
    HighlightFocusChars();  // Focused highlight is high priority, so put it at last.
}

void EditView::HighlightFocusNearBracket()
{
    // Handle left brackets.
    // Check left hand.
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    auto lhsChar = cursor.selectedText();
    auto iter = leftBrackets_.find(lhsChar);
    if (iter != leftBrackets_.end()) {
        qDebug() << "Match left bracket: " << lhsChar << " on left hand.";
        // Found left brackets on left hand.
        cursor = textCursor();
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
        cursor.clearSelection();
        qDebug() << "!!current char: " << document()->characterAt(cursor.position());
        auto res = FindPairingBracketCursor(cursor, QTextCursor::Right, iter.key()[0], iter.value()[0]);
        auto pairingCursor = res.first;
        auto success = res.second;
        if (success) {
            HighlightBrackets(cursor, pairingCursor);
            return;
        }
    }

    // Check right hand.
    cursor = textCursor();
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    auto rhsChar = cursor.selectedText();
    iter = leftBrackets_.find(rhsChar);
    if (iter != leftBrackets_.end()) {
        qDebug() << "Match left bracket: " << rhsChar << " on right hand.";
        // Found left brackets on right hand.
        cursor = textCursor();
        cursor.clearSelection();
        qDebug() << "!!current char: " << document()->characterAt(cursor.position());
        auto res = FindPairingBracketCursor(cursor, QTextCursor::Right, iter.key()[0], iter.value()[0]);
        auto pairingCursor = res.first;
        auto success = res.second;
        if (success) {
            HighlightBrackets(cursor, pairingCursor);
            return;
        }
    }


    // Handle right brackets.
    // Check left hand.
    cursor = textCursor();
    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    lhsChar = cursor.selectedText();
    iter = rightBrackets_.find(lhsChar);
    if (iter != rightBrackets_.end()) {
        qDebug() << "Match right bracket: " << lhsChar << " on left hand.";
        // Found right brackets on left hand.
        cursor = textCursor();
        cursor.clearSelection();
        auto res = FindPairingBracketCursor(cursor, QTextCursor::Left, iter.key()[0], iter.value()[0]);
        auto pairingCursor = res.first;
        auto success = res.second;
        if (success) {
            HighlightBrackets(pairingCursor, cursor);
            return;
        }
    }

    // Check right hand.
    cursor = textCursor();
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    rhsChar = cursor.selectedText();
    iter = rightBrackets_.find(rhsChar);
    if (iter != rightBrackets_.end()) {
        qDebug() << "Match right bracket: " << rhsChar << " on right hand.";
        // Found right brackets on right hand.
        cursor = textCursor();
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
        cursor.clearSelection();
        auto res = FindPairingBracketCursor(cursor, QTextCursor::Left, iter.key()[0], iter.value()[0]);
        auto pairingCursor = res.first;
        auto success = res.second;
        if (success) {
            HighlightBrackets(pairingCursor, cursor);
            return;
        }
    }

    qDebug() << "Not found pairing brackets";
}

void EditView::UnderpaintCurrentBlock()
{
//    if (isReadOnly()) {
//        return;
//    }

    QPoint bottomRight(viewport()->width() -1, viewport()->height() - 1);
    QTextCursor visibleBottomCursor = cursorForPosition(bottomRight);
    int visibleBottomPos = visibleBottomCursor.position();

    QTextCursor cursor = textCursor();
    auto block = cursor.block();
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    QList<QTextEdit::ExtraSelection> extraSelections;

    int startLine = 0;
    // Suppose all lines have the same height.
    auto lineHeight = block.layout()->lineAt(0).height();
    auto offset = contentOffset();
    if (offset.ry() < 0) {
        auto offsetLine = (-offset.ry()) / lineHeight;
        startLine = offsetLine;
    }
    // Underline all visible lines in the block.
    qDebug() << "startLine: " << startLine;
    for (int i = startLine; i < block.lineCount(); ++i) {
        auto posInText = block.position() + block.layout()->lineAt(i).textStart();
        if (!block.isValid() || !block.isVisible() || posInText >= visibleBottomPos) {
            break;
        }
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(40, 40, 50);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = cursor;
        selection.cursor.clearSelection();
        extraSelections.append(selection);

        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
    }
    setExtraSelections(extraSelections);
}

// Get line number from coordinate y.
int EditView::GetBlockNumber(int y)
{
    auto block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int lineHeight = qRound(blockBoundingRect(block).height());
    int bottom = top + lineHeight;
    while (block.isValid() && block.isVisible() && top <= y) {
        block = block.next();
        top = bottom;
        lineHeight = qRound(blockBoundingRect(block).height());
        bottom = top + lineHeight;
        ++blockNumber;
    }
    auto res = --blockNumber;
    if (res < 0) {
        qCritical() << ", the block number should not be less than 0.";
    }
    qDebug() << ", number: " << res;
    return res;
}

void EditView::ZoomIn()
{
    auto currentFont = font();
    currentFont.setPointSize(font().pointSize() + 1);
    setFont(currentFont);
    currentFontSize_ = currentFont.pointSize();
}

void EditView::ZoomOut()
{
    auto currentFont = font();
    currentFont.setPointSize(font().pointSize() - 1);
    setFont(currentFont);
    currentFontSize_ = currentFont.pointSize();
}

void EditView::HandleLineNumberAreaPaintEvent(QPaintEvent *event)
{
    auto currentFont = font();
    currentFont.setPointSize(currentFontSize_ + 1);
    lineNumberArea_->setFont(currentFont);

    QPainter painter(lineNumberArea_);
    painter.fillRect(event->rect(), QColor(38, 38, 38));

    QTextBlock block = firstVisibleBlock();
    int lineHeight = qRound(blockBoundingRect(block).height());
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + lineHeight;

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            if (currentBlockNumber_ == blockNumber) {
                painter.setPen(QColor(255, 0, 143));
            } else {
                painter.setPen(QColor(43, 145, 175));
            }
            QRectF rect = QRectF(0, top, lineNumberArea_->width(), fontMetrics().height());
//            qDebug() << ", top: " << top << ", bottom: " << bottom << ", lineHeight: " << lineHeight << ", number: " << number;
            painter.drawText(rect, Qt::AlignRight | Qt::AlignVCenter, number);
        }

        block = block.next();
        top = bottom;
        lineHeight = qRound(blockBoundingRect(block).height());
        bottom = top + lineHeight;
        ++blockNumber;
    }
}

void EditView::paintEvent(QPaintEvent *event)
{
//    qDebug() << ", " << event->rect();
    QPlainTextEdit::paintEvent(event);
    if (highlighterInvalid()) {
        HighlightFocus();
        highlighterInvalid_ = false;
    }
}

void EditView::showEvent(QShowEvent *event)
{
    qDebug() << "event: " << event
               << ", " << fileName() << ": " << undoAvail() << ", " << redoAvail();

    MainWindow::Instance().SetUndoAvailable(undoAvail());
    MainWindow::Instance().SetRedoAvailable(redoAvail());

    ApplyWrapTextState();
    ApplySpecialCharsVisible();
    ApplyTabCharNum();

    QPlainTextEdit::showEvent(event);
}

void EditView::resizeEvent(QResizeEvent *event)
{
    qDebug() << "EditView::resizeEvent";
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    lineNumberArea_->setGeometry(QRect(cr.left(), cr.top(), GetLineNumberAreaWidth(), cr.height()));

    qDebug() << "contentOffset: " << contentOffset();
    qDebug() << "firstVisibleBlock.rect: " << blockBoundingRect(firstVisibleBlock());
}

void EditView::wheelEvent(QWheelEvent *event) {
    highlighterInvalid_ = true;

    // If Ctrl-Key pressed.
    if (QApplication::keyboardModifiers() != Qt::ControlModifier) {
        QPlainTextEdit::wheelEvent(event);
        qDebug() << "contentOffset: " << contentOffset();
        qDebug() << "firstVisibleBlock.rect: " << blockBoundingRect(firstVisibleBlock());
        qDebug() << "firstVisibleBlock.firstLineNumber: " << firstVisibleBlock().firstLineNumber();
        return;
    }
    if (event->delta() > 0) {
        ZoomIn();
    } else {
        ZoomOut();


//        QString text1("不是\nabcdef\n123456\nhello, world\nyes.\n如果");
//        QString text2("是的\nabcdefg\n0123456\nbye, world\nyes.\n假如");
//        diff_match_patch dmp;
//        auto diffs = dmp.diff_main(text1, text2, true);
//        dmp.diff_cleanupSemantic(diffs);
//        const auto html = dmp.diff_prettyHtml(diffs);
//        QString res;
//        for (const auto &diff : diffs) {
//            qCritical() << diff.toString();
//            res += diff.toString();
//            res += "\n";
//        }
//        setPlainText(res + "\n\n" + html);
    }
}

void EditView::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "event: " << event;
    if (event->key() == Qt::Key_Control) {
        auto cursor = textCursor();
        cursor.setPosition(hoverPos_);
        JumpHint(cursor);
        jumpAvailable_ = true;
    }
    QPlainTextEdit::keyPressEvent(event);
}

void EditView::keyReleaseEvent(QKeyEvent *event)
{
    qDebug() << "event: " << event;
    if (event->key() == Qt::Key_Control) {
        HighlightFocus();  // Clear the chars marked by keyPressEvent(Key_Control)
        jumpAvailable_ = false;
    }
    QPlainTextEdit::keyReleaseEvent(event);
}

void EditView::mousePressEvent(QMouseEvent *event)
{
    QPlainTextEdit::mousePressEvent(event);
}

void EditView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (jumpAvailable_) {
            Jump();
        }
    }
    QPlainTextEdit::mouseReleaseEvent(event);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    editView_->HandleLineNumberAreaPaintEvent(event);
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(editView_->GetLineNumberAreaWidth(), 0);
}

void LineNumberArea::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        auto blockNumber = editView_->GetBlockNumber(event->pos().ry());
        auto lineCount = editView_->GotoBlock(blockNumber);
        editView_->SelectCurrentBlock(lineCount);
    }
}

void LineNumberArea::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        auto blockNumber = editView_->GetBlockNumber(event->pos().ry());
        auto lineCount = editView_->GotoBlock(blockNumber);
        editView_->SelectCurrentBlock(lineCount);
    }
}
