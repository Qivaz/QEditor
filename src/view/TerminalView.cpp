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
#include "Logger.h"
#include "ansiescapecodehandler.h"
#include <MainWindow.h>
#include <QScrollBar>

namespace QEditor {
TerminalView::TerminalView(const QString &ip, int port, const QString &user, const QString &pwd, QWidget *parent)
    : EditView(user + "@" + ip + ":" + QString::number(port), parent),
      ip_(ip),
      port_(port),
      user_(user),
      pwd_(pwd),
      savedCharFormat_(currentCharFormat()) {
    //    setCenterOnScroll(true);
    CreateConnection();

    QTextOption textOption;
    textOption.setFlags(QTextOption::Flag(0));
    document()->setDefaultTextOption(textOption);
}

void TerminalView::CreateConnection() {
    sshClient_ = new SshClient(ip_, port_, user_, pwd_);
    sshClient_->Initialize();
    // Handle event.
    connect(sshClient_, SIGNAL(sigConnectStateChanged(bool, const QString &, int)), this,
            SLOT(HandleConnectStateChanged(bool, const QString &, int)));
    connect(sshClient_, SIGNAL(sigShellConnected(const QString &, int)), this,
            SLOT(HandleShellConnected(const QString &, int)));
    connect(sshClient_, SIGNAL(sigShellDataArrived(const QString &, const QString &, int)), this,
            SLOT(HandleShellDataArrived(const QString &, const QString &, int)));
    // Request event.
    connect(this, SIGNAL(sigShellSend(const QString &)), sshClient_, SLOT(HandleShellSend(const QString &)));
    connect(this, SIGNAL(sigDisconnected()), sshClient_, SLOT(HandleDisconnected()));
}

void TerminalView::mousePressEvent(QMouseEvent *event) {
    qDebug() << "event: " << event;
    EditView::mousePressEvent(event);
}

void TerminalView::mouseMoveEvent(QMouseEvent *event) {
    qDebug() << "event: " << event;
    EditView::mouseMoveEvent(event);
}

void TerminalView::mouseDoubleClickEvent(QMouseEvent *event) {
    qDebug() << "event: " << event;
    EditView::mouseDoubleClickEvent(event);
}

void TerminalView::contextMenuEvent(QContextMenuEvent *event) { EditView::contextMenuEvent(event); }

void TerminalView::keyPressEvent(QKeyEvent *event) {
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

    cmdEntered_ = false;
    if (event->key() == Qt::Key_Return) {
        cmdEntered_ = true;
        moveCursor(QTextCursor::End);
        insertPlainText(event->text());
        cmdBuffer_.append(event->text()) /*.append('\n')*/;
    } else if (event->key() == Qt::Key_Tab) {
        cmdBuffer_.append(event->text());
    } else if (event->key() == Qt::Key_Up || event->text() == "\u0010") {
        cmdBuffer_.append(tr("\u0010"));
    } else if (event->key() == Qt::Key_Down || event->text() == "\u000E") {
        cmdBuffer_.append(tr("\u000E"));
    } else if (event->text() == "\u0003" || event->text() == "\u0004") {  // Ctrl+C("\u0003") or Ctrl+D("\u0004")
        cmdBuffer_.clear();
        cmdBuffer_.append(event->text());
    } else if (!event->text().isEmpty()) {
        int pos = -1;
        // document()->characterCount() contains \n and EOF.
        // Backward offset must >= 0.
        auto offset = document()->characterCount() - 1 - textCursor().position();
        qCritical() << "cmd: " << cmdBuffer_ << cmdBuffer_.length() << ", offset: " << offset
                    << ", position: " << textCursor().position() << ", char: " << event->text();
        if (offset > cmdBuffer_.length()) {
            // It the cursor is not in command input area, move cursor to end and append input char to inputed cmd's
            // tail.
            moveCursor(QTextCursor::End);
            pos = cmdBuffer_.length();
            MainWindow::Instance().statusBar()->showMessage(
                QString(tr("Append \'%1\' in command line tail.")).arg(event->text()), 10000);
        } else {
            pos = cmdBuffer_.length() - offset;
        }
        cmdBuffer_.insert(pos, event->text());
        qCritical() << "cmd: " << cmdBuffer_ << ", pos: " << pos << ", char: " << event->text();

        insertPlainText(event->text());
    } else {
        // Error.
    }

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Tab || event->key() == Qt::Key_Up ||
        event->key() == Qt::Key_Down || event->text() == "\u0003" ||
        event->text() == "\u0004") {  // Enter, Tab, Ctrl+C("\u0003") or Ctrl+D("\u0004")
        if (sshClient_->connected()) {
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

            //                emit sigShellSend(sendingCmd_);
            //                cmdBuffer_.clear();
            //                return;
            //            }

            //            sshClient_->HandleShellSend(sendingCmd_);
            emit sigShellSend(sendingCmd_ /* + "\n"*/);
            cmdBuffer_.clear();
        }
    }
}

