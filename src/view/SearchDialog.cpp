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

#include "SearchDialog.h"
#include "Logger.h"
#include "MainWindow.h"
#include "SearchTargets.h"
#include "Settings.h"
#include "ui_SearchDialog.h"
#include <QScrollBar>
#include <QTextBlock>

#ifdef Q_OS_WIN
namespace WinTheme {
extern bool IsDarkTheme();
extern void SetDark_qApp();
extern void SetDarkTitleBar(HWND hwnd);
}  // namespace WinTheme
#endif

namespace QEditor {
SearchDialog::SearchDialog(QWidget *parent, int index)
    : QDialog(parent), ui_(new Ui::UISearchDialog()), historyMenu_(new QMenu(this)) {
    ui_->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    //    setAttribute(Qt::WA_DeleteOnClose);

    auto settings = Settings();
    qreal opa = settings.Get("dialog", "opacity", 0.9).toDouble();
    setWindowOpacity(opa * 0.8);

    historyMenu_->setStyleSheet(
        "QMenu{color:gray; background-color:rgb(68,68,68); margin:2px 2px; border:none;} QMenu::item{color:gray; "
        "background-color:rgb(68,68,68); padding:5px 5px;} QMenu::item:selected{color:lightGray; "
        "background-color:rgb(9,71,113);}"
        "QMenu::item:pressed{border:1px solid rgb(60,60,60); background-color:rgb(29,91,133);} "
        "QMenu::separator{height:1px; background-color:rgb(80,80,80);}");

    ui_->tabWidget->setCurrentIndex(index);
    auto historyLambda = [this](QObject *watched, QEvent *event) -> bool {
        if (event->type() == QEvent::MouseButtonDblClick) {
            auto watchedLineEdit = qobject_cast<QLineEdit *>(watched);
            if (!watchedLineEdit->hasFocus()) {
                watchedLineEdit->setFocus();
                return true;
            }
            historyMenu_->clear();
            SearchTargets::LoadTargets();
            const auto &targets = SearchTargets::targets();
            if (targets.isEmpty()) {
                if (!watchedLineEdit->hasSelectedText()) {
                    watchedLineEdit->setSelection(0, watchedLineEdit->text().size());
                    return true;
                }
                return false;
            }
            for (const auto &target : targets) {
                QAction *historyAction = new QAction(target, this);
                historyMenu_->addAction(historyAction);

                connect(historyAction, &QAction::triggered, watched, [watchedLineEdit, target]() {
                    if (watchedLineEdit != nullptr) {
                        watchedLineEdit->setText(target);
                    }
                });
            }
            qDebug() << watchedLineEdit->pos() << watchedLineEdit->x() << watchedLineEdit->y()
                     << watchedLineEdit->size() << watchedLineEdit->width() << watchedLineEdit->height();
            auto size = watchedLineEdit->size();
            auto centerPos = watchedLineEdit->mapToGlobal(watchedLineEdit->pos());
            constexpr auto padding = 5;
            historyMenu_->popup(QPoint(centerPos.x() - size.width() / 2 + padding, centerPos.y() + size.height() / 2));
        } else if (event->type() == QEvent::KeyPress) {
            historyMenu_->hide();
            auto watchedLineEdit = qobject_cast<QLineEdit *>(watched);
            watchedLineEdit->setFocus();

            SearchTargets::LoadTargets();
            const auto &targets = SearchTargets::targets();
            auto keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Down) {
                // Save current inputting text.
                if (historyIndex_ == -1) {
                    searchInput_ = watchedLineEdit->text();
                }

                if (historyIndex_ < targets.size() - 1) {
                    ++historyIndex_;
                }
                if (historyIndex_ >= 0 && historyIndex_ < targets.size()) {
                    watchedLineEdit->setText(targets[historyIndex_]);
                    watchedLineEdit->setSelection(0, watchedLineEdit->text().size());
                }
            } else if (keyEvent->key() == Qt::Key_Up) {
                if (historyIndex_ == -1) {
                    watchedLineEdit->setSelection(0, watchedLineEdit->text().size());
                    return false;
                }

                if (historyIndex_ >= 0) {
                    --historyIndex_;
                }
                if (historyIndex_ == -1) {
                    watchedLineEdit->setText(searchInput_);
                    watchedLineEdit->setSelection(0, watchedLineEdit->text().size());
                } else if (historyIndex_ >= 0 && historyIndex_ < targets.size()) {
                    watchedLineEdit->setText(targets[historyIndex_]);
                    watchedLineEdit->setSelection(0, watchedLineEdit->text().size());
                }
            } else {
                historyIndex_ = -1;
                searchInput_.clear();
            }
        }
        return false;  // Must return false.
    };
    if (index == 0) {  // Find
        ui_->lineEditFindFindWhat->setFocus();
    } else {  // Replace
        ui_->lineEditReplaceFindWhat->setFocus();
    }

