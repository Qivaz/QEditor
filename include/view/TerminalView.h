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

#ifndef TERMINALVIEW_H
#define TERMINALVIEW_H

#include "EditView.h"
#include "SshClient.h"

namespace QEditor {
class TerminalView : public EditView
{
    Q_OBJECT
public:
    TerminalView(const QString &ip, int port, const QString &user, const QString &pwd, QWidget *parent = nullptr);
    ~TerminalView() {
        qCritical();
        if (sshClient_ != nullptr) {
            delete sshClient_;
        }
    }

    SshClient *sshClient() const { return sshClient_; }
    void setSshClient(SshClient *sshClient) { sshClient_ = sshClient; }

    void CreateConnection();

    QString ip() const;

    int port() const;

    QString user() const;

    QString pwd() const;

private:
    void HandleAnsiEscapeCode(const QString &constMsg, QTextCursor &cursor);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void HandleConnectStateChanged(bool state, const QString &ip, int port);
    void HandleShellConnected(const QString &ip, int port);
    void HandleShellDataArrived(const QString &msg, const QString &ip, int port);

signals:
    void sigShellSend(const QString &msg);
    void sigDisconnected();

private:
    // SSH Terminal.
    SshClient *sshClient_{nullptr};

    QString ip_;
    int port_;
    QString user_;
    QString pwd_;

    QString cmdBuffer_;
    QString sendingCmd_;
    bool sending_{false};

    bool cmdEntered_{false};
    int promptPos_{-1};

    bool connectState_{false};
};
}  // namespace QEditor

#endif  // TERMINALVIEW_H
