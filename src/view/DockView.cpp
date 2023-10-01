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

#include "DockView.h"
#include "Logger.h"
#include <QEvent>
#include <QResizeEvent>
#include <QWidget>

namespace QEditor {
DockView::DockView(QWidget *parent) : QDockWidget(parent) {
//    QPalette pal;
//    pal.setColor(QPalette::Window, QColor(28, 28, 28));
//    setAutoFillBackground(true);
//    setPalette(pal);

    SetDockQss(this, "9pt", "lightGray", "rgb(35, 35, 35)", "2px", "5px");
}

void DockView::SetDockQss(QDockWidget *dockView, const QString &fontSize, const QString &textColor,
                          const QString &backColor, const QString &leftPadding, const QString &topPadding) {
    QStringList qss;
    qss.append(QString("QDockWidget{border: 10px solid red; font-size:%1; color:%2;}").arg(fontSize).arg(textColor));
    qss.append(QString("QDockWidget::title{background:%1; padding-left:%2; padding-top:%3;}")
                   .arg(backColor)
                   .arg(leftPadding)
                   .arg(topPadding));
    qss.append(QString("QDockWidget::close-button,QDockWidget::float-button{border-image:url(:/images/x.svg);}"));
    qss.append(
        QString("QDockWidget::close-button:hover,QDockWidget::float-button:hover{border-image:url(:/images/x.svg); "
                "background:red;}"));
    qss.append(
        QString("QDockWidget::close-button:pressed,QDockWidget::float-button:pressed{padding:1px -1px -1px 1px;}"));
    dockView->setStyleSheet(qss.join(""));
}
}  // namespace QEditor