    // QCoreApplication::postEvent(this, event);
    ui_->lineEditFindFindWhat->installEventFilter(new LambdaEventFilter(ui_->lineEditFindFindWhat, historyLambda));
    ui_->lineEditReplaceFindWhat->installEventFilter(
        new LambdaEventFilter(ui_->lineEditReplaceFindWhat, historyLambda));

    ui_->checkBoxFindWrapAround->setChecked(true);

    connect(ui_->lineEditFindFindWhat, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (!editView()->AllowHighlightScrollbar()) {
            return;
        }
        const auto &lineNums = MainWindow::Instance().GetSearcher()->FindAllLineNum(text);
        auto &scrollbarInfos = editView()->scrollbarLineInfos()[ScrollBarHighlightCategory::kCategorySearch];
        scrollbarInfos.clear();
        scrollbarInfos.emplace_back(std::make_pair(lineNums, QColor(0xff00c000)));
        editView()->setHightlightScrollbarInvalid(true);
    });
    ui_->lineEditFindFindWhat->setText(GetSelectedText());
    connect(ui_->lineEditReplaceFindWhat, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (!editView()->AllowHighlightScrollbar()) {
            return;
        }
        const auto &lineNums = MainWindow::Instance().GetSearcher()->FindAllLineNum(text);
        auto &scrollbarInfos = editView()->scrollbarLineInfos()[ScrollBarHighlightCategory::kCategorySearch];
        scrollbarInfos.clear();
        scrollbarInfos.emplace_back(std::make_pair(lineNums, QColor(0xff00c000)));
        editView()->setHightlightScrollbarInvalid(true);
    });
    ui_->lineEditReplaceFindWhat->setText(GetSelectedText());
}

SearchDialog::~SearchDialog() { delete ui_; }

EditView *SearchDialog::editView() { return MainWindow::Instance().editView(); }

void SearchDialog::Start(int index) {
#define FORCE_DARK_THEME
#ifdef Q_OS_WIN
#ifdef FORCE_DARK_THEME
    WinTheme::SetDarkTitleBar(reinterpret_cast<HWND>(winId()));
#else
    if (WinTheme::IsDarkTheme()) {
        WinTheme::SetDarkTitleBar(reinterpret_cast<HWND>(winId()));
    }
#endif
#endif

    searcher_ = MainWindow::Instance().GetSearcher();

    if (index == 0) {  // Find.
        ui_->lineEditFindFindWhat->setText(GetSelectedText());
        ui_->lineEditFindFindWhat->setFocus();
    } else {  // Replace.
        ui_->lineEditReplaceFindWhat->setText(GetSelectedText());
        ui_->lineEditReplaceFindWhat->setFocus();
    }
    setCurrentTabIndex(index);
    show();

    auto settings = Settings();
    auto backward = settings.Get("searcher", "backward", false).toBool();
    auto wholeWord = settings.Get("searcher", "whole_word", false).toBool();
    auto matchCase = settings.Get("searcher", "case_sensitive", false).toBool();
    auto wrapAround = settings.Get("searcher", "wrap_around", false).toBool();
    auto searchMode = settings.Get("searcher", "search_mode", 0).toInt();
    ui_->checkBoxFindBackward->setChecked(backward);
    ui_->checkBoxFindWholeWord->setChecked(wholeWord);
    ui_->checkBoxFindMatchCase->setChecked(matchCase);
    ui_->checkBoxFindWrapAround->setChecked(wrapAround);
    if (searchMode == 1) {
        ui_->radioButtonFindExtended->setChecked(true);
    } else if (searchMode == 2) {
        ui_->radioButtonFindRe->setChecked(true);
    } else {
        ui_->radioButtonFindNormal->setChecked(true);
    }
}

void SearchDialog::InitSetting() {
    searcher_->setCheckBoxFindBackward(ui_->checkBoxFindBackward->isChecked());
    searcher_->setCheckBoxFindWholeWord(ui_->checkBoxFindWholeWord->isChecked());
    searcher_->setCheckBoxFindMatchCase(ui_->checkBoxFindMatchCase->isChecked());
    searcher_->setCheckBoxFindWrapAround(ui_->checkBoxFindWrapAround->isChecked());
    searcher_->setRadioButtonFindNormal(ui_->radioButtonFindNormal->isChecked());
    searcher_->setRadioButtonFindExtended(ui_->radioButtonFindExtended->isChecked());
    searcher_->setRadioButtonFindRe(ui_->radioButtonFindRe->isChecked());
}

