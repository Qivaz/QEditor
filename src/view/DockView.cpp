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
#include "MainWindow.h"
#include "Logger.h"
#include <QEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QWidget>

namespace QEditor {
DockView::DockView(QWidget *parent, int width, int height) : QDockWidget(parent) {
    // QPalette pal;
    // pal.setColor(QPalette::Window, QColor(28, 28, 28));
    // setAutoFillBackground(true);
    // setPalette(pal);

    SetDockQss(this, "9pt", "lightGray", "rgb(68,68,68)", "rgb(35, 35, 35)", "2px", "5px");

    QTimer::singleShot(10, [this, width, height]() {
        MainWindow::Instance().resizeDocks({this}, {width}, Qt::Horizontal);
        MainWindow::Instance().resizeDocks({this}, {height}, Qt::Vertical);
    });
}

void DockView::SetDockQss(QDockWidget *dockView, const QString &fontSize, const QString &textColor,
                          const QString &floatingBackColor, const QString &titleBackColor, const QString &leftPadding,
                          const QString &topPadding) {
    QStringList qss;
    qss.append(QString("QDockWidget{border: 10px solid red; font-size:%1; color:%2; background-color:%3;}")
                   .arg(fontSize)
                   .arg(textColor)
                   .arg(floatingBackColor));
    qss.append(QString("QDockWidget::title{background:%1; padding-left:%2; padding-top:%3;}")
                   .arg(titleBackColor)
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
