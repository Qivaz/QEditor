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

#include "MainTabView.h"

#include <QAbstractButton>
#include <QApplication>
#include <QCursor>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QStatusBar>
#include <QTabBar>
#include <QTextCodec>

#include "FileEncoding.h"
#include "FileRecorder.h"
#include "MainWindow.h"
#include "Toast.h"
#include "Logger.h"

TabView::TabView(QWidget *parent) : QTabWidget(parent), menu_(new QMenu())
{
    setAttribute(Qt::WA_StyledBackground);
    setMovable(true);
    setTabsClosable(true);
    setStyleSheet("QTabWidget { color: darkGray; background-color: rgb(28, 28, 28); selection-color: red; selection-background-color: green; border: none; }"
                  "QTabBar::tab { color: white; background-color: rgb(54, 54, 54); border: 1px solid rgb(54, 54, 54); "\
                                "border-left-style: solid; border-left-width: 0.5px; border-left-color: gray; "\
                                "border-right-style: solid; border-right-width: 0.5px; border-right-color: gray;"\
                                "padding: 3px; border-top-left-radius: 5px; border-top-right-radius: 5px; }"
//                  "QTabBar::tab:selected { background: QColor(0, 0, 0); border-bottom-color: #CBC7FF; border-bottom-width: 3px}"
                  "QTabBar::tab:selected { background: QColor(0, 0, 0); border-bottom-color: #1769AA; border-bottom-width: 3px}"
                  "QTabBar::tab:hover { background-color: QColor(0, 0, 0); }"
                  "QTabBar::close-button { border-image: url(:/images/x-circle.svg); }"
                  "QTabBar::close-button:hover { background: red; border-image: url(:/images/x.svg); }");


    menu_->setStyleSheet(
                         "\
                         QMenu {\
                             background-color: rgb(28, 28, 28);\
                             margin: 2px 2px;\
                         }\
                         QMenu::item {\
                             color: rgb(225, 225, 225);\
                             background-color: rgb(28, 28, 28);\
                             padding: 5px 5px;\
                         }\
                         QMenu::item:selected {\
                             background-color: rgb(0, 122, 204);\
                         }\
                         QMenu::item:pressed {\
                             border: 1px solid rgb(60, 60, 60); \
                             background-color: lightGray; \
                         }\
                        ");

    connect(tabBar(), &QTabBar::currentChanged, this, &TabView::HandleCurrentIndexChanged);
    connect(tabBar(), &QTabBar::tabBarDoubleClicked, this, &TabView::HandleTabBarDoubleClicked);
    connect(tabBar(), &QTabBar::tabBarClicked, this, &TabView::HandleTabBarClicked);
    connect(this, &QTabWidget::tabCloseRequested, this, &TabView::HandleTabCloseRequested);
}

void TabView::ChangeTabCloseButtonToolTip(int index, const QString &tip)
{
    auto closeButton = (QAbstractButton*)(tabBar()->tabButton(index, QTabBar::ButtonPosition::RightSide));
    closeButton->setToolTip(tip);
}

// Update with current edit view if 'index' is -1.
// Otherwise, with edit view at 'index'.
void TabView::UpdateWindowTitle(int index)
{
    EditView *editView = nullptr;
    if (index == -1) {
        editView = CurrentEditView();
    } else {
        editView = GetEditView(index);
    }
    if (editView == nullptr) {
        return;
    }
    MainWindow *win = (MainWindow*)(this->parent());
    auto title = editView->fileName() + " - " + QCoreApplication::applicationName();
    if (editView->ShouldSave()) {
        title = "* " + title;
    }
    win->setWindowTitle(title);
}

void TabView::HandleCurrentIndexChanged(int index)
{
    qDebug() << "index: " << index;
    auto editView = GetEditView(index);
    if (editView == nullptr) {
        return;
    }
    editView->UpdateStatusBarWithCursor();
    editView->TrigerParser();
    if (editView->fileLoaded()) {
        RecordStep(editView, editView->textCursor().position());
    }
    if (MainWindow::Instance().IsExplorerDockViewShowing()) {
        MainWindow::Instance().SetExplorerDockViewPosition(editView->filePath());
    }

    UpdateWindowTitle();

    // Update status bar info. if file encoding changes.
    MainWindow::Instance().UpdateStatusBarRareInfo("Unix", CurrentEditView()->fileEncoding().description(), 0);
}

