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

#include "MainWindow.h"
#include "ComboView.h"
#include "ExplorerTreeView.h"
#include "Logger.h"
#include "RecentFiles.h"
#ifdef OPEN_TERM
#include "OpenTerminalDialog.h"
#include "RemoteExplorerTreeView.h"
#endif
#include "SearchDialog.h"
#include "SearchResultList.h"
#include "Settings.h"
#include "Toast.h"
#include "win/WinTheme.h"
#include <MainWindow.h>
#include <QLayoutItem>
#include <QSizePolicy>
#include <QtWidgets>

namespace QEditor {
MainWindow::MainWindow() : tabView_(new TabView(this)) {
    auto settings = Settings();
    toolBarVisible_ = settings.Get("view", "toolbar_visible", true).toBool();
    shouldWrapText_ = settings.Get("view", "wrap_text", true).toBool();
    specialCharsVisible_ = settings.Get("view", "all_chars_visible", false).toBool();
    explorerVisible_ = settings.Get("view", "explorer_visible", true).toBool();
    outlineVisible_ = settings.Get("view", "outline_visible", true).toBool();
    hierarchyVisible_ = settings.Get("view", "hierarchy_visible", true).toBool();
    qreal opa = settings.Get("window", "opacity", 1).toDouble();

    setAttribute(Qt::WA_InputMethodEnabled);

    // Not show action icon in menu bar.
    qApp->setAttribute(Qt::AA_DontShowIconsInMenus);

    setAcceptDrops(true);
    setWindowOpacity(opa);

    // setStyleSheet(
    //     "background:rgb(68,68,68); selection-color:lightGray; selection-background-color:rgb(9,71,113); border:1px,solid,rgb(255,0,0);");
    setStyleSheet("QMainWindow{background:rgb(68,68,68);} QMainWindow::separator{background:rgb(54,54,54); width: 1px; height: 0px; margin: 0px; padding: 0px;}");

    if (explorerVisible_) {
        ShowExplorerDockView();
    } else {
        HideExplorerDockView();
    }
    if (outlineVisible_) {
        ShowOutlineDockView();
    } else {
        HideOutlineDockView();
    }
    if (hierarchyVisible_) {
        ShowHierarchyDockView();
    } else {
        HideHierarchyDockView();
    }

    setCentralWidget(tabView_);

    CreateActions();
    CreateStatusBar();

    ReadSettings();

#ifndef QT_NO_SESSIONMANAGER
    connect(qApp, &QGuiApplication::commitDataRequest, this, &MainWindow::HandleCommitData);
#endif

    // Delay to show the window.
    connect(
        this, &MainWindow::Show, this, []() { QTimer::singleShot(500, []() { MainWindow::Instance().show(); }); },
        Qt::QueuedConnection);

    setUnifiedTitleAndToolBarOnMac(true);

#define FORCE_DARK_THEME
#ifdef Q_OS_WIN
#ifdef FORCE_DARK_THEME
    WinTheme::SetDark_qApp();
    WinTheme::SetDarkTitleBar(reinterpret_cast<HWND>(winId()));
#else
    if (WinTheme::IsDarkTheme()) {
        WinTheme::SetDark_qApp();
        WinTheme::SetDarkTitleBar(reinterpret_cast<HWND>(winId()));
    }
#endif
#endif
    // Set global style sheet.
    qApp->setStyleSheet(
        "QToolTip{color:white; background-color:rgb(54,54,54); border:2px solid rgb(54,54,54);}"
        "QMessageBox{color:lightGray; background-color:rgb(54,54,54);} QMessageBox QLabel{color:lightGray}");

    installEventFilter(this);

    QPalette pal = QPalette();
    // pal.setColor(QPalette::ToolTipBase, QColor(0, 0, 255));
    pal.setColor(QPalette::ToolTipText, Qt::lightGray);
    QToolTip::setPalette(pal);
}

void MainWindow::CreateActions() {
    menuBar()->setStyleSheet(
        "QMenuBar{color:lightGray; selection-background-color:rgb(9,71,113); background-color:rgb(28,28,28); border:none;}"
        "QMenu{color:lightGray; selection-background-color:rgb(9,71,113); background-color:rgb(40,40,40); border:none;}"
        // "QMenu::item{height:45px; margin:0px;}"
        "QMenu::separator{height:1px; background-color:rgb(80,80,80);}");

    auto toolBarStyle =
        "QToolBar{color:lightGray; background-color:rgb(28,28,28); border-top: 0px; border-bottom: 0px;}"
        "QToolBar QToolButton {border:2px solid transparent; background-color:rgb(28,28,28);}"
        "QToolBar QToolButton:hover{border:2px solid transparent; background-color:rgb(54,54,54);}"
        "QToolBar QToolButton:enabled{border:1.5px solid rgb(40,40,40);}"
        "QToolBar QToolButton:pressed{border:1.5px solid rgb(9,71,113); background-color:rgb(28,28,28);}"
        "QToolBar QToolButton:checked{border:1.5px solid rgb(35,54,80); background-color:rgb(28,28,28);}";

    // File menu.
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar *fileToolBar = addToolBar(tr("File"));
    fileToolBar->setStyleSheet(toolBarStyle);

    // TODO:
    // Use addPixmap() set on/off icons.
    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/file.svg"));
    QAction *newAct = new QAction(newIcon, tr("&New"), this);
    // newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::NewFile);
    fileMenu->addAction(newAct);
    fileToolBar->addAction(newAct);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/folder-open.svg"));
    QAction *openAct = new QAction(openIcon, tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::Open);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);

    recentFilesMenu_ = fileMenu->addMenu(tr("Open Recent"));
    RecentFiles::LoadFiles();
    UpdateRecentFilesMenu();

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.svg"));
    QAction *saveAct = new QAction(saveIcon, tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::Save);
    fileMenu->addAction(saveAct);
    fileToolBar->addAction(saveAct);

    const QIcon saveAllIcon = QIcon::fromTheme("document-save-all");
    QAction *saveAllAct = fileMenu->addAction(saveAllIcon, tr("Save all"), this, &MainWindow::SaveAll);
    saveAllAct->setShortcuts(QKeySequence::SaveAs);
    saveAllAct->setStatusTip(tr("Save all open documents"));

    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    QAction *saveAsAct = fileMenu->addAction(saveAsIcon, tr("Save &As..."), this, &MainWindow::SaveAs);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));

    // Edit menu.
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    QToolBar *editToolBar = addToolBar(tr("Edit"));
    editToolBar->setStyleSheet(toolBarStyle);
