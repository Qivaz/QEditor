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

#ifndef IRPARSER_H
#define IRPARSER_H

#include "EditView.h"
#include "IParser.h"
#include "RangeMap.h"

namespace QEditor {
class DummyParser : public IParser {
    Q_OBJECT
public:
    explicit DummyParser(QObject* parent = nullptr) : IParser(parent) {}
    virtual ~DummyParser() = default;

    void ParseFuncGraph() override {}
    const QString& GetEntry() const override { return entryFunc_; };
    FuncGraphInfo GetFuncGraphInfo(const QString&) const override { return FuncGraphInfo(); }
    const QVector<FuncGraphInfo>& funcGraphInfos() const override { return funcGraphInfos_; }
    int GetIndexByCursorPosition(int) const override { return 0; }

    const QMap<QString, NodeInfo>& ParseNodes(const QString&) override { return QMap<QString, NodeInfo>(); }

private:
    QVector<FuncGraphInfo> funcGraphInfos_;
    QString entryFunc_;
    QMap<QString, NodeInfo> nodesMap_;
};

class IrParser : public IParser {
    Q_OBJECT
public:
    explicit IrParser(EditView* editView, QObject* parent = nullptr);
    virtual ~IrParser() = default;

    void ParseFuncGraph() override;
    const QString& GetEntry() const override { return entryFunc_; };
    FuncGraphInfo GetFuncGraphInfo(const QString& funcName) const override {
        const auto& simpleFuncName = funcName.section('.', 0, 0);
        return funcGraphNameInfoMap_.value(simpleFuncName);
    }
    const QVector<FuncGraphInfo>& funcGraphInfos() const override { return funcGraphInfos_; }
    int GetIndexByCursorPosition(int cursorPos) const override { return funcGraphPos_.at(cursorPos); }

    const QMap<QString, NodeInfo>& ParseNodes(const QString& funcName) override;

private:
    const QMap<QString, NodeInfo>& ParseNodes(const FuncGraphInfo&);

    constexpr static auto kIrEntry = "#IR entry      : ";
    constexpr static auto kSubGraph = "subgraph";
    constexpr static auto kSubGraphDefinePreciseNameRe = "(?<=subgraph @).*(?=\\()";
    constexpr static auto kSubGraphAttrStart = "subgraph attr:";
    constexpr static auto kSubGraphDefStart = "subgraph @";
    constexpr static auto kSubGraphDefEndRe = "^\\}$";
    constexpr static auto kSubGraphDefNameRe = ".*(?=\\()";
    constexpr static auto kSubGraphReturnStart = "Return(";
    constexpr static auto kSubGraphReturnValueStart = "      : (<";
    constexpr static auto kSubGraphReturnValueRe = ".*((?=\\, sequence_nodes)|(?=>\\)))";
    constexpr static auto kSubGraphReturnValueRe1 = ".*(?=\\, sequence_nodes)";
    constexpr static auto kSubGraphReturnValueRe2 = ".*(?=>\\))";
    constexpr static auto kSubGraphReturnValue1 = ", sequence_nodes";
    constexpr static auto kSubGraphReturnValue2 = ">)";

    constexpr static auto kNodeVariableDef = "  %";

    EditView* editView_;
    QVector<FuncGraphInfo> funcGraphInfos_;
    RangeMap<int, int> funcGraphPos_;
    QMap<QString, FuncGraphInfo> funcGraphNameInfoMap_;
    QString entryFunc_;
    QMap<QString, NodeInfo> nodesMap_;
};
} // namespace QEditor

#endif // IRPARSER_H
