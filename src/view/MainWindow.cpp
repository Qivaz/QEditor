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

#include <QtWidgets>
#include <QLayoutItem>
#include <QSizePolicy>
#include <MainWindow.h>
#include <MainWindow.h>

#include "ExplorerTreeView.h"
#include "ComboView.h"
#include "SearchDialog.h"
#include "SearchResultList.h"
#include "Settings.h"
#include "Toast.h"
#include "win/WinTheme.h"
#include "Logger.h"

MainWindow::MainWindow()
    : tabView_(new TabView(this))
{
    auto settings = Settings();
    shouldWrapText_ = settings.Get("view", "wraptext").toBool();
    specialCharsVisible_ = settings.Get("view", "allcharsvisible").toBool();
    explorerVisible_ = settings.Get("view", "explorervisible").toBool();
    outlineVisible_ = settings.Get("view", "outlinevisible").toBool();
    qreal opa = settings.Get("window", "opacity").toDouble();

    setAcceptDrops(true);
    setWindowOpacity(opa);

//    setStyleSheet("QMainWindow::separator{ background: rgb(68, 68, 68);"
//                " width: 1px; height: 1px;}");

    setStyleSheet("background: rgb(68, 68, 68); border: 5px, solid, rgb(255, 0, 0);");
    setCentralWidget(tabView_);

    CreateActions();
    CreateStatusBar();

    ReadSettings();

#ifndef QT_NO_SESSIONMANAGER
    QGuiApplication::setFallbackSessionManagementEnabled(false);
    connect(qApp, &QGuiApplication::commitDataRequest, this, &MainWindow::HandleCommitData);
#endif

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

    installEventFilter(this);

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
}

void MainWindow::HandleCurrentTabChanged(int index)
{
    qDebug() << "MainWindow::tabIndexChanged, " << index;
}

void MainWindow::ShowSearchDockView()
{
    if (!searchDockView_->isVisible()) {
        qDebug() << "Search Dock invisible";
        searchDockView_->show();
    } else {
        qDebug() << "Search Dock visible";
    }
}

DockView *MainWindow::CreateSearchDockView()
{
    if (searchDockView_ == nullptr) {
        searchDockView_ = new DockView();
    }
    searchDockView_->setWindowTitle("Search result:");
//    searchDockView_->setFont(QFont("Consolas", 11));
    searchDockView_->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable/* | QDockWidget::DockWidgetFloatable*/);
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
    auto tree = ((ExplorerTreeView*)explorerDockView_->widget());
    if (tree == nullptr) {
        return;
    }
    tree->GotoPathPosition(path);
}

void MainWindow::ShowExplorerDockView()
{
    if (explorerDockView_ == nullptr) {
        auto dir = CreateExplorerDockView();
        dir->setWidget(new ExplorerTreeView(this));
    }
    explorerDockView_->show();
}

void MainWindow::HideExplorerDockView()
{
    if (explorerDockView_ == nullptr) {
        auto dir = CreateExplorerDockView();
        dir->setWidget(new ExplorerTreeView(this));
    }
    explorerDockView_->hide();
}

