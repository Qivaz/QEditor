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
#include "FileEncoding.h"
#include "FileRecorder.h"
#include "Logger.h"
#include "MainWindow.h"
#include "RecentFiles.h"
#include "Toast.h"
#include <QAbstractButton>
#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QStatusBar>
#include <QTabBar>
#include <QTextCodec>
#include <QToolButton>

namespace QEditor {
TabView::TabView(QWidget *parent) : QTabWidget(parent), menu_(new QMenu(parent)) {
    setAttribute(Qt::WA_StyledBackground);
    setMovable(true);
    setTabsClosable(true);
    setStyleSheet(
        "QTabWidget{color:darkGray; background-color:rgb(28,28,28); selection-color:red; "
        "selection-background-color:green; border:none;}"
        "QTabBar::tab{color:white; background-color:rgb(54,54,54); border:1px solid rgb(54,54,54); "
        "border-left-style:solid; border-left-width:0.5px; border-left-color:gray; "
        "border-right-style:solid; border-right-width:0.5px; border-right-color:gray;"
        "padding:3px; border-top-left-radius:5px; border-top-right-radius:5px;}"
        "QTabBar::tab:selected{background:QColor(0,0,0); border-bottom-color:#1769AA; border-bottom-width:3px}"
        "QTabBar::tab:hover{background-color:QColor(0,0,0);}"
        "QTabBar::close-button{border-image:url(:/images/x-circle.svg);}"
        "QTabBar::close-button:hover{background:red; border-image:url(:/images/x.svg);}");

    menu_->setStyleSheet(
        "QMenu{color:lightGray; background-color:rgb(40,40,40); margin:2px 2px;border:none;} "
        "QMenu::item{color:rgb(225,225,225); background-color:rgb(40,40,40); "
        "padding:5px 5px;} QMenu::item:selected{background-color:rgb(9,71,113);}"
        "QMenu::item:pressed{border:1px solid rgb(60,60,60); background-color:rgb(29,91,133);} "
        "QMenu::separator{height:1px; background-color:rgb(80,80,80);}");

    connect(tabBar(), &QTabBar::currentChanged, this, &TabView::HandleCurrentIndexChanged);
    connect(tabBar(), &QTabBar::tabBarDoubleClicked, this, &TabView::HandleTabBarDoubleClicked);
    connect(tabBar(), &QTabBar::tabBarClicked, this, &TabView::HandleTabBarClicked);
    connect(this, &QTabWidget::tabCloseRequested, this, &TabView::HandleTabCloseRequested);

    setFont(QFont("Consolas", 10));
}

void TabView::ChangeTabCloseButtonToolTip(int index, const QString &tip) {
    auto closeButton = (QAbstractButton *)(tabBar()->tabButton(index, QTabBar::ButtonPosition::RightSide));
    closeButton->setToolTip(tip);
}

// Update with current edit view if 'index' is -1.
// Otherwise, with edit view at 'index'.
void TabView::UpdateWindowTitle(int index) {
    EditView *editView = nullptr;
    if (index == -1) {
        editView = CurrentEditView();
    } else {
        editView = GetEditView(index);
    }
    QString title;
    if (editView != nullptr) {
        title = editView->fileName();
        if (editView->ShouldSave()) {
            title = "* " + title;
        }
    } else {
        if (index == -1) {
            title = tabText(currentIndex());
        } else {
            title = tabText(index);
        }
    }

    title += " - " + QCoreApplication::applicationName();
    MainWindow *win = (MainWindow *)(this->parent());
    win->setWindowTitle(title);
}

void TabView::HandleCurrentIndexChanged(int index) {
    qDebug() << "index: " << index;
    UpdateWindowTitle(index);

    auto editView = GetEditView(index);
#if defined(USE_DIFF_TEXT_VIEW)
    if (editView == nullptr) {
        editView = GetDiffView(index);
    }
#endif
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

    // Update status bar info. if file encoding changes.
    MainWindow::Instance().UpdateStatusBarRareInfo("Unix", editView->fileEncoding().name(), 0);
}

void TabView::HandleTabBarClicked(int index) {
    qDebug() << "index: " << index;
    if (index == -1) {
        return;
    }
    if (QApplication::mouseButtons() == Qt::RightButton) {
        qDebug() << "TabView::barClicked(RightButton), index: " << index;
        menu_->clear();
        QAction *closeAction = new QAction(tr("Close  (Click Close Button)"));
        menu_->addAction(closeAction);
        connect(closeAction, &QAction::triggered, this, &TabView::TabCloseMaybeSave);
        menu_->popup(QCursor::pos());

        QAction *forceCloseAction = new QAction(tr("Force Close  (Double Click)"));
        menu_->addAction(forceCloseAction);
        connect(forceCloseAction, &QAction::triggered, this, &TabView::TabForceClose);
        menu_->popup(QCursor::pos());

        QAction *closeAllAction = new QAction(tr("Force Close All"));
        menu_->addAction(closeAllAction);
        connect(closeAllAction, &QAction::triggered, this, [&]() { clear(); });
        menu_->popup(QCursor::pos());

        QAction *closeOtherAction = new QAction(tr("Force Close Other"));
        menu_->addAction(closeOtherAction);
        connect(closeOtherAction, &QAction::triggered, this, [&]() { clear(); });
        menu_->popup(QCursor::pos());

        auto editView = GetEditView(index);
        if (editView != nullptr) {
            menu_->addSeparator();

            auto filePath = editView->filePath();
            QAction *copyPathAction = new QAction(tr("Copy Full Path"));
            menu_->addAction(copyPathAction);
            connect(copyPathAction, &QAction::triggered, this, [filePath]() {
                QClipboard *clipboard = QGuiApplication::clipboard();
                clipboard->setText(filePath);
            });
            menu_->popup(QCursor::pos());

            auto fileName = editView->fileName();
            QAction *copyNameAction = new QAction(tr("Copy File Name"));
            menu_->addAction(copyNameAction);
            connect(copyNameAction, &QAction::triggered, this, [fileName]() {
                QClipboard *clipboard = QGuiApplication::clipboard();
                clipboard->setText(fileName);
            });
            menu_->popup(QCursor::pos());

            menu_->addSeparator();

            auto folderPath = QFileInfo(filePath).canonicalPath();
#if defined(Q_OS_WIN)
            QAction *openExplorerAction = new QAction(tr("Reveal in File Explorer"));
#else
            QAction *openExplorerAction = new QAction(tr("Open Containing Folder"));
#endif
            menu_->addAction(openExplorerAction);
            connect(openExplorerAction, &QAction::triggered, this, [folderPath, filePath]() {
#if defined(Q_OS_WIN)
                QString cmd = QDir::toNativeSeparators(filePath);
                // Have to add \ before all spaces. Using quotes does not work.
                cmd.replace(QString(" "), QString("\ "));
                QProcess::startDetached("explorer /select," + cmd);
#elif defined(Q_OS_LINUX)
                // If use 'QProcess::startDetached("xdg-open", QStringList(QDir::toNativeSeparators(folderPath)))', can't show the browser with the file selected.
                QString cmd = "dbus-send --session --print-reply --dest=org.freedesktop.FileManager1 --type=method_call /org/freedesktop/FileManager1 org.freedesktop.FileManager1.ShowItems array:string:\"file://%1\" string:\"\"";
                QProcess::startDetached(cmd.arg(filePath));
#else  // Q_OS_OSX
                QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
#endif
            });
            menu_->popup(QCursor::pos());

            menu_->addSeparator();

            static bool has_before_path = false;
            QAction *viewDiffWithAction = new QAction(tr("View Diff between..."));
            menu_->addAction(viewDiffWithAction);
            connect(viewDiffWithAction, &QAction::triggered, this, [editView, this]() {
                diffFormerEditView_ = editView;
                has_before_path = true;
            });
            if (has_before_path) {
                QAction *viewDiffWithPreviousAction =
                    new QAction(tr("View Diff with") + " \'" + diffFormerEditView_->fileName() + "\'");
                menu_->addAction(viewDiffWithPreviousAction);
                connect(viewDiffWithPreviousAction, &QAction::triggered, this, [editView, this]() {
                    ViewDiff(diffFormerEditView_, editView);
                    diffFormerEditView_ = nullptr;
                    has_before_path = false;
                });
                menu_->popup(QCursor::pos());
            }
        } else {
            auto diffView = GetDiffView(index);
            if (diffView != nullptr && diffView->diffFormerEditView() != nullptr &&
                diffView->diffLatterEditView() != nullptr) {
                menu_->addSeparator();
                QAction *swapDiffAction = new QAction(tr("Swap Diff"));
                menu_->addAction(swapDiffAction);
                connect(swapDiffAction, &QAction::triggered, this, [index, this]() { SwapDiff(index); });
            }
        }
    }
}

void TabView::HandleTabBarDoubleClicked(int index) {
    qDebug() << "index: " << index;
    if (index == -1) {
        return;
    }
    (void)TabForceClose();
}

void TabView::HandleTabCloseRequested(int index) {
    auto currentEditView = GetEditView(index);
    if (currentEditView == nullptr) {
        DeleteWidget(index);
        return;
    }
    qDebug() << "index: " << index << ", should save: " << currentEditView->ShouldSave() << ", "
             << currentEditView->filePath();
    (void)TabCloseMaybeSaveInner(currentEditView);
}

bool TabView::ActionSave() {
    auto currentEditView = CurrentEditView();
    if (!currentEditView->ShouldSave()) {
        return true;
    }

    if (currentEditView->filePath().isEmpty()) {  // New file.
        if (currentEditView->SaveAs()) {
            openFiles().insert(currentEditView->filePath());
            RecentFiles::UpdateFiles(currentEditView->filePath());
            MainWindow::Instance().UpdateRecentFilesMenu();
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

bool TabView::ActionSaveAs() {
    auto currentEditView = CurrentEditView();
    if (currentEditView->filePath().isEmpty()) {  // New file.
        if (currentEditView->SaveAs()) {
            openFiles().insert(currentEditView->filePath());
            RecentFiles::UpdateFiles(currentEditView->filePath());
            MainWindow::Instance().UpdateRecentFilesMenu();
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
                RecentFiles::UpdateFiles(currentEditView->filePath());
                MainWindow::Instance().UpdateRecentFilesMenu();
            }
            return true;
        }
    }
    return false;
}

bool TabView::TabCloseMaybeSave() {
    auto currentEditView = CurrentEditView();
    if (currentEditView == nullptr) {
        DeleteWidget(currentWidget());
        return true;
    }
    return TabCloseMaybeSaveInner(currentEditView);
}

bool TabView::TabCloseMaybeSaveInner(EditView *editView) {
    if (editView->filePath().isEmpty()) {  // New file.
        if (editView->document()->isEmpty()) {
            // Close tab.
            if (editView->newFileNum() != 0) {
                NewFileNum::SetNumber(editView->newFileNum(), false);
            }
            DeleteWidget(editView);
            return true;
        }
        QMessageBox warningBox(QMessageBox::Question, tr(Constants::kAppName),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);
        warningBox.setButtonText(QMessageBox::Save, tr("Save"));
        warningBox.setButtonText(QMessageBox::Discard, tr("Discard"));
        warningBox.setButtonText(QMessageBox::Cancel, tr("Cancel"));
        int res = warningBox.exec();
        if (res == QMessageBox::Discard || (res == QMessageBox::Save && editView->SaveAs())) {
            // Close tab.
            if (editView->newFileNum() != 0) {
                NewFileNum::SetNumber(editView->newFileNum(), false);
            }
            DeleteWidget(editView);
        }
    } else {  // Open file.
        if (editView->MaybeSave()) {
            // Close tab.
            openFiles().remove(editView->filePath());
            DeleteWidget(editView);
        }
    }
    return false;
}

bool TabView::TabForceClose() {
    // Close without save.
    auto currentEditView = CurrentEditView();
    if (currentEditView != nullptr) {
        openFiles().remove(currentEditView->filePath());  // Just force remove.
        if (currentEditView->newFileNum() != 0) {
            NewFileNum::SetNumber(currentEditView->newFileNum(), false);
        }
        DeleteWidget(currentEditView);
    } else {
        DeleteWidget(currentWidget());
    }
    return true;
}

void TabView::AutoStore() {
    QVector<EditView *> editViews;
    for (int i = 0; i < count(); ++i) {
        auto editView = GetEditView(i);
        if (editView == nullptr) {
            continue;
        }
        editViews.push_back(editView);
    }
    FileRecorder fileRecorder;
    fileRecorder.SetPos(currentIndex());
    fileRecorder.SetEditViews(editViews);
    fileRecorder.StoreFiles();
}

bool TabView::AutoLoad() {
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
        if (fileInfo.IsTerminal()) {  // Terminal view.
            OpenSsh(fileInfo.ip_, fileInfo.port_, fileInfo.user_, fileInfo.pwd_);
            continue;
        }
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
            filePathTip = qFileInfo.canonicalFilePath();

            // Don't open file not exist anymore.
            QFile file(filePathTip);
            if (!file.open(QFile::ReadOnly | QFile::Text)) {
                QMessageBox::warning(
                    this, tr(Constants::kAppName),
                    tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(filePathTip), file.errorString()));
                continue;
            }
            // Don't open file multiple times.
            if (openFiles_.contains(filePathTip)) {
                Toast::Instance().Show(Toast::kWarning, filePathTip + tr(" already opened."));
                continue;
            }

            openFiles_.insert(filePathTip);
            RecentFiles::UpdateFiles(filePathTip);
            MainWindow::Instance().UpdateRecentFilesMenu();

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

void TabView::DeleteWidget(int index) {
    QWidget *widget = this->widget(index);
    if (widget == nullptr) {
        qFatal("The widget should not be null");
    }
    removeTab(index);
    delete widget;
}

void TabView::DeleteWidget(QWidget *widget) {
    if (widget == nullptr) {
        qFatal("The widget should not be null");
    }
    removeTab(indexOf(widget));
    delete widget;
}

void TabView::NewFile() {
    int num = NewFileNum::GetNumber();
    QString fileName = QString(tr("new ")) + QString::number(num);
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

#if defined(USE_DIFF_TEXT_VIEW)
void TabView::ViewDiff(DiffView *diffView, const QList<FormattedText> &formattedTexts) {
    auto cursor = diffView->textCursor();

    auto &scrollbarInfos = diffView->scrollbarLineInfos()[ScrollBarHighlightCategory::kCategoryDiff];
    if (diffView->AllowHighlightScrollbar()) {
        scrollbarInfos.clear();
    }

    std::vector<int> diffPosList;
    foreach(auto &ft, formattedTexts) {
        qDebug() << "text: " << ft.text_ << ", format: " << ft.format_.foreground() << ft.format_.background();
        if (diffView->AllowHighlightScrollbar() && ft.format_.background() == Diff::kNewBgColor) {
            diffPosList.emplace_back(diffView->textCursor().position());
        }
        cursor.insertText(ft.text_, ft.format_);
    }

    if (diffView->AllowHighlightScrollbar()) {
        diffView->HandleLineOffset();
        //        cursor = diffView->textCursor();
        int lastLine = -1;
        std::vector<int> lines;
        for (const auto &pos : diffPosList) {
            cursor.setPosition(pos);
            const auto line = diffView->LineNumber(cursor);
            if (line == lastLine) {
                continue;
            }
            lastLine = line;
            lines.emplace_back(line);
        }
        scrollbarInfos.emplace_back(std::make_pair(lines, Qt::red));
        diffView->setHightlightScrollbarInvalid(true);
    }

    cursor.movePosition(QTextCursor::Start);
    diffView->setTextCursor(cursor);
}
#else
void TabView::ViewDiff(DiffView *diffView, const QString &html) {
    diffView->appendHtml(html);
    qDebug() << "html: " << html << ", text: " << diffView->toPlainText();
}
#endif

void TabView::ViewDiff(const QString &former, const QString &latter) {
    diff_.Impose(former, latter);
#if defined(USE_DIFF_TEXT_VIEW)
    const auto &formattedTexts = diff_.ToFormattedText();
#else
    const QString &html = diff_.ToLineHtml();
    qDebug() << "html: " << html;
#endif
    QString diffName = former.left(20) + "... → " + latter.left(20) + "...";
    auto diffView = new DiffView(this);
    addTab(diffView, diffName);
    setTabIcon(count() - 1, QIcon::fromTheme("diff", QIcon(":/images/diff.svg")));
    setCurrentIndex(count() - 1);
    QString diffTip = QString(tr("Diff: ")) + diffName;
    setTabToolTip(count() - 1, diffTip);
    diffView->setFocus();
    diffView->setFont(QFont("Consolas", 16));

#if defined(USE_DIFF_TEXT_VIEW)
    ViewDiff(diffView, formattedTexts);
#else
    ViewDiff(diffView, html);
#endif
}

void TabView::ViewDiff(const EditView *former, const EditView *latter) {
    diff_.Impose(former->toPlainText(), latter->toPlainText());
#if defined(USE_DIFF_TEXT_VIEW)
    const auto &formattedTexts = diff_.ToFormattedText();
#else
    const QString &html = diff_.ToLineHtml();
    qDebug() << "html: " << html;
#endif
    QString diffName = former->fileName() + QString(" → ") + latter->fileName();
    auto diffView = new DiffView(this);
    diffView->setDiffFormerEditView(former);
    diffView->setDiffLatterEditView(latter);
    addTab(diffView, diffName);
    setTabIcon(count() - 1, QIcon::fromTheme("diff", QIcon(":/images/diff.svg")));
    setCurrentIndex(count() - 1);
    auto beforeName = former->filePath().isEmpty() ? former->fileName() : former->filePath();
    auto afterName = latter->filePath().isEmpty() ? latter->fileName() : latter->filePath();
    QString diffTip = QString(tr("Diff: ")) + beforeName + QString(" → ") + afterName;
    setTabToolTip(count() - 1, diffTip);
    diffView->setFocus();
    diffView->setFont(QFont("Consolas", 16));

#if defined(USE_DIFF_TEXT_VIEW)
    ViewDiff(diffView, formattedTexts);
#else
    ViewDiff(diffView, html);
#endif
}

void TabView::SwapDiff(int index) {
    auto diffView = GetDiffView(index);
    if (diffView == nullptr) {
        qFatal("diffView is null");
    }
    if (diffView->diffFormerEditView() == nullptr) {
        return;
    }
    if (diffView->diffLatterEditView() == nullptr) {
        return;
    }
    diffView->clear();
    auto tmpDiffView = diffView->diffLatterEditView();
    diffView->setDiffLatterEditView(diffView->diffFormerEditView());
    diffView->setDiffFormerEditView(tmpDiffView);
    auto former = diffView->diffFormerEditView();
    auto latter = diffView->diffLatterEditView();
    diff_.Impose(former->toPlainText(), latter->toPlainText());
#if defined(USE_DIFF_TEXT_VIEW)
    const auto &formattedTexts = diff_.ToFormattedText();
#else
    const QString &html = diff_.ToLineHtml();
    qDebug() << "html: " << html;
#endif
    QString diffName = former->fileName() + QString(" → ") + latter->fileName();
    setTabText(index, diffName);
    setTabIcon(index, QIcon::fromTheme("diff", QIcon(":/images/diff.svg")));
    auto beforeName = former->filePath().isEmpty() ? former->fileName() : former->filePath();
    auto afterName = latter->filePath().isEmpty() ? latter->fileName() : latter->filePath();
    QString diffTip = QString(tr("Diff: ")) + beforeName + QString(" → ") + afterName;
    setTabToolTip(index, diffTip);

#if defined(USE_DIFF_TEXT_VIEW)
    ViewDiff(diffView, formattedTexts);
#else
    ViewDiff(diffView, html);
#endif
}

void TabView::OpenFile() { OpenFile(QFileDialog::getOpenFileName(this)); }

void TabView::OpenFile(const QString &filePath) {
    if (filePath.isEmpty()) {
        return;
    }
    QFileInfo fileInfo = QFileInfo(filePath);

    // Don't open file multiple times.
    if (openFiles_.contains(fileInfo.canonicalFilePath())) {
        // Toast::Instance().Show(Toast::kWarning, fileInfo.canonicalFilePath() + " already opened.");
        auto index = FindEditViewIndex(fileInfo.canonicalFilePath());
        if (index == -1) {
            Toast::Instance().Show(Toast::kWarning,
                                   fileInfo.canonicalFilePath() + tr(" already opened, but not found!"));
        } else {
            setCurrentIndex(index);
        }
        return;
    }
    openFiles_.insert(fileInfo.canonicalFilePath());
    RecentFiles::UpdateFiles(fileInfo.canonicalFilePath());
    MainWindow::Instance().UpdateRecentFilesMenu();

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

bool TabView::LoadFile(EditView *editView, const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(
            this, tr(Constants::kAppName),
            tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(filePath), file.errorString()));
        return false;
    }
    return LoadFile(editView, filePath, FileEncoding(file));
}

bool TabView::LoadFile(EditView *editView, const QString &filePath, FileEncoding &&fileEncoding,
                       bool forceUseFileEncoding) {
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(
            this, tr(Constants::kAppName),
            tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(filePath), file.errorString()));
        return false;
    }

    // Check Byte Order Mark.
    if (forceUseFileEncoding || fileEncoding.hasBom()) {
        QTextStream in(&file);
        in.setCodec(fileEncoding.codec());
        const auto &text = in.readAll();
        qDebug() << "text: " << text << ", bytearry: " << text.toUtf8();
        editView->setPlainText(text);
        editView->setFileEncoding(std::move(fileEncoding));
    } else {
        // If no BOM.
        const auto &ansiText = fileEncoding.ProcessAnsi(file);
        editView->setPlainText(ansiText);
        editView->setFileEncoding(std::move(fileEncoding));
    }
    qDebug() << "encoding: " << editView->fileEncoding().name() << ", file: " << file.fileName();

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

void TabView::OpenSsh(const QString &ip, int port, const QString &user, const QString &pwd) {
    auto terminalView = new TerminalView(ip, port, user, pwd, this);
    addTab(terminalView, terminalView->fileName());
    setCurrentIndex(count() - 1);
    setTabToolTip(count() - 1, terminalView->fileName());
    setTabIcon(count() - 1, QIcon::fromTheme("terminal-open", QIcon(":/images/terminal.svg")));
    terminalView->setFocus();
}

void TabView::ChangeTabDescription(const QFileInfo &fileInfo, int index) {
    if (index == -1) {
        index = currentIndex();
    }
    setTabText(index, fileInfo.fileName());
    setTabToolTip(index, fileInfo.canonicalFilePath());
}

void TabView::ApplyWrapTextState(int index) {
    auto textView = GetEditView(index);
    if (textView == nullptr) {
        return;
    }
    textView->ApplyWrapTextState();
}

void TabView::ApplySpecialCharsVisible(int index) {
    auto textView = GetEditView(index);
    if (textView == nullptr) {
        return;
    }
    textView->ApplySpecialCharsVisible();
}

void TabView::tabInserted(int index) {
    auto editView = qobject_cast<EditView *>(widget(index));
    if (editView == nullptr) {
        return;
    }
    qDebug() << "index: " << index << ", widget: " << editView->fileName();
    ChangeTabCloseButtonToolTip(index, tr("Double click to force close."));
    //    UpdateWindowTitle(index);
    //    GetEditView(index)->TrigerParser();

    //    ApplyWrapTextState(index);  // TODO: Cause transparent window...

    AutoStore();
}

void TabView::tabRemoved(int index) {
    // Should notice that 'index' is the position of previous tab closed.
    AutoStore();
}

void TabView::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // Double click right blank area of tab bar to new a file.
        NewFile();
    }
}
}  // namespace QEditor