#ifndef QT_NO_CLIPBOARD
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/images/cut.svg"));
    cutAct_ = new QAction(cutIcon, tr("Cu&t"), this);
    cutAct_->setShortcuts(QKeySequence::Cut);
    cutAct_->setStatusTip(
        tr("Cut the current selection's contents to the "
           "clipboard"));
    connect(cutAct_, &QAction::triggered, this, &MainWindow::Cut);
    editMenu->addAction(cutAct_);
    editToolBar->addAction(cutAct_);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.svg"));
    copyAct_ = new QAction(copyIcon, tr("&Copy"), this);
    copyAct_->setShortcuts(QKeySequence::Copy);
    copyAct_->setStatusTip(
        tr("Copy the current selection's contents to the "
           "clipboard"));
    connect(copyAct_, &QAction::triggered, this, &MainWindow::Copy);
    editMenu->addAction(copyAct_);
    editToolBar->addAction(copyAct_);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/images/paste.svg"));
    QAction *pasteAct = new QAction(pasteIcon, tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(
        tr("Paste the clipboard's contents into the current "
           "selection"));
    connect(pasteAct, &QAction::triggered, this, &MainWindow::Paste);
    editMenu->addAction(pasteAct);
    editToolBar->addAction(pasteAct);

    menuBar()->addSeparator();
#endif  // !QT_NO_CLIPBOARD

    // Undo&Redo
    QToolBar *undoRedoToolBar = addToolBar(tr("Undo&Redo"));
    undoRedoToolBar->setStyleSheet(toolBarStyle);

    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(":/images/action-undo.svg"));
    undoAct_ = new QAction(undoIcon, tr("&Undo"), this);
    undoAct_->setShortcuts(QKeySequence::Undo);
    undoAct_->setStatusTip(tr("Undo"));
    connect(undoAct_, &QAction::triggered, this, &MainWindow::Undo);
    editMenu->addAction(undoAct_);
    undoRedoToolBar->addAction(undoAct_);

    const QIcon redoIcon = QIcon::fromTheme("edit-redo", QIcon(":/images/action-redo.svg"));
    redoAct_ = new QAction(redoIcon, tr("&Redo"), this);
    redoAct_->setShortcuts(QKeySequence::Redo);
    redoAct_->setStatusTip(tr("Redo"));
    connect(redoAct_, &QAction::triggered, this, &MainWindow::Redo);
    editMenu->addAction(redoAct_);
    undoRedoToolBar->addAction(redoAct_);

    // Select menu.
    QMenu *selectMenu = menuBar()->addMenu(tr("&Select"));
    QToolBar *searchToolBar = addToolBar(tr("Select"));
    searchToolBar->setStyleSheet(toolBarStyle);

    const QIcon findIcon = QIcon::fromTheme("select-search", QIcon(":/images/search.svg"));
    QAction *findAct = new QAction(findIcon, tr("&Find..."), this);
    findAct->setShortcuts(QKeySequence::Find);
    findAct->setStatusTip(tr("Find"));
    connect(findAct, &QAction::triggered, this, &MainWindow::Find);
    selectMenu->addAction(findAct);
    searchToolBar->addAction(findAct);

    QAction *findNextAct = new QAction(tr("&Find Next"), this);
    findNextAct->setShortcuts(QKeySequence::FindNext);
    findNextAct->setStatusTip(tr("Find next"));
    connect(findNextAct, &QAction::triggered, this, &MainWindow::FindNext);
    selectMenu->addAction(findNextAct);

    QAction *findPreviousAct = new QAction(tr("&Find Previous"), this);
    findPreviousAct->setShortcuts(QKeySequence::FindPrevious);
    findPreviousAct->setStatusTip(tr("Find previous"));
    connect(findPreviousAct, &QAction::triggered, this, &MainWindow::FindPrevious);
    selectMenu->addAction(findPreviousAct);

    const QIcon replaceIcon = QIcon::fromTheme("select-replace", QIcon(":/images/replace.svg"));
    QAction *replaceAct = new QAction(replaceIcon, tr("&Replace..."), this);
    auto replaceKeySeq = QKeySequence(Qt::CTRL + Qt::Key_H);
    replaceAct->setShortcut(replaceKeySeq);  // QKeySequence::Replace
    replaceAct->setStatusTip(tr("Replace"));
    connect(replaceAct, &QAction::triggered, this, &MainWindow::Replace);
    selectMenu->addAction(replaceAct);
    searchToolBar->addAction(replaceAct);

    selectMenu->addSeparator();

    const QIcon gotoLineIcon = QIcon::fromTheme("select-got-line", QIcon(":/images/goto-line.svg"));
    QAction *gotoLineAct = new QAction(gotoLineIcon, tr("&Go to Line..."), this);
    auto gotoLineKeySeq = QKeySequence(Qt::CTRL + Qt::Key_G);  // QKeySequence(tr("F3, Ctrl+G"))
    gotoLineAct->setShortcut(gotoLineKeySeq);
    gotoLineAct->setStatusTip(tr("GotoLine"));
    connect(gotoLineAct, &QAction::triggered, this, &MainWindow::GotoLine);
    selectMenu->addAction(gotoLineAct);
    searchToolBar->addAction(gotoLineAct);

    const QIcon selectAllIcon = QIcon::fromTheme("select-all-lines", QIcon(":/images/select-all.svg"));
    QAction *selectAllAct = new QAction(selectAllIcon, tr("&Select All Lines"), this);
    selectAllAct->setShortcuts(QKeySequence::SelectAll);
    selectAllAct->setStatusTip(tr("SelectAll"));
    connect(selectAllAct, &QAction::triggered, this, &MainWindow::SelectAll);
    selectMenu->addAction(selectAllAct);
    searchToolBar->addAction(selectAllAct);

    selectMenu->addSeparator();

    QToolBar *markToolBar = addToolBar(tr("Mark"));
    markToolBar->setStyleSheet(toolBarStyle);

    const QIcon markIcon = QIcon::fromTheme("select-mark", QIcon(":/images/highlighter.svg"));
    QAction *markAct = new QAction(markIcon, tr("&Mark or Unmark"), this);
    auto markKeySeq = QKeySequence(Qt::SHIFT + Qt::Key_F8);
    markAct->setShortcut(markKeySeq);
    markAct->setStatusTip(tr("Mark & Unmack"));
    connect(markAct, &QAction::triggered, this, &MainWindow::MarkUnmarkCursorText);
    selectMenu->addAction(markAct);
    markToolBar->addAction(markAct);

    const QIcon unmarkAllIcon = QIcon::fromTheme("select-unmark-all", QIcon(":/images/eraser.svg"));
    QAction *unmarkAllAct = new QAction(unmarkAllIcon, tr("&Unmark All"), this);
    auto unmarkAllKeySeq = QKeySequence(Qt::CTRL, Qt::SHIFT, Qt::Key_F8);
    unmarkAllAct->setShortcut(unmarkAllKeySeq);
    unmarkAllAct->setStatusTip(tr("Mark"));
    connect(unmarkAllAct, &QAction::triggered, this, &MainWindow::UnmarkAll);
    selectMenu->addAction(unmarkAllAct);
    markToolBar->addAction(unmarkAllAct);

    selectMenu->addSeparator();

    QToolBar *stepToolBar = addToolBar(tr("Step"));
    stepToolBar->setStyleSheet(toolBarStyle);

    const QIcon stepBackIcon = QIcon::fromTheme("step-back", QIcon(":/images/arrow-thick-to-left.svg"));
    QAction *stepBackAct = new QAction(stepBackIcon, tr("&Step Back"), this);
    auto stepBackSeq = QKeySequence(Qt::ALT + Qt::Key_Left);
    stepBackAct->setShortcut(stepBackSeq);
    stepBackAct->setStatusTip(tr("Step back"));
    connect(stepBackAct, &QAction::triggered, this, &MainWindow::StepBack);
    selectMenu->addAction(stepBackAct);
    stepToolBar->addAction(stepBackAct);

    const QIcon stepForwardIcon = QIcon::fromTheme("step-forward", QIcon(":/images/arrow-thick-to-right.svg"));
    QAction *stepForwardAct = new QAction(stepForwardIcon, tr("&Step Forward"), this);
    auto stepForwardSeq = QKeySequence(Qt::ALT + Qt::Key_Right);
    stepForwardAct->setShortcut(stepForwardSeq);
    stepForwardAct->setStatusTip(tr("Step forward"));
    connect(stepForwardAct, &QAction::triggered, this, &MainWindow::StepForward);
    selectMenu->addAction(stepForwardAct);
    stepToolBar->addAction(stepForwardAct);

    // View menu.
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    QToolBar *viewToolBar = addToolBar(tr("View"));
    viewToolBar->setStyleSheet(toolBarStyle);

    const QIcon zoomInIcon = QIcon::fromTheme("view-zoomin", QIcon(":/images/zoom-in.svg"));
    QAction *zoomInAct = new QAction(zoomInIcon, tr("Zoom In"), this);
    zoomInAct->setStatusTip(tr("ZoomIn"));
    connect(zoomInAct, &QAction::triggered, this, &MainWindow::ZoomIn);
    viewMenu->addAction(zoomInAct);
    viewToolBar->addAction(zoomInAct);

    const QIcon zoomOutIcon = QIcon::fromTheme("view-zoomout", QIcon(":/images/zoom-out.svg"));
    QAction *zoomOutAct = new QAction(zoomOutIcon, tr("Zoom Out"), this);
    zoomOutAct->setStatusTip(tr("ZoomOut"));
    connect(zoomOutAct, &QAction::triggered, this, &MainWindow::ZoomOut);
    viewMenu->addAction(zoomOutAct);
    viewToolBar->addAction(zoomOutAct);

    viewMenu->addSeparator();

    QAction *showToolBar = new QAction(tr("Show Tool Bar"), this);
    showToolBar->setToolTip(tr("Show/Hide Tool Bar"));
    connect(showToolBar, &QAction::triggered, this, [this](){
        toolBarVisible_ = !toolBarVisible_;
        QList<QToolBar*> toolbars = findChildren<QToolBar*>();
        for (const auto &toolbar : toolbars) {
            if (toolbar == nullptr) {
                continue;
            }
            if (toolBarVisible_) {
                toolbar->show();
            } else {
                toolbar->hide();
            }
        }

        Settings().Set("view", "toolbar_visible", toolBarVisible_);
    });
    showToolBar->setCheckable(true);
    if (toolBarVisible_) {
        showToolBar->setChecked(true);
    }
    viewMenu->addAction(showToolBar);

    viewMenu->addSeparator();

    const QIcon wrapTextIcon = QIcon::fromTheme("view-wrap-text", QIcon(":/images/wrap-text.svg"));
    QAction *wrapTextAct = new QAction(wrapTextIcon, tr("Wrap Text"), this);
    wrapTextAct->setStatusTip(tr("WrapText"));
    wrapTextAct->setCheckable(true);
    wrapTextAct->setChecked(shouldWrapText_);
    connect(wrapTextAct, &QAction::triggered, this, &MainWindow::SwitchWrapText);
    viewMenu->addAction(wrapTextAct);
    viewToolBar->addAction(wrapTextAct);

    const QIcon showAllCharsIcon = QIcon::fromTheme("view-show-chars", QIcon(":/images/show-all-chars.svg"));
    QAction *showAllCharsAct = new QAction(showAllCharsIcon, tr("Show All Chars"), this);
    showAllCharsAct->setStatusTip(tr("ShowSpecialChars"));
    showAllCharsAct->setCheckable(true);
    showAllCharsAct->setChecked(specialCharsVisible_);
    connect(showAllCharsAct, &QAction::triggered, this, &MainWindow::SwitchSpecialCharsVisible);
    viewMenu->addAction(showAllCharsAct);
    viewToolBar->addAction(showAllCharsAct);

    viewMenu->addSeparator();

    QToolBar *viewToolBar2 = addToolBar(tr("View addin"));
    viewToolBar2->setStyleSheet(toolBarStyle);

    const QIcon showDirIcon = QIcon::fromTheme("view-show-dir", QIcon(":/images/folder.svg"));
    QAction *showDirAct = new QAction(showDirIcon, tr("Show Explorer Window"), this);
    showDirAct->setStatusTip(tr("ShowExplorerWindow"));
    showDirAct->setCheckable(true);
    showDirAct->setChecked(explorerVisible_);
    connect(showDirAct, &QAction::triggered, this, &MainWindow::SwitchExplorerWindowVisible);
    viewMenu->addAction(showDirAct);
    viewToolBar2->addAction(showDirAct);

    const QIcon showOutlineIcon = QIcon::fromTheme("view-show-outline", QIcon(":/images/brackets-contain.svg"));
    QAction *showOutlineAct = new QAction(showOutlineIcon, tr("Show Outline Window"), this);
    showOutlineAct->setStatusTip(tr("ShowOutlineWindow"));
    showOutlineAct->setCheckable(true);
    showOutlineAct->setChecked(outlineVisible_);
    connect(showOutlineAct, &QAction::triggered, this, &MainWindow::SwitchOutlineWindowVisible);
    viewMenu->addAction(showOutlineAct);
    viewToolBar2->addAction(showOutlineAct);

    const QIcon showHierarchyIcon = QIcon::fromTheme("view-show-hierarchy", QIcon(":/images/sitemap.svg"));
    QAction *showHierarchyAct = new QAction(showHierarchyIcon, tr("Show Hierarchy Window"), this);
    showHierarchyAct->setStatusTip(tr("ShowHierarchyWindow"));
    showHierarchyAct->setCheckable(true);
    showHierarchyAct->setChecked(hierarchyVisible_);
    connect(showHierarchyAct, &QAction::triggered, this, &MainWindow::SwitchHierarchyWindowVisible);
    viewMenu->addAction(showHierarchyAct);
    viewToolBar2->addAction(showHierarchyAct);