int SearchDialog::currentTabIndex() { return ui_->tabWidget->currentIndex(); }

void SearchDialog::setCurrentTabIndex(int index) { ui_->tabWidget->setCurrentIndex(index); }

void SearchDialog::closeEvent(QCloseEvent *) {
    historyIndex_ = -1;
    searchInput_.clear();
    if (editView()->AllowHighlightScrollbar()) {
        auto &scrollbarInfos = editView()->scrollbarLineInfos()[ScrollBarHighlightCategory::kCategorySearch];
        scrollbarInfos.clear();
        editView()->setHightlightScrollbarInvalid(true);
    }
}

void SearchDialog::hideEvent(QHideEvent *) {
    historyIndex_ = -1;
    searchInput_.clear();
    if (editView()->AllowHighlightScrollbar()) {
        auto &scrollbarInfos = editView()->scrollbarLineInfos()[ScrollBarHighlightCategory::kCategorySearch];
        scrollbarInfos.clear();
        editView()->setHightlightScrollbarInvalid(true);
    }
}

const QString SearchDialog::GetSelectedText() {
    if (editView() == nullptr) {
        return "";
    }
    auto textCursor = editView()->textCursor();
    if (!textCursor.hasSelection()) {
        textCursor.select(QTextCursor::WordUnderCursor);
    }
    auto text = textCursor.selectedText();
    qDebug() << ", selectedText: " << text;
    return text;
}

void SearchDialog::on_pushButtonFindFindNext_clicked() {
    if (editView() == nullptr) {
        return;
    }
    auto const &target = ui_->lineEditFindFindWhat->text();
    InitSetting();

    // Record search string history.
    MainWindow::Instance().setSearchingString(target);
    SearchTargets::UpdateTargets(target);

    auto cursor = searcher_->FindNext(target, editView()->textCursor());
    if (!cursor.isNull()) {
        editView()->setTextCursor(cursor);
    }
}

void SearchDialog::on_pushButtonFindFindAllInCurrent_clicked() {
    if (editView() == nullptr) {
        return;
    }
    if (searchResultList_ == nullptr) {
        searchResultList_ = MainWindow::Instance().GetSearchResultList();
    }
    MainWindow::Instance().ShowSearchDockView();

    auto sessionItem = searchResultList_->StartSearchSession(editView());
    qDebug() << "Find all start....";
    auto const &target = ui_->lineEditFindFindWhat->text();
    InitSetting();

    // Record search string history.
    MainWindow::Instance().setSearchingString(target);
    SearchTargets::UpdateTargets(target);

    QProgressDialog progressDialog(this);
    progressDialog.setMinimumWidth(540);
    progressDialog.setCancelButtonText(tr("&Cancel"));

    constexpr auto findProgressValue = 60;
    constexpr auto addListProgressValue = 35;
    constexpr auto finishProgressValue = 5;
    constexpr auto maxProgressValue = findProgressValue + addListProgressValue + finishProgressValue;
    progressDialog.setWindowTitle(tr("Finding all positions..."));
    progressDialog.setRange(0, maxProgressValue);
    std::vector<QTextCursor> findResult = searcher_->FindAll(target, [&progressDialog](int progress) {
        if (progressDialog.wasCanceled()) {
            return false;
        }
        progressDialog.setValue(findProgressValue * progress / maxProgressValue);
        QCoreApplication::processEvents();
        return true;
    });
    qDebug() << "Find all finish....";

    // TODO:
    // To support display dynamic visible list items, and do fetchMore for user scrolling.
    progressDialog.setWindowTitle(tr("Preparing result list..."));
    for (size_t i = 0; i < findResult.size(); ++i) {
        if (progressDialog.wasCanceled()) {
            searchResultList_->FinishSearchSession(sessionItem, target, i, false);
            return;
        }
        const auto &item = findResult[i];
        qDebug() << "Display item start....";
        const auto currentBlock = item.block();
        int lineNum = item.blockNumber();
        const auto highlightingTarget = item.selectedText().toHtmlEscaped();
        const auto htmlTarget = QString("<span style=\"font-size:14px;font-family:Consolas;color:#BCE08C\">") +
                                highlightingTarget + QString("</span>");
        const auto plainText = currentBlock.text();
        auto escapedStr = plainText.toHtmlEscaped();
        escapedStr.replace(highlightingTarget, htmlTarget, Qt::CaseSensitive);
        qDebug() << "Line " << lineNum << ": highlightingTarget: " << highlightingTarget
                 << ", htmlTarget: " << htmlTarget << ", currentStr: " << escapedStr;

        auto htmlText = QString("<div style=\"font-size:14px;font-family:Consolas;color:#BEBEBE\">") + tr("Line ") +
                        QString("<span style=\"font-size:14px;font-family:Consolas;color:#2891AF\">") +
                        QString::number(lineNum + 1) + QString("</span>") + ":  " + escapedStr + QString("</div>");
        qDebug() << "Line " << lineNum << ": text: " << htmlText;
        searchResultList_->AddSearchResult(sessionItem, lineNum, htmlText, plainText, item);
        progressDialog.setValue(findProgressValue + addListProgressValue * (i + 1) / findResult.size());
        QCoreApplication::processEvents();
        qDebug() << "Display item end....";
    }
    progressDialog.setWindowTitle(tr("Showing results in list..."));
    searchResultList_->FinishSearchSession(sessionItem, target, findResult.size());
    progressDialog.setValue(maxProgressValue);
    QCoreApplication::processEvents();
}

