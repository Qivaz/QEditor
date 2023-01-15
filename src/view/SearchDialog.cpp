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

SearchDialog::SearchDialog(QWidget *parent, int index) :
    QDialog(parent),
    ui_(new Ui::UISearchDialog)
{
    ui_->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    qreal opa = Settings().Get("window", "opacity").toDouble();
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

TabView* SearchDialog::tabView() { return MainWindow::Instance().tabView(); }

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

    if (index == 0) {  // Find.
        ui_->lineEditFindFindWhat->setText(GetSelectedText());
    } else {  // Replace.
        ui_->lineEditReplaceFindWhat->setText(GetSelectedText());
    }
    setCurrentTabIndex(index);
    show();
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

template <class T>
bool SearchDialog::Find(const T &target) {
    int flag = 0;
    if (ui_->checkBoxFindBackward->isChecked()) {
        flag |= QTextDocument::FindFlag::FindBackward;
    }
    if (ui_->checkBoxFindMatchCase->isChecked()) {
        flag |= QTextDocument::FindFlag::FindCaseSensitively;
    }
    if (ui_->checkBoxFindWholeWord->isChecked()) {
        flag |= QTextDocument::FindFlag::FindWholeWords;
    }

    bool res = false;
    if (editView()->find(target, QTextDocument::FindFlags(flag))) {
        res = true;
        return res;
    }

    bool atStart = true;  // TODO: Only re-find if not at start position.
    if (ui_->checkBoxFindWrapAround->isChecked() && atStart) {
        auto currentCursor = editView()->textCursor();
        QScrollBar *scrollBar = editView()->verticalScrollBar();
        auto sliderPos = scrollBar->sliderPosition();
        if (ui_->checkBoxFindBackward->isChecked()) {
            editView()->moveCursor(QTextCursor::End);
        } else {
            editView()->moveCursor(QTextCursor::Start);
        }
        if (editView()->find(target, QTextDocument::FindFlags(flag))) {
            res = true;
        } else {
            // If not found, recover the text cursor.
            editView()->setTextCursor(currentCursor);
            scrollBar->setSliderPosition(sliderPos);
        }
    }
    return res;
}

void SearchDialog::FindNext(const QString &text)
{
    bool res;
    if (ui_->radioButtonFindRe->isChecked()) {
        const QRegExp reTarget = QRegExp(text);
        res = Find<QRegExp>(reTarget);
    } else {
        res = Find<QString>(text);
    }

    if (res) {
        qDebug() << "SearchDialog::on_pushButtonFindFindNext_clicked, found " << text;
    } else {
        qDebug() << "SearchDialog::on_pushButtonFindFindNext_clicked, not found " << text;
    }
    qDebug() << "SearchDialog::on_pushButtonFindFindNext_clicked, count: " << editView()->blockCount();
}

void SearchDialog::on_pushButtonFindFindNext_clicked()
{   
    auto const &target = ui_->lineEditFindFindWhat->text();
    FindNext(target);
}

void SearchDialog::on_radioButtonFindRe_toggled(bool checked)
{
    if (checked) {
        ui_->checkBoxFindWholeWord->setDisabled(true);
        ui_->checkBoxFindMatchCase->setDisabled(true);
    }
}

template <class T>
std::vector<QTextCursor> SearchDialog::FindAll(const T &target) {
    int flag = 0;
    if (ui_->checkBoxFindBackward->isChecked()) {
        flag |= QTextDocument::FindFlag::FindBackward;
    }
    if (ui_->checkBoxFindMatchCase->isChecked()) {
        flag |= QTextDocument::FindFlag::FindCaseSensitively;
    }
    if (ui_->checkBoxFindWholeWord->isChecked()) {
        flag |= QTextDocument::FindFlag::FindWholeWords;
    }

    std::vector<QTextCursor> cursors;
    QTextCursor cursor = editView()->textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    bool finish = cursor.isNull();
    while (!finish) {
        cursor = editView()->document()->find(target, cursor, QTextDocument::FindFlags(flag));
        finish = cursor.isNull();
        if (!finish) {
            cursors.emplace_back(cursor);
        }
    }
    return cursors;
}

void SearchDialog::ReplaceInsensitiveStr(QString &text, const QString &target)
{
    int index = 0;
    qDebug() << "text: " << text;
    while((index = text.indexOf(target, index, Qt::CaseInsensitive)) != -1) {
        auto substr = text.mid(index, target.length());
        auto tr = QString("<font color = #BCE08C>") + substr + QString("</font>");
        text.replace(index, substr.length(), tr);
        index += tr.length();
        qDebug() << "index: " << index;
    }
}

void SearchDialog::on_pushButtonFindFindAllInCurrent_clicked()
{
    if (searchResultList_ == nullptr) {
        searchResultList_ = new SearchResultList(tabView());
        auto dockView = MainWindow::Instance().CreateSearchDockView();
        dockView->setWidget(searchResultList_);
        searchResultList_->setParent(dockView);
        searchResultList_->setFont(QFont("Consolas", 11));
    }
    MainWindow::Instance().ShowSearchDockView();

    auto sessionItem = searchResultList_->StartSearchSession(editView());
    qDebug() << "Find all start....";
    std::vector<QTextCursor> res;
    auto const &target = ui_->lineEditFindFindWhat->text();
    if (ui_->radioButtonFindRe->isChecked()) {
        const QRegExp reTarget = QRegExp(target);
        res = FindAll<QRegExp>(reTarget);
    } else {
        res = FindAll<QString>(target);
    }
    qDebug() << "Find all finish....";
    int matchCount = 0;

    // TODO:
    // To support display dynamic visible list items, and do fetchMore for user scrolling.
    for (const auto &item : res) {
        auto currentBlock = item.block();
        int lineNum = item.blockNumber();
        auto highlighting_target = item.selectedText().toHtmlEscaped();
        auto htmlTarget = QString("<font color=#BCE08C>") + highlighting_target + QString("</font>");
        auto currentStr = currentBlock.text();
        auto escapedStr = currentStr.toHtmlEscaped();
        escapedStr.replace(highlighting_target, htmlTarget, Qt::CaseSensitive);
        qDebug() << "Line " << lineNum << ": highlighting_target: " << highlighting_target << ", htmlTarget: " << htmlTarget
                   << ", currentStr: " << escapedStr;

        auto text = /*QString("<html>") + */"Line " + QString::number(lineNum + 1) + ":  " + escapedStr/* + "</html>"*/;
//        const auto &text = item.selectedText();
        qDebug() << "Line " << lineNum << ": text: " << text;
        ++matchCount;
        searchResultList_->AddSearchResult(sessionItem, lineNum, text, item);
    }
    searchResultList_->FinishSearchSession(sessionItem, QString("(Search \"") + target + "\": " + QString::number(matchCount) + " hits)");
    ++sessionCount_;
    searchResultList_->UpdateTopTitle(QString::number(sessionCount_) + " results:");
}

void SearchDialog::on_pushButtonFindCount_clicked()
{
    std::vector<QTextCursor> res;
    auto const &target = ui_->lineEditFindFindWhat->text();
    if (ui_->radioButtonFindRe->isChecked()) {
        const QRegExp reTarget = QRegExp(target);
        res = FindAll<QRegExp>(reTarget);
    } else {
        res = FindAll<QString>(target);
    }
    auto info = QString("<b><font color=#67A9FF size=4>") + QString::number(res.size()) +
                " matches in " + editView()->fileName() + "</font></b>";
    ui_->labelInfo->setText(info);
}

void SearchDialog::on_pushButtonFindCancel_clicked()
{
    hide();
}

void SearchDialog::on_pushButtonReplaceCancel_clicked()
{
    hide();
}

void SearchDialog::on_pushButtonReplaceFindNext_clicked()
{
    auto const &target = ui_->lineEditReplaceFindWhat->text();
    FindNext(target);
}

// Replace 'target' with 'text'.
template <class T>
bool SearchDialog::Replace(const T &target, const QString &text) {
    // Check find options.
    bool wrapAround = (ui_->checkBoxFindWrapAround->isChecked());
    QString startStr;
    QString endStr;
    if (ui_->checkBoxFindBackward->isChecked()) {
        startStr = "bottom";
        endStr = "top";
    } else {
        startStr = "top";
        endStr = "bottom";
    }

    // Move the search start pos to the start of selection if has selection.
    if (editView()->textCursor().hasSelection()) {
        int start;
        if (ui_->checkBoxFindBackward->isChecked()) {
            start = editView()->textCursor().selectionEnd();
        } else {
            start = editView()->textCursor().selectionStart();
        }
        auto cursor = editView()->textCursor();
        cursor.setPosition(start);
        editView()->setTextCursor(cursor);
    }

    // To find and replace.
    bool res = Find(target);
    if (res) {  // Replace success.
        QTextCursor cursor = editView()->textCursor();
        cursor.insertText(text);
        res = Find(target);
        if (res) {
            auto info = QString("<b><font color=#67A9FF size=4>") +
                                "1 occurrence were replaced" +
                                ", to continue replacing.</font></b>";
            ui_->labelInfo->setText(info);
        } else {
            if (wrapAround) {
                auto info = QString("<b><font color=#67A9FF size=4>") +
                                    "1 occurrence were replaced. No more occurrence to replace." + "</font></b>";
                ui_->labelInfo->setText(info);
            } else {
                auto info = QString("<b><font color=#67A9FF size=4>") +
                                    "1 occurrence were replaced." + endStr +
                                    " has been reached." + "</font></b>";
                ui_->labelInfo->setText(info);
            }
        }
    } else {  // Replace failure.
        if (wrapAround) {  // No found in the whole text.
            auto info = QString("<b><font color=#67A9FF size=4>") +
                                "No more occurrence to replace." + "</font></b>";
            ui_->labelInfo->setText(info);
        } else {  // Not found from current cursor to TOP or BOTTOM.
            auto info = QString("<b><font color=#67A9FF size=4>") +
                                "No occurrence to replace, the " + endStr + " has been reached." + "</font></b>";
            ui_->labelInfo->setText(info);
        }
    }
    return res;
}

void SearchDialog::on_pushButtonReplaceReplace_clicked()
{
    bool res;
    auto const &target = ui_->lineEditReplaceFindWhat->text();
    auto const &text = ui_->lineEditReplaceReplaceWith->text();
    if (ui_->radioButtonFindRe->isChecked()) {
        const QRegExp reTarget = QRegExp(target);
        res = Replace<QRegExp>(reTarget, text);
    } else {
        res = Replace<QString>(target, text);
    }

    if (res) {
        qDebug() << "SearchDialog::on_pushButtonReplaceReplace_clicked, found, " << target << " -> "<< text;
    } else {
        qDebug() << "SearchDialog::on_pushButtonReplaceReplace_clicked, not found " << target;
    }
    qDebug() << "SearchDialog::on_pushButtonReplaceReplace_clicked, count: " << editView()->blockCount();
}


// Replace all 'target' with 'text'.
template <class T>
int SearchDialog::ReplaceAll(const T &target, const QString &text) {
    int flag = 0;
    if (ui_->checkBoxFindBackward->isChecked()) {
        flag |= QTextDocument::FindFlag::FindBackward;
    }
    if (ui_->checkBoxFindMatchCase->isChecked()) {
        flag |= QTextDocument::FindFlag::FindCaseSensitively;
    }
    if (ui_->checkBoxFindWholeWord->isChecked()) {
        flag |= QTextDocument::FindFlag::FindWholeWords;
    }

    // Move the search start pos to the start of selection if has selection.
    if (editView()->textCursor().hasSelection()) {
        int start = editView()->textCursor().selectionStart();
        auto cursor = editView()->textCursor();
        cursor.setPosition(start);
        editView()->setTextCursor(cursor);
    }
    // Reserve the original cursor firstly.
    QTextCursor originalCursor = editView()->textCursor();

    // To replace all targets.
    int count = 0;
    editView()->textCursor().beginEditBlock();
    QTextCursor cursor = editView()->textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    while (true) {
        cursor = editView()->document()->find(target, cursor, QTextDocument::FindFlags(flag));
        if (cursor.isNull()) {
            break;
        }
        cursor.insertText(text);
        ++count;
    }
    editView()->textCursor().endEditBlock();

    editView()->setTextCursor(originalCursor);
    return count;
}

void SearchDialog::on_pushButtonReplaceReplaceAll_clicked()
{
    int count;
    auto const &target = ui_->lineEditReplaceFindWhat->text();
    auto const &text = ui_->lineEditReplaceReplaceWith->text();
    if (ui_->radioButtonFindRe->isChecked()) {
        const QRegExp reTarget = QRegExp(target);
        count = ReplaceAll<QRegExp>(reTarget, text);
    } else {
        count = ReplaceAll<QString>(target, text);
    }
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