#ifdef OPEN_TERM
    // Terminal menu.
    QMenu *terminalMenu = menuBar()->addMenu(tr("&Terminal"));
    QToolBar *terminalToolBar = addToolBar(tr("Terminal"));
    terminalToolBar->setStyleSheet(toolBarStyle);

    const QIcon openSshIcon = QIcon::fromTheme("term-open-ssh", QIcon(":/images/terminal.svg"));
    QAction *openSshAct = new QAction(openSshIcon, tr("Open SSH"), this);
    openSshAct->setStatusTip(tr("Open SSH"));
    connect(openSshAct, &QAction::triggered, this, []() {
        (new OpenTerminalDialog(&MainWindow::Instance()))->show();
        return true;
    });
    terminalMenu->addAction(openSshAct);
    terminalToolBar->addAction(openSshAct);
#endif

    // Help menu.
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::About);
    aboutAct->setStatusTip(tr("Show the application's About box"));

#ifndef QT_NO_CLIPBOARD
    cutAct_->setEnabled(false);
    copyAct_->setEnabled(false);
    undoAct_->setEnabled(false);
    redoAct_->setEnabled(false);
#endif  // !QT_NO_CLIPBOARD

    // Show/Hide the whole tool bars.
    QList<QToolBar*> toolbars = findChildren<QToolBar*>();
    for (const auto &toolbar : toolbars) {
        if (toolbar == nullptr) {
            continue;
        }
        if (toolBarVisible_) {
            toolbar->show();
        } else {
            toolbar->hide();
        }
    }
}