void SearchDialog::on_pushButtonFindCount_clicked() {
    if (editView() == nullptr) {
        return;
    }
    std::vector<QTextCursor> res;
    auto const &target = ui_->lineEditFindFindWhat->text();
    InitSetting();

    // Record search string history.
    MainWindow::Instance().setSearchingString(target);
    SearchTargets::UpdateTargets(target);

    res = searcher_->FindAll(target);
    auto info = QString("<b><font color=#67A9FF size=4>") + QString::number(res.size()) + tr(" matches in ") +
                editView()->fileName() + "</font></b>";
    ui_->labelInfo->setText(info);
}

void SearchDialog::on_pushButtonFindCancel_clicked() { close(); }

void SearchDialog::on_pushButtonReplaceCancel_clicked() { close(); }

void SearchDialog::on_pushButtonReplaceFindNext_clicked() {
    if (editView() == nullptr) {
        return;
    }
    auto const &target = ui_->lineEditReplaceFindWhat->text();
    InitSetting();

    // Record search string history.
    MainWindow::Instance().setSearchingString(target);
    SearchTargets::UpdateTargets(target);

    auto res = searcher_->FindNext(target, editView()->textCursor());
    if (!res.isNull()) {
        editView()->setTextCursor(res);
    }
}

void SearchDialog::on_pushButtonReplaceReplace_clicked() {
    auto const &target = ui_->lineEditReplaceFindWhat->text();
    auto const &text = ui_->lineEditReplaceReplaceWith->text();
    InitSetting();

    // Record search string history.
    MainWindow::Instance().setSearchingString(target);
    SearchTargets::UpdateTargets(target);

    searcher_->setInfo("");
    searcher_->Replace(target, text, ui_->checkBoxFindBackward->isChecked());
    ui_->labelInfo->setText(searcher_->info());
}

void SearchDialog::on_pushButtonReplaceReplaceAll_clicked() {
    if (editView() == nullptr) {
        return;
    }
    int count;
    auto const &target = ui_->lineEditReplaceFindWhat->text();
    auto const &text = ui_->lineEditReplaceReplaceWith->text();
    InitSetting();

    // Record search string history.
    MainWindow::Instance().setSearchingString(target);
    SearchTargets::UpdateTargets(target);

    count = searcher_->ReplaceAll(target, text);
    auto info = QString("<b><font color=#67A9FF size=4>") + QString::number(count) +
                tr(" occurrences were replaced in ") + editView()->fileName() + "</font></b>";
    ui_->labelInfo->setText(info);
}

void SearchDialog::on_lineEditFindFindWhat_textChanged(const QString &) { ui_->labelInfo->clear(); }

void SearchDialog::on_lineEditReplaceFindWhat_textChanged(const QString &) { ui_->labelInfo->clear(); }

EditView *Searcher::editView() { return MainWindow::Instance().editView(); }

TabView *Searcher::tabView() { return MainWindow::Instance().tabView(); }

QString Searcher::info() const { return info_; }

void Searcher::setInfo(const QString &info) { info_ = info; }

