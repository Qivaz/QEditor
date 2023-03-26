/**
 * Copyright 2023 QEditor QH
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

#include "OpenTerminalDialog.h"
#include "ui_OpenTerminalDialog.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#include <winsock2.h>
// Need link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

#include "MainWindow.h"
#include "Settings.h"
#include "TerminalView.h"
#include "Toast.h"

#ifdef Q_OS_WIN
namespace WinTheme {
extern bool IsDarkTheme();
extern void SetDark_qApp();
extern void SetDarkTitleBar(HWND hwnd);
}  // namespace WinTheme
#endif

namespace QEditor {
OpenTerminalDialog::OpenTerminalDialog(QWidget *parent) : QDialog(parent), ui_(new Ui::OpenTerminalDialog) {
    ui_->setupUi(this);
    if (ui_->lineEditIp->text().isEmpty()) {
        ui_->lineEditIp->setFocus();
    }

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

    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    qreal opa = Settings().Get("dialog", "opacity", 0.9).toDouble();
    setWindowOpacity(opa * 0.8);

    setWindowModality(Qt::WindowModal);
}

OpenTerminalDialog::~OpenTerminalDialog() { delete ui_; }

TabView *OpenTerminalDialog::tabView() { return MainWindow::Instance().tabView(); }

static inline bool IsValidIpAddress(const QString &ip) {
#if defined(Q_OS_WIN)
    auto res = inet_addr(ip.toStdString().c_str());
    if (res == INADDR_ANY || res == INADDR_NONE) {
        return false;
    }
    return true;
#else
    struct sockaddr_in sa;
    return (inet_pton(AF_INET, ip.toStdString().c_str(), &sa.sin_addr) != 0);
#endif
}

void OpenTerminalDialog::on_pushButtonConnect_clicked() {
    QString ip = ui_->lineEditIp->text();
    if (!IsValidIpAddress(ip)) {
        ui_->lineEditIp->setFocus();
        ui_->lineEditIp->selectAll();
        Toast::Instance().Show(Toast::kError, QString(tr("The input IP is invalid: %1")).arg(ip));
        return;
    }
    QString port = ui_->lineEditPort->text();
    bool success;
    int pt = port.toInt(&success);
    if (!success || pt < 0 || pt > 255) {
        ui_->lineEditPort->setFocus();
        ui_->lineEditPort->selectAll();
        Toast::Instance().Show(Toast::kError, QString(tr("The input port is invalid: %1")).arg(port));
        return;
    }
    QString user = ui_->lineEditUser->text();
    QString pwd = ui_->lineEditPwd->text();
    tabView()->OpenSsh(ip, pt, user, pwd);

    close();
}

void OpenTerminalDialog::on_pushButtonCancel_clicked() { close(); }
}  // namespace QEditor