void MainWindow::HandleCurrentTabChanged(int index) { qDebug() << "MainWindow::tabIndexChanged, " << index; }

void MainWindow::ShowSearchDockView() {
    if (!searchDockView_->isVisible()) {
        qDebug() << "Search Dock invisible";
        searchDockView_->show();
    } else {
        qDebug() << "Search Dock visible";
    }
}

DockView *MainWindow::CreateSearchDockView() {
    if (searchDockView_ == nullptr) {
        searchDockView_ = new DockView(this, 1000, 300);
    }
    searchDockView_->setWindowTitle(tr("Search result:"));
    // searchDockView_->setFont(QFont("Consolas", 11));
    searchDockView_->setFeatures(QDockWidget::DockWidgetClosable |
                                 QDockWidget::DockWidgetMovable /* | QDockWidget::DockWidgetFloatable*/);
    addDockWidget(Qt::BottomDockWidgetArea, searchDockView_);
    return searchDockView_;
}

bool MainWindow::IsExplorerDockViewShowing() {
    if (explorerDockView_ == nullptr) {
        return false;
    }
    return explorerDockView_->isVisible();
}

void MainWindow::SetExplorerDockViewPosition(const QString &path) {
    if (explorerDockView_ == nullptr) {
        return;
    }
    auto tree = ((ExplorerTreeView *)explorerDockView_->widget());
    if (tree == nullptr) {
        return;
    }
    tree->GotoPathPosition(path);
}

void MainWindow::ShowExplorerDockView() {
    if (explorerDockView_ == nullptr) {
        auto dir = CreateExplorerDockView();
        auto treeView = new ExplorerTreeView(this);
        dir->setWidget(treeView);
    }

    explorerDockView_->show();
}

void MainWindow::HideExplorerDockView() {
    if (explorerDockView_ == nullptr) {
        auto dir = CreateExplorerDockView();
        auto treeView = new ExplorerTreeView(this);
        dir->setWidget(treeView);
    }
    explorerDockView_->hide();
}

DockView *MainWindow::CreateExplorerDockView() {
    if (explorerDockView_ == nullptr) {
        explorerDockView_ = new DockView(this);
        explorerDockView_->setSavedMaxWidth(explorerDockView_->maximumWidth());
        // We must set both minimum and maximum width to 0 to hide widget. Here set minimum firstly.
        explorerDockView_->setMinimumWidth(0);
    }
    explorerDockView_->setWindowTitle(tr("EXPLORER"));
    // explorerDockView_->setFont(QFont("Consolas", 11));
    explorerDockView_->setFeatures(
        /*QDockWidget::DockWidgetClosable | */ QDockWidget::DockWidgetMovable /* | QDockWidget::DockWidgetFloatable*/);
    addDockWidget(Qt::LeftDockWidgetArea, explorerDockView_);
    return explorerDockView_;
}

bool MainWindow::IsOutlineDockViewShowing() {
    if (outlineDockView_ == nullptr) {
        return false;
    }
    return outlineDockView_->isVisible();
}

void MainWindow::SetOutlineDockViewPosition(int cursorPos) {
    if (outlineDockView_ == nullptr) {
        return;
    }
    auto list = ((OutlineList *)outlineDockView_->widget());
    if (list == nullptr) {
        return;
    }
    if (list->topLevelItemCount() == 0) {
        return;
    }

    try {
        // TODO: Why setSelectionMode(QAbstractItemView::SingleSelection) not work.
        const auto &selectedItems = list->selectedItems();
        for (const auto &item : selectedItems) {
            item->setSelected(false);
        }

        // Select the item relevant to the text cursor.
        auto index = list->GetIndexByCursorPos(cursorPos);
        list->topLevelItem(index)->setSelected(true);
    } catch (const std::out_of_range &e) {
        qDebug() << "Out of range: " << cursorPos;
        return;
    }
}

void MainWindow::UpdateOutlineDockView(OutlineList *list) {
    if (outlineDockView_ == nullptr) {
        CreateOutlineDockView();
    }
    outlineDockView_->setWidget(list);
    ShowOutlineDockView();
}

void MainWindow::ShowOutlineDockView() {
    if (outlineDockView_ == nullptr) {
        CreateOutlineDockView();
    }
    outlineDockView_->show();
}

void MainWindow::HideOutlineDockView() {
    if (outlineDockView_ == nullptr) {
        CreateOutlineDockView();
    }
    outlineDockView_->hide();
}

DockView *MainWindow::CreateOutlineDockView() {
    if (outlineDockView_ == nullptr) {
        outlineDockView_ = new DockView(this);
        outlineDockView_->setSavedMaxWidth(outlineDockView_->maximumWidth());
        // We must set both minimum and maximum width to 0 to hide widget. Here set minimum firstly.
        outlineDockView_->setMinimumWidth(0);
    }
    outlineDockView_->setWindowTitle(tr("OUTLINE"));
    // overviewDockView_->setFont(QFont("Consolas", 11));
    outlineDockView_->setFeatures(
        /*QDockWidget::DockWidgetClosable | */ QDockWidget::DockWidgetMovable /* | QDockWidget::DockWidgetFloatable*/);
    addDockWidget(Qt::LeftDockWidgetArea, outlineDockView_);
    return outlineDockView_;
}

