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

#ifndef EDITVIEW_H
#define EDITVIEW_H

#include "FileEncoding.h"
#include "FileType.h"
#include "IParser.h"
#include "Logger.h"
#include "TextHighlighter.h"
#include <QFileInfo>
#include <QGraphicsView>
#include <QPlainTextEdit>
#include <QRandomGenerator>
#include <QScrollBar>

namespace QEditor {
class TabView;
class IParser;
class OutlineList;
class FunctionHierarchy;
class NewFileNum;

enum ScrollBarHighlightCategory : int { kCategoryFocus, kCategorySearch, kCategoryMark, kCategoryDiff };

class EditView : public QPlainTextEdit {
    Q_OBJECT
   public:
    EditView(QWidget *parent = nullptr);
    EditView(const QString &fileName, QWidget *parent = nullptr);
    EditView(const QFileInfo &fileInfo, QWidget *parent = nullptr);
    ~EditView() = default;

    using ScrollBarInfo = QHash<int, std::vector<std::pair<std::vector<int>, QColor>>>;

    void Init();

    void SetCurrentFile(const QString &filePath);
    bool SaveFile(const QString &filePath);
    bool Save();
    bool SaveAs();
    bool MaybeSave();
    void Close();

    void HandleLineNumberAreaPaintEvent(QPaintEvent *event);
    int GetLineNumberAreaWidth();

    int newFileNum() const { return newFileNum_; }
    void setNewFileNum(int fileNum) { newFileNum_ = fileNum; }
    QString fileName() const { return fileName_; }
    void setFileName(const QString &fileName) { fileName_ = fileName; }
    QString filePath() const { return filePath_; }
    void setFilePath(const QString &filePath);

    void GotoCursor(const QTextCursor &cursor);
    int GotoBlock(int blockNumber);
    void SelectCurrentBlock(int lineCount);
    void SelectBlocks(int startBlockNumber, int endBlockNumber);

    QVector<QString> markTexts() const { return markTexts_; }
    void setMarkTexts(const QVector<QString> strs) { markTexts_ = strs; }
    void AddMarkText(const QString &str);
    bool RemoveMarkText(const QString &str);
    void ClearMarkTexts();
    QColor GetMarkTextBackground(int i);
    void HighlightMarkTexts();

    std::pair<QTextCursor, bool> FindPairingBracketCursor(QTextCursor cursor, QTextCursor::MoveOperation direct,
                                                          const QChar &startBracketChar, const QChar &endBracketChar);

    int visibleBlockCount() {
        QPoint bottomRight(viewport()->width() - 1, viewport()->height() - 1);
        int endPos = cursorForPosition(bottomRight).position();
        return endPos;
    }
    int GetBlockNumber(int y);

    void ZoomIn();
    void ZoomOut();

    bool ShouldSave() { return contentChanged_; }
    void SetShouldSave(bool saved) { contentChanged_ = saved; }
    // Should call after addTab().
    virtual void SetModifiedIcon(bool modified);
    // Should call after addTab().
    virtual void SetModified(bool modified);

    bool fileLoaded() { return fileLoaded_; };
    void setFileLoaded(bool fileLoaded) { fileLoaded_ = fileLoaded; }

    TabView *tabView() { return tabView_; }

    bool undoAvail() { return undoAvail_; };
    bool redoAvail() { return redoAvail_; };

    void ApplyWrapTextState();
    void ApplySpecialCharsVisible();
    void ApplyTabCharNum();

    FileEncoding &fileEncoding() { return fileEncoding_; }
    void setFileEncoding(FileEncoding &&fileEncoding);
    void ChangeFileEncoding(FileEncoding &&fileEncoding);

    void UpdateStatusBarWithCursor();

    void HighlightFocus();

    void TrigerParser();

    virtual void Hover(QTextCursor &cursor);

    QString GetCursorText();
    QString GetCursorText(QTextCursor &cursor);

    void JumpHint(QTextCursor &cursor);
    void Jump();

    bool AllowRichParsing() {
        QFile file(filePath_);
        if (file.size() > Constants::kMaxParseFileSize) {
            return false;
        }
        return (blockCount() <= Constants::kMaxParseLineNum &&
                document()->characterCount() <= Constants::kMaxParseCharNum);
    }

    bool AllowHighlightScrollbar() {
        QFile file(filePath_);
        if (file.size() > Constants::kMaxHighlightScrollbarFileSize) {
            return false;
        }
        return (blockCount() <= Constants::kMaxHighlightScrollbarLineNum &&
                document()->characterCount() <= Constants::kMaxHighlightScrollbarCharNum);
    }

    qreal LineSpacing() {
        QFont currentFont = font();
        //        currentFont.setPointSize(fontSize_ * fontZoom_ / 100);
        qreal spacing = QFontMetricsF(currentFont).lineSpacing();
        //        if (lineSpacing_ != 100) spacing *= qreal(lineSpacing_) / 100;
        return spacing;
    }

    int currentBlockNumber() const;

    ScrollBarInfo &scrollbarLineInfos();

    bool hightlightScrollbarInvalid() const;

    void setHightlightScrollbarInvalid(bool hightlightScrollbarInvalid);

    int LineNumber(const QTextCursor &cursor) const;
    int LineCount() const;

    std::vector<int> lineOffset() const;

    int lastPos() const;
    void setLastPos(int lastPos);

    void HandleLineOffset();

   protected:
    void showEvent(QShowEvent *) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

