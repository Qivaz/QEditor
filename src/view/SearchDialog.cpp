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
#include "ui_SearchDialog.h"

#include <QScrollBar>
#include <QTextBlock>
#include <SearchDialog.h>

#include "MainWindow.h"
#include "Settings.h"
#include "Logger.h"

#ifdef Q_OS_WIN
namespace WinTheme {
extern bool IsDarkTheme();
extern void SetDark_qApp();
extern void SetDarkTitleBar(HWND hwnd);
}
#endif

namespace QEditor {
SearchDialog::SearchDialog(QWidget *parent, int index) :
    QDialog(parent),
    ui_(new Ui::UISearchDialog())
{
    ui_->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    qreal opa = Settings().Get("dialog", "opacity", 0.9).toDouble();
    setWindowOpacity(opa * 0.8);

    ui_->tabWidget->setCurrentIndex(index);
    if (index == 0) {  // Find
        ui_->lineEditFindFindWhat->setFocus();
    } else {  // Replace
        ui_->lineEditReplaceFindWhat->setFocus();
    }
    ui_->checkBoxFindWrapAround->setChecked(true);

    ui_->lineEditFindFindWhat->setText(GetSelectedText());
    ui_->lineEditReplaceFindWhat->setText(GetSelectedText());
}

SearchDialog::~SearchDialog()
{
    delete ui_;
}

EditView* SearchDialog::editView() { return MainWindow::Instance().editView(); }

void SearchDialog::Start(int index)
{
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
    } else {  // Replace.
        ui_->lineEditReplaceFindWhat->setText(GetSelectedText());
    }
    setCurrentTabIndex(index);
    show();
}

void SearchDialog::InitSetting()
{
    searcher_->setCheckBoxFindBackward(ui_->checkBoxFindBackward->isChecked());
    searcher_->setCheckBoxFindWholeWord(ui_->checkBoxFindWholeWord->isChecked());
    searcher_->setCheckBoxFindMatchCase(ui_->checkBoxFindMatchCase->isChecked());
    searcher_->setCheckBoxFindWrapAround(ui_->checkBoxFindWrapAround->isChecked());
    searcher_->setRadioButtonFindNormal(ui_->radioButtonFindNormal->isChecked());
    searcher_->setRadioButtonFindExtended(ui_->radioButtonFindExtended->isChecked());
    searcher_->setRadioButtonFindRe(ui_->radioButtonFindRe->isChecked());
}

int SearchDialog::currentTabIndex()
{
    return ui_->tabWidget->currentIndex();
}

void SearchDialog::setCurrentTabIndex(int index)
{
    ui_->tabWidget->setCurrentIndex(index);
}

const QString SearchDialog::GetSelectedText()
{
    auto textCursor = editView()->textCursor();
    if (!textCursor.hasSelection()) {
        textCursor.select(QTextCursor::WordUnderCursor);
    }
    auto text = textCursor.selectedText();
    qDebug() << ", selectedText: " << text;
    return text;
}

void SearchDialog::on_pushButtonFindFindNext_clicked()
{
    auto const &target = ui_->lineEditFindFindWhat->text();
    InitSetting();
    auto cursor = searcher_->FindNext(target, editView()->textCursor(), ui_->checkBoxFindBackward->isChecked());
    if (!cursor.isNull()) {
        editView()->setTextCursor(cursor);
    }
}

void SearchDialog::on_radioButtonFindRe_toggled(bool checked)
{
    if (checked) {
        ui_->checkBoxFindWholeWord->setDisabled(true);
        ui_->checkBoxFindMatchCase->setDisabled(true);
    }
}

