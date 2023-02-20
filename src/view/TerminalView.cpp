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

#include "TerminalView.h"

#include <QScrollBar>

#include "ansiescapecodehandler.h"

#include "Toast.h"
#include "Logger.h"

using Utils::AnsiEscapeCodeHandler;
using Utils::FormattedText;

namespace QEditor {
TerminalView::TerminalView(const QString &ip, int port, const QString &user, const QString &pwd, QWidget *parent)
    : EditView(user + "@" + ip + ":" + QString::number(port), parent), ip_(ip), port_(port), user_(user), pwd_(pwd)
{
//    setCenterOnScroll(true);
    CreateConnection();

    QTextOption textOption;
    textOption.setFlags(0);
    document()->setDefaultTextOption(textOption);
}

void TerminalView::CreateConnection()
{
    sshClient_ = new SshClient(ip_, port_, user_, pwd_);
    sshClient_->Initialize();
    connect(sshClient_, SIGNAL(sigConnectStateChanged(bool, const QString&, int)), this, SLOT(ConnectStateChanged(bool, const QString&, int)));
    connect(sshClient_, SIGNAL(sigShellConnected(const QString&, int)), this, SLOT(ShellConnected(const QString&, int)));
    connect(sshClient_, SIGNAL(sigDataArrived(const QString&, const QString&, int)), this, SLOT(DataArrived(const QString&, const QString&, int)));
    connect(this, SIGNAL(sigSend(const QString&)), sshClient_, SLOT(slotSend(const QString&)));
    connect(this, SIGNAL(sigDisconnected()), sshClient_, SLOT(slotDisconnected()));
}

void TerminalView::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "event: " << event;
    EditView::mousePressEvent(event);
}

void TerminalView::mouseMoveEvent(QMouseEvent *event)
{
    qDebug() << "event: " << event;
    EditView::mouseMoveEvent(event);
}

void TerminalView::mouseDoubleClickEvent(QMouseEvent *event)
{
    qDebug() << "event: " << event;
    EditView::mouseDoubleClickEvent(event);
}

void TerminalView::contextMenuEvent(QContextMenuEvent *event)
{
    EditView::contextMenuEvent(event);
}

void TerminalView::keyPressEvent(QKeyEvent *event)
{
    qCritical() << "event: " << event << ", cursor: " << textCursor().position();
    switch (event->key()) {
        case Qt::Key_Backspace:
            if (cmdBuffer_.length() > 0) {
                QTextCursor cursor = textCursor();
                // document()->characterCount() contains \n and EOF.
                auto offset = document()->characterCount() - 1 - cursor.position();
                auto pos = cmdBuffer_.length() - offset - 1;
                if (pos >= 0) {
                    cmdBuffer_.remove(pos, 1);
                    cursor.deletePreviousChar();
                }
            }
            return;
        case Qt::Key_Delete:
            if (cmdBuffer_.length() > 0) {
                QTextCursor cursor = textCursor();
                // document()->characterCount() contains \n and EOF.
                auto offset = document()->characterCount() - 1 - cursor.position();
                cmdBuffer_.remove(cmdBuffer_.length() - offset, 1);
                cursor.deleteChar();
            }
            return;
        case Qt::Key_Left: {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
            setTextCursor(cursor);
            return;
        }
        case Qt::Key_Right: {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
            setTextCursor(cursor);
            return;
        }
//        case Qt::Key_Up: {
//            QTextCursor cursor = textCursor();
//            cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor);
//            setTextCursor(cursor);
//            return;
//        }
//        case Qt::Key_Down: {
//            QTextCursor cursor = textCursor();
//            cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
//            setTextCursor(cursor);
//            return;
//        }
    }

    if (event->key() == Qt::Key_Return) {
        moveCursor(QTextCursor::End);
        insertPlainText(event->text());
        cmdBuffer_.append(event->text())/*.append('\n')*/;
    } else if (event->key() == Qt::Key_Tab) {
        cmdBuffer_.append(event->text());
    } else if (event->key() == Qt::Key_Up) {
        cmdBuffer_.append(tr("\u0010"));
    } else if (event->key() == Qt::Key_Down) {
        cmdBuffer_.append(tr("\u000E"));
    } else if (event->text() == "\u0003" || event->text() == "\u0004") {  // Ctrl+C("\u0003") or Ctrl+D("\u0004")
        cmdBuffer_.clear();
        cmdBuffer_.append(event->text());
    } else if (!event->text().isEmpty()) {
        int pos = -1;
        // document()->characterCount() contains \n and EOF.
        // Backward offset must >= 0.
        auto offset = document()->characterCount() - 1 - textCursor().position();
        qCritical() << "cmd: " << cmdBuffer_ << cmdBuffer_.length() << ", offset: " << offset << ", position: " << textCursor().position() << ", char: " << event->text();
        if (offset > cmdBuffer_.length()) {
            // It the cursor is not in command input area, move cursor to end and append input char to inputed cmd's tail.
            moveCursor(QTextCursor::End);
            pos = cmdBuffer_.length();
            Toast::Instance().Show(Toast::kInfo, QString("Append \'%1\' in command line tail.").arg(event->text()));
        } else {
            pos = cmdBuffer_.length() - offset;
        }
        cmdBuffer_.insert(pos, event->text());
        qCritical() << "cmd: " << cmdBuffer_ << ", pos: " << pos << ", char: " << event->text();

        insertPlainText(event->text());
    } else {
        // Error.
    }

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Tab ||
        event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ||
        event->text() == "\u0003" || event->text() == "\u0004") {  // Enter, Tab, Ctrl+C("\u0003") or Ctrl+D("\u0004")
        if(sshClient_->connected()){
            // If cmdBuffer_ is empty, we send it for 'Key_Return';
            sendingCmd_ = cmdBuffer_;
            qCritical() << "cmdBuffer_: " << cmdBuffer_;
            sending_ = true;

//            if (event->key() == Qt::Key_Tab) {
//                QTextCursor startCursor = textCursor();
//                startCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
//                for (int i = 0; i < cmdBuffer_.length() - 1; ++i) {
//                    bool res = startCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
//                    if (!res) {
//                        break;
//                    }
//                }
//                startCursor.deleteChar();

//                emit sigSend(sendingCmd_);
//                cmdBuffer_.clear();
//                return;
//            }

//            sshClient_->slotSend(sendingCmd_);
            emit sigSend(sendingCmd_/* + "\n"*/);
            cmdBuffer_.clear();
        }
    }
}