bool MainWindow::IsHierarchyDockViewShowing() {
    if (hierarchyDockView_ == nullptr) {
        return false;
    }
    return hierarchyDockView_->isVisible();
}

void MainWindow::UpdateHierarchyDockView(FunctionHierarchy *view) {
    if (hierarchyDockView_ == nullptr) {
        CreateHierarchyDockView();
    }
    hierarchyDockView_->setWidget(view);
    ShowHierarchyDockView();
}

void MainWindow::ShowHierarchyDockView() {
    if (hierarchyDockView_ == nullptr) {
        CreateHierarchyDockView();
    }
    hierarchyDockView_->show();
}

void MainWindow::HideHierarchyDockView() {
    if (hierarchyDockView_ != nullptr && hierarchyDockView_->isVisible()) {
        hierarchyDockView_->hide();
    }
    HideNodeHierarchyDockView();
}

DockView *MainWindow::CreateHierarchyDockView() {
    if (hierarchyDockView_ == nullptr) {
        hierarchyDockView_ = new DockView(this, 500, 500);
        hierarchyDockView_->setSavedMaxWidth(hierarchyDockView_->maximumWidth());
        // We must set both minimum and maximum width to 0 to hide widget. Here set minimum firstly.
        hierarchyDockView_->setMinimumWidth(0);
    }
    hierarchyDockView_->setWindowTitle(tr("FUNCTION HIERARCHY"));
    hierarchyDockView_->setFeatures(QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::RightDockWidgetArea, hierarchyDockView_);
    return hierarchyDockView_;
}

bool MainWindow::IsNodeHierarchyDockViewShowing() {
    if (nodeHierarchyDockView_ == nullptr) {
        return false;
    }
    return nodeHierarchyDockView_->isVisible();
}

void MainWindow::UpdateNodeHierarchyDockView(AnfNodeHierarchy *view) {
    if (nodeHierarchyDockView_ == nullptr) {
        CreateNodeHierarchyDockView();
    }
    nodeHierarchyDockView_->setWindowTitle(tr("NODE HIERARCHY") + "  ( " + view->funcName() + " )");
    if (anfNodeHierarchy_ != nullptr) {
        delete anfNodeHierarchy_;
        anfNodeHierarchy_ = nullptr;
    }
    anfNodeHierarchy_ = view;
    nodeHierarchyDockView_->setWidget(view);
    ShowNodeHierarchyDockView();
}

void MainWindow::ShowNodeHierarchyDockView() {
    if (nodeHierarchyDockView_ != nullptr && !nodeHierarchyDockView_->isVisible()) {
        nodeHierarchyDockView_->show();
    }
}

void MainWindow::HideNodeHierarchyDockView() {
    if (nodeHierarchyDockView_ != nullptr && nodeHierarchyDockView_->isVisible()) {
        nodeHierarchyDockView_->hide();
    }
}

DockView *MainWindow::CreateNodeHierarchyDockView() {
    if (nodeHierarchyDockView_ == nullptr) {
        nodeHierarchyDockView_ = new DockView(this, 500, 500);
        nodeHierarchyDockView_->setSavedMaxWidth(nodeHierarchyDockView_->maximumWidth());
        // We must set both minimum and maximum width to 0 to hide widget. Here set minimum firstly.
        nodeHierarchyDockView_->setMinimumWidth(0);
    }
    nodeHierarchyDockView_->setWindowTitle(tr("NODE HIERARCHY"));
    nodeHierarchyDockView_->setFeatures(QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::RightDockWidgetArea, nodeHierarchyDockView_);
    return nodeHierarchyDockView_;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    qDebug() << "event: " << event->answerRect();
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
    qDebug() << "event: " << event->pos() << ", " << event->mimeData()->text() << ", " << event->mimeData()->urls()
             << ", " << event->mimeData()->html();
    auto urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        qDebug() << "file: " << urls[0].toLocalFile();
        OpenWith(urls[0].toLocalFile());
    }
}

void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    if (!init_) {
        init_ = true;
        // Defer parse to avoid tabbar concealing some tabs.
        QTimer::singleShot(100, [this]() {
            (void)tabView_->AutoLoad();
            auto ev = editView();
            if (ev != nullptr) {
                ev->setFocus();
            }
        });
    }
}

void MainWindow::closeEvent(QCloseEvent *event) { tabView_->AutoStore(); }

bool MainWindow::IsLeftOrRightSeparator(const QPointF &pos) {
    constexpr auto distance_threhold = 3;
    return (pos.x() < distance_threhold || std::abs(pos.x() - rect().width()) < distance_threhold);
}

bool MainWindow::hierarchyVisible() const
{
    return hierarchyVisible_;
}

bool MainWindow::outlineVisible() const
{
    return outlineVisible_;
}

bool MainWindow::explorerVisible() const
{
    return explorerVisible_;
}

Searcher *MainWindow::GetSearcher() {
    if (searcher_ == nullptr) {
        searcher_ = new Searcher();
    }
    return searcher_;
}

SearchResultList *MainWindow::GetSearchResultList() {
    if (searchResultList_ == nullptr) {
        searchResultList_ = new SearchResultList(tabView());
        auto dockView = MainWindow::Instance().CreateSearchDockView();
        dockView->setWidget(searchResultList_);
        // searchResultList_->setParent(dockView);
        searchResultList_->setFont(QFont("Consolas", 11));
    }
    return searchResultList_;
}

QString MainWindow::searchingString() const { return searchingString_; }