// Find for Extended Option.
bool Searcher::_Find(const QStringList &target, const QTextCursor &startCursor, QTextCursor &targetCursor,
                     bool backward) {
    if (editView() == nullptr) {
        return false;
    }
    int firstStart = -1;
    QTextCursor currentCursor = startCursor;
    while (true) {
        // The first block.
        int start;
        int end;
        int count = 0;
        QTextCursor cursor;
        const auto &firstTarget = target[0];
        if (!_Find(firstTarget, currentCursor, cursor, backward)) {
            qDebug() << "Not found: " << firstStart;
            return false;
        }
        if (!cursor.hasSelection()) {
            qDebug() << "Has no selection: " << firstStart;
            return false;
        }
        start = cursor.selectionStart();
        end = cursor.selectionEnd();

        count += firstTarget.length();

        // Have searched all content, finish.
        if (cursor.position() == firstStart) {
            break;
        }
        if (firstStart == -1) {
            firstStart = cursor.position();
        }

        const auto &firstChar = editView()->document()->characterAt(start);
        const auto &lastChar = editView()->document()->characterAt(end);
        qDebug() << "start: " << start << firstChar << ", end: " << end << lastChar
                 << ", selection: " << cursor.selectedText();
        if (target.length() > 1) {                             // Multiple lines, not one line.
            if (firstChar == '\n' || firstChar == "\u2029") {  // Multiple lines target. Starts with '\n'.
                ++count;                                       // Include '\n'
            } else {
                // Check if ends with '\n'.
                if (lastChar == '\0') {  // EOF
                    qDebug() << "Reach end of file.";
                } else if (lastChar != '\n' && lastChar != "\u2029") {
                    qDebug() << "lastChar: " << lastChar;
                    int newStart;
                    if (backward) {
                        newStart = start - 1;
                    } else {
                        newStart = start + 1;
                    }
                    qDebug() << "newStart: " << newStart;
                    currentCursor.setPosition(newStart, QTextCursor::MoveAnchor);
                    continue;
                } else {
                    ++count;  // Include '\n'
                }
            }
        }

        // The middle blocks.
        auto block = cursor.block();
        bool match = true;
        // If multiple lines target && starts with '\n', use current block firstly.
        if (firstChar != '\n' && firstChar != "\u2029") {
            block = block.next();
        }
        for (int i = 1; i < target.length() - 1; ++i) {
            qDebug() << "block text: " << block.text() << ", " << block.blockNumber();
            qDebug() << "target[i]: " << target[i];
            if (block.text() != target[i]) {
                match = false;
                break;
            }
            count += target[i].length() + 1;  // "+ 1" to include '\n'
            block = block.next();
        }
        if (!match) {
            int newStart;
            if (backward) {
                newStart = start - 1;
            } else {
                newStart = start + 1;
            }
            qDebug() << "newStart: " << newStart;
            currentCursor.setPosition(newStart, QTextCursor::MoveAnchor);
            continue;
        }

        // The last block.
        if (target.length() > 1) {
            const auto &lastTarget = target[target.length() - 1];
            if (!lastTarget.isEmpty()) {  // Not ends with '\n'.
                if (!block.text().startsWith(lastTarget)) {
                    qDebug() << "Match last block failed, lastTarget: " << lastTarget << ", block: " << block.text();
                    int newStart;
                    if (backward) {
                        newStart = start - 1;
                    } else {
                        newStart = start + 1;
                    }
                    qDebug() << "newStart: " << newStart;
                    currentCursor.setPosition(newStart, QTextCursor::MoveAnchor);
                    continue;
                }
                count += lastTarget.length();
            }
        }

        // Found.
        if (backward) {
            cursor.setPosition(start + count, QTextCursor::MoveAnchor);
            for (int i = 0; i < count; ++i) {
                bool res = cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
                if (!res) {
                    qCritical() << "Move position failed, res: " << res;
                    return false;
                }
            }
        } else {
            cursor.setPosition(start, QTextCursor::MoveAnchor);
            for (int i = 0; i < count; ++i) {
                bool res = cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                if (!res) {
                    qCritical() << "Move position failed, res: " << res;
                    return false;
                }
            }
        }
        targetCursor = cursor;
        qDebug() << "cursor: " << cursor.position();
        return true;
    }
    return false;
}

