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

#include "IrParser.h"

#include "Toast.h"

namespace QEditor {
IrParser::IrParser(EditView *editView, QObject *parent) : IParser(parent), editView_(editView)
{
    ParseFuncGraph();
}

void IrParser::ParseFuncGraph(){
    auto funcStartCursor = editView_->textCursor();
    if (funcStartCursor.isNull()) {
        qDebug() << "textCursor is invalid.";
        return;
    }
    funcStartCursor.setPosition(0, QTextCursor::MoveAnchor);

    auto entryCursor = editView_->document()->find(kIrEntry, funcStartCursor);
    const auto &entryBlockText = entryCursor.block().text();
    constexpr auto entryStart = "@";
    auto entryStartPos = entryBlockText.indexOf(entryStart);
    if (entryStartPos == -1) {
        qDebug() << "Invalid ir file.";
        return;
    }
    auto entryFuncName = entryBlockText.mid(entryStartPos + 1);
    qDebug() << "entryFuncName: " << entryFuncName;
    entryFunc_ = entryFuncName;

    while (true) {
        // Match subgraph start, usually 'subgraph attr'.
        funcStartCursor = editView_->document()->find(kSubGraphAttrStart, funcStartCursor);
        if (funcStartCursor.isNull()) {
            break;
        }
        int funcStart = funcStartCursor.selectionStart();

        // Match subgraph define start.
        funcStartCursor = editView_->document()->find(kSubGraphDefStart, funcStartCursor);
        if (funcStartCursor.isNull()) {
            break;
        }
        const QRegExp subGraphDefName = QRegExp(kSubGraphDefNameRe);
        auto subgraphDefNameCursor = editView_->document()->find(subGraphDefName, funcStartCursor);
        auto subgraphName = subgraphDefNameCursor.selectedText();
        auto subgraphSimpleName = subgraphName.section('.', 0, 0);

        // Match subgraph Return node.
        auto returnStartCursor = editView_->document()->find(kSubGraphReturnStart, subgraphDefNameCursor);
        if (returnStartCursor.isNull()) {
            Toast::Instance().Show(Toast::kError, QString(tr("No Return node found in subgraph @%1")).arg(subgraphName));
            break;
        }
        // Return variable name
        const auto &returnLineText = returnStartCursor.block().text();
        auto returnVariableStart = returnLineText.indexOf(kSubGraphReturnStart);
        if (returnVariableStart == -1) {
            Toast::Instance().Show(Toast::kError, QString(tr("Invalid Return node found in subgraph @%1")).arg(subgraphName));
            break;
        }
        returnVariableStart += strlen(kSubGraphReturnStart) + 1;
        auto returnVariableEnd = returnLineText.indexOf(")", returnVariableStart);
        if (returnVariableEnd == -1) {
            Toast::Instance().Show(Toast::kError, QString(tr("Invalid Return node found in subgraph @%1")).arg(subgraphName));
            break;
        }
        auto returnVariable = returnLineText.mid(returnVariableStart, returnVariableEnd - returnVariableStart);
        qDebug() << "returnVariable: " << returnVariable;

        // Return value
        auto returnValueStartCursor = editView_->document()->find(kSubGraphReturnValueStart, returnStartCursor);
        if (returnValueStartCursor.isNull()) {
            Toast::Instance().Show(Toast::kError, QString(tr("Invalid Return node found in subgraph @%1")).arg(subgraphName));
            break;
        }

        // Find the end of subgraph.
        const QRegExp subGraphDefEnd = QRegExp(kSubGraphDefEndRe);
        auto funcEndCursor = editView_->document()->find(subGraphDefEnd, returnValueStartCursor);
        if (funcEndCursor.isNull()) {
            Toast::Instance().Show(Toast::kError, QString(tr("Incomplete subgraph @%1")).arg(subgraphName));
            break;
        }
        int funcEnd = funcEndCursor.selectionEnd() + 1;  // Set the end after '}'.
        qDebug() << funcEndCursor.selectedText();

        // Find all callees.
        QVector<QString> callees;
        QSet<QString> calleeSet;
        auto startBlock = funcStartCursor.block();
        const auto &endBlock = funcEndCursor.block();
        do {
            const auto &startBlockText = startBlock.text();
            // Direct call.
            constexpr auto calleeStartStr = "call @";
            constexpr auto calleeStartLen = 6;
            auto calleeStart = startBlockText.indexOf(calleeStartStr);
            if (calleeStart != -1) {
                calleeStart += calleeStartLen;
                auto calleeNameEnd = startBlockText.indexOf("(", calleeStart);
                if (calleeNameEnd != -1) {
                    auto calleeName = startBlockText.mid(calleeStart, calleeNameEnd - calleeStart);
                    qDebug() << "calleeName: " << calleeName;
                    if (!calleeSet.contains(calleeName)) {
                        callees.push_back(calleeName);
                        calleeSet.insert(calleeName);
                    }
                    startBlock = startBlock.next();
                    continue;
                }
            }

            // Switch call.
            constexpr auto switchCalleeStartStr = "Switch(";
            constexpr auto switchCalleeStartLen = 7;
            auto switchCalleeStart = startBlockText.indexOf(switchCalleeStartStr);
            if (switchCalleeStart != -1) {
                switchCalleeStart += switchCalleeStartLen;
                auto switchCalleeEnd = startBlockText.indexOf(")", switchCalleeStart);
                if (switchCalleeEnd != -1) {
                    auto switchCalleeArgs = startBlockText.mid(switchCalleeStart, switchCalleeEnd - switchCalleeStart);
                    constexpr auto argsSeparator = ", ";
                    QStringList args = switchCalleeArgs.split(argsSeparator);
                    if (args.size() != 3) {
                        qDebug() << "Switch call argument size should be 3, but got " << startBlockText;
                        continue;
                    }
                    if (args[1].startsWith("@")) {
                        auto trueBranchCalleeName = args[1].mid(1);
                        qDebug() << "trueBranchCalleeName: " << trueBranchCalleeName;
                        if (!calleeSet.contains(trueBranchCalleeName)) {
                            callees.push_back(trueBranchCalleeName);
                            calleeSet.insert(trueBranchCalleeName);
                        }
                    }
                    if (args[2].startsWith("@")) {
                        auto falseBranchCalleeName = args[2].mid(1);
                        qDebug() << "falseBranchCalleeName: " << falseBranchCalleeName;
                        if (!calleeSet.contains(falseBranchCalleeName)) {
                            callees.push_back(falseBranchCalleeName);
                            calleeSet.insert(falseBranchCalleeName);
                        }
                    }
                    startBlock = startBlock.next();
                    continue;
                }
            }

            // CNode function union call.
            constexpr auto cnodeUnionCalleeStartStr = "[@FuncUnion(";
            constexpr auto cnodeUnionCalleeStartLen = 12;
            auto cnodeUnionCalleeStart = startBlockText.indexOf(cnodeUnionCalleeStartStr);
            if (cnodeUnionCalleeStart != -1) {
                cnodeUnionCalleeStart += cnodeUnionCalleeStartLen;
                auto cnodeUnionCalleeEnd = startBlockText.indexOf(")", cnodeUnionCalleeStart);
                if (cnodeUnionCalleeEnd != -1) {
                    auto cnodeUnionCalleeArgs = startBlockText.mid(cnodeUnionCalleeStart, cnodeUnionCalleeEnd - cnodeUnionCalleeStart);
                    constexpr auto argsSeparator = ", ";
                    QStringList args = cnodeUnionCalleeArgs.split(argsSeparator);
                    if (args.size() != 2) {
                        qDebug() << "Union call argument size should be 2, but got " << startBlockText;
                        continue;
                    }
                    if (args[0].startsWith("@")) {
                        auto trueBranchCalleeName = args[0].mid(1);
                        qDebug() << "trueBranchCalleeName: " << trueBranchCalleeName;
                        if (!calleeSet.contains(trueBranchCalleeName)) {
                            callees.push_back(trueBranchCalleeName);
                            calleeSet.insert(trueBranchCalleeName);
                        }
                    }
                    if (args[1].startsWith("@")) {
                        auto falseBranchCalleeName = args[1].mid(1);
                        qDebug() << "falseBranchCalleeName: " << falseBranchCalleeName;
                        if (!calleeSet.contains(falseBranchCalleeName)) {
                            callees.push_back(falseBranchCalleeName);
                            calleeSet.insert(falseBranchCalleeName);
                        }
                    }
                    startBlock = startBlock.next();
                    continue;
                }
            }

            // CNode function call.
            constexpr auto cnodeCalleeStartStr = "[@";
            constexpr auto cnodeCalleeStartLen = 2;
            auto cnodeCalleeStart = startBlockText.indexOf(cnodeCalleeStartStr);
            if (cnodeCalleeStart != -1) {
                cnodeCalleeStart += cnodeCalleeStartLen;
                auto cnodeCalleeEnd = startBlockText.indexOf("](", cnodeCalleeStart);
                if (cnodeCalleeEnd != -1) {
                    auto cnodeCalleeName = startBlockText.mid(cnodeCalleeStart, cnodeCalleeEnd - cnodeCalleeStart);
                    qDebug() << "CNode calleeName: " << cnodeCalleeName;
                    if (!calleeSet.contains(cnodeCalleeName)) {
                        callees.push_back(cnodeCalleeName);
                        calleeSet.insert(cnodeCalleeName);
                    }
                    startBlock = startBlock.next();
                    continue;
                }
            }

            startBlock = startBlock.next();
        } while (startBlock != endBlock);

        QString returnValue;
#if 0
        // Get return value RE1, with 'sequence_nodes'.
        const QRegExp subGraphReturnValue1 = QRegExp(kSubGraphReturnValueRe1);
        auto returnValueCursor = editView_->document()->find(subGraphReturnValue1, returnValueStartCursor);
        if (!returnValueCursor.isNull() && returnValueCursor.position() < funcEnd) {
            returnValue = returnValueCursor.selectedText();
            qDebug() << returnValue;
            funcStartCursor = returnValueCursor;
            qDebug() << subgraphName << subgraphDefNameCursor.selectionStart() << returnValue
                       << funcStart << funcEnd;
            FuncGraphInfo info({subgraphName, subgraphDefNameCursor.selectionStart(),
                                returnValue, funcStart, funcEnd});
            funcGraphNameInfoMap_.insert(subgraphSimpleName, info);
            funcGraphInfos_.push_back(info);
            funcGraphPos_.insert(RangeMapValue<int, int>(Range<int>(funcStart, funcEnd), funcGraphInfos_.size() - 1));
            continue;
        }
        // Get return value RE2, without 'sequence_nodes'.
        const QRegExp subGraphReturnValue2 = QRegExp(kSubGraphReturnValueRe2);
        returnValueCursor = editView_->document()->find(subGraphReturnValue2, returnValueStartCursor);
        if (!returnValueCursor.isNull() && returnValueCursor.position() < funcEnd) {
            returnValue = returnValueCursor.selectedText();
            qDebug() << returnValue;
            funcStartCursor = returnValueCursor;
            qDebug() << subgraphName << subgraphDefNameCursor.selectionStart() << returnValue
                       << funcStart << funcEnd;
            FuncGraphInfo info({subgraphName, subgraphDefNameCursor.selectionStart(),
                                returnValue, funcStart, funcEnd});
            funcGraphNameInfoMap_.insert(subgraphSimpleName, info);
            funcGraphInfos_.push_back(info);
            funcGraphPos_.insert(RangeMapValue<int, int>(Range<int>(funcStart, funcEnd), funcGraphInfos_.size() - 1));
            continue;
        }
#else
        auto returnValueBlock = returnValueStartCursor.block();
        int returnValueBlockStartPos = returnValueStartCursor.selectionEnd() - returnValueBlock.position();

        // Get return value, with 'sequence_nodes'.
        int returnValueBlockEndPos = returnValueBlock.text().indexOf(kSubGraphReturnValue1, returnValueBlockStartPos);
        if (returnValueBlockEndPos != -1) {
            returnValue = returnValueBlock.text().mid(returnValueBlockStartPos, returnValueBlockEndPos - returnValueBlockStartPos);
            qDebug() << returnValue;
            funcStartCursor = funcEndCursor;
            qDebug() << subgraphName << subgraphDefNameCursor.selectionStart() << returnValue
                       << funcStart << funcEnd;
            FuncGraphInfo info({subgraphName, subgraphDefNameCursor.selectionStart(),
                                returnVariable, returnValue, funcStart, funcEnd, callees});
            funcGraphNameInfoMap_.insert(subgraphSimpleName, info);
            funcGraphInfos_.push_back(info);
            funcGraphPos_.insert(RangeMapValue<int, int>(Range<int>(funcStart, funcEnd), funcGraphInfos_.size() - 1));
            continue;
        }

        // Get return value, without 'sequence_nodes'.
        returnValueBlockEndPos = returnValueBlock.text().indexOf(kSubGraphReturnValue2, returnValueBlockStartPos);
        if (returnValueBlockEndPos != -1) {
            returnValue = returnValueBlock.text().mid(returnValueBlockStartPos, returnValueBlockEndPos - returnValueBlockStartPos);
            qDebug() << returnValue;
            funcStartCursor = funcEndCursor;
            qDebug() << subgraphName << subgraphDefNameCursor.selectionStart() << returnValue
                       << funcStart << funcEnd;
            FuncGraphInfo info({subgraphName, subgraphDefNameCursor.selectionStart(),
                                returnVariable, returnValue, funcStart, funcEnd, callees});
            funcGraphNameInfoMap_.insert(subgraphSimpleName, info);
            funcGraphInfos_.push_back(info);
            funcGraphPos_.insert(RangeMapValue<int, int>(Range<int>(funcStart, funcEnd), funcGraphInfos_.size() - 1));
            continue;
        }
#endif
        // No return value found.
        returnValue = "<Unknown>";
        funcStartCursor = funcEndCursor;
        qDebug() << subgraphName << subgraphDefNameCursor.selectionStart() << returnValue
                   << funcStart << funcEnd;
        FuncGraphInfo info({subgraphName, subgraphDefNameCursor.selectionStart(),
                            returnVariable, returnValue, funcStart, funcEnd, callees});
        funcGraphNameInfoMap_.insert(subgraphSimpleName, info);
        funcGraphInfos_.push_back(info);
        funcGraphPos_.insert(RangeMapValue<int, int>(Range<int>(funcStart, funcEnd), funcGraphInfos_.size() - 1));
    }
}

const QMap<QString, NodeInfo> &IrParser::ParseNodes(const QString &funcName) {
    nodesMap_.clear();
    const FuncGraphInfo &funcGraphInfo = GetFuncGraphInfo(funcName);
    if (funcGraphInfo.pos_ == -1) {
        qDebug() << "FuncGraphInfo is invalid, " << funcGraphInfo.name_;
        return nodesMap_;
    }
    auto endCursor = editView_->textCursor();
    if (endCursor.isNull()) {
        qDebug() << "textCursor is invalid, " << funcGraphInfo.name_;
        return nodesMap_;
    }
    endCursor.setPosition(funcGraphInfo.end_ - 1, QTextCursor::MoveAnchor);
    const auto &endBlock = endCursor.block();
    auto startCursor = editView_->textCursor();
    startCursor.setPosition(funcGraphInfo.start_, QTextCursor::MoveAnchor);
    auto startBlock = startCursor.block();
    const auto variableDefLen = strlen(kNodeVariableDef);
    constexpr auto assignOperation = " = ";
    const auto assignOperationLen = strlen(assignOperation);
    do {
        const auto &startBlockText = startBlock.text();
        auto variableDefStart = startBlockText.indexOf(kNodeVariableDef);
        if (variableDefStart != -1) {
            variableDefStart += variableDefLen;
            auto variableDefEnd = startBlockText.indexOf("(", variableDefStart);
            if (variableDefEnd == -1) {
                startBlock = startBlock.next();
                continue;
            }
            auto variableName = startBlockText.mid(variableDefStart, variableDefEnd - variableDefStart);
            qDebug() << "variableName: " << variableName;

            auto assignOperationPos = startBlockText.indexOf(assignOperation, variableDefEnd + 1);
            if (assignOperationPos == -1) {
                startBlock = startBlock.next();
                continue;
            }
            auto opStart = assignOperationPos + assignOperationLen;
            auto opEnd = startBlockText.indexOf("(", opStart);
            if (opEnd == -1) {
                startBlock = startBlock.next();
                continue;
            }
            // Ignore "$(".
            qDebug() << "startBlockText.at(opEnd - 1): " << startBlockText.at(opEnd - 1);
            if (startBlockText.at(opEnd - 1) == '$') {
                opEnd = startBlockText.indexOf("(", opEnd + 1);
                if (opEnd == -1) {
                    startBlock = startBlock.next();
                    continue;
                }
            }
            auto opName = startBlockText.mid(opStart, opEnd - opStart);
            qDebug() << "opName: " << opName;

            QVector<QString> inputs;
            if (opName.startsWith("%")) {
                auto opInput = opName.mid(1, opName.indexOf("[") - 1);
                inputs.push_back(opInput);
            }

            constexpr auto argumentsStartStr = "(";
            auto argumentsStart = startBlockText.indexOf(argumentsStartStr, opEnd);
            if (argumentsStart == -1) {
                startBlock = startBlock.next();
                continue;
            }
            auto cursor = editView_->textCursor();
            cursor.setPosition(startBlock.position() + argumentsStart, QTextCursor::MoveAnchor);
            auto res = editView_->FindPairingBracketCursor(cursor, QTextCursor::Right, '(', ')');
            auto pairingCursor = res.first;
            auto success = res.second;
            if (success) {
                while (cursor.position() < pairingCursor.position()) {
                    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                }

                auto argumentsStr = cursor.selectedText();
                argumentsStr = argumentsStr.mid(1, argumentsStr.length() - 2);  // Remove start '(' and end ')'.
                qDebug() << "argumentsStr: " << argumentsStr;
                constexpr auto argsSeparator = ", ";
                QStringList args = argumentsStr.split(argsSeparator);
                bool hasConstantInput = false;
                for (const auto &arg : args) {
                    if (arg.startsWith("%")) {
                        const auto &argName = arg.mid(1);
                        qDebug() << "argName: " << argName;
                        inputs.push_back(argName);
                    } else {
                        hasConstantInput = true;
                    }
                    qDebug() << "arg: " << arg;
                }
                auto pos = startBlock.position() + variableDefStart;
                qDebug() << variableName << opName << inputs;
                NodeInfo nodeInfo({variableName, opName, pos, inputs, hasConstantInput});
                nodesMap_.insert(variableName, std::move(nodeInfo));
            }
        }

        startBlock = startBlock.next();
    } while (startBlock != endBlock);
    return nodesMap_;
}
}  // namespace QEditor
