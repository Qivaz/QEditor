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

#include <QDialog>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QRect>

namespace QEditor {
class ToastDialog;
class Toast : public QObject {
    Q_OBJECT
   public:
    static Toast &Instance();
    enum Level { kInfo, kWarning, kError };
    void Show(Level level, const QString &text);

   private:
    Toast();
    void timerEvent(QTimerEvent *) override;

    ToastDialog *dialog_{nullptr};
    int timerId_{0};
};

#define FRAME_RADIUS
class ToastDialog : public QDialog {
    Q_OBJECT
   public:
    ToastDialog();

    void Show(Toast::Level level, const QString &text);

   private:
    QLabel *label_;
#ifdef FRAME_RADIUS
    QFrame *frame_;
#endif
};
}  // namespace QEditor

#endif  // TOAST_H