template <class T>
bool Searcher::_Find(const T &target, const QTextCursor &startCursor, QTextCursor &targetCursor, bool backward,
                     bool first) {
    if (editView() == nullptr) {
        return false;
    }
    int flag = 0;
    if (backward) {
        flag |= QTextDocument::FindFlag::FindBackward;
    }
    if (checkBoxFindMatchCase_) {
        flag |= QTextDocument::FindFlag::FindCaseSensitively;
    }
    if (checkBoxFindWholeWord_) {
        flag |= QTextDocument::FindFlag::FindWholeWords;
    }

    if constexpr (std::is_same_v<T, QString>) {
        // Multiple lines. Targets starts with '\n' if target is empty.
        if (target.isEmpty()) {
            QTextCursor cursor = startCursor;
            // Notice that, block.text().length() not includes '\n', but block.length() includes '\n'.
            auto block = startCursor.block();
            if (backward) {
                block = block.previous();

                // If cursor at the block end position, cursor.block() is its next block.
                if (startCursor.hasSelection() &&
                    (startCursor.selectedText() == '\n' || startCursor.selectedText() == "\u2029")) {
                    block = block.previous();
                }
            }
            if (block.isValid()) {
                const auto &firstChar = editView()->document()->characterAt(block.position() + block.length() - 1);
                const auto &lastChar = editView()->document()->characterAt(block.position() + block.length());
                qDebug() << "firstChar: " << firstChar << ", lastChar: " << lastChar << "block text: " << block.text();
                if (lastChar == '\0') {  // EOF
                    qDebug() << "Reach end of file.";
                } else if (firstChar != '\n' && firstChar != "\u2029") {
                    qDebug() << "First Char: " << firstChar << ", Last Char: " << lastChar;
                } else {
                    qDebug() << "Found \\n";  // Include '\n'
                    // When we get cursor.block(), the block is that where the cursor selectionEnd() stays at.
                    cursor.setPosition(block.position() + block.length() - 1,
                                       QTextCursor::MoveAnchor);  // Set position before '\n'.
                    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                    targetCursor = cursor;
                    return true;
                }
            }
        } else {
            auto cursor = editView()->document()->find(target, startCursor, QTextDocument::FindFlags(flag));
            if (!cursor.isNull()) {
                targetCursor = cursor;
                return true;
            }
        }
    } else {
        auto cursor = editView()->document()->find(target, startCursor, QTextDocument::FindFlags(flag));
        if (!cursor.isNull()) {
            targetCursor = cursor;
            return true;
        }
    }

    bool res = false;
    if (checkBoxFindWrapAround_ && first) {
        QScrollBar *scrollBar = editView()->verticalScrollBar();
        auto sliderPos = scrollBar->sliderPosition();
        QTextCursor anewCursor = startCursor;
        if (backward) {
            anewCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        } else {
            anewCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
        }

        res = _Find(target, anewCursor, targetCursor, backward, false);
        if (!res) {
            // If not found, recover the text cursor.
            scrollBar->setSliderPosition(sliderPos);
        }
    }
    return res;
}

static void HandleEscapeChars(QString &text) {
    //    text.replace("\\a", "\a");  // 0x07, Alert bell
    //    text.replace("\\b", "\b");  // 0x08, Backspace
    //    text.replace("\\e", "\e");  // 0x1B, Escape char
    //    text.replace("\\f", "\f");  // 0x0C, Formfeed Page Break
    //    text.replace("\\n", "\n");  // 0x0A, Line Feed
    //    text.replace("\\r", "\r");  // 0x0D, Carriage Return
    //    text.replace("\\t", "\t");  // 0x09, Horizontal Tab
    //    text.replace("\\v", "\v");  // 0x0B, Vertical Tab
    //    text.replace("\\", "\u005c");  // 0x5C, Backslash
    //    text.replace("\\'", "\'");  // 0x27, Single quotation mark
    //    text.replace("\\"", "\"");  // 0x22, Double quotation mark
    //    text.replace("\\?", "\?");  // 0x3F, Question mark
    //    text.replace("\\ddd", "d");  // Octal interpreted.
    //    text.replace("\\xxx", "xxx");  // Hexadecimal interpreted.
    //    text.replace("\\uhhhh", "hhhh");  // Unicode code.

    text.replace("\\r\\n", "\n");
    text.replace("\\r", "\n");
    text.replace("\\n", "\n");
    text.replace("\\t", "\t");
}

QTextCursor Searcher::FindPrevious(const QString &text, const QTextCursor &startCursor) {
    bool backwardCheck = checkBoxFindBackward_;
    return _FindNext(text, startCursor, !backwardCheck);
}

QTextCursor Searcher::FindNext(const QString &text, const QTextCursor &startCursor) {
    bool backwardCheck = checkBoxFindBackward_;
    return _FindNext(text, startCursor, backwardCheck);
}

QTextCursor Searcher::_FindPrevious(const QString &text, const QTextCursor &startCursor, bool backward) {
    bool backwardCheck = checkBoxFindBackward_;
    return _FindNext(text, startCursor, backward ? backwardCheck : !backwardCheck);
}