void SearchDialog::on_pushButtonFindFindAllInCurrent_clicked()
{
    if (searchResultList_ == nullptr) {
        searchResultList_ = MainWindow::Instance().GetSearchResultList();
    }
    MainWindow::Instance().ShowSearchDockView();

    auto sessionItem = searchResultList_->StartSearchSession(editView());
    qDebug() << "Find all start....";
    std::vector<QTextCursor> res;
    auto const &target = ui_->lineEditFindFindWhat->text();
    InitSetting();
    res = searcher_->FindAll(target);
    qDebug() << "Find all finish....";
    int matchCount = 0;

    // TODO:
    // To support display dynamic visible list items, and do fetchMore for user scrolling.
    for (const auto &item : res) {
        qDebug() << "Display item start....";
        const auto currentBlock = item.block();
        int lineNum = item.blockNumber();
        const auto highlightingTarget = item.selectedText().toHtmlEscaped();
        const auto htmlTarget = QString("<span style=\"font-size:14px;font-family:Consolas;color:#BCE08C\">") + highlightingTarget + QString("</span>");
        const auto plainText = currentBlock.text();
        auto escapedStr = plainText.toHtmlEscaped();
        escapedStr.replace(highlightingTarget, htmlTarget, Qt::CaseSensitive);
        qDebug() << "Line " << lineNum << ": highlightingTarget: " << highlightingTarget << ", htmlTarget: " << htmlTarget
                 << ", currentStr: " << escapedStr;

        auto htmlText = QString("<div style=\"font-size:14px;font-family:Consolas;color:#BEBEBE\">") +
                    "Line " + QString("<span style=\"font-size:14px;font-family:Consolas;color:#2891AF\">") +
                    QString::number(lineNum + 1) + QString("</span>") + ":  " + escapedStr + QString("</div>");
        qDebug() << "Line " << lineNum << ": text: " << htmlText;
        ++matchCount;
        searchResultList_->AddSearchResult(sessionItem, lineNum, htmlText, plainText, item);
        qDebug() << "Display item end....";
    }
    searchResultList_->FinishSearchSession(sessionItem, target, matchCount);
}

void SearchDialog::on_pushButtonFindCount_clicked()
{
    std::vector<QTextCursor> res;
    auto const &target = ui_->lineEditFindFindWhat->text();
    InitSetting();
    res = searcher_->FindAll(target);
    auto info = QString("<b><font color=#67A9FF size=4>") + QString::number(res.size()) +
                " matches in " + editView()->fileName() + "</font></b>";
    ui_->labelInfo->setText(info);
}

void SearchDialog::on_pushButtonFindCancel_clicked()
{
    close();
}

void SearchDialog::on_pushButtonReplaceCancel_clicked()
{
    close();
}

void SearchDialog::on_pushButtonReplaceFindNext_clicked()
{
    auto const &target = ui_->lineEditReplaceFindWhat->text();
    InitSetting();
    auto res = searcher_->FindNext(target, editView()->textCursor(), ui_->checkBoxFindBackward->isChecked());
    if (!res.isNull()) {
        editView()->setTextCursor(res);
    }
}

void SearchDialog::on_pushButtonReplaceReplace_clicked()
{
    auto const &target = ui_->lineEditReplaceFindWhat->text();
    auto const &text = ui_->lineEditReplaceReplaceWith->text();
    InitSetting();
    searcher_->setInfo("");
    searcher_->Replace(target, text, ui_->checkBoxFindBackward->isChecked());
    ui_->labelInfo->setText(searcher_->info());
}

void SearchDialog::on_pushButtonReplaceReplaceAll_clicked()
{
    int count;
    auto const &target = ui_->lineEditReplaceFindWhat->text();
    auto const &text = ui_->lineEditReplaceReplaceWith->text();
    InitSetting();
    count = searcher_->ReplaceAll(target, text);
    auto info = QString("<b><font color=#67A9FF size=4>") + QString::number(count) +
                " occurrences were replaced in " + editView()->fileName() + "</font></b>";
    ui_->labelInfo->setText(info);
}

void SearchDialog::on_lineEditFindFindWhat_textChanged(const QString &)
{
    ui_->labelInfo->clear();
}

void SearchDialog::on_lineEditReplaceFindWhat_textChanged(const QString &)
{
    ui_->labelInfo->clear();
}

EditView* Searcher::editView() { return MainWindow::Instance().editView(); }

TabView* Searcher::tabView() { return MainWindow::Instance().tabView(); }

QString Searcher::info() const
{
    return info_;
}

void Searcher::setInfo(const QString &info)
{
    info_ = info;
}

