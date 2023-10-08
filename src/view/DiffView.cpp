/**
 * Copyright 2023 QEditor QH
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

#include "DiffView.h"
#include <QApplication>
#include <QScrollBar>
#include <QWheelEvent>

namespace QEditor {
DiffHtmlView::DiffHtmlView(QWidget *parent) : QTextEdit(parent) {
    setStyleSheet(
        "color:rgb(215,215,210); background-color:rgb(28,28,28); selection-color:lightGray; selection-background-color:rgb(9,71,113); border:none;");
    verticalScrollBar()->setStyleSheet(
        "QScrollBar{background:rgb(28,28,28); border:none; width:10px;}"
        "QScrollBar::handle{background:rgb(54,54,54); border:none;}"
        "QScrollBar::add-line:vertical{border:none; background:none;}"
        "QScrollBar::sub-line:vertical{border:none; background:none;}");
    horizontalScrollBar()->setStyleSheet(
        "QScrollBar{background:rgb(28,28,28); border:none; height:10px;}"
        "QScrollBar::handle{background:rgb(54,54,54); border:none;}"
        "QScrollBar::add-line:horizontal{border:none;background:none;}"
        "QScrollBar::sub-line:horizontal{border:none;background:none;}");
    setFont(QFont("Consolas", 11));
    currentFontSize_ = font().pointSize();
}

void DiffHtmlView::wheelEvent(QWheelEvent *event) {
    // If Ctrl-Key pressed.
    if (QApplication::keyboardModifiers() != Qt::ControlModifier) {
        QTextEdit::wheelEvent(event);
        return;
    }

    if ((!event->pixelDelta().isNull() && event->pixelDelta().y() > 0) ||
        (!event->angleDelta().isNull() && event->angleDelta().y() > 0)) {
        ZoomIn();
    } else {
        ZoomOut();
    }
}

void DiffHtmlView::ZoomIn() {
    auto currentFont = font();
    currentFont.setPointSize(font().pointSize() + 1);
    setFont(currentFont);
    currentFontSize_ = currentFont.pointSize();
}

void DiffHtmlView::ZoomOut() {
    auto currentFont = font();
    currentFont.setPointSize(font().pointSize() - 1);
    setFont(currentFont);
    currentFontSize_ = currentFont.pointSize();
}

DiffTextView::DiffTextView(QWidget *parent) : EditView(parent) {
    setFont(QFont("Consolas", 11));
    // setReadOnly(true);
    // setTextInteractionFlags(textInteractionFlags() | Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
}
}  // namespace QEditor
