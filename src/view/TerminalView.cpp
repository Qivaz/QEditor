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

namespace QEditor {
TerminalView::TerminalView(const QString &ip, int port, const QString &user, const QString &pwd, QWidget *parent)
    : EditView(user + "@" + ip + ":" + QString::number(port), parent), ip_(ip), port_(port), user_(user), pwd_(pwd)
{
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
    connect(sshClient_, SIGNAL(sigDataArrived(const QString&, const QString&, int)), this, SLOT(DataArrived(const QString&, const QString&, int)));
    connect(this, SIGNAL(sigSend(QString)), sshClient_, SLOT(slotSend(QString)));
    connect(this, SIGNAL(sigDisconnected()), sshClient_, SLOT(slotDisconnected()));
}

void TerminalView::keyPressEvent(QKeyEvent *event)
{
    qCritical() << "event: " << event;
    if (event->key() == Qt::Key_Return) {
        if(sshClient_->connected() && !cmdBuffer_.isEmpty()){
            sshClient_->Send(cmdBuffer_ + "\n");
            cmdBuffer_.clear();
        }
    } else if (!event->text().isEmpty()) {
        moveCursor(QTextCursor::End);
        insertPlainText(event->text());
        cmdBuffer_.append(event->text());
    }
}

void TerminalView::keyReleaseEvent(QKeyEvent *event)
{
    qCritical() << "event: " << event;
}

void TerminalView::mousePressEvent(QMouseEvent *event)
{
    qCritical() << "event: " << event;
}

void TerminalView::mouseMoveEvent(QMouseEvent *event)
{
    qCritical() << "event: " << event;
}

void TerminalView::mouseDoubleClickEvent(QMouseEvent *event)
{
    qCritical() << "event: " << event;
}

void TerminalView::contextMenuEvent(QContextMenuEvent *event)
{
    EditView::contextMenuEvent(event);
}

void TerminalView::ConnectStateChanged(bool state, const QString &ip, int port)
{
    Q_UNUSED(ip)
    Q_UNUSED(port)

    qCritical() << "state: " << state;
    connectState_ = state;
}

void TerminalView::DataArrived(const QString &msg, const QString &ip, int port)
{
    Q_UNUSED(ip)
    Q_UNUSED(port)

    qCritical() << "msg: " << msg;
    appendPlainText(msg);
}
}  // namespace QEditor