void TerminalView::keyReleaseEvent(QKeyEvent *event)
{
    qDebug() << "event: " << event;
//    EditView::keyReleaseEvent(event);
}

void TerminalView::ConnectStateChanged(bool state, const QString &ip, int port)
{
    Q_UNUSED(ip)
    Q_UNUSED(port)

    qDebug() << "state: " << state;
    if(state) {
        Toast::Instance().Show(Toast::kInfo, QString("Try to connecting %1:%2.").arg(ip).arg(port));
    } else {
        connectState_ = state;
        Toast::Instance().Show(Toast::kWarning, QString("%1:%2 is disconnected.").arg(ip).arg(port));
    }
}

void TerminalView::ShellConnected(const QString &ip, int port)
{
    connectState_ = true;
    Toast::Instance().Show(Toast::kInfo, QString("%1:%2 is connected.").arg(ip).arg(port));
}

void TerminalView::DataArrived(const QString &msg, const QString &ip, int port)
{
    Q_UNUSED(ip)
    Q_UNUSED(port)

    QString pureRecvMsg = msg;
    qCritical() << "msg: " << msg << ", cmd: " << sendingCmd_;
    for (int i = 0; i < sendingCmd_.length() && i < msg.length(); ++i) {
        const auto &c = sendingCmd_[i];
        if (c == msg[i]) {
            pureRecvMsg = pureRecvMsg.mid(1);
        }
        if (c == '\r') {  // Send \r, received \r\n.
            pureRecvMsg = pureRecvMsg.mid(1);
        }
    }
    qCritical() << "pureRecvMsg: " << pureRecvMsg;
    // Handle \b (\u0008).
    if (pureRecvMsg.startsWith('\b')) {
        const QString constPureRecvMsg = pureRecvMsg;
        for (const auto &c : constPureRecvMsg) {
            if (c == '\b') {
                pureRecvMsg.remove(0, 1);
                textCursor().deletePreviousChar();
            } else {
                break;
            }
        }
    }
    // Handle bell.
    if (pureRecvMsg.startsWith('\u0007')) {
        pureRecvMsg.remove(0, 1);
    }

    auto savedCharFormat = currentCharFormat();
    AnsiEscapeCodeHandler handler;
    auto fts = handler.parseText(FormattedText(pureRecvMsg, currentCharFormat()));
    foreach (auto &ft, fts) {
        qCritical() << "text: " << ft.text << ", format: " << ft.format.foreground() << ft.format.background();
        auto cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(ft.text, ft.format);
    }
    setCurrentCharFormat(savedCharFormat);
    if (textCursor().position() == document()->characterCount() - 1) {
        ensureCursorVisible();  // verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    }

    sendingCmd_.clear();
}

QString TerminalView::pwd() const
{
    return pwd_;
}

QString TerminalView::user() const
{
    return user_;
}

int TerminalView::port() const
{
    return port_;
}

QString TerminalView::ip() const
{
    return ip_;
}
}  // namespace QEditor