DockView *MainWindow::CreateExplorerDockView()
{
    if (explorerDockView_ == nullptr) {
        explorerDockView_ = new DockView(this);
    }
    explorerDockView_->setWindowTitle("EXPLORER");
//    explorerDockView_->setFont(QFont("Consolas", 11));
    explorerDockView_->setFeatures(/*QDockWidget::DockWidgetClosable | */QDockWidget::DockWidgetMovable/* | QDockWidget::DockWidgetFloatable*/);
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
    auto list = ((OutlineList*)outlineDockView_->widget());
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

void MainWindow::UpdateOutlineDockView(OutlineList *list)
{
    if (!IsOutlineDockViewShowing()) {
        return;
    }
    outlineDockView_->setWidget(list);
}

void MainWindow::ShowOutlineDockView()
{
    if (outlineDockView_ == nullptr) {
        CreateOutlineDockView();
    }
    outlineDockView_->show();
}

void MainWindow::HideOutlineDockView()
{
    if (outlineDockView_ == nullptr) {
        CreateOutlineDockView();
    }
    outlineDockView_->hide();
}

DockView *MainWindow::CreateOutlineDockView()
{
    if (outlineDockView_ == nullptr) {
        outlineDockView_ = new DockView();
    }
    outlineDockView_->setWindowTitle("OUTLINE");
//    overviewDockView_->setFont(QFont("Consolas", 11));
    outlineDockView_->setFeatures(/*QDockWidget::DockWidgetClosable | */QDockWidget::DockWidgetMovable/* | QDockWidget::DockWidgetFloatable*/);
    addDockWidget(Qt::LeftDockWidgetArea, outlineDockView_);
    return outlineDockView_;
}

bool MainWindow::IsHierarchyDockViewShowing() {
    if (hierarchyDockView_ == nullptr) {
        return false;
    }
    return hierarchyDockView_->isVisible();
}

void MainWindow::UpdateHierarchyDockView(FunctionHierarchy *view)
{
    if (hierarchyDockView_ == nullptr) {
        CreateHierarchyDockView();
    }
    hierarchyDockView_->setWidget(view);
    ShowHierarchyDockView();
}

void MainWindow::ShowHierarchyDockView()
{
    if (hierarchyDockView_ != nullptr && !hierarchyDockView_->isVisible()) {
        hierarchyDockView_->show();
    }
}

void MainWindow::HideHierarchyDockView()
{
    if (hierarchyDockView_ != nullptr && hierarchyDockView_->isVisible()) {
        hierarchyDockView_->hide();
    }
    HideNodeHierarchyDockView();
}

DockView *MainWindow::CreateHierarchyDockView()
{
    if (hierarchyDockView_ == nullptr) {
        hierarchyDockView_ = new DockView();
    }
    hierarchyDockView_->setWindowTitle("FUNCTION HIERARCHY");
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

void MainWindow::UpdateNodeHierarchyDockView(AnfNodeHierarchy *view)
{
    if (nodeHierarchyDockView_ == nullptr) {
        CreateNodeHierarchyDockView();
    }
    nodeHierarchyDockView_->setWindowTitle("NODE HIERARCHY  ( " + view->funcName() + " )");
    if (anfNodeHierarchy_ != nullptr) {
        delete anfNodeHierarchy_;
        anfNodeHierarchy_ = nullptr;
    }
    anfNodeHierarchy_ = view;
    nodeHierarchyDockView_->setWidget(view);
    ShowNodeHierarchyDockView();
}

void MainWindow::ShowNodeHierarchyDockView()
{
    if (nodeHierarchyDockView_ != nullptr && !nodeHierarchyDockView_->isVisible()) {
        nodeHierarchyDockView_->show();
    }
}

void MainWindow::HideNodeHierarchyDockView()
{
    if (nodeHierarchyDockView_ != nullptr && nodeHierarchyDockView_->isVisible()) {
        nodeHierarchyDockView_->hide();
    }
}

DockView *MainWindow::CreateNodeHierarchyDockView()
{
    if (nodeHierarchyDockView_ == nullptr) {
        nodeHierarchyDockView_ = new DockView();
    }
    nodeHierarchyDockView_->setWindowTitle("NODE HIERARCHY");
    nodeHierarchyDockView_->setFeatures(QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::RightDockWidgetArea, nodeHierarchyDockView_);
    return nodeHierarchyDockView_;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug() << "event: " << event->answerRect();
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    qDebug() << "event: " << event->pos()
             << ", " << event->mimeData()->text()
             << ", " << event->mimeData()->urls()
             << ", " << event->mimeData()->html();
    auto urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        qDebug() << "file: " << urls[0].toLocalFile();
        OpenWith(urls[0].toLocalFile());
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (!init_) {
        init_ = true;
        (void)tabView_->AutoLoad();
        auto ev = editView();
        if (ev != nullptr) {
            ev->setFocus();
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    tabView_->AutoStore();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    const auto editView = this->editView();
    if (event->type() == QEvent::HoverMove && editView != nullptr) {
        QHoverEvent *hoverEvent = static_cast<QHoverEvent*>(event);
        auto pos = editView->viewport()->mapFrom(this, hoverEvent->pos());
        auto cursor = editView->cursorForPosition(pos);
        editView->Hover(cursor);
    }
    return QObject::eventFilter(obj, event);
}

void MainWindow::NewFile()
{
    tabView_->NewFile();
}

void MainWindow::Open()
{
    tabView_->OpenFile();
}

void MainWindow::OpenWith(const QString &filePath)
{
    tabView_->OpenFile(filePath);
}

bool MainWindow::Save()
{
    return tabView_->ActionSave();
}

bool MainWindow::Find()
{
    qDebug() << "MainWindow::find()";
    if (searchDialog_ == nullptr) {
        searchDialog_ = new SearchDialog(this);
    }
    searchDialog_->Start(0);
    return true;
}

bool MainWindow::Replace()
{
    qDebug() << "MainWindow::find()";
    if (searchDialog_ == nullptr) {
        searchDialog_ = new SearchDialog(this);
    }
    searchDialog_->Start(1);
    return true;
}

bool MainWindow::Test()
{
    qDebug() << "MainWindow::test()";
    QTextCursor cursor = editView()->textCursor();
//    cursor.setPosition(cursor.position(), QTextCursor::MoveAnchor);
    cursor.setPosition(cursor.position()/* + 10*/, QTextCursor::KeepAnchor);
//    cursor.select(QTextCursor::WordUnderCursor);
//    cursor.select(QTextCursor::Document);
//    cursor.select(QTextCursor::BlockUnderCursor);
    cursor.select(QTextCursor::LineUnderCursor);
    QTextCharFormat fmt;
    fmt.setForeground(Qt::green);
    fmt.setBackground(Qt::blue);
    cursor.mergeCharFormat(fmt);
    cursor.clearSelection();


//    auto tc = editView()->textCursor();
//    tc.select(QTextCursor::WordUnderCursor);
//    qDebug() << "selectedText: " << tc.selectedText();
    return true;
}

bool MainWindow::MarkUnmarkCursorText()
{
    const auto &text = editView()->GetCursorText();
    if (!editView()->RemoveMarkText(text)) {
        editView()->AddMarkText(text);
    }
    editView()->HighlightMarkTexts();
    return true;
}

bool MainWindow::UnmarkAll()
{
    editView()->ClearMarkTexts();
    editView()->HighlightFocus();
    return true;
}

bool MainWindow::StepBack()
{
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

bool MainWindow::StepForward()
{
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

bool MainWindow::ZoomIn()
{
    editView()->ZoomIn();
    return true;
}

bool MainWindow::ZoomOut()
{
    editView()->ZoomOut();
    return true;
}

bool MainWindow::SaveAll()
{
    return false;
}

bool MainWindow::SaveAs()
{
    return tabView_->ActionSaveAs();
}

void MainWindow::About()
{
   QMessageBox::about(this, tr("About ") + Constants::kAppName,
                      tr("<font color=\"lightgray\"><b>") + Constants::kAppName +
                      tr("<br/>Version: V22R10") + tr("</b><br/>") +
                      tr("----------<br/>The <b>") + Constants::kAppName +
                      "</b> is a compact text editor "
                      "with common functions, such as search, replace, "
                      "mark, and simple syntax highlighting.<br/><br/>"
                      "<b>Author: <a href=\"mailto:zhang-qh@hotmail.com\">Q</a></b></font>");
}

void MainWindow::ReadSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = screen()->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

void MainWindow::WriteSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

bool MainWindow::MaybeSave()
{
    return true;
}

#ifndef QT_NO_SESSIONMANAGER
void MainWindow::HandleCommitData(QSessionManager &manager)
{
    if (manager.allowsInteraction()) {
        manager.cancel();
    } else {
        // Non-interactive: save without asking
    }
}
#endif

QString GetToolBarQss(const QString &image_default, const QString &image_enable)
{
    QStringList qss;
    qss.append(QString("QToolBar { background-color: rgb(28, 28, 28); image: %1; }").arg(image_default));
    qss.append(QString("QToolBar QToolButton { border: 1px solid transparent; background-color: rgb(28, 28, 28); }"));
    qss.append(QString("QToolBar QToolButton:hover { border: 1px solid transparent; background-color: rgb(60, 60, 90); }"));
    qss.append(QString("QToolBar QToolButton:enabled { border: 1px solid rgb(80, 80, 100); image: %1; }").arg(image_enable));
    qss.append(QString("QToolBar QToolButton:pressed { border: 1px solid rgb(23, 105, 170); background-color: rgb(28, 28, 28);}"));
    return qss.join("");
}

void MainWindow::CreateActions()
{
    // TODO:
    // QMenu{ and QMenu::separator not work.
    menuBar()->setStyleSheet("color: lightGray; selection-background-color: DodgerBlue; background-color: rgb(28, 28, 28); border: none;\
                             QMenu{color: green; background-color: rgb(255, 28, 28);}\
                             QMenu::separator {height: 1px; background: rgb(255, 0, 0);}");

    auto toolBarStyle = "QToolBar { color: lightGray;" "background-color: rgb(28, 28, 28); }"\
                        "QToolBar QToolButton { border: 2px solid transparent; background-color: rgb(28, 28, 28); }"
                        "QToolBar QToolButton:hover { border: 2px solid transparent; background-color: rgb(60, 60, 90); }"
                        "QToolBar QToolButton:enabled { border: 2px solid rgb(80, 80, 100); }"
                        "QToolBar QToolButton:pressed { border: 2px solid rgb(23, 105, 170); background-color: rgb(28, 28, 28);}"
                        "QToolBar QToolButton:checked { border: 2px solid rgb(23, 105, 170); background-color: rgb(28, 28, 28);}";
    // File menu.
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar *fileToolBar = addToolBar(tr("File"));
    fileToolBar->setStyleSheet(toolBarStyle);

    // TODO:
    // Use addPixmap() set on/off icons.
    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/file.svg"));
    QAction *newAct = new QAction(newIcon, tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
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
    cutAct_->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct_, &QAction::triggered, this, &MainWindow::Cut);
    editMenu->addAction(cutAct_);
    editToolBar->addAction(cutAct_);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.svg"));
    copyAct_ = new QAction(copyIcon, tr("&Copy"), this);
    copyAct_->setShortcuts(QKeySequence::Copy);
    copyAct_->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct_, &QAction::triggered, this, &MainWindow::Copy);
    editMenu->addAction(copyAct_);
    editToolBar->addAction(copyAct_);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/images/paste.svg"));
    QAction *pasteAct = new QAction(pasteIcon, tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, &QAction::triggered, this, &MainWindow::Paste);
    editMenu->addAction(pasteAct);
    editToolBar->addAction(pasteAct);

    menuBar()->addSeparator();
#endif // !QT_NO_CLIPBOARD

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
    QAction *findAct = new QAction(findIcon, tr("&Find"), this);
    findAct->setShortcuts(QKeySequence::Find);
    findAct->setStatusTip(tr("Find"));
    connect(findAct, &QAction::triggered, this, &MainWindow::Find);
    selectMenu->addAction(findAct);
    searchToolBar->addAction(findAct);

    const QIcon replaceIcon = QIcon::fromTheme("select-replace", QIcon(":/images/replace.svg"));
    QAction *replaceAct = new QAction(replaceIcon, tr("&Replace"), this);
    auto replaceKeySeq = QKeySequence(Qt::CTRL + Qt::Key_H);
    replaceAct->setShortcut(replaceKeySeq);  // QKeySequence::Replace
    replaceAct->setStatusTip(tr("Replace"));
    connect(replaceAct, &QAction::triggered, this, &MainWindow::Replace);
    selectMenu->addAction(replaceAct);
    searchToolBar->addAction(replaceAct);

    selectMenu->addSeparator();

    const QIcon gotoLineIcon = QIcon::fromTheme("select-got-line", QIcon(":/images/goto-line.svg"));
    QAction *gotoLineAct = new QAction(gotoLineIcon, tr("&Goto line"), this);
    auto gotoLineKeySeq = QKeySequence(Qt::CTRL + Qt::Key_G);  // QKeySequence(tr("F3, Ctrl+G"))
    gotoLineAct->setShortcut(gotoLineKeySeq);
    gotoLineAct->setStatusTip(tr("GotoLine"));
    connect(gotoLineAct, &QAction::triggered, this, &MainWindow::GotoLine);
    selectMenu->addAction(gotoLineAct);
    searchToolBar->addAction(gotoLineAct);

    const QIcon selectAllIcon = QIcon::fromTheme("select-all-lines", QIcon(":/images/select-all.svg"));
    QAction *selectAllAct = new QAction(selectAllIcon, tr("&Select all lines"), this);
    selectAllAct->setShortcuts(QKeySequence::SelectAll);
    selectAllAct->setStatusTip(tr("SelectAll"));
    connect(selectAllAct, &QAction::triggered, this, &MainWindow::SelectAll);
    selectMenu->addAction(selectAllAct);
    searchToolBar->addAction(selectAllAct);

    selectMenu->addSeparator();

    QToolBar *markToolBar = addToolBar(tr("Mark"));
    markToolBar->setStyleSheet(toolBarStyle);

    const QIcon markIcon = QIcon::fromTheme("select-mark", QIcon(":/images/highlighter.svg"));
    QAction *markAct = new QAction(markIcon, tr("&Mark or unmark"), this);
    auto markKeySeq = QKeySequence(Qt::SHIFT + Qt::Key_F8);
    markAct->setShortcut(markKeySeq);
    markAct->setStatusTip(tr("Mark & Unmack"));
    connect(markAct, &QAction::triggered, this, &MainWindow::MarkUnmarkCursorText);
    selectMenu->addAction(markAct);
    markToolBar->addAction(markAct);

    const QIcon unmarkAllIcon = QIcon::fromTheme("select-unmark-all", QIcon(":/images/eraser.svg"));
    QAction *unmarkAllAct = new QAction(unmarkAllIcon, tr("&Unmark all"), this);
    auto unmarkAllKeySeq = QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F8);
    unmarkAllAct->setShortcut(unmarkAllKeySeq);
    unmarkAllAct->setStatusTip(tr("Mark"));
    connect(unmarkAllAct, &QAction::triggered, this, &MainWindow::UnmarkAll);
    selectMenu->addAction(unmarkAllAct);
    markToolBar->addAction(unmarkAllAct);

    selectMenu->addSeparator();

    QToolBar *stepToolBar = addToolBar(tr("Step"));
    stepToolBar->setStyleSheet(toolBarStyle);

    const QIcon stepBackIcon = QIcon::fromTheme("step-back", QIcon(":/images/arrow-thick-to-left.svg"));
    QAction *stepBackAct = new QAction(stepBackIcon, tr("&Step back"), this);
    auto stepBackSeq = QKeySequence(Qt::ALT + Qt::Key_Left);
    stepBackAct->setShortcut(stepBackSeq);
    stepBackAct->setStatusTip(tr("Step back"));
    connect(stepBackAct, &QAction::triggered, this, &MainWindow::StepBack);
    selectMenu->addAction(stepBackAct);
    stepToolBar->addAction(stepBackAct);

    const QIcon stepForwardIcon = QIcon::fromTheme("step-forward", QIcon(":/images/arrow-thick-to-right.svg"));
    QAction *stepForwardAct = new QAction(stepForwardIcon, tr("&Step forward"), this);
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
    QAction *zoomInAct = new QAction(zoomInIcon, tr("Zoom in"), this);
    zoomInAct->setStatusTip(tr("ZoomIn"));
    connect(zoomInAct, &QAction::triggered, this, &MainWindow::ZoomIn);
    viewMenu->addAction(zoomInAct);
    viewToolBar->addAction(zoomInAct);

    const QIcon zoomOutIcon = QIcon::fromTheme("view-zoomout", QIcon(":/images/zoom-out.svg"));
    QAction *zoomOutAct = new QAction(zoomOutIcon, tr("Zoom out"), this);
    zoomOutAct->setStatusTip(tr("ZoomOut"));
    connect(zoomOutAct, &QAction::triggered, this, &MainWindow::ZoomOut);
    viewMenu->addAction(zoomOutAct);
    viewToolBar->addAction(zoomOutAct);

    viewMenu->addSeparator();

    const QIcon wrapTextIcon = QIcon::fromTheme("view-wrap-text", QIcon(":/images/wrap-text.svg"));
    QAction *wrapTextAct = new QAction(wrapTextIcon, tr("Wrap text"), this);
    wrapTextAct->setStatusTip(tr("WrapText"));
    wrapTextAct->setCheckable(true);
    wrapTextAct->setChecked(shouldWrapText_);
    connect(wrapTextAct, &QAction::triggered, this, &MainWindow::SwitchWrapText);
    viewMenu->addAction(wrapTextAct);
    viewToolBar->addAction(wrapTextAct);

    const QIcon showAllCharsIcon = QIcon::fromTheme("view-show-chars", QIcon(":/images/show-all-chars.svg"));
    QAction *showAllCharsAct = new QAction(showAllCharsIcon, tr("Show all chars"), this);
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
    QAction *showDirAct = new QAction(showDirIcon, tr("Show explorer window"), this);
    showDirAct->setStatusTip(tr("ShowExplorerWindow"));
    showDirAct->setCheckable(true);
    showDirAct->setChecked(explorerVisible_);
    connect(showDirAct, &QAction::triggered, this, &MainWindow::SwitchExplorerWindowVisible);
    viewMenu->addAction(showDirAct);
    viewToolBar2->addAction(showDirAct);

    const QIcon showOutlineIcon = QIcon::fromTheme("view-show-outline", QIcon(":/images/brackets-contain.svg"));
    QAction *showOutlineAct = new QAction(showOutlineIcon, tr("Show outline window"), this);
    showOutlineAct->setStatusTip(tr("ShowOutlineWindow"));
    showOutlineAct->setCheckable(true);
    showOutlineAct->setChecked(outlineVisible_);
    connect(showOutlineAct, &QAction::triggered, this, &MainWindow::SwitchOutlineWindowVisible);
    viewMenu->addAction(showOutlineAct);
    viewToolBar2->addAction(showOutlineAct);

    const QIcon showHierarchyIcon = QIcon::fromTheme("view-show-hierarchy", QIcon(":/images/sitemap.svg"));
    QAction *showHierarchyAct = new QAction(showHierarchyIcon, tr("Show hierarchy window"), this);
    showHierarchyAct->setStatusTip(tr("ShowHierarchyWindow"));
    showHierarchyAct->setCheckable(true);
    showHierarchyAct->setChecked(outlineVisible_);
    connect(showHierarchyAct, &QAction::triggered, this, &MainWindow::SwitchHierarchyWindowVisible);
    viewMenu->addAction(showHierarchyAct);
    viewToolBar2->addAction(showHierarchyAct);

    // Help menu.
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::About);
    aboutAct->setStatusTip(tr("Show the application's About box"));

#ifndef QT_NO_CLIPBOARD
    cutAct_->setEnabled(false);
    copyAct_->setEnabled(false);
    undoAct_->setEnabled(false);
    redoAct_->setEnabled(false);
#endif // !QT_NO_CLIPBOARD
}

void MainWindow::SelectAll()
{
    editView()->selectAll();
}

void MainWindow::GotoLine()
{
    if (gotoLineDialog_ == nullptr) {
        gotoLineDialog_ = new GotoLineDialog(this);
    }
    gotoLineDialog_->show();
}

void MainWindow::SyncWrapTextState()
{
    QPlainTextEdit::LineWrapMode mode;
    if (shouldWrapText_) {
        mode = QPlainTextEdit::LineWrapMode::WidgetWidth;
    } else {
        mode = QPlainTextEdit::LineWrapMode::NoWrap;
    }
    for (int i = 0; i < tabView()->count(); ++i) {
        tabView()->GetEditView(i)->setLineWrapMode(mode);
    }
}

void MainWindow::SwitchWrapText()
{
    shouldWrapText_ = !shouldWrapText_;
    SyncWrapTextState();

    Settings().Set("view", "wraptext", shouldWrapText_);
}

void MainWindow::SyncShowSpecialCharsVisible(bool shouldShow)
{
    QTextOption textOption;
    if (shouldShow) {
        textOption.setFlags(QTextOption::ShowTabsAndSpaces |
                            QTextOption::ShowLineAndParagraphSeparators |
                            QTextOption::ShowDocumentTerminator);
    } else {
        textOption.setFlags(0);
    }
    for (int i = 0; i < tabView()->count(); ++i) {
        tabView()->GetEditView(i)->document()->setDefaultTextOption(textOption);
        tabView()->GetEditView(i)->ApplyTabCharNum();
    }
}

void MainWindow::SwitchSpecialCharsVisible()
{
    specialCharsVisible_ = !specialCharsVisible_;
    SyncShowSpecialCharsVisible(specialCharsVisible_);

    Settings().Set("view", "allcharsvisible", specialCharsVisible_);
}

void MainWindow::SwitchExplorerWindowVisible()
{
    if (editView() == nullptr) {
        return;
    }
    explorerVisible_ = !explorerVisible_;
    if (IsExplorerDockViewShowing() == explorerVisible_) {
        return;
    }

    if (explorerVisible_) {
        ShowExplorerDockView();
        MainWindow::Instance().SetExplorerDockViewPosition(editView()->filePath());
    } else {
        HideExplorerDockView();
    }

    Settings().Set("view", "explorervisible", explorerVisible_);
}

void MainWindow::SwitchOutlineWindowVisible()
{
    if (editView() == nullptr) {
        return;
    }
    outlineVisible_ = !outlineVisible_;
    if (IsOutlineDockViewShowing() == outlineVisible_) {
        return;
    }

    if (outlineVisible_) {
        ShowOutlineDockView();
        editView()->TrigerParser();
    } else {
        HideOutlineDockView();
    }

    Settings().Set("view", "outlinevisible", outlineVisible_);
}

void MainWindow::SwitchHierarchyWindowVisible()
{
    if (editView() == nullptr) {
        return;
    }
    hierarchyVisible_ = !hierarchyVisible_;
    if (IsHierarchyDockViewShowing() == hierarchyVisible_) {
        return;
    }

    if (hierarchyVisible_) {
        ShowHierarchyDockView();
        editView()->TrigerParser();
    } else {
        HideHierarchyDockView();
    }

    Settings().Set("view", "hierarchyvisible", hierarchyVisible_);
}

void MainWindow::Copy()
{
    editView()->copy();
}

void MainWindow::Cut()
{
    editView()->cut();
}

void MainWindow::Paste()
{
    editView()->paste();
}

void MainWindow::SetCopyAvailable(bool avail)
{
    copyAct_->setEnabled(avail);
    cutAct_->setEnabled(avail);
}

void MainWindow::Undo()
{
    editView()->undo();
}

void MainWindow::Redo()
{
    editView()->redo();
}

void MainWindow::SetUndoAvailable(bool avail)
{
    undoAct_->setEnabled(avail);
}

void MainWindow::SetRedoAvailable(bool avail)
{
    redoAct_->setEnabled(avail);
}

void MainWindow::UpdateStatusBarFrequentInfo(const QString &ln, const QString &col, const QString &pos,
                                             const QString &len, const QString &lines)
{
    line_->setText(ln);
    column_->setText(col);
    position_->setText(pos);

    length_->setText(len);
    lines_->setText(lines);
}

void MainWindow::UpdateStatusBarRareInfo(const QString &newLineSeq, const QString &encodingSeq,
                                         const QString &insertModeSeq)
{
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

void MainWindow::CreateStatusBar()
{
    auto qss = "color: lightGray;"
               "background-color: rgb(54, 54, 54);";
    auto small_qss = "color: lightGray;"
                     "background-color: rgb(54, 54, 54);"
                     "padding-left: 20px;";
    auto normal_item_qss = "color: lightGray;"
                            "background-color: rgb(54, 54, 54);"
                            "min-width: 60px";
    auto last_item_qss = "color: lightGray;"
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
    newLineChar_->addItems({Constants::kFormatNewLineUnix,
                            Constants::kFormatNewLineDos,
                            Constants::kFormatNewLineMac});
    statusBar()->addPermanentWidget(newLineChar_);
    newLineChar_->ShrinkForPopup();
    newLineChar_->ShrinkForChosen();
    connect(newLineChar_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int index){ HandleNewLineCharChanged(index); });

    // File encoding.
    encoding_ = new ComboView(this);
    encoding_->addItems({Constants::kEncodingSystem,    Constants::kEncodingUtf8,      Constants::kEncodingUtf8Bom,
                         Constants::kEncodingUtf16,     Constants::kEncodingUtf16Be,   Constants::kEncodingUtf16Le,
                         Constants::kEncodingUtf32,     Constants::kEncodingUtf32Be,   Constants::kEncodingUtf32Le});
    statusBar()->addPermanentWidget(encoding_);
    encoding_->ShrinkForPopup();
    encoding_->ShrinkForChosen();
    connect(encoding_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int index){ HandleEncodingChanged(index); });

    insertMode_ = new QLabel("INS", this);  // INS/OVR
    insertMode_->setStyleSheet(small_qss);
    statusBar()->addPermanentWidget(insertMode_);

    statusBar()->showMessage(tr("Ready"));
    statusBar()->setStyleSheet(qss);
//    statusBar()->setStyleSheet("color: lightGray;" "background-color: black; border-color: rgb(0, 0, 0); border:1px solid lightGray;");
//                               "QStatusBar::item{border: 0px}");
}

void MainWindow::HandleNewLineCharChanged(int index)
{
    qDebug() << "index: " << index;
    newLineChar_->ShrinkForChosen();
}

void MainWindow::HandleEncodingChanged(int index)
{
    qDebug() << "index: " << index;
    encoding_->ShrinkForChosen();

    // Handle the user click.
    if (statusBarComboNonUserClick_) {
        qDebug() << "Non user click, return.";
        return;
    }
    if (editView()->ShouldSave()) {
        Toast::Instance().Show(Toast::kError, "Can't convert the encoding since the text is modified.");
        return;
    }
    auto mib = editView()->fileEncoding().GetMibByDescription(encoding_->currentText());
    auto fileEncoding = FileEncoding(mib);
    qDebug() << "currentText: " << encoding_->currentText() << ", itemText: " << encoding_->itemText(index)
             << ", new mib: " << fileEncoding.hasBom() << fileEncoding.mibEnum()
             << ", current mib: " << editView()->fileEncoding().hasBom() << editView()->fileEncoding().mibEnum();
    if (fileEncoding.hasBom() == editView()->fileEncoding().hasBom() &&
        fileEncoding.mibEnum() == editView()->fileEncoding().mibEnum()) {
        return;
    }
    editView()->ChangeFileEncoding(std::move(fileEncoding));
}
