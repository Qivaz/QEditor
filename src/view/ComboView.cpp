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

#include "ComboView.h"
#include "Logger.h"
#include <QLineEdit>
#include <QListWidget>

namespace QEditor {
ComboView::ComboView(QWidget *parent, bool fixed) : QComboBox(parent), fixed_(fixed) {
    auto qss =
        "QComboBox {\
                    color: lightGray; background: transparent;\
                    border: none;\
                    padding: 1px 1px 1px 1px;\
                    min-width: 3px;\
                    padding-left: 1px;\
                }"
        "QComboBox:enabled:hover, QComboBox:enabled:focus {\
                    color: lightGray; background-color: rgb(68, 68, 68);\
                    border: 1px solid gray;\
                    border-radius: 3px;\
                    padding: 1px 1px 1px 3px;\
                    min-width: 3px;\
                    padding-left: 1px;\
                    }"
        //                "QComboBox:!editable:on, QComboBox::drop-down:editable:on {\
//                    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\
//                                                stop: 0 #D3D3D3, stop: 0.4 #D8D8D8,\
//                                                stop: 0.5 #DDDDDD, stop: 1.0 #E1E1E1);\
//                    }"
        "QComboBox::drop-down {\
                    border: none; background: transparent; }"
        "QComboBox::down-arrow {\
                    image: url(:images/chevron-down.svg); width: 15px;\
                    }"
        "QComboBox QAbstractItemView {\
                        color: lightGray;\
                        border: 1px solid rgb(68, 68, 100);\
                        background: rgb(68, 68, 68);\
                        outline: rgb(68, 68, 255);\
                }"
        "QComboBox QAbstractItemView::item {\
                        color: lightGray;\
                }"
        "QComboBox QAbstractItemView::item:selected {\
                        background: rgb(232, 241, 250);\
                        color: rgb(255, 65, 132);\
                }";

    auto qssStr = QString(qss);

    setLineEdit(new QLineEdit(this));
    lineEdit()->setAlignment(Qt::AlignCenter);
    lineEdit()->setReadOnly(true);
    setStyleSheet(qssStr);
}

void ComboView::ShrinkForPopup() {
    if (fixed_ && maxWidth_ != 0) {
        qDebug() << ", maxWidth_: " << maxWidth_;
        view()->setFixedWidth(maxWidth_ * 1.5 + 15);
        return;
    }

    qreal maxWidth = 0;
    QFontMetricsF fm(font());
    for (int i = 0; i < count(); ++i) {
        auto text = itemText(i);
        QRectF rect = fm.boundingRect(text);
        auto width = rect.width();
        maxWidth = qMax(maxWidth, width);
        qDebug() << "text: " << text << ", width: " << width;
    }
    maxWidth_ = maxWidth;
    view()->setFixedWidth(maxWidth * 1.5 + 15);
}

void ComboView::ShrinkForChosen() {
    QFontMetricsF fm(font());
    auto text = lineEdit()->text();
    qDebug() << "text: " << text;
    QRectF rect = fm.boundingRect(text);
    auto width = rect.width();
    lineEdit()->setFixedWidth(width * 1.2);
    setFixedWidth(width * 1.2 + 15);
}

void ComboView::showPopup() {
    ShrinkForPopup();
    QComboBox::showPopup();
}

void ComboView::hidePopup() {
    ShrinkForChosen();
    QComboBox::hidePopup();
}
}  // namespace QEditor
