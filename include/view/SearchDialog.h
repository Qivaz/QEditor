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

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include "EditView.h"
#include "MainTabView.h"
#include "SearchResultList.h"

namespace Ui {
class UISearchDialog;
}

namespace QEditor {
class SearchDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SearchDialog(QWidget *parent = nullptr, int index = 0);
    ~SearchDialog();

    void Start(int index);

    int currentTabIndex();
    void setCurrentTabIndex(int index);

    EditView* editView();
    TabView* tabView();

    QTextCursor FindNext(const QString &text, const QTextCursor &startCursor);
    QTextCursor FindPrevious(const QString &text, const QTextCursor &startCursor);

private slots:
    void on_pushButtonFindFindNext_clicked();

    void on_radioButtonFindRe_toggled(bool checked);

    void on_pushButtonFindFindAllInCurrent_clicked();

    void on_pushButtonReplaceCancel_clicked();

    void on_pushButtonFindCancel_clicked();

    void on_pushButtonFindCount_clicked();

    void on_pushButtonReplaceFindNext_clicked();

    void on_pushButtonReplaceReplace_clicked();

    void on_pushButtonReplaceReplaceAll_clicked();

    void on_lineEditFindFindWhat_textChanged(const QString &arg1);

    void on_lineEditReplaceFindWhat_textChanged(const QString &arg1);

private:
    void ReplaceInsensitiveStr(QString &str, const QString &target);
    const QString GetSelectedText();


    QTextCursor FindNext(const QString &text, const QTextCursor &startCursor, bool backward);
    QTextCursor FindPrevious(const QString &text, const QTextCursor &startCursor, bool backward);

    bool Find(const QStringList &target, const QTextCursor &startCursor, QTextCursor &targetCursor, bool backward);
    template <class T>
    bool Find(const T &target, const QTextCursor &startCursor, QTextCursor &targetCursor, bool backward);
    std::vector<QTextCursor> FindAll(const QString &target);

    void Replace(const QString &target, const QString &text, bool backward);
    int ReplaceAll(const QString &target, const QString &text);

private:
    Ui::UISearchDialog *ui_;
    SearchResultList *searchResultList_{nullptr};
};
}  // namespace QEditor

#endif // DIALOG_H