void TabView::HandleTabBarClicked(int index)
{
    qDebug() << "index: " << index;
    if (index == -1) {
        return;
    }
    setCurrentIndex(index);
    if (QApplication::mouseButtons() == Qt::RightButton) {
        qDebug() << "TabView::barClicked(RightButton), index: " << index;
        menu_->clear();
        QAction *closeAction = new QAction("Close  (Click close button)");
        menu_->addAction(closeAction);
        connect(closeAction, &QAction::triggered, this, &TabView::TabCloseMaybeSave);
        menu_->popup(QCursor::pos());

        QAction *forceCloseAction = new QAction("Force close  (Double click)");
        menu_->addAction(forceCloseAction);
        connect(forceCloseAction, &QAction::triggered, this, &TabView::TabForceClose);
        menu_->popup(QCursor::pos());

        QAction *closeAllAction = new QAction("Force close all");
        menu_->addAction(closeAllAction);
        connect(closeAllAction, &QAction::triggered, this, [&]() {
            clear();
        });
        menu_->popup(QCursor::pos());
    }
}

void TabView::HandleTabBarDoubleClicked(int index)
{
    qDebug() << "index: " << index;
    if (index == -1) {
        return;
    }
    (void)TabForceClose();
}

void TabView::HandleTabCloseRequested(int index)
{
    auto currentEditView = GetEditView(index);
    qDebug() << "index: " << index << ", should save: " << currentEditView->ShouldSave()
               << ", " << currentEditView->filePath();
    (void)TabCloseMaybeSaveInner(currentEditView);
}

bool TabView::ActionSave()
{
    auto currentEditView = CurrentEditView();
    if (!currentEditView->ShouldSave()) {
        return true;
    }

    if (currentEditView->filePath().isEmpty()) {  // New file.
        if (currentEditView->SaveAs()) {
            openFiles().insert(currentEditView->filePath());
            if (currentEditView->newFileNum() != 0) {
                NewFileNum::SetNumber(currentEditView->newFileNum(), false);
            }
            return true;
        }
    } else {  // Open file.
        return currentEditView->SaveFile(currentEditView->filePath());
    }
    return false;
}

bool TabView::ActionSaveAs()
{
    auto currentEditView = CurrentEditView();
    if (currentEditView->filePath().isEmpty()) {  // New file.
        if (currentEditView->SaveAs()) {
            openFiles().insert(currentEditView->filePath());
            if (currentEditView->newFileNum() != 0) {
                NewFileNum::SetNumber(currentEditView->newFileNum(), false);
            }
            return true;
        }
    } else {  // Open file.
        auto oldFilePath = currentEditView->filePath();
        if (currentEditView->SaveAs()) {
            if (currentEditView->filePath() != oldFilePath) {
                openFiles().remove(oldFilePath);
                openFiles().insert(currentEditView->filePath());
            }
            return true;
        }
    }
    return false;
}


bool TabView::TabCloseMaybeSave()
{
    auto currentEditView = CurrentEditView();
    return TabCloseMaybeSaveInner(currentEditView);
}

