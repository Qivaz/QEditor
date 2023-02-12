#include "RichEditView.h"

#include <QApplication>
#include <QScrollBar>
#include <QWheelEvent>

namespace QEditor {
RichEditView::RichEditView(QWidget *parent) : QTextEdit(parent) {
    setStyleSheet("color: darkGray;"
                  "background-color: rgb(28, 28, 28);"
                  "selection-color: lightGray;"
                  "selection-background-color: rgb(76, 76, 167);"
                  "border: none;"
                  /*"font-family: '文泉驿等宽正黑';"*/);

    verticalScrollBar()->setStyleSheet("QScrollBar {border: none;}"
                                       "QScrollBar::add-line:vertical { \
                                          border: none; \
                                          background: none; \
                                        } \
                                        QScrollBar::sub-line:vertical { \
                                          border: none; \
                                          background: none; \
                                        }");
    horizontalScrollBar()->setStyleSheet("QScrollBar {border: none;}"
                                         "QScrollBar::add-line:horizontal { \
                                            border: none; \
                                            background: none; \
                                          } \
                                          QScrollBar::sub-line:horizontal { \
                                            border: none; \
                                            background: none; \
                                          }");

    setFont(QFont("Consolas", 11));
    currentFontSize_ = font().pointSize();
}

void RichEditView::wheelEvent(QWheelEvent *event) {
    // If Ctrl-Key pressed.
    if (QApplication::keyboardModifiers() != Qt::ControlModifier) {
        QTextEdit::wheelEvent(event);
        return;
    }

    if (event->delta() > 0) {
        ZoomIn();
    } else {
        ZoomOut();
    }
}

void RichEditView::ZoomIn()
{
    auto currentFont = font();
    currentFont.setPointSize(font().pointSize() + 1);
    setFont(currentFont);
    currentFontSize_ = currentFont.pointSize();
}

void RichEditView::ZoomOut()
{
    auto currentFont = font();
    currentFont.setPointSize(font().pointSize() - 1);
    setFont(currentFont);
    currentFontSize_ = currentFont.pointSize();
}
}  // namespace QEditor
