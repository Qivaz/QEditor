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

#ifndef DIFFVIEW_H
#define DIFFVIEW_H

#include "EditView.h"
#include <QTextEdit>

namespace QEditor {
class DiffHtmlView : public QTextEdit {
    Q_OBJECT
   public:
    DiffHtmlView(QWidget *parent = nullptr);
    ~DiffHtmlView() = default;

    void ZoomIn();
    void ZoomOut();

    const EditView *diffFormerEditView() const { return diffFormerEditView_; }
    void setDiffFormerEditView(const EditView *diffFormerEditView) { diffFormerEditView_ = diffFormerEditView; }
    const EditView *diffLatterEditView() const { return diffLatterEditView_; }
    void setDiffLatterEditView(const EditView *diffLatterEditView) { diffLatterEditView_ = diffLatterEditView; }

   protected:
    void wheelEvent(QWheelEvent *event);

   private:
    int currentFontSize_{font().pointSize()};

    const EditView *diffFormerEditView_{nullptr};
    const EditView *diffLatterEditView_{nullptr};
};

class DiffTextView : public EditView {
    Q_OBJECT
   public:
    DiffTextView(QWidget *parent = nullptr);
    ~DiffTextView() = default;

    const EditView *diffFormerEditView() const { return diffFormerEditView_; }
    void setDiffFormerEditView(const EditView *diffFormerEditView) { diffFormerEditView_ = diffFormerEditView; }
    const EditView *diffLatterEditView() const { return diffLatterEditView_; }
    void setDiffLatterEditView(const EditView *diffLatterEditView) { diffLatterEditView_ = diffLatterEditView; }

    void SetModifiedIcon(bool) override {}
    void SetModified(bool) override {}
    void Hover(QTextCursor &) override {}

   protected:
    void UnderpaintCurrentBlock() override {
        // Clear all previous selections.
        QList<QTextEdit::ExtraSelection> extraSelections;
        setExtraSelections(extraSelections);
    }

    void keyPressEvent(QKeyEvent *event) override {
        // ReadOnly
        if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down || event->key() == Qt::Key_Left ||
            event->key() == Qt::Key_Right || event->key() == Qt::Key_Home || event->key() == Qt::Key_End ||
            event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown) {
            QPlainTextEdit::keyPressEvent(event);
        }
    }
    void keyReleaseEvent(QKeyEvent *event) override {
        // ReadOnly
        if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down || event->key() == Qt::Key_Left ||
            event->key() == Qt::Key_Right || event->key() == Qt::Key_Home || event->key() == Qt::Key_End ||
            event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown) {
            QPlainTextEdit::keyPressEvent(event);
        }
    }

   private:
    const EditView *diffFormerEditView_{nullptr};
    const EditView *diffLatterEditView_{nullptr};
};

#define USE_DIFF_TEXT_VIEW
#if defined(USE_DIFF_TEXT_VIEW)
using DiffView = DiffTextView;
#else
using DiffView = DiffHtmlView;
#endif
}  // namespace QEditor

#endif  // DIFFVIEW_H