bool TabView::TabCloseMaybeSaveInner(EditView *editView)
{
    if (editView->filePath().isEmpty()) {  // New file.
        if (editView->document()->isEmpty()) {
            // Close tab.
            removeTab(indexOf(editView));
            if (editView->newFileNum() != 0) {
                NewFileNum::SetNumber(editView->newFileNum(), false);
            }
            return true;
        }
        const QMessageBox::StandardButton res
            = QMessageBox::warning(this, tr(Constants::kAppName),
                                   tr("The document has been modified.\n"
                                      "Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (res == QMessageBox::Discard ||
            (res == QMessageBox::Save && editView->SaveAs())) {
            // Close tab.
            removeTab(indexOf(editView));
            if (editView->newFileNum() != 0) {
                NewFileNum::SetNumber(editView->newFileNum(), false);
            }
        }
    } else {  // Open file.
        if (editView->MaybeSave()) {
            // Close tab.
            openFiles().remove(editView->filePath());
            removeTab(indexOf(editView));
        }
    }
    return false;
}

bool TabView::TabForceClose()
{
    // Close without save.
    auto currentEditView = CurrentEditView();
    openFiles().remove(currentEditView->filePath());  // Just force remove.
    removeTab(indexOf(currentEditView));
    if (currentEditView->newFileNum() != 0) {
        NewFileNum::SetNumber(currentEditView->newFileNum(), false);
    }
    return true;
}

void TabView::AutoStore()
{
    QVector<EditView*> editViews;
    for (int i = 0; i < count(); ++i) {
        editViews.push_back(GetEditView(i));
    }
    FileRecorder fileRecorder;
    fileRecorder.SetPos(currentIndex());
    fileRecorder.SetEditViews(editViews);
    fileRecorder.StoreFiles();
}

bool TabView::AutoLoad()
{
    // If a file opened before auto load.
    auto currentEditView = CurrentEditView();

    FileRecorder fileRecorder;
    fileRecorder.LoadFiles();
    int fileCount = fileRecorder.GetFileCount();
    for (int i = 0; i < fileCount; ++i) {
        QString fileName;
        QString filePathTip;
        EditView *editView;
        bool modified = false;
        auto fileInfo = fileRecorder.GetFileInfo(i);
        if (fileInfo.IsNewFile()) {  // New file.
            fileName = QString("new ") + QString::number(fileInfo.num_);
            filePathTip = fileName;
            editView = new EditView(fileName, this);
            editView->setNewFileNum(fileInfo.num_);
            modified = true;
            NewFileNum::SetNumber(editView->newFileNum(), true);
        } else {  // Open file.
            QFileInfo qFileInfo = QFileInfo(fileInfo.path_);
            fileName = qFileInfo.fileName();
            filePathTip = qFileInfo.filePath();

            // Don't open file not exist anymore.
            QFile file(filePathTip);
            if (!file.open(QFile::ReadOnly | QFile::Text)) {
                QMessageBox::warning(this, tr(Constants::kAppName),
                                     tr("Cannot read file %1:\n%2.")
                                     .arg(QDir::toNativeSeparators(filePathTip), file.errorString()));
                continue;
            }
            // Don't open file multiple times.
            if (openFiles_.contains(filePathTip)) {
                Toast::Instance().Show(Toast::kWarning, filePathTip + " already opened.");
                continue;
            }

            openFiles_.insert(filePathTip);
            editView = new EditView(qFileInfo, this);
            if (fileInfo.IsChangedOpenFile()) {
                modified = true;
            }
        }

        if (fileInfo.IsOriginalOpenFile()) {
            if (!LoadFile(editView, fileInfo.path_)) {
                delete editView;
                continue;
            }
        } else {
            const auto &text = fileRecorder.GetText(i);
            int mibEnum = fileRecorder.GetMibEnum(i);
//            qDebug() << "load plain text: " << text << ", mibEnum: " << mibEnum << ", for tab " << i;
            editView->setFileEncoding(FileEncoding(mibEnum));
            editView->setPlainText(text);
            editView->setFileLoaded(true);
        }

        addTab(editView, fileName);
        editView->SetModified(modified);
        setCurrentIndex(count() - 1);
        setTabToolTip(count() - 1, filePathTip);
    }

    // Not switch the current tab, if a file opened before auto load.
    if (currentEditView != nullptr) {
        setCurrentWidget(currentEditView);
        return true;
    }

    qDebug() << "fileCount: " << fileCount << ", focused index: " << fileRecorder.GetPos();
    bool loaded = (fileCount != 0);
    if (loaded) {
        setCurrentIndex(fileRecorder.GetPos());
    }
    return loaded;
}

void TabView::NewFile()
{
    int num = NewFileNum::GetNumber();
    QString fileName = QString("new ") + QString::number(num);
    auto editView = new EditView(fileName, this);
    editView->setNewFileNum(num);
    qDebug() << "fileName: " << fileName << ", num: " << num << ", editView: " << editView;
    addTab(editView, fileName);
    qDebug() << ", tabView_: " << this;
    editView->SetModified(true);
    setCurrentIndex(count() - 1);
    setTabToolTip(count() - 1, fileName);
    editView->setFocus();
}

void TabView::OpenFile()
{
    OpenFile(QFileDialog::getOpenFileName(this));
}

void TabView::OpenFile(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return;
    }
    QFileInfo fileInfo = QFileInfo(filePath);

    // Don't open file multiple times.
    if (openFiles_.contains(fileInfo.filePath())) {
        Toast::Instance().Show(Toast::kWarning, fileInfo.filePath() + " already opened.");
        return;
    }
    openFiles_.insert(fileInfo.filePath());

    auto editView = new EditView(fileInfo, this);
    editView->setNewFileNum(0);  // Set new file number as 0 for open file.
    if (!LoadFile(editView, filePath)) {
        delete editView;
        return;
    }

    addTab(editView, fileInfo.fileName());
    editView->SetModified(false);
    setCurrentIndex(count() - 1);
    setTabToolTip(count() - 1, filePath);
}

