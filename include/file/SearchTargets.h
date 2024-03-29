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

#ifndef SEARCHTARGETS_H
#define SEARCHTARGETS_H

#include "Constants.h"
#include <QDataStream>
#include <QObject>

namespace QEditor {
class SearchTargets : public QObject {
    Q_OBJECT
   public:
    static void UpdateTargets(const QString &);
    static void Clear();

    static void StoreTargets();
    static void LoadTargets();

    static const QStringList &targets() { return targets_; }

   private:
    static QStringList targets_;

    static const QString kAppInternalSearchTargetsDirName_;
    static const QString kAppInternalSearchTargetsFileName_;

    static const int kMaxSearchTargetsNum_ = Constants::kMaxSearchTargetsNum;
};
}  // namespace QEditor

#endif  // SEARCHTARGETS_H
