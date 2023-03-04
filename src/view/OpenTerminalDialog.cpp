#include "OpenTerminalDialog.h"
#include "ui_OpenTerminalDialog.h"

#if defined (Q_OS_WIN)
#include <winsock2.h>
#include <windows.h>
// Need link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

#include "MainWindow.h"
#include "TerminalView.h"
#include "Toast.h"

namespace QEditor {
OpenTerminalDialog::OpenTerminalDialog(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::OpenTerminalDialog)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui_->setupUi(this);
    if (ui_->lineEditIp->text().isEmpty()) {
        ui_->lineEditIp->setFocus();
    }
}

OpenTerminalDialog::~OpenTerminalDialog()
{
    delete ui_;
}

TabView* OpenTerminalDialog::tabView() { return MainWindow::Instance().tabView(); }

static inline bool IsValidIpAddress(const QString &ip)
{
#if defined (Q_OS_WIN)
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

void OpenTerminalDialog::on_pushButtonConnect_clicked()
{
    QString ip = ui_->lineEditIp->text();
    if (!IsValidIpAddress(ip)) {
        ui_->lineEditIp->setFocus();
        ui_->lineEditIp->selectAll();
        Toast::Instance().Show(Toast::kError, QString("The input IP is invalid: %1").arg(ip));
        return;
    }
    QString port = ui_->lineEditPort->text();
    bool success;
    int pt = port.toInt(&success);
    if (!success || pt < 0 || pt > 255) {
        ui_->lineEditPort->setFocus();
        ui_->lineEditPort->selectAll();
        Toast::Instance().Show(Toast::kError, QString("The input port is invalid: %1").arg(port));
        return;
    }
    QString user = ui_->lineEditUser->text();
    QString pwd = ui_->lineEditPwd->text();
    tabView()->OpenSsh(ip, pt, user, pwd);

    close();
}

void OpenTerminalDialog::on_pushButtonCancel_clicked()
{
    close();
}
}  // namespace QEditor