bool TabView::LoadFile(EditView *editView, const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr(Constants::kAppName),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(filePath), file.errorString()));
        return false;
    }
    return LoadFile(editView, filePath, FileEncoding(file));
}

bool TabView::LoadFile(EditView *editView, const QString &filePath, FileEncoding &&fileEncoding, bool forceUseFileEncoding)
{
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr(Constants::kAppName),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(filePath), file.errorString()));
        return false;
    }

    // Check Byte Order Mark.
    if (forceUseFileEncoding || fileEncoding.hasBom()) {
        QTextStream in(&file);
        in.setCodec(fileEncoding.codec());
        const auto &text = in.readAll();
//        qDebug() << "text: " << text << ", bytearry: " << text.toUtf8();
        editView->setPlainText(text);
        editView->setFileEncoding(std::move(fileEncoding));
    } else {
        // If no BOM, we try to decode by UTF8 firstly,
        // then decode by System if failed.
        const QByteArray &data = file.readAll();
        QTextCodec::ConverterState state;
        // FileEncoding.codec is UTF8 in default.
        const auto &text = fileEncoding.codec()->toUnicode(data.constData(), data.size(), &state);
//        qDebug() << "text: " << text << ", bytearry: " << data.data();
        if (state.invalidChars > 0) {
            auto systemCodec = QTextCodec::codecForLocale();  // Use system codec.
            if (systemCodec == nullptr) {
                qFatal("codecForLocale() returns null.");
            }
            const auto &ansiText = systemCodec->toUnicode(data.constData());
            editView->setPlainText(ansiText);
            // Change fileEncoding from UTF8 to System.
            fileEncoding.setCodec(systemCodec);
        } else {
            editView->setPlainText(text);
        }
        editView->setFileEncoding(std::move(fileEncoding));
    }
    qDebug() << "encoding: " << editView->fileEncoding().description()
               << ", file: " << file.fileName();

#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif
#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif

    MainWindow::Instance().statusBar()->showMessage(tr("File loaded"), 2000);
    qDebug() << "File loaded, " << editView->fileName();
    editView->setFileLoaded(true);
    return true;
}

void TabView::ChangeTabDescription(const QFileInfo &fileInfo, int index)
{
    if (index == -1) {
        index = currentIndex();
    }
    setTabText(index, fileInfo.fileName());
    setTabToolTip(index, fileInfo.filePath());
}

void TabView::ApplyWrapTextState(int index)
{
    GetEditView(index)->ApplyWrapTextState();
}

void TabView::ApplySpecialCharsVisible(int index)
{
    GetEditView(index)->ApplySpecialCharsVisible();
}

void TabView::tabInserted(int index)
{
    qDebug() << "index: " << index << ", widget: " << ((EditView*)widget(index))->fileName();
    ChangeTabCloseButtonToolTip(index, "Double click to force close.");
//    UpdateWindowTitle(index);
//    GetEditView(index)->TrigerParser();

//    ApplyWrapTextState(index);  // TODO: Cause transparent window...
}

void TabView::tabRemoved(int index)
{
    // Should notice that 'index' is the position of previous tab closed.
}