bool Searcher::Find(const QStringList &target, const QTextCursor &startCursor, QTextCursor &targetCursor, bool backward) {
    int firstStart = -1;
    QTextCursor currentCursor = startCursor;
    while (true) {
        // The first block.
        int count = 0;
        QTextCursor cursor;
        if (!Find(target[0], currentCursor, cursor, backward)) {
            qDebug();
            return false;
        }
        if (!cursor.hasSelection()) {
            qDebug();
            return false;
        }
        count += target[0].length();

        // Have searched all content, finish.
        if (cursor.selectionStart() == firstStart) {
            qDebug();
            break;
        }
        if (firstStart == -1) {
            firstStart = cursor.selectionStart();
        }

        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();
        qDebug() << "start: " << start << editView()->document()->characterAt(start)
                    << ", end: " << end << editView()->document()->characterAt(end);
        if (target.length() > 1 &&
            editView()->document()->characterAt(end) != '\n' &&
            editView()->document()->characterAt(end) != "\u2029") {
            qDebug();
            currentCursor = cursor;
            continue;
        }
        ++count;

        // The middle blocks.
        auto block = cursor.block();
        bool match = true;
        block = block.next();
        for (int i = 1; i < target.length() - 1; ++i) {
            qDebug() << "block text: " << block.text() << ", " << block.blockNumber();
            qDebug() << "target[i]: " << target[i];
            if (block.text() != target[i]) {
                match = false;
                break;
            }
            count += target[i].length() + 1;
            block = block.next();
        }
        if (!match) {
            qDebug();
            currentCursor = cursor;
            continue;
        }

        // The last block.
        if (target.length() > 1) {
            if (!block.text().startsWith(target[target.length() - 1])) {
                qDebug();
                currentCursor = cursor;
                continue;
            }
            count += target[target.length() - 1].length();
        }

        // Found.
        cursor.setPosition(start, QTextCursor::MoveAnchor);
        for (int i = 0; i < count; ++i) {
            bool res = cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            if (!res) {
                qDebug();
                return false;
            }
        }
        targetCursor = cursor;
        qDebug() << "cursor: " << cursor.position();
        return true;
    }
    return false;
}

template <class T>
bool Searcher::Find(const T &target, const QTextCursor &startCursor, QTextCursor &targetCursor, bool backward) {
    int flag = 0;
    if (backward) {
        flag |= QTextDocument::FindFlag::FindBackward;
    }
    if (checkBoxFindMatchCase) {
        flag |= QTextDocument::FindFlag::FindCaseSensitively;
    }
    if (checkBoxFindWholeWord) {
        flag |= QTextDocument::FindFlag::FindWholeWords;
    }

    bool res = false;
    auto cursor = editView()->document()->find(target, startCursor, QTextDocument::FindFlags(flag));
    if (!cursor.isNull()) {
        targetCursor = cursor;
        res = true;
        return res;
    }

    bool atStart = true;  // TODO: Only re-find if not at start position.
    if (checkBoxFindWrapAround && atStart) {
        QScrollBar *scrollBar = editView()->verticalScrollBar();
        auto sliderPos = scrollBar->sliderPosition();
        QTextCursor anewCursor = startCursor;
        if (backward) {
            anewCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        } else {
            anewCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
        }
        auto cursor = editView()->document()->find(target, anewCursor, QTextDocument::FindFlags(flag));
        if (!cursor.isNull()) {
            targetCursor = cursor;
            res = true;
        } else {
            // If not found, recover the text cursor.
            scrollBar->setSliderPosition(sliderPos);
        }
    }
    return res;
}

