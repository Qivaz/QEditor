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

#include "Toast.h"
#include "MainWindow.h"
#include <QDialog>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>

namespace QEditor {
Toast::Toast() { dialog_ = new ToastDialog(); }

Toast &Toast::Instance() {
    static Toast _toast;
    return _toast;
}

void Toast::Show(Toast::Level level, const QString &text) {
    dialog_->Show(level, text);
    // Kill the timer triggered before.
    if (timerId_ != 0) {
        killTimer(timerId_);
        timerId_ = 0;
        dialog_->accept();  // Hidden.
        dialog_->hide();
    }
    int interval = 1000;
    if (level == kWarning) {
        interval = 1000;
    } else if (level == kError) {
        interval = 1500;
    }
    timerId_ = startTimer(interval);
    MainWindow::Instance().statusBar()->showMessage(text, 10000);
}

void Toast::timerEvent(QTimerEvent *) {
    killTimer(timerId_);
    timerId_ = 0;
    dialog_->accept();  // Hidden.
    dialog_->hide();
}

ToastDialog::ToastDialog() {
    auto layout = new QHBoxLayout(this);
    label_ = new QLabel(this);
    label_->setStyleSheet("color:white; background:transparent;");
    label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(label_, 1);

#ifdef FRAME_RADIUS
    frame_ = new QFrame(this);
    frame_->setLayout(layout);
    auto dialogLayout = new QHBoxLayout(this);
    dialogLayout->addWidget(frame_, 1);
    setLayout(dialogLayout);
#else
    setLayout(layout);
#endif
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
#ifdef FRAME_RADIUS
    setAttribute(Qt::WA_TranslucentBackground);
#endif
    // setAttribute(Qt::WA_DeleteOnClose);
}

void ToastDialog::Show(Toast::Level level, const QString &text) {
#ifdef FRAME_RADIUS
    const QString qss(
        "QFrame{background-color:%1; border:none; border-top-left-radius:10px; border-top-right-radius:10px; "
        "border-bottom-left-radius:10px; border-bottom-right-radius:10px;}");
    if (level == Toast::kInfo) {
        frame_->setStyleSheet(qss.arg("#000000"));
    } else if (level == Toast::kWarning) {
        frame_->setStyleSheet(qss.arg("#001166"));
    } else {
        frame_->setStyleSheet(qss.arg("#660000"));
    }
#else
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0, 0, 0, 200));
    if (level == Toast::kInfo) {
        pal.setColor(QPalette::Window, QColor(0, 0, 0, 200));
    } else if (level == Toast::kWarning) {
        pal.setColor(QPalette::Window, QColor(0, 0, 255, 200));
    } else {
        pal.setColor(QPalette::Window, QColor(255, 0, 0, 200));
    }
    setPalette(pal);
#endif
    label_->setText(text);
    setWindowFlag(Qt::WindowStaysOnTopHint);
    show();
}

}  // namespace QEditor
