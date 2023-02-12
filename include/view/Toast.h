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

#ifndef TOAST_H
#define TOAST_H

#include <QObject>
#include <QRect>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QEvent>

namespace QEditor {
class ToastDialog;
class Toast : public QObject
{
    Q_OBJECT
public:
    static Toast &Instance();
    enum Level
    {
        kInfo,
        kWarning,
        kError
    };
    void Show(Level level, const QString &text);

private:
    Toast();
    void timerEvent(QTimerEvent *event) override;

    ToastDialog *dialog_{nullptr};
    int timerId_{0};
};

#define FRAME_RADIUS
class ToastDialog: public QDialog
{
    Q_OBJECT
public:
    ToastDialog()
    {
        auto layout = new QHBoxLayout(this);
        label_ = new QLabel(this);
        label_->setStyleSheet("color: white; background: transparent");
        layout->addWidget(label_, 1);

        closeButton_ = new QLabel(this);
        closeButton_->installEventFilter(this);
        closeButton_->setStyleSheet("background: transparent");
        layout->addWidget(closeButton_);

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
        setAttribute(Qt::WA_ShowWithoutActivating,true);
#ifdef FRAME_RADIUS
        setAttribute(Qt::WA_TranslucentBackground, true);
#endif
    }

    void Show(Toast::Level level, const QString& text)
    {
#ifdef FRAME_RADIUS
        const QString qss("QFrame{background-color:%1; border:none;"
                          "border-top-left-radius:10px; border-top-right-radius:10px;"
                          "border-bottom-left-radius:10px;border-bottom-right-radius:10px;}");
        if (level == Toast::kInfo) {
            frame_->setStyleSheet(qss.arg("#000000"));
        } else if (level == Toast::kWarning) {
            frame_->setStyleSheet(qss.arg("#001166"));
        } else {
            frame_->setStyleSheet(qss.arg("#660000"));
        }
#else
        QPalette pal = palette();
        pal.setColor(QPalette::Window, QColor(0,0,0,200));
        if (level == Toast::kInfo) {
            pal.setColor(QPalette::Window, QColor(0,0,0,200));
        } else if (level == Toast::kWarning) {
            pal.setColor(QPalette::Window, QColor(0,0,255,200));
        } else {
            pal.setColor(QPalette::Window, QColor(255,0,0,200));
        }
        setPalette(pal);
#endif
        label_->setText(text);
        setWindowFlag(Qt::WindowStaysOnTopHint);
        show();
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (obj == closeButton_) {
            if (event->type() == QEvent::MouseButtonRelease) {
                accept();
            }
        }
        return QObject::eventFilter(obj, event);
    }

private:
    QLabel *label_;
    QLabel *closeButton_;
#ifdef FRAME_RADIUS
    QFrame *frame_;
#endif
};
}  // namespace QEditor

#endif // TOAST_H