void TerminalView::keyReleaseEvent(QKeyEvent *event) {
    qDebug() << "event: " << event;
    //    EditView::keyReleaseEvent(event);
}

void TerminalView::HandleConnectStateChanged(bool state, const QString &ip, int port) {
    Q_UNUSED(ip)
    Q_UNUSED(port)

    qDebug() << "state: " << state;
    if (state) {
        MainWindow::Instance().statusBar()->showMessage(QString(tr("Try to connecting %1:%2.")).arg(ip).arg(port),
                                                        10000);
    } else {
        connectState_ = state;
        MainWindow::Instance().statusBar()->showMessage(QString(tr("%1:%2 is disconnected.")).arg(ip).arg(port), 10000);
    }
}

void TerminalView::HandleShellConnected(const QString &ip, int port) {
    connectState_ = true;
    MainWindow::Instance().statusBar()->showMessage(QString(tr("%1:%2 is connected.")).arg(ip).arg(port), 10000);
}

void TerminalView::HandleAnsiEscapeCode(const QString &constMsg, QTextCursor &cursor) {
    if (constMsg.startsWith('\r') || constMsg.startsWith('\u001B')) {
        for (int i = 0; i < constMsg.length(); ++i) {
            const auto &c = constMsg[i];
            if (c == '\r') {
                //                auto cursor = textCursor();
                cursor.setPosition(promptPos_);
                //                setTextCursor(cursor);
            } else if (c == '\u001B') {  // Escape
                if (constMsg.length() <= i + 2) {
                    qCritical() << "Recv: " << constMsg;
                    qFatal("Receive invalid message.");
                }
                const auto &c1 = constMsg[i + 1];
                const auto &c2 = constMsg[i + 2];
                if (c1 != '[') {
                    break;
                }
                switch (c2.toLatin1()) {
                    case 'A': {  // Up
                                 //                        QTextCursor cursor = textCursor();
                        cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor);
                        //                        setTextCursor(cursor);
                        break;
                    }
                    case 'B': {  // Down
                                 //                        QTextCursor cursor = textCursor();
                        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
                        //                        setTextCursor(cursor);
                        break;
                    }
                    case 'C': {  // Right
                                 //                        QTextCursor cursor = textCursor();
                        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
                        //                        setTextCursor(cursor);
                        break;
                    }
                    case 'D': {  // Left
                                 //                        QTextCursor cursor = textCursor();
                        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
                        //                        setTextCursor(cursor);
                        break;
                    }
                    case 'K': {  // Only one 'K', Erase from cursor to end of line.
                                 //                        QTextCursor cursor = textCursor();
                        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
                        //                        setTextCursor(cursor);
                        break;
                    }
                    default:
                        break;
                }
            } else {
                break;
            }
        }
    }
}

void TerminalView::HandleShellDataArrived(const QString &msg, const QString &ip, int port) {
    Q_UNUSED(ip)
    Q_UNUSED(port)

    QString strippedMsg = msg;
    qCritical() << "msg: " << msg << ", cmd: " << sendingCmd_;
    // Handle feedback sent command, skip them.
    for (int i = 0; i < sendingCmd_.length() && i < msg.length(); ++i) {
        const auto &c = sendingCmd_[i];
        if (c == msg[i]) {
            strippedMsg = strippedMsg.mid(1);
        }
        if (c == '\r') {  // Send \r, received \r\n.
            strippedMsg = strippedMsg.mid(1);
        }
    }
    // Handle bell, skip.
    if (strippedMsg.startsWith('\u0007')) {
        strippedMsg.remove(0, 1);
    }

    // Handle \b (\u0008).
    if (strippedMsg.startsWith('\b')) {
        const QString constRecvMsg = strippedMsg;
        for (const auto &c : constRecvMsg) {
            if (c == '\b') {
                strippedMsg.remove(0, 1);
                textCursor().deletePreviousChar();
            } else {
                break;
            }
        }
    }

    qCritical() << "stripped msg: " << strippedMsg;
    auto cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    HandleAnsiEscapeCode(strippedMsg, cursor);
    ::Utils::AnsiEscapeCodeHandler handler;
    auto fts = handler.parseText(::Utils::FormattedText(strippedMsg, currentCharFormat()));
    foreach(auto &ft, fts) {
        qCritical() << "text: " << ft.text << ", format: " << ft.format.foreground() << ft.format.background();
        cursor.insertText(ft.text, ft.format);
        HandleAnsiEscapeCode(strippedMsg, cursor);
    }
    setCurrentCharFormat(savedCharFormat_);
    if (textCursor().position() == document()->characterCount() - 1) {
        ensureCursorVisible();  // verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    }
    if (cmdEntered_) {
        promptPos_ = document()->characterCount() - 1;
    }

    sendingCmd_.clear();
}

QString TerminalView::pwd() const { return pwd_; }

QString TerminalView::user() const { return user_; }

int TerminalView::port() const { return port_; }

QString TerminalView::ip() const { return ip_; }
}  // namespace QEditor