    virtual void UnderpaintCurrentBlock();

   private slots:
    void HandleBlockCountChanged(int newBlockCount);
    void HandleCursorPositionChanged();
    void HandleSelectionChanged();
    void HandleUpdateRequest(const QRect &rect, int dy);
    void HandleTextChanged();
    void HandleContentsChange(int from, int charsRemoved, int charsAdded);
    void HandleContentsChanged();
    void HandleCopyAvailable(bool avail);
    void HandleUndoAvailable(bool avail);
    void HandleRedoAvailable(bool avail);

    // Context menu operations.
    bool Find();
    bool Replace();
    bool MarkUnmarkCursorText();
    bool UnmarkAll();

   private:
    friend class HighlightScrollBar;
    void UpdateLineNumberArea(const QRect &rect, int dy);

    void HighlightFocusChars();
    void HighlightFocusNearBracket();
    void HighlightBrackets(const QTextCursor &leftCursor, const QTextCursor &rightCursor);

    void HighlightVisibleChars(const QString &text, const QColor &foreground = QColor(Qt::lightGray),
                               const QColor &background = QColor(52, 58, 64));  // QColor(54, 54, 100)
    // Return false if too many highlighting times.
    bool HighlightChars(int startPos, int count, const QColor &foreground = QColor(Qt::lightGray),
                        const QColor &background = QColor(52, 58, 64),  // QColor(54, 54, 100)
                        bool underline = false);
    bool highlighterInvalid() {
        // If mark texts change, or selected text changes.
        return highlighterInvalid_;
    }

    TabView *tabView_;
    QWidget *lineNumberArea_;
    int currentBlockNumber_{0};
    int visibleBlockCount_{0};
    int currentFontSize_{font().pointSize()};

    // 0 if open file edit.
    int newFileNum_{0};
    QString fileName_;
    QString filePath_;

    FileType fileType_;

    // We not use QSyntaxHighlighter for user interacting, to set false if want quick highlight to work.
    bool highlighterInvalid_{true};
    QString selectedText_;
    TextHighlighter *highlighter_{nullptr};
    QVector<QString> markTexts_;
    const QMap<QString, QString> leftBrackets_ = {{"(", ")"}, {"[", "]"}, {"{", "}"}, {"<", ">"}};
    const QMap<QString, QString> rightBrackets_ = {{")", "("}, {"]", "["}, {"}", "{"}, {">", "<"}};
    bool contentChanged_{false};
    bool fileLoaded_{false};

    bool copyAvail_{false};
    bool undoAvail_{false};
    bool redoAvail_{false};

    FileEncoding fileEncoding_ /*{106}*/;

    IParser *parser_{nullptr};
    OutlineList *overviewList_{nullptr};
    FunctionHierarchy *hierarchy_{nullptr};

    int lastPos_{-1};

    int hoverPos_;
    bool jumpAvailable_{false};
    FuncGraphInfo funcGraphInfo_;

    int timerId_{0};
    QMenu *menu_;

    ScrollBarInfo scrollbarLineInfos_;
    bool hightlightScrollbarInvalid_{false};

    // QSizeF documentSize_;
    std::vector<int> lineOffset_;
    bool layoutRequested_{false};
    bool resized_{false};
};

class NewFileNum : public QObject {
    Q_OBJECT
   public:
    // Starts from 1.
    static int GetNumber() {
        for (int i = 0; i < numbers_use_status_.size(); ++i) {
            if (!numbers_use_status_[i]) {
                numbers_use_status_[i] = true;
                qDebug() << "Reuse number: " << i + 1;
                return i + 1;
            }
        }
        numbers_use_status_.push_back(true);
        qDebug() << "New number: " << numbers_use_status_.size();
        return numbers_use_status_.size();
    }
    // The 'number' must exceed 0.
    static void SetNumber(int number, bool use) {
        if (number <= 0) {
            qCritical() << "Wrong number: " << number;
            return;
        }
        int pos = number - 1;
        if (pos < numbers_use_status_.size()) {
            numbers_use_status_[pos] = use;
            qDebug() << "Reset number: " << number;
            return;
        }

        // Extend to the 'pos', then set as use.
        for (int i = 0; i <= pos; ++i) {
            numbers_use_status_.push_back(false);
        }
        numbers_use_status_[pos] = use;
        qDebug() << "Expand to number: " << numbers_use_status_.size();
    }

   private:
    static QVector<bool> numbers_use_status_;
};

class LineNumberArea : public QWidget {
    Q_OBJECT
   public:
    LineNumberArea(EditView *editView) : QWidget(editView), editView_(editView) {}

    QSize sizeHint() const override;

   protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

   private:
    EditView *editView_{nullptr};

    int startBlockNumber_{-1};
};

class HighlightScrollBar : public QScrollBar {
    Q_OBJECT
   public:
    HighlightScrollBar(EditView *editView, QWidget *parent = nullptr) : QScrollBar(parent), editView_(editView) {}

   protected:
    void paintEvent(QPaintEvent *) override;
    void sliderChange(SliderChange change) override;

   private:
    std::vector<std::pair<int, QColor>> PreparePositions();
    void PaintLines(QPainter &painter, const QRect &aboveHandleRect, const QRect &handleRect,
                    const QRect &belowHandleRect, const QColor &color, int pos, bool inHandle);

    EditView *editView_{nullptr};
    int height_{0};
};
}  // namespace QEditor

#endif  // EDITVIEW_H
