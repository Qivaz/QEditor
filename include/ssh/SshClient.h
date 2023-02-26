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

#ifndef SSHCLIENT_H
#define SSHCLIENT_H

#include <QHostAddress>
#include <QTimer>
#include <QThread>

#include <sshconnection.h>
#include <sshremoteprocess.h>
#include <sftpchannel.h>

#define RECONNET_SPAN_TIME (1000*10)  //连接状态心跳

namespace QEditor {
class  SshClient : public QObject
{
    Q_OBJECT
public:
    explicit SshClient(const QString &ip, int port, const QString &user, const QString &pwd);
    ~SshClient();

    void Initialize();
    void Uninitialize();

    int Send(const QString &message);
    bool connected() const { return connected_; }

signals:
    void sigInitializeInThread();
    void sigConnectStateChanged(bool state, const QString &ip, int port);
    void sigShellConnected(const QString &ip, int port);
    void sigShellDataArrived(const QString &msg, const QString &ip, int port);

private:
    QString IpAndPort(){return ip_ + ":" + QString::number(port_);}

public slots:
    void HandleDisconnected();
    void HandleResetConnection(const QString &ipPort);
    void HandleShellSend(const QString &message);
    void HandleShellReceived();

private slots:
    void HandleInitializeInThread();
    void HandleCreateConnection();
    void HandleConnected();
    void HandleThreadFinished();
    void HandleSshConnectError(const QSsh::SshError &sshError);

    void HandleShellStart();
    void HandleShellError();

    void HandleChannelInitialized();
    void HandleChannelInitializationFailure(const QString &);
    void HandleChannelJobFinished(const QSsh::SftpJobId, const QString &);
    void HandleChannelClosed();
    void HandleFileInfoAvailable(QSsh::SftpJobId job, const QList<QSsh::SftpFileInfo> &fileInfoList);

private:
    QThread *thread_{nullptr};
    bool connected_{false};
    bool shellConnected_{false};

    QTimer *timer_{nullptr};

    QString ip_ = "";
    int port_ = -1;
    QString user_;
    QString pwd_;
    QString ipPort_;

    QSsh::SshConnectionParameters parameters_;
    QSsh::SshConnection *sshConnection_ = nullptr;
    QSharedPointer<QSsh::SshRemoteProcess> shell_;
    QSharedPointer<QSsh::SftpChannel> channel_;
};
}  // namespace QEditor

#endif // SSHCLIENT_H
