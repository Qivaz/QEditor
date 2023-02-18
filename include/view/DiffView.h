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

#include <QTextEdit>

namespace QEditor {
class EditView;
class DiffView : public QTextEdit
{
    Q_OBJECT
public:
    DiffView(QWidget *parent = nullptr);
    ~DiffView() = default;

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
}  // namespace QEditor

#endif  // DIFFVIEW_H
