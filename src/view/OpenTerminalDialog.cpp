#include "OpenTerminalDialog.h"
#include "ui_OpenTerminalDialog.h"

#include "MainWindow.h"
#include "TerminalView.h"

namespace QEditor {
OpenTerminalDialog::OpenTerminalDialog(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::OpenTerminalDialog)
{
    ui_->setupUi(this);
}

OpenTerminalDialog::~OpenTerminalDialog()
{
    delete ui_;
}

TabView* OpenTerminalDialog::tabView() { return MainWindow::Instance().tabView(); }

void OpenTerminalDialog::on_pushButtonConnect_clicked()
{
    QString ip = ui_->comboBoxIp->currentText();
    QString port = ui_->lineEditPort->text();
    QString user = ui_->comboBoxUser->currentText();
    QString pwd = ui_->lineEditPwd->text();
    auto terminalView = new TerminalView(ip, port.toInt(), user, pwd);
    tabView()->addTab(terminalView, terminalView->fileName());
    tabView()->setCurrentIndex(tabView()->count() - 1);
    tabView()->setTabToolTip(tabView()->count() - 1, terminalView->fileName());
    terminalView->setFocus();

    hide();
}

void OpenTerminalDialog::on_pushButtonCancel_clicked()
{
    hide();
}
}  // namespace QEditor
