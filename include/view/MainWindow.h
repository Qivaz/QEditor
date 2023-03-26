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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "AnfNodeHierarchy.h"
#include "ComboView.h"
#include "DiffView.h"
#include "DockView.h"
#include "EditView.h"
#include "FunctionHierarchy.h"
#include "GotoLineDialog.h"
#include "MainTabView.h"
#include "OutlineList.h"
#include "SearchDialog.h"
#include <QGraphicsView>
#include <QMainWindow>
#include <QStatusBar>

QT_BEGIN_NAMESPACE
class QSessionManager;
QT_END_NAMESPACE

namespace QEditor {
class MainWindow : public QMainWindow {
    Q_OBJECT
   private:
    MainWindow();

   public:
    MainWindow(const MainWindow &) = delete;
    MainWindow(const MainWindow &&) = delete;
    ~MainWindow() {
        if (encoding_ != nullptr) {
            delete encoding_;
        }

        if (tabView_ != nullptr) {
            delete tabView_;
        }
        if (searchDockView_ != nullptr) {
            delete searchDockView_;
        }
    }

    static MainWindow &Instance() {
        static MainWindow _mainWindow;
        return _mainWindow;
    }

    void HandleCurrentTabChanged(int index);

    EditView *editView() {
        auto editView = qobject_cast<EditView *>(tabView_->currentWidget());
        if (editView == nullptr) {
            return nullptr;
        }
        return editView;
    }
    DiffView *diffView() {
        auto diffView = qobject_cast<DiffView *>(tabView_->currentWidget());
        if (diffView == nullptr) {
            return nullptr;
        }
        return diffView;
    }
    TabView *tabView() { return tabView_; }

    void ShowSearchDockView();
    DockView *CreateSearchDockView();

    bool IsExplorerDockViewShowing();
    void SetExplorerDockViewPosition(const QString &path);
    void ShowExplorerDockView();
    void HideExplorerDockView();
    DockView *CreateExplorerDockView();

    bool IsOutlineDockViewShowing();
    void SetOutlineDockViewPosition(int cursorPos);
    void UpdateOutlineDockView(OutlineList *list);
    void ShowOutlineDockView();
    void HideOutlineDockView();
    DockView *CreateOutlineDockView();

    bool IsHierarchyDockViewShowing();
    void UpdateHierarchyDockView(FunctionHierarchy *view);
    void ShowHierarchyDockView();
    void HideHierarchyDockView();
    DockView *CreateHierarchyDockView();

    bool IsNodeHierarchyDockViewShowing();
    void UpdateNodeHierarchyDockView(AnfNodeHierarchy *view);
    void ShowNodeHierarchyDockView();
    void HideNodeHierarchyDockView();
    DockView *CreateNodeHierarchyDockView();

    void SelectAll();
    void GotoLine();

    void SetCopyAvailable(bool avail);
    void Copy();
    void Cut();
    void Paste();

    void SetUndoAvailable(bool avail);
    void SetRedoAvailable(bool avail);
    void Undo();
    void Redo();

    void SwitchWrapText();
    void SyncWrapTextState();
    bool shouldWrapText() { return shouldWrapText_; }

    void SyncShowSpecialCharsVisible(bool shouldHide);
    void SwitchSpecialCharsVisible();
    bool specialCharsVisible() { return specialCharsVisible_; }

    void SwitchExplorerWindowVisible();
    void SwitchOutlineWindowVisible();
    void SwitchHierarchyWindowVisible();

    int tabCharNum() { return tabCharNum_; }

    void UpdateStatusBarFrequentInfo(const QString &ln, const QString &col, const QString &pos, const QString &len,
                                     const QString &lines);
    void UpdateStatusBarRareInfo(const QString &newLineSeq, const QString &encodingSeq, const QString &insertModeSeq);

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void RequestShow() { emit Show(); }

    void UpdateRecentFilesMenu();

    QString searchingString() const;
    void setSearchingString(const QString &searchingString);

    SearchResultList *GetSearchResultList();

    Searcher *GetSearcher();

   public slots:
    bool Find();
    bool FindNext();
    bool FindPrevious();
    bool Replace();

    bool MarkUnmarkCursorText();
    bool UnmarkAll();

   protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

   private slots:
    void OpenRecentFile();

    void NewFile();
    void Open();
    void OpenWith(const QString &filePath);
    bool Save();
    bool SaveAll();
    bool SaveAs();

    bool ZoomIn();
    bool ZoomOut();

    void About();

#ifndef QT_NO_SESSIONMANAGER
    void HandleCommitData(QSessionManager &);
#endif
    void HandleNewLineCharChanged(int index);
    void HandleEncodingChanged(int index);

    bool StepBack();
    bool StepForward();

   signals:
    void Show();

   private:
    void CreateActions();
    void CreateStatusBar();
    void ReadSettings();
    void WriteSettings();
    bool MaybeSave();

    bool IsLeftOrRightSeparator(const QPoint &pos);

    bool init_{false};
    TabView *tabView_{nullptr};
    DockView *searchDockView_{nullptr};
    SearchDialog *searchDialog_{nullptr};

    DockView *explorerDockView_{nullptr};
    DockView *outlineDockView_{nullptr};
    DockView *hierarchyDockView_{nullptr};
    DockView *nodeHierarchyDockView_{nullptr};
    AnfNodeHierarchy *anfNodeHierarchy_{nullptr};

    bool moveSeparatorToHide_{false};

    QAction *copyAct_{nullptr};
    QAction *cutAct_{nullptr};
    QAction *undoAct_{nullptr};
    QAction *redoAct_{nullptr};

    // TODO: Save in settings.
    bool shouldWrapText_{true};
    bool specialCharsVisible_{true};
    bool explorerVisible_{true};
    bool outlineVisible_{true};
    bool hierarchyVisible_{true};

    int tabCharNum_{2};

    // Status bar items.
    QLabel *line_{nullptr};
    QLabel *column_{nullptr};
    QLabel *position_{nullptr};
    QLabel *lines_{nullptr};
    QLabel *length_{nullptr};
    ComboView *encoding_{nullptr};
    ComboView *newLineChar_{nullptr};
    QLabel *insertMode_{nullptr};

    // We reuse the 'QComboBox::currentIndexChanged()' for its display
    // changing(Init&Sync) triggered by 'setCurrentIndex()' and user click.
    // So we distinguish them with this flag:
    // True for Init&Sync, otherwise user click.
    bool statusBarComboNonUserClick_{false};

    QMenu *recentFilesMenu_{nullptr};
    QList<QAction *> recentFileActions_;

    SearchResultList *searchResultList_{nullptr};
    Searcher *searcher_{nullptr};
    QString searchingString_;
};
}  // namespace QEditor

#endif
