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

#ifndef COMBOVIEW_H
#define COMBOVIEW_H

#include <QComboBox>
#include <QLineEdit>

namespace QEditor {
class ComboView : public QComboBox {
    Q_OBJECT
public:
    ComboView(QWidget* parent = nullptr, bool fixed = true);
    virtual ~ComboView() { delete lineEdit(); }

    void ShrinkForPopup();
    void ShrinkForChosen();

    void showPopup() override;
    void hidePopup() override;

private:
    bool fixed_{false};
    qreal maxWidth_{0};
};
} // namespace QEditor

#endif // COMBOVIEW_H
