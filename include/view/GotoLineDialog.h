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

#ifndef GOTOLINEDIALOG_H
#define GOTOLINEDIALOG_H

#include "EditView.h"
#include "MainTabView.h"
#include <QDialog>

namespace Ui {
class UIGotoLineDialog;
}

namespace QEditor {
class GotoLineDialog : public QDialog {
    Q_OBJECT
public:
    explicit GotoLineDialog(QWidget* parent = nullptr);
    ~GotoLineDialog();

    EditView* editView();
    TabView* tabView();

    void showEvent(QShowEvent*) override;

private slots:
    void on_pushButtonOk_clicked();

    void on_pushButtonCacel_clicked();

private:
    Ui::UIGotoLineDialog* ui_;
};
} // namespace QEditor

#endif // GOTOLINEDIALOG_H
