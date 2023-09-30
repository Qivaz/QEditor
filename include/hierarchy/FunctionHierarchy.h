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

#ifndef FUNCTIONHIERARCHY_H
#define FUNCTIONHIERARCHY_H

#include "FunctionHierarchyScene.h"
#include "IParser.h"
#include "IrParser.h"
#include <QGraphicsView>

namespace QEditor {
class FunctionHierarchy : public QGraphicsView {
    Q_OBJECT
   public:
    FunctionHierarchy(IParser *parser = new DummyParser(), QWidget *parent = nullptr);

   private:
    FunctionHierarchyScene *scene_{nullptr};
};
}  // namespace QEditor

#endif  // FUNCTIONHIERARCHY_H
