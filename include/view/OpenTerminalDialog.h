#ifndef OPENTERMINALDIALOG_H
#define OPENTERMINALDIALOG_H

#include "MainTabView.h"
#include <QDialog>

namespace Ui {
class OpenTerminalDialog;
}

namespace QEditor {
class OpenTerminalDialog : public QDialog {
    Q_OBJECT
   public:
    explicit OpenTerminalDialog(QWidget *parent = nullptr);
    ~OpenTerminalDialog();

    TabView *tabView();

   private slots:
    void on_pushButtonConnect_clicked();
    void on_pushButtonCancel_clicked();

   private:
    Ui::OpenTerminalDialog *ui_;
};
}  // namespace QEditor

#endif  // OPENTERMINALDIALOG_H