void MainWindow::setSearchingString(const QString &searchingString) { searchingString_ = searchingString; }

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    qDebug() << "event: " << event->type() << ", obj: " << obj;
    const auto editView = this->editView();
    if (event->type() == QEvent::HoverMove) {
        QHoverEvent *hoverEvent = static_cast<QHoverEvent *>(event);
        if (IsLeftOrRightSeparator(hoverEvent->position())) {
            setCursor(Qt::SplitHCursor);
        } else {
            unsetCursor();
            if (editView != nullptr) {
                auto pos = editView->viewport()->mapFrom(this, hoverEvent->pos());
                auto cursor = editView->cursorForPosition(pos);
                editView->Hover(cursor);
            }
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        auto mouseEvent = static_cast<QMouseEvent *>(event);
        if (IsLeftOrRightSeparator(mouseEvent->position())) {
            // Restore maxmum width to show the widget and separator.
            if (explorerDockView_ != nullptr) {
                explorerDockView_->setMaximumWidth(explorerDockView_->savedMaxWidth());
            }
            if (outlineDockView_ != nullptr) {
                outlineDockView_->setMaximumWidth(outlineDockView_->savedMaxWidth());
            }
            if (hierarchyDockView_ != nullptr) {
                hierarchyDockView_->setMaximumWidth(hierarchyDockView_->savedMaxWidth());
            }
            if (nodeHierarchyDockView_ != nullptr) {
                nodeHierarchyDockView_->setMaximumWidth(nodeHierarchyDockView_->savedMaxWidth());
            }
        }
        moveSeparatorToHide_ = isSeparator(mouseEvent->pos());
        mouseButtonPressPos_ = mouseEvent->pos();
    } else if (event->type() == QEvent::MouseButtonRelease) {
        auto mouseEvent = static_cast<QMouseEvent *>(event);
        qDebug() << mouseEvent;
        bool horizontalMove = (std::abs(mouseButtonPressPos_.x() - mouseEvent->pos().x()) >
                               std::abs(mouseButtonPressPos_.y() - mouseEvent->pos().y()));
        if (moveSeparatorToHide_ && horizontalMove) {
            constexpr auto threshold_distance = 3;
            // Set both minimum and maxmum width to 0 to hide the widget.
            QPoint leftMovePoint = QPoint(mouseEvent->pos().x() + threshold_distance, mouseEvent->pos().y());
            if ((explorerDockView_ != nullptr &&
                 explorerDockView_->rect().contains(explorerDockView_->mapFrom(this, leftMovePoint), true)) ||
                (outlineDockView_ != nullptr &&
                 outlineDockView_->rect().contains(outlineDockView_->mapFrom(this, leftMovePoint), true))) {
                if (explorerDockView_ != nullptr) {
                    explorerDockView_->setMaximumWidth(0);
                }
                if (outlineDockView_ != nullptr) {
                    outlineDockView_->setMaximumWidth(0);
                }
            }
            QPoint rightMovePoint = QPoint(mouseEvent->pos().x() - threshold_distance, mouseEvent->pos().y());
            if ((hierarchyDockView_ != nullptr &&
                 hierarchyDockView_->rect().contains(hierarchyDockView_->mapFrom(this, rightMovePoint), true)) ||
                (nodeHierarchyDockView_ != nullptr &&
                 nodeHierarchyDockView_->rect().contains(nodeHierarchyDockView_->mapFrom(this, rightMovePoint), true))) {
                if (hierarchyDockView_ != nullptr) {
                    hierarchyDockView_->setMaximumWidth(0);
                }
                if (nodeHierarchyDockView_ != nullptr) {
                    nodeHierarchyDockView_->setMaximumWidth(0);
                }
            }
        }
        moveSeparatorToHide_ = false;
    }
    return QObject::eventFilter(obj, event);
}

void MainWindow::OpenRecentFile() {
    QAction *action = qobject_cast<QAction *>(sender());
    if (action != nullptr) {
        OpenWith(action->data().toString());
    }
}

void MainWindow::NewFile() { tabView_->NewFile(); }

void MainWindow::Open() { tabView_->OpenFile(); }

void MainWindow::OpenWith(const QString &filePath) { tabView_->OpenFile(filePath); }

bool MainWindow::Save() { return tabView_->ActionSave(); }

bool MainWindow::Find() {
    qDebug() << "MainWindow::find()";
    if (searchDialog_ == nullptr) {
        searchDialog_ = new SearchDialog(this);
    }
    searchDialog_->Start(0);
    return true;
}

bool MainWindow::FindNext() {
    if (searchingString().isEmpty()) {
        return false;
    }
    auto cursor = searcher_->FindNext(searchingString(), editView()->textCursor());
    if (!cursor.isNull()) {
        editView()->setTextCursor(cursor);
        return true;
    }
    return false;
}

bool MainWindow::FindPrevious() {
    if (searchingString().isEmpty()) {
        return false;
    }
    auto cursor = searcher_->FindPrevious(searchingString(), editView()->textCursor());
    if (!cursor.isNull()) {
        editView()->setTextCursor(cursor);
        return true;
    }
    return false;
}

bool MainWindow::Replace() {
    qDebug() << "MainWindow::find()";
    if (searchDialog_ == nullptr) {
        searchDialog_ = new SearchDialog(this);
    }
    searchDialog_->Start(1);
    return true;
}

bool MainWindow::MarkUnmarkCursorText() {
    auto editView = this->editView();
    if (editView != nullptr) {
        const auto &text = editView->GetCursorText();
        if (!editView->RemoveMarkText(text)) {
            editView->AddMarkText(text);
        }
        editView->HighlightFocus();
        return true;
    }
    return false;
}

bool MainWindow::UnmarkAll() {
    auto editView = this->editView();
    if (editView != nullptr) {
        editView->ClearMarkTexts();
        editView->HighlightFocus();
        return true;
    }
    return false;
}

bool MainWindow::StepBack() {
    if (tabView()->backwardSteps().size() <= 1) {
        return false;
    }
    tabView()->setStepRunning(true);
    // Move current position to forward steps list.
    tabView()->forwardSteps().push_back(tabView()->backwardSteps().takeLast());

    // Switch the position to the last one.
    auto const current = tabView()->backwardSteps().back();
    auto currentEditView = current.first;
    auto cursor = currentEditView->textCursor();
    cursor.setPosition(current.second);
    tabView()->setCurrentWidget(currentEditView);
    currentEditView->setFocus();
    currentEditView->GotoCursor(cursor);

    tabView()->setStepRunning(false);
    return true;
}

bool MainWindow::StepForward() {
    if (tabView()->forwardSteps().isEmpty()) {
        return false;
    }
    tabView()->setStepRunning(true);
    // Switch the position to the previous one.
    auto const current = tabView()->forwardSteps().takeLast();
    auto currentEditView = current.first;
    auto cursor = currentEditView->textCursor();
    cursor.setPosition(current.second);
    tabView()->setCurrentWidget(currentEditView);
    currentEditView->setFocus();
    currentEditView->GotoCursor(cursor);

    // Move current position to backward steps list.
    tabView()->backwardSteps().push_back(current);

    tabView()->setStepRunning(false);
    return true;
}

bool MainWindow::ZoomIn() {
    auto editView = this->editView();
    if (editView != nullptr) {
        editView->ZoomIn();
        return true;
    }
    auto diffView = this->diffView();
    if (diffView != nullptr) {
        diffView->ZoomIn();
        return true;
    }
    return false;
}

bool MainWindow::ZoomOut() {
    auto editView = this->editView();
    if (editView != nullptr) {
        editView->ZoomOut();
        return true;
    }
    auto diffView = this->diffView();
    if (diffView != nullptr) {
        diffView->ZoomOut();
        return true;
    }
    return false;
}

