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

#ifndef TABVIEW_H
#define TABVIEW_H

#include "Diff.h"
#include "DiffView.h"
#include "EditView.h"
#include "Logger.h"
#include "TerminalView.h"
#include <QAction>
#include <QMenu>
#include <QTabWidget>

namespace QEditor {
class TabView : public QTabWidget {
    Q_OBJECT
   public:
    TabView(QWidget *parent = nullptr);

    void HandleCurrentIndexChanged(int index);
    void HandleTabBarClicked(int index);
    void HandleTabBarDoubleClicked(int index);
    void HandleTabCloseRequested(int index);

    void UpdateWindowTitle(int index = -1);
    void ChangeTabCloseButtonToolTip(int index, const QString &tip);

    bool ActionSave();
    bool ActionSaveAs();
    bool TabCloseMaybeSave();
    bool TabCloseMaybeSaveInner(EditView *editView);
    bool TabForceClose();

    void AutoStore();
    bool AutoLoad();

    void DeleteWidget(int index);
    void DeleteWidget(QWidget *widget);

    EditView *CurrentEditView() {
        // The tab widget is definitely a EditView.
        auto editView = qobject_cast<EditView *>(currentWidget());
        if (editView == nullptr) {
            return nullptr;
        }
#if defined(USE_DIFF_TEXT_VIEW)
        // DiffView is sub class of EditView. Ignore.
        auto diffView = qobject_cast<DiffView *>(currentWidget());
        if (diffView != nullptr) {
            return nullptr;
        }
#endif
        return editView;
    }

    EditView *GetEditView(int index) {
        // The tab widget is definitely a EditView.
        auto editView = qobject_cast<EditView *>(widget(index));
        if (editView == nullptr) {
            return nullptr;
        }
#if defined(USE_DIFF_TEXT_VIEW)
        // DiffView is sub class of EditView. Ignore.
        auto diffView = qobject_cast<DiffView *>(widget(index));
        if (diffView != nullptr) {
            return nullptr;
        }
#endif
        return editView;
    }

    DiffView *GetDiffView(int index) {
        auto diffView = qobject_cast<DiffView *>(widget(index));
        if (diffView == nullptr) {
            return nullptr;
        }
        return diffView;
    }

    int FindEditViewIndex(const QString &filePath) {
        for (int i = 0; i < count(); ++i) {
            auto editView = GetEditView(i);
            if (editView == nullptr) {
                continue;
            }
            if (editView->filePath() == filePath) {
                return i;
            }
        }
        return -1;
    }

    void ViewDiff(const QString &former, const QString &latter);
    void ViewDiff(const EditView *former, const EditView *latter);
    void SwapDiff(int index);
    void NewFile();
    void OpenFile();
    void OpenFile(const QString &filePath);
    bool LoadFile(EditView *editView, const QString &filePath);
    bool LoadFile(EditView *editView, const QString &filePath, FileEncoding &&fileEncoding,
                  bool forceUseFileEncoding = false);
    void OpenSsh(const QString &ip, int port, const QString &user, const QString &pwd);

    void ChangeTabDescription(const QFileInfo &fileInfo, int index = -1);
    void ApplyWrapTextState(int index);
    void ApplySpecialCharsVisible(int index);

    QSet<QString> &openFiles() { return openFiles_; }

    bool stepRunning() { return stepRunning_; }
    void setStepRunning(bool stepRunning) { stepRunning_ = stepRunning; }
    QList<QPair<EditView *, int>> &backwardSteps() { return backwardSteps_; }
    QList<QPair<EditView *, int>> &forwardSteps() { return forwardSteps_; }
    void RecordStep(EditView *editView, int pos) {
        if (!stepRunning()) {
            // Not to add the same position.
            if (!backwardSteps().isEmpty()) {
                const auto &lastStep = backwardSteps().last();
                if (lastStep.first == editView && lastStep.second == pos) {
                    return;
                }
            }

            backwardSteps().push_back({editView, pos});
            forwardSteps().clear();
        }
    }

    const QString &formerDiffStr() const { return formerDiffStr_; }
    void setFormerDiffStr(const QString &formerDiffStr) { formerDiffStr_ = formerDiffStr; }

   protected:
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void tabInserted(int index) override;
    void tabRemoved(int index) override;

   private:
    QMenu *menu_;
    QSet<QString> openFiles_;

    bool stepRunning_{false};
    QList<QPair<EditView *, int>> backwardSteps_;
    QList<QPair<EditView *, int>> forwardSteps_;

    EditView *diffFormerEditView_{nullptr};
    Diff diff_;

    QString formerDiffStr_{""};
};
}  // namespace QEditor

#endif  // TABVIEW_H
