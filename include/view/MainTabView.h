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

#include <QTabWidget>
#include <QMenu>
#include <QAction>

#include "EditView.h"
#include "Logger.h"

class TabView : public QTabWidget
{
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

    EditView *CurrentEditView()
    {
        // The tab widget is definitely a EditView.
        return ((EditView*)currentWidget());
    }

    EditView *GetEditView(int index)
    {
        // The tab widget is definitely a EditView.
        return (EditView*)widget(index);
    }

    int FindEditViewIndex(const QString &filePath)
    {
        for (int i = 0; i < count(); ++i) {
            if (GetEditView(i)->filePath() == filePath) {
                return i;
            }
        }
        return -1;
    }

    void NewFile();
    void OpenFile();
    void OpenFile(const QString &filePath);
    bool LoadFile(EditView *editView, const QString &filePath);
    bool LoadFile(EditView *editView, const QString &filePath,
                  FileEncoding &&fileEncoding, bool forceUseFileEncoding = false);

    void ChangeTabDescription(const QFileInfo &fileInfo, int index = -1);
    void ApplyWrapTextState(int index);
    void ApplySpecialCharsVisible(int index);

    QSet<QString> &openFiles() { return openFiles_; }

    bool stepRunning() { return stepRunning_; }
    void setStepRunning(bool stepRunning) { stepRunning_ = stepRunning; }
    QList<QPair<EditView*, int>> &backwardSteps() { return backwardSteps_; }
    QList<QPair<EditView*, int>> &forwardSteps() { return forwardSteps_; }
    void RecordStep(EditView *editView, int pos)
    {
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

protected:
    void tabInserted(int index) override;
    void tabRemoved(int index) override;

private:
    QMenu *menu_;
    QSet<QString> openFiles_;

    bool stepRunning_{false};
    QList<QPair<EditView*, int>> backwardSteps_;
    QList<QPair<EditView*, int>> forwardSteps_;
};

#endif // TABVIEW_H
