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

#ifndef RANGEMAP_H
#define RANGEMAP_H

#include "Logger.h"
#include <functional>
#include <map>

namespace QEditor {
template <typename T>
class Range {
   public:
    Range(const T &center) : min_(center), max_(center) {}
    Range(const T &min, const T &max) : min_(min), max_(max) {}

    T min() const { return min_; }
    T max() const { return max_; }

    friend std::ostream &operator<<(std::ostream &os, const Range &range) {
        os << std::string("{") << range.min() << "~" << range.max << "}";
        return os;
    }

   private:
    T min_;
    T max_;
};

template <typename T>
struct LeftOfRange : public std::binary_function<Range<T>, Range<T>, bool> {
    bool operator()(const Range<T> &lhs, const Range<T> &rhs) const {
        auto res = lhs.min() < rhs.min() && lhs.max() <= rhs.min();
        return res;
    }
};

template <typename T, typename S>
using RangeMap = std::map<Range<T>, S, LeftOfRange<T>>;

template <typename T, typename S>
using RangeMapValue = typename std::map<Range<T>, S, LeftOfRange<T>>::value_type;

#if 0
void test() {
    RangeMap<int, int> posToLine;
    int top = 0;
    int bottom = 100;
    int number = 0;
    for (int i = 0; i < 10; ++i) {
        auto res = posToLine.insert(RangeMapValue<int, int>(Range<int>(top, bottom), number));
        if (!res.second) {
            qCritical() << "Fail to insert: " << " top: " << top << ", bottom: " << bottom << ", number: " << number;
            qDebug() << "size: " << posToLine.size();
            for (auto &item : posToLine) {
                qDebug() << "item: " << item.first.min() << "~" << item.first.max() << ", " << item.second;
            }
        }
        top = bottom + 1;
        bottom += 100;
        number += 1;
    }

    qDebug() << "number: " << posToLine.at(0);
    qDebug() << "number: " << posToLine.at(50);
    qDebug() << "number: " << posToLine.at(99);
    qDebug() << "number: " << posToLine.at(299);
    qDebug() << "number: " << posToLine.at(499);
}
#endif
}  // namespace QEditor

#endif  // RANGEMAP_H
