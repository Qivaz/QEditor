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

#include "GotoLineDialog.h"
#include "Logger.h"
#include "MainWindow.h"
#include "Settings.h"
#include "ui_GotoLineDialog.h"

#ifdef Q_OS_WIN
namespace WinTheme {
extern bool IsDarkTheme();
extern void SetDark_qApp();
extern void SetDarkTitleBar(HWND hwnd);
}  // namespace WinTheme
#endif

namespace QEditor {
GotoLineDialog::GotoLineDialog(QWidget *parent) : QDialog(parent), ui_(new Ui::UIGotoLineDialog) {
    ui_->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    qreal opa = Settings().Get("dialog", "opacity", 0.9).toDouble();
    setWindowOpacity(opa * 0.8);

    setWindowModality(Qt::WindowModal);
}

GotoLineDialog::~GotoLineDialog() { delete ui_; }

EditView *GotoLineDialog::editView() { return MainWindow::Instance().editView(); }

TabView *GotoLineDialog::tabView() { return MainWindow::Instance().tabView(); }

void GotoLineDialog::showEvent(QShowEvent *) {
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

    ui_->lineEditGotoLine->clear();
}

void GotoLineDialog::on_pushButtonOk_clicked() {
    auto lineStr = ui_->lineEditGotoLine->text();
    int line = lineStr.toInt();
    if (line <= 0 || line > editView()->document()->blockCount()) {
        qCritical() << "The line number is wrong: " << line;
        return;
    }
    editView()->GotoBlock(line - 1);
    close();
}

void GotoLineDialog::on_pushButtonCacel_clicked() { hide(); }
}  // namespace QEditor