// Should save original cursor, and restore it if failed.
QTextCursor Searcher::_FindNext(const QString &text, const QTextCursor &startCursor, bool backward) {
    if (text.isEmpty()) {
        return QTextCursor();
    }

    bool res;
    QTextCursor cursor;
    if (radioButtonFindRe_) {
        const QRegularExpression reTarget = QRegularExpression(text);
        res = _Find<QRegularExpression>(reTarget, startCursor, cursor, backward);
    } else {
        if (radioButtonFindExtended_) {
            auto extendedText = text;
            HandleEscapeChars(extendedText);
            qDebug() << "extendedText: " << extendedText;
            auto extendedTexts = extendedText.split("\n");
            qDebug() << "extendedTexts: " << extendedTexts;
            res = _Find(extendedTexts, startCursor, cursor, backward);
        } else {
            res = _Find<QString>(text, startCursor, cursor, backward);
        }
    }

    if (res) {
        qDebug() << "Found: " << text << ", pos: " << cursor.position();
        return cursor;
    } else {
        qDebug() << "Not found: " << text;
        return QTextCursor();
    }
}

std::vector<QTextCursor> Searcher::FindAll(const QString &target, std::function<bool(int)> progressCallback) {
    std::vector<QTextCursor> cursors;
    if (editView() == nullptr) {
        return cursors;
    }
    setCheckBoxFindBackward(false);
    setCheckBoxFindWrapAround(false);
    QTextCursor savedCursor = editView()->textCursor();
    QTextCursor cursor = editView()->textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    int firstStart = -1;
    while (true) {
        cursor = _FindNext(target, cursor, false);
        if (!cursor.isNull()) {
            if (cursor.selectionStart() == firstStart) {
                break;
            }
            if (firstStart == -1) {
                firstStart = cursor.selectionStart();
            }
            if (progressCallback) {
                constexpr long long maxProgressValue = 100;
                int progress = maxProgressValue * cursor.position() / editView()->document()->characterCount();
                auto success = progressCallback(progress);
                qDebug() << "progress: " << progress << ", pos: " << cursor.position()
                         << ", count: " << editView()->document()->characterCount();
                if (!success) {
                    break;
                }
            }
            cursors.emplace_back(cursor);
        } else {
            break;
        }
    }
    editView()->setTextCursor(savedCursor);
    return cursors;
}

std::vector<int> Searcher::FindAllLineNum(const QString &target) {
    std::vector<int> lineNums;
    if (editView() == nullptr) {
        return lineNums;
    }

    // Force options.
    setCheckBoxFindMatchCase(true);
    setRadioButtonFindNormal(true);

    QTextCursor savedCursor = editView()->textCursor();
    QTextCursor cursor = editView()->textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    int firstStart = -1;
    while (true) {
        cursor = FindNext(target, cursor);
        if (!cursor.isNull()) {
            if (cursor.selectionStart() == firstStart) {
                break;
            }
            if (firstStart == -1) {
                firstStart = cursor.selectionStart();
            }
            const auto lineNum = editView()->LineNumber(cursor);
            qDebug() << "cursor: " << cursor.position() << cursor.blockNumber() << cursor.block().firstLineNumber()
                     << lineNum << cursor.block().text();
            lineNums.emplace_back(lineNum);
        } else {
            break;
        }
    }
    editView()->setTextCursor(savedCursor);
    return lineNums;
}

// Replace 'target' with 'text'.
void Searcher::Replace(const QString &target, const QString &text, bool backward) {
    if (editView() == nullptr) {
        return;
    }
    // Check find options.
    bool wrapAround = (checkBoxFindWrapAround_);
    QString startStr;
    QString endStr;
    if (backward) {
        startStr = tr("bottom");
        endStr = tr("top");
    } else {
        startStr = tr("top");
        endStr = tr("bottom");
    }

    // Handle \r, \n, and \t.
    auto extendedText = text;
    if (radioButtonFindExtended_) {
        HandleEscapeChars(extendedText);
    }

    // Move the search start pos to the start of selection if has selection.
    if (editView()->textCursor().hasSelection()) {
        int start;
        if (backward) {
            start = editView()->textCursor().selectionEnd();
        } else {
            start = editView()->textCursor().selectionStart();
        }
        auto cursor = editView()->textCursor();
        cursor.setPosition(start);
        editView()->setTextCursor(cursor);
    }

    // To find and replace.
    auto res = _FindNext(target, editView()->textCursor(), backward);
    if (!res.isNull()) {  // Find success.
        editView()->setTextCursor(res);
        res.insertText(extendedText);
        res = _FindNext(target, res, backward);
        if (!res.isNull()) {
            editView()->setTextCursor(res);
            auto info = QString("<b><font color=#67A9FF size=4>") +
                        tr("1 occurrence were replaced, to continue replacing.") + "</font></b>";
            setInfo(info);
        } else {
            if (wrapAround) {
                auto info = QString("<b><font color=#67A9FF size=4>") +
                            tr("1 occurrence were replaced. No more occurrence to replace.") + "</font></b>";
                setInfo(info);
            } else {
                auto info = QString("<b><font color=#67A9FF size=4>") + tr("1 occurrence were replaced. ") + endStr +
                            tr(" has been reached.") + "</font></b>";
                setInfo(info);
            }
        }
    } else {               // Find failure.
        if (wrapAround) {  // No found in the whole text.
            auto info =
                QString("<b><font color=#67A9FF size=4>") + tr("No more occurrence to replace.") + "</font></b>";
            setInfo(info);
        } else {  // Not found from current cursor to TOP or BOTTOM.
            auto info = QString("<b><font color=#67A9FF size=4>") + tr("No occurrence to replace, the ") + endStr +
                        tr(" has been reached.") + "</font></b>";
            setInfo(info);
        }
    }
}

