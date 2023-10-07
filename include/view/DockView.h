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

#ifndef DOCKVIEW_H
#define DOCKVIEW_H

#include <QDockWidget>

namespace QEditor {
class DockView : public QDockWidget {
    Q_OBJECT
   public:
    DockView(QWidget *parent = nullptr);
    ~DockView() { setWidget(nullptr); }

    void SetDockQss(QDockWidget *dockView, const QString &fontSize, const QString &textColor,
                    const QString &floatingBackColor, const QString &titleBackColor, const QString &leftPadding,
                    const QString &topPadding);

    int savedMaxWidth() const { return savedMaxWidth_; }
    void setSavedMaxWidth(int savedMaxWidth) {
        if (savedMaxWidth > savedMaxWidth_) {
            savedMaxWidth_ = savedMaxWidth;
        }
    }

   private:
    int savedMaxWidth_{0};
};
}  // namespace QEditor

#endif  // DOCKVIEW_H