static void HandleEscapeChars(QString &text)
{
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

QTextCursor Searcher::FindPrevious(const QString &text, const QTextCursor &startCursor)
{
    bool backwardCheck = checkBoxFindBackward;
    return FindNext(text, startCursor, !backwardCheck);
}

QTextCursor Searcher::FindPrevious(const QString &text, const QTextCursor &startCursor, bool backward)
{
    bool backwardCheck = checkBoxFindBackward;
    return FindNext(text, startCursor, backward ? backwardCheck : !backwardCheck);
}

QTextCursor Searcher::FindNext(const QString &text, const QTextCursor &startCursor)
{
    bool backwardCheck = checkBoxFindBackward;
    return FindNext(text, startCursor, backwardCheck);
}

// Should save original cursor, and restore it if failed.
QTextCursor Searcher::FindNext(const QString &text, const QTextCursor &startCursor, bool backward)
{
    MainWindow::Instance().setSearchingString(text);
    bool res;
    QTextCursor cursor;
    if (radioButtonFindRe) {
        const QRegExp reTarget = QRegExp(text);
        res = Find<QRegExp>(reTarget, startCursor, cursor, backward);
    } else {
        if (radioButtonFindExtended) {
            auto extendedText = text;
            HandleEscapeChars(extendedText);
            qDebug() << extendedText;
            auto extendedTexts = extendedText.split("\n");
            qDebug() << extendedTexts;
            res = Find(extendedTexts, startCursor, cursor, backward);
        } else {
            res = Find<QString>(text, startCursor, cursor, backward);
        }
    }

    if (res) {
        qDebug() << "SearchDialog::on_pushButtonFindFindNext_clicked, found " << text;
        return cursor;
    } else {
        qDebug() << "SearchDialog::on_pushButtonFindFindNext_clicked, not found " << text;
        return QTextCursor();
    }
}

std::vector<QTextCursor> Searcher::FindAll(const QString &target) {
    std::vector<QTextCursor> cursors;
    QTextCursor savedCursor = editView()->textCursor();
    QTextCursor cursor = editView()->textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    int firstStart = -1;
    while (true) {
        qDebug();
        cursor = FindNext(target, cursor);
        if (!cursor.isNull()) {
            if (cursor.selectionStart() == firstStart) {
                break;
            }
            if (firstStart == -1) {
                firstStart = cursor.selectionStart();
            }
            cursors.emplace_back(cursor);
        } else {
            break;
        }
    }
    editView()->setTextCursor(savedCursor);
    return cursors;
}

// Replace 'target' with 'text'.
void Searcher::Replace(const QString &target, const QString &text, bool backward) {
    // Check find options.
    bool wrapAround = (checkBoxFindWrapAround);
    QString startStr;
    QString endStr;
    if (backward) {
        startStr = "bottom";
        endStr = "top";
    } else {
        startStr = "top";
        endStr = "bottom";
    }

    // Handle \r, \n, and \t.
    auto extendedText = text;
    if (radioButtonFindExtended) {
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
    auto res = FindNext(target, editView()->textCursor(), backward);
    if (!res.isNull()) {  // Find success.
        editView()->setTextCursor(res);
        res.insertText(extendedText);
        res = FindNext(target, res, backward);
        if (!res.isNull()) {
            editView()->setTextCursor(res);
            auto info = QString("<b><font color=#67A9FF size=4>") +
                                "1 occurrence were replaced" +
                                ", to continue replacing.</font></b>";
            setInfo(info);
        } else {
            if (wrapAround) {
                auto info = QString("<b><font color=#67A9FF size=4>") +
                                    "1 occurrence were replaced. No more occurrence to replace." + "</font></b>";
                setInfo(info);
            } else {
                auto info = QString("<b><font color=#67A9FF size=4>") +
                                    "1 occurrence were replaced." + endStr +
                                    " has been reached." + "</font></b>";
                setInfo(info);
            }
        }
    } else {  // Find failure.
        if (wrapAround) {  // No found in the whole text.
            auto info = QString("<b><font color=#67A9FF size=4>") +
                                "No more occurrence to replace." + "</font></b>";
            setInfo(info);
        } else {  // Not found from current cursor to TOP or BOTTOM.
            auto info = QString("<b><font color=#67A9FF size=4>") +
                                "No occurrence to replace, the " + endStr + " has been reached." + "</font></b>";
            setInfo(info);
        }
    }
}

// Replace all 'target' with 'text'.
int Searcher::ReplaceAll(const QString &target, const QString &text) {
    // Move the search start pos to the start of selection if has selection.
    if (editView()->textCursor().hasSelection()) {
        int start = editView()->textCursor().selectionStart();
        auto cursor = editView()->textCursor();
        cursor.setPosition(start);
        editView()->setTextCursor(cursor);
    }

    // Handle \r, \n, and \t.
    auto extendedText = text;
    if (radioButtonFindExtended) {
        HandleEscapeChars(extendedText);
    }

    // Reserve the original cursor firstly.
    QTextCursor originalCursor = editView()->textCursor();

    // To replace all targets.
    auto res = FindAll(target);
    for (auto it = res.rbegin(); it != res.rend(); ++it) {
        it->insertText(extendedText);
    }

    editView()->setTextCursor(originalCursor);
    return res.size();
}

void Searcher::setRadioButtonFindRe(bool value)
{
    radioButtonFindRe = value;
}

void Searcher::setRadioButtonFindExtended(bool value)
{
    radioButtonFindExtended = value;
}

void Searcher::setRadioButtonFindNormal(bool value)
{
    radioButtonFindNormal = value;
}

void Searcher::setCheckBoxFindWrapAround(bool value)
{
    checkBoxFindWrapAround = value;
}

void Searcher::setCheckBoxFindMatchCase(bool value)
{
    checkBoxFindMatchCase = value;
}

void Searcher::setCheckBoxFindWholeWord(bool value)
{
    checkBoxFindWholeWord = value;
}

void Searcher::setCheckBoxFindBackward(bool value)
{
    checkBoxFindBackward = value;
}
}  // namespace QEditor
