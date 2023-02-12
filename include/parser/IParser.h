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

#ifndef IPARSER_H
#define IPARSER_H

#include <QObject>
#include <QSet>

namespace QEditor {
struct FuncGraphInfo
{
    QString name_;
    int pos_{-1};
    QString returnVariable_;
    QString returnValue_;
    int start_{-1};
    int end_{-1};
    QVector<QString> callees_;
};

struct NodeInfo
{
    QString variableName_;    // Variable name
    QString operatorName_;  // Operator name
    int pos_{-1};
    QVector<QString> varInputs_;
    bool hasConstantInput_;
};

class IParser : public QObject
{
    Q_OBJECT
public:
    explicit IParser(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IParser() = default;

    virtual void ParseFuncGraph() = 0;
    virtual const QString& GetEntry() const = 0;
    virtual FuncGraphInfo GetFuncGraphInfo(const QString &funcName) const = 0;
    virtual const QVector<FuncGraphInfo> &funcGraphInfos() const = 0;
    virtual int GetIndexByCursorPosition(int cursorPos) const = 0;

    virtual const QMap<QString, NodeInfo> &ParseNodes(const QString &funcName) = 0;
};
}  // namespace QEditor

#endif // IPARSER_H