bool MainWindow::SaveAll() { return false; }

bool MainWindow::SaveAs() { return tabView_->ActionSaveAs(); }

void MainWindow::About() {
    QMessageBox::about(this, tr("About ") + Constants::kAppName,
                       tr("<font color=\"lightgray\"><b>") + Constants::kAppName + tr("<br/>Version: ") +
                           Constants::kVersionStr + tr("</b><br/>") + tr("----------<br/>The <b>") +
                           Constants::kAppName +
                           tr("</b> is a compact text editor "
                              "with common functions, such as search, replace, "
                              "mark, and simple syntax highlighting.<br/><br/>"
                              "<b>Author: <a href=\"mailto:zhang-qh@hotmail.com\">Q</a></b></font>"));
}

void MainWindow::ReadSettings() {
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = screen()->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2, (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

void MainWindow::WriteSettings() {
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

bool MainWindow::MaybeSave() { return true; }

#ifndef QT_NO_SESSIONMANAGER
void MainWindow::HandleCommitData(QSessionManager &manager) {
    if (manager.allowsInteraction()) {
        manager.cancel();
    } else {
        // Non-interactive: save without asking
    }
}
#endif

QString GetToolBarQss(const QString &image_default, const QString &image_enable) {
    QStringList qss;
    qss.append(QString("QToolBar{background-color:rgb(28,28,28); image:%1;}").arg(image_default));
    qss.append(QString("QToolBar QToolButton{border:1px solid transparent; background-color:rgb(28,28,28);}"));
    qss.append(QString("QToolBar QToolButton:hover{border:1px solid transparent; background-color:rgb(60,60,90);}"));
    qss.append(QString("QToolBar QToolButton:enabled{border:1px solid rgb(80,80,100); image:%1;}").arg(image_enable));
    qss.append(
        QString("QToolBar QToolButton:pressed{border:1px solid rgb(23,105,170); background-color:rgb(28,28,28);}"));
    return qss.join("");
}

void MainWindow::UpdateRecentFilesMenu() {
    recentFileActions_.clear();
    recentFilesMenu_->clear();

    auto recentFilesNum = RecentFiles::files().size();
    if (recentFilesNum > 0) {
        recentFilesMenu_->setEnabled(true);
    } else {
        recentFilesMenu_->setEnabled(false);
    }
    qDebug() << "recentFilesNum: " << recentFilesNum;
    for (auto i = 0; i < recentFilesNum; ++i) {
        auto recentFile = RecentFiles::files()[i];
        qDebug() << "recentFile: " << recentFile;
        auto recentFileAction = new QAction(this);
        recentFileAction->setVisible(true);
        recentFileAction->setText(QString::number(i + 1) + ": " + recentFile);
        recentFileAction->setData(recentFile);
        connect(recentFileAction, &QAction::triggered, this, &MainWindow::OpenRecentFile);
        recentFilesMenu_->addAction(recentFileAction);
        recentFileActions_.append(recentFileAction);
    }

    recentFilesMenu_->addSeparator();

    auto clearRecentFilesAction = new QAction(this);
    clearRecentFilesAction->setText(tr("Clear Recently Opened"));
    connect(clearRecentFilesAction, &QAction::triggered, this, [this]() {
        recentFileActions_.clear();
        recentFilesMenu_->clear();
        recentFilesMenu_->setEnabled(false);
        RecentFiles::Clear();
    });
    recentFilesMenu_->addAction(clearRecentFilesAction);
    recentFileActions_.append(clearRecentFilesAction);
}

void MainWindow::SelectAll() {
    auto editView = this->editView();
    if (editView != nullptr) {
        editView->selectAll();
        return;
    }
    auto diffView = this->diffView();
    if (diffView != nullptr) {
        diffView->selectAll();
    }
}

void MainWindow::GotoLine() { (new GotoLineDialog(this))->show(); }

void MainWindow::SyncWrapTextState() {
    QPlainTextEdit::LineWrapMode mode;
    if (shouldWrapText_) {
        mode = QPlainTextEdit::LineWrapMode::WidgetWidth;
    } else {
        mode = QPlainTextEdit::LineWrapMode::NoWrap;
    }
    for (int i = 0; i < tabView()->count(); ++i) {
        auto textView = tabView()->GetEditView(i);
        if (textView == nullptr) {
            continue;
        }
        textView->setLineWrapMode(mode);
    }
}

void MainWindow::SwitchWrapText() {
    shouldWrapText_ = !shouldWrapText_;
    SyncWrapTextState();

    Settings().Set("view", "wrap_text", shouldWrapText_);
}

void MainWindow::SyncShowSpecialCharsVisible(bool shouldShow) {
    QTextOption textOption;
    if (shouldShow) {
        textOption.setFlags(QTextOption::ShowTabsAndSpaces | QTextOption::ShowLineAndParagraphSeparators |
                            QTextOption::ShowDocumentTerminator);
    } else {
        textOption.setFlags(QTextOption::Flag(0));
    }
    for (int i = 0; i < tabView()->count(); ++i) {
        auto textView = tabView()->GetEditView(i);
        if (textView == nullptr) {
            continue;
        }
        textView->document()->setDefaultTextOption(textOption);
        textView->ApplyTabCharNum();
    }
}

void MainWindow::SwitchSpecialCharsVisible() {
    specialCharsVisible_ = !specialCharsVisible_;
    SyncShowSpecialCharsVisible(specialCharsVisible_);

    Settings().Set("view", "all_chars_visible", specialCharsVisible_);
}

void MainWindow::SwitchExplorerWindowVisible() {
    explorerVisible_ = !explorerVisible_;
    if (IsExplorerDockViewShowing() != explorerVisible_) {
        if (explorerVisible_) {
            ShowExplorerDockView();
            if (editView() != nullptr) {
                MainWindow::Instance().SetExplorerDockViewPosition(editView()->filePath());
            }
        } else {
            HideExplorerDockView();
        }
    }

    Settings().Set("view", "explorer_visible", explorerVisible_);
}

void MainWindow::SwitchOutlineWindowVisible() {
    outlineVisible_ = !outlineVisible_;
    if (IsOutlineDockViewShowing() != outlineVisible_) {
        if (outlineVisible_) {
            ShowOutlineDockView();
            if (editView() != nullptr) {
                editView()->TrigerParser();
            }
        } else {
            HideOutlineDockView();
        }
    }

    Settings().Set("view", "outline_visible", outlineVisible_);
}

void MainWindow::SwitchHierarchyWindowVisible() {
    hierarchyVisible_ = !hierarchyVisible_;
    if (IsHierarchyDockViewShowing() != hierarchyVisible_) {
        if (hierarchyVisible_) {
            ShowHierarchyDockView();
            if (editView() != nullptr) {
                editView()->TrigerParser();
            }
        } else {
            HideHierarchyDockView();
        }
    }

    Settings().Set("view", "hierarchy_visible", hierarchyVisible_);
}

void MainWindow::Copy() {
    auto editView = this->editView();
    if (editView != nullptr) {
        editView->copy();
        return;
    }
    auto diffView = this->diffView();
    if (diffView != nullptr) {
        diffView->copy();
    }
}

void MainWindow::Cut() {
    auto editView = this->editView();
    if (editView != nullptr) {
        editView->cut();
        return;
    }
    auto diffView = this->diffView();
    if (diffView != nullptr) {
        diffView->cut();
    }
}

void MainWindow::Paste() {
    auto editView = this->editView();
    if (editView != nullptr) {
        editView->paste();
        return;
    }
    auto diffView = this->diffView();
    if (diffView != nullptr) {
        diffView->paste();
    }
}

void MainWindow::SetCopyAvailable(bool avail) {
    copyAct_->setEnabled(avail);
    cutAct_->setEnabled(avail);
}

void MainWindow::Undo() {
    auto editView = this->editView();
    if (editView != nullptr) {
        editView->undo();
        return;
    }
    auto diffView = this->diffView();
    if (diffView != nullptr) {
        diffView->undo();
    }
}

void MainWindow::Redo() {
    auto editView = this->editView();
    if (editView != nullptr) {
        editView->redo();
        return;
    }
    auto diffView = this->diffView();
    if (diffView != nullptr) {
        diffView->redo();
    }
}

void MainWindow::SetUndoAvailable(bool avail) { undoAct_->setEnabled(avail); }

void MainWindow::SetRedoAvailable(bool avail) { redoAct_->setEnabled(avail); }

void MainWindow::UpdateStatusBarFrequentInfo(const QString &ln, const QString &col, const QString &pos,
                                             const QString &len, const QString &lines) {
    line_->setText(ln);
    column_->setText(col);
    position_->setText(pos);

    length_->setText(len);
    lines_->setText(lines);
}

void MainWindow::UpdateStatusBarRareInfo(const QString &newLineSeq, const QString &encodingSeq,
                                         const QString &insertModeSeq) {
    qDebug() << "newLine: " << newLineSeq << ", encoding: " << encodingSeq << "/" << encoding_->currentIndex();
    newLineChar_->setCurrentIndex(newLineChar_->findText(newLineSeq));
    newLineChar_->ShrinkForPopup();
    newLineChar_->ShrinkForChosen();
    statusBarComboNonUserClick_ = true;
    encoding_->setCurrentIndex(encoding_->findText(encodingSeq));
    statusBarComboNonUserClick_ = false;
    encoding_->ShrinkForPopup();
    encoding_->ShrinkForChosen();
}

void MainWindow::CreateStatusBar() {
    auto qss =
        "color: lightGray;"
        "background-color: rgb(54, 54, 54);";
    auto small_qss =
        "color: lightGray;"
        "background-color: rgb(54, 54, 54);"
        "padding-left: 20px;";
    auto normal_item_qss =
        "color: lightGray;"
        "background-color: rgb(54, 54, 54);"
        "min-width: 60px";
    auto last_item_qss =
        "color: lightGray;"
        "background-color: rgb(54, 54, 54);"
        "min-width: 60px;"
        "padding-right: 100px;";

    line_ = new QLabel(this);
    line_->setStyleSheet(normal_item_qss);
    statusBar()->addPermanentWidget(line_);
    column_ = new QLabel(this);
    column_->setStyleSheet(normal_item_qss);
    statusBar()->addPermanentWidget(column_);
    position_ = new QLabel(this);
    position_->setStyleSheet(last_item_qss);
    statusBar()->addPermanentWidget(position_);

    lines_ = new QLabel(this);
    lines_->setStyleSheet(normal_item_qss);
    statusBar()->addPermanentWidget(lines_);
    length_ = new QLabel(this);
    length_->setStyleSheet(last_item_qss);
    statusBar()->addPermanentWidget(length_);

    // New line char.
    newLineChar_ = new ComboView(this);
    newLineChar_->addItems({Constants::kFormatNewLineUnix, Constants::kFormatNewLineDos, Constants::kFormatNewLineMac});
    statusBar()->addPermanentWidget(newLineChar_);
    newLineChar_->ShrinkForPopup();
    newLineChar_->ShrinkForChosen();
    connect(newLineChar_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int index) { HandleNewLineCharChanged(index); });

    // File encoding.
    encoding_ = new ComboView(this);
    encoding_->addItems(FileEncoding::encodingNames());
    statusBar()->addPermanentWidget(encoding_);
    encoding_->ShrinkForPopup();
    encoding_->ShrinkForChosen();
    connect(encoding_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int index) { HandleEncodingChanged(index); });

    insertMode_ = new QLabel(tr("INS"), this);  // INS/OVR
    insertMode_->setStyleSheet(small_qss);
    statusBar()->addPermanentWidget(insertMode_);

    statusBar()->showMessage(tr("Ready"));
    statusBar()->setStyleSheet(qss);
    // statusBar()->setStyleSheet("color: lightGray;" "background-color: black; border-color: rgb(0, 0, 0);
    // border:1px solid lightGray;");
    //                            "QStatusBar::item{border: 0px}");
}

void MainWindow::HandleNewLineCharChanged(int index) {
    qDebug() << "index: " << index;
    newLineChar_->ShrinkForChosen();
}

void MainWindow::HandleEncodingChanged(int index) {
    qDebug() << "index: " << index;
    encoding_->ShrinkForChosen();

    // Handle the user click.
    if (statusBarComboNonUserClick_) {
        qDebug() << "Non user click, return.";
        return;
    }
    auto editView = this->editView();
    if (editView == nullptr) {
        return;
    }
    if (editView->ShouldSave()) {
        Toast::Instance().Show(Toast::kError, tr("Can't convert the encoding since the text is modified."));
        return;
    }

    auto fileEncoding = FileEncoding(encoding_->currentText());
    qDebug() << "currentText: " << encoding_->currentText() << ", itemText: " << encoding_->itemText(index)
             << ", new mib: " << fileEncoding.hasBom() << fileEncoding.mibEnum()
             << ", current mib: " << editView->fileEncoding().hasBom() << editView->fileEncoding().mibEnum();
    if (fileEncoding.hasBom() == editView->fileEncoding().hasBom() &&
        fileEncoding.mibEnum() == editView->fileEncoding().mibEnum()) {
        return;
    }
    editView->ChangeFileEncoding(std::move(fileEncoding));
}
}  // namespace QEditor