// Replace all 'target' with 'text'.
int Searcher::ReplaceAll(const QString &target, const QString &text) {
    if (editView() == nullptr) {
        return 0;
    }
    // Move the search start pos to the start of selection if has selection.
    if (editView()->textCursor().hasSelection()) {
        int start = editView()->textCursor().selectionStart();
        auto cursor = editView()->textCursor();
        cursor.setPosition(start);
        editView()->setTextCursor(cursor);
    }

    // Handle \r, \n, and \t.
    auto extendedText = text;
    if (radioButtonFindExtended_) {
        HandleEscapeChars(extendedText);
    }

    // Reserve the original cursor firstly.
    QTextCursor originalCursor = editView()->textCursor();

    // To replace all targets.
    editView()->textCursor().beginEditBlock();
    auto res = FindAll(target);
    for (auto it = res.rbegin(); it != res.rend(); ++it) {
        it->insertText(extendedText);
    }
    editView()->textCursor().endEditBlock();

    editView()->setTextCursor(originalCursor);
    return res.size();
}

void Searcher::setRadioButtonFindRe(bool value) { radioButtonFindRe_ = value; }

void Searcher::setRadioButtonFindExtended(bool value) { radioButtonFindExtended_ = value; }

void Searcher::setRadioButtonFindNormal(bool value) { radioButtonFindNormal_ = value; }

void Searcher::setCheckBoxFindWrapAround(bool value) { checkBoxFindWrapAround_ = value; }

void Searcher::setCheckBoxFindMatchCase(bool value) { checkBoxFindMatchCase_ = value; }

void Searcher::setCheckBoxFindWholeWord(bool value) { checkBoxFindWholeWord_ = value; }

void Searcher::setCheckBoxFindBackward(bool value) { checkBoxFindBackward_ = value; }

void SearchDialog::on_checkBoxFindBackward_toggled(bool checked) { Settings().Set("searcher", "backward", checked); }

void SearchDialog::on_checkBoxFindWholeWord_toggled(bool checked) { Settings().Set("searcher", "whole_word", checked); }

void SearchDialog::on_checkBoxFindMatchCase_toggled(bool checked) {
    Settings().Set("searcher", "case_sensitive", checked);
}

void SearchDialog::on_checkBoxFindWrapAround_toggled(bool checked) {
    Settings().Set("searcher", "wrap_around", checked);
}

void SearchDialog::on_radioButtonFindNormal_toggled(bool checked) {
    if (checked) {
        Settings().Set("searcher", "search_mode", 0);
    }
}

void SearchDialog::on_radioButtonFindExtended_toggled(bool checked) {
    if (checked) {
        Settings().Set("searcher", "search_mode", 1);
    }
}

void SearchDialog::on_radioButtonFindRe_toggled(bool checked) {
    if (checked) {
        ui_->checkBoxFindWholeWord->setDisabled(true);
        ui_->checkBoxFindWholeWord->setHidden(true);
        ui_->checkBoxFindMatchCase->setDisabled(true);
        ui_->checkBoxFindMatchCase->setHidden(true);
        Settings().Set("searcher", "search_mode", 2);
    } else {
        ui_->checkBoxFindWholeWord->setEnabled(true);
        ui_->checkBoxFindWholeWord->setVisible(true);
        ui_->checkBoxFindMatchCase->setEnabled(true);
        ui_->checkBoxFindMatchCase->setVisible(true);
    }
}
}  // namespace QEditor
