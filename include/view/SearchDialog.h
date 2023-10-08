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

#include "EditView.h"
#include "MainTabView.h"
#include "SearchResultList.h"
#include <QDialog>
#include <QProgressDialog>

namespace Ui {
class UISearchDialog;
}

namespace QEditor {
class Searcher;

class SearchDialog : public QDialog {
    Q_OBJECT
   public:
    explicit SearchDialog(QWidget *parent = nullptr, int index = 0);
    ~SearchDialog();

    void Start(int index);

    int currentTabIndex();
    void setCurrentTabIndex(int index);

   protected:
    void closeEvent(QCloseEvent *) override;
    void hideEvent(QHideEvent *event) override;

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

    void on_checkBoxFindBackward_toggled(bool checked);

    void on_checkBoxFindWholeWord_toggled(bool checked);

    void on_checkBoxFindMatchCase_toggled(bool checked);

    void on_checkBoxFindWrapAround_toggled(bool checked);

    void on_radioButtonFindNormal_toggled(bool checked);

    void on_radioButtonFindExtended_toggled(bool checked);

   private:
    void InitSetting();
    EditView *editView();
    const QString GetSelectedText();

   private:
    Ui::UISearchDialog *ui_;
    SearchResultList *searchResultList_{nullptr};
    Searcher *searcher_{nullptr};

    QMenu *historyMenu_;
    QLineEdit *historyLineEdit_{nullptr};

    int historyIndex_{-1};
    QString searchInput_;
};

class Searcher : public QObject {
    Q_OBJECT
   public:
    Searcher() = default;
    ~Searcher() = default;

    QTextCursor FindNext(const QString &text, const QTextCursor &startCursor);
    QTextCursor FindPrevious(const QString &text, const QTextCursor &startCursor);

    std::vector<QTextCursor> FindAll(const QString &target,
                                     std::function<bool(int)> progressCallback = std::function<bool(int)>());
    std::vector<int> FindAllLineNum(const QString &target);

    void Replace(const QString &target, const QString &text, bool backward);
    int ReplaceAll(const QString &target, const QString &text);

    void setCheckBoxFindBackward(bool value);

    void setCheckBoxFindWholeWord(bool value);

    void setCheckBoxFindMatchCase(bool value);

    void setCheckBoxFindWrapAround(bool value);

    void setRadioButtonFindNormal(bool value);

    void setRadioButtonFindExtended(bool value);

    void setRadioButtonFindRe(bool value);

    QString info() const;
    void setInfo(const QString &info);

   private:
    QTextCursor _FindNext(const QString &text, const QTextCursor &startCursor, bool backward);
    QTextCursor _FindPrevious(const QString &text, const QTextCursor &startCursor, bool backward);

    bool _Find(const QStringList &target, const QTextCursor &startCursor, QTextCursor &targetCursor, bool backward);
    template <class T>
    bool _Find(const T &target, const QTextCursor &startCursor, QTextCursor &targetCursor, bool backward,
               bool first = true);

    EditView *editView();
    TabView *tabView();

   private:
    bool checkBoxFindBackward_;
    bool checkBoxFindWholeWord_;
    bool checkBoxFindMatchCase_;
    bool checkBoxFindWrapAround_;

    bool radioButtonFindNormal_;
    bool radioButtonFindExtended_;
    bool radioButtonFindRe_;

    QString info_;
};

// template<typename ...ARGS>
class LambdaEventFilter : public QObject {
    Q_OBJECT
   public:
    bool eventFilter(QObject *watched, QEvent *event) override { return lambdaContainer->CallLambda(watched, event); }

    template <typename LAMBDA>
    LambdaEventFilter(QObject *parent, LAMBDA lambda /*, ARGS... args*/) : QObject(parent) {
        lambdaContainer = new LambdaContainer<LAMBDA>(lambda /*, args...*/);
    }

    ~LambdaEventFilter() override {
        if (lambdaContainer != nullptr) {
            delete lambdaContainer;
            lambdaContainer = nullptr;
        }
    }

   private:
    class AbstractLambdaContainer : public QObject {
       public:
        virtual bool CallLambda(QObject *watched, QEvent *event) = 0;
    };

    template <typename LAMBDA>
    class LambdaContainer : public AbstractLambdaContainer {
       public:
        LambdaContainer(LAMBDA lambda /*, ARGS... args*/) : lambda_(lambda) /*, args_(std::make_tuple(args...))*/ {}

        bool CallLambda(QObject *watched, QEvent *event) {
            auto args = std::tuple_cat(std::make_tuple(watched, event) /*, args_*/);
            return std::apply(lambda_, args);
        }

       private:
        LAMBDA lambda_;
        // std::tuple<ARGS...> args_;
    };

    AbstractLambdaContainer *lambdaContainer;
};
}  // namespace QEditor

#endif  // DIALOG_H
