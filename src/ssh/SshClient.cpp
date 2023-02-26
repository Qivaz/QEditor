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

#include "SshClient.h"

#include "Logger.h"

namespace QEditor {
SshClient::SshClient(const QString &ip, int port, const QString &user, const QString &pwd)
{
    ip_ = ip;
    port_ = port;
    user_ = user;
    pwd_ = pwd;
    ipPort_ = ip_ + ":" + QString::number(port_);
}

SshClient::~SshClient()
{
    qCritical();
    if (thread_ != nullptr) {
        thread_->requestInterruption();
        thread_->quit();

        // We should not delete thread_ here, cause we called this->moveToThread(thread_); before.
        // To delete thread_ after delete this(SshClient instance).
        // delete thread_;
        // thread_ = nullptr;
    }
    if (timer_ != nullptr) {
        timer_->stop();
        delete timer_;
        timer_ = nullptr;
    }
    if(sshConnection_ != nullptr){
        delete sshConnection_;
        sshConnection_ = nullptr;
    }
}

void SshClient::Initialize()
{
    thread_ = new QThread(this);
//    connect(thread_, SIGNAL(finished()), this, SLOT(HandleThreadFinished()));
    this->moveToThread(thread_);
    thread_->start();

    // To use signals and slots afterwards.
    connect(this, SIGNAL(sigInitializeInThread()), this, SLOT(HandleInitializeInThread()));
    emit sigInitializeInThread();
}

void SshClient::Uninitialize()
{
    thread_->quit();
}

int SshClient::Send(const QString &message)
{
    qCritical() << "Send: " << message;
    int size = 0;
    if(connected_ && shellConnected_){
       size = shell_->write(message.toLatin1().data());
       qCritical() << "Send successfully, size: " << size;
    }else{
       qCritical() << "Not connected or shell is not connected: " << IpAndPort();
    }
    return size;
}

void SshClient::HandleDisconnected()
{
    qCritical();
    sshConnection_->disconnectFromHost();
}

void SshClient::HandleResetConnection(const QString &ipPort)
{
    if(this->IpAndPort() == ipPort){
        this->HandleDisconnected();
    }
}

void SshClient::HandleShellSend(const QString &message)
{
    Send(message);
}

void SshClient::HandleShellReceived()
{
    qCritical();
    QByteArray recvByteArray = shell_->readAllStandardOutput();
    QString recv = QString::fromUtf8(recvByteArray);
    if(recv.isEmpty()) {
        return;
    }
    qCritical() << "Receive: " << recv;
    emit sigShellDataArrived(recv, ip_, port_);
}

void SshClient::HandleInitializeInThread()
{
    parameters_.port = port_;
    parameters_.userName = user_;
    parameters_.password = pwd_;
    parameters_.host = ip_;
    parameters_.timeout = 10;
    parameters_.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypePassword;
    HandleCreateConnection();  // Create connection.

    if (timer_ == nullptr) {
        timer_ = new QTimer(this);
    }
    timer_->setInterval(RECONNET_SPAN_TIME);
    connect(timer_, SIGNAL(timeout()), this, SLOT(HandleCreateConnection()));
    timer_->start();  // Start heartbeat timer, call slotCreateConnection to check if reconnection needed in circles.
}

void SshClient::HandleCreateConnection()
{
    qCritical();
    if(connected_) {
        return;
    }
    if(sshConnection_ == nullptr){
        sshConnection_ = new QSsh::SshConnection(parameters_);
        connect(sshConnection_, SIGNAL(connected()), SLOT(HandleConnected()));
        connect(sshConnection_, SIGNAL(error(QSsh::SshError)), SLOT(HandleSshConnectError(const QSsh::SshError&)));
    }
    sshConnection_->connectToHost();
    qCritical() << "Try to connect: " << IpAndPort();
}

void SshClient::HandleConnected()
{
    qCritical() << "Ssh connected: " << IpAndPort();
    timer_->stop();

    // Create the shell.
    shell_ = sshConnection_->createRemoteShell();
    connect(shell_.data(), SIGNAL(started()), SLOT(HandleShellStart()));
    connect(shell_.data(), SIGNAL(readyReadStandardOutput()), SLOT(HandleShellReceived()));
    connect(shell_.data(), SIGNAL(readyReadStandardError()), SLOT(HandleShellError()));
    shell_.data()->start();

    // Create sftp channel.
    channel_ = sshConnection_->createSftpChannel();
    connect(channel_.data(), SIGNAL(initialized()), this, SLOT(HandleChannelInitialized()));
    connect(channel_.data(), SIGNAL(channelError(QString)), this,SLOT(HandleChannelInitializationFailure(QString)));
    connect(channel_.data(), SIGNAL(finished(QSsh::SftpJobId, QString)), this, SLOT(HandleChannelJobFinished(QSsh::SftpJobId, QString)));
    connect(channel_.data(), SIGNAL(closed()), this, SLOT(HandleChannelClosed()));
    connect(channel_.data(), SIGNAL(fileInfoAvailable(QSsh::SftpJobId job, const QList<QSsh::SftpFileInfo> &fileInfoList)),
            this, SLOT(HandleFileInfoAvailable(QSsh::SftpJobId job, const QList<QSsh::SftpFileInfo> &fileInfoList)));
    channel_->initialize();

    connected_ = true;
    emit sigConnectStateChanged(connected_, ip_, port_);
}

void SshClient::HandleThreadFinished()
{
    qCritical();
    thread_->deleteLater();
    this->deleteLater();
}

void SshClient::HandleSshConnectError(const QSsh::SshError &sshError)
{
    shellConnected_ = false;
    connected_ = false;
    emit sigConnectStateChanged(connected_, ip_, port_);
    timer_->start();

    switch(sshError){
    case QSsh::SshNoError:
        qCritical() << "SshNoError: " << IpAndPort();
        break;
    case QSsh::SshSocketError:
        qCritical() << "SshSocketError: " << IpAndPort();  // If cable detached.
        break;
    case QSsh::SshTimeoutError:
        qCritical() << "SshTimeoutError: " << IpAndPort();
        break;
    case QSsh::SshProtocolError:
        qCritical() << "SshProtocolError: " << IpAndPort();
        break;
    case QSsh::SshHostKeyError:
        qCritical() << "SshHostKeyError: " << IpAndPort();
        break;
    case QSsh::SshKeyFileError:
        qCritical() << "SshKeyFileError: " << IpAndPort();
        break;
    case QSsh::SshAuthenticationError:
        qCritical() << "SshAuthenticationError: " << IpAndPort();
        break;
    case QSsh::SshClosedByServerError:
        qCritical() << "SshClosedByServerError: " << IpAndPort();
        break;
    case QSsh::SshInternalError:
        qCritical() << "SshInternalError: " << IpAndPort();
        break;
    default:
        break;
    }
}

void SshClient::HandleShellStart()
{
    qCritical() << "Shell connected: " << IpAndPort();
    shellConnected_ = true;
    emit sigShellConnected(ip_, port_);
}

void SshClient::HandleShellError()
{
    qCritical() << "Shell error happpended: " << IpAndPort();
}

void SshClient::HandleChannelInitialized()
{
    qCritical() << "Channel initialized: " << IpAndPort();
//    QSsh::SftpJobId job = channel_->listDirectory("/");
//    if (job == QSsh::SftpInvalidJob) {
//        qCritical() << "Start job failed.";
//    }
    const QSsh::SftpJobId statJob = channel_->statFile("/home/zqh/tmp1/tmp");
    if (statJob == QSsh::SftpInvalidJob) {
        qCritical() << "Start job failed.";
    }
//    const QSsh::SftpJobId uploadJob = channel_->uploadFile("/home/zqh/tmp1/tmp", "/home/zqh/tmp2/tmp", QSsh::SftpOverwriteExisting);
//    if (uploadJob == QSsh::SftpInvalidJob) {
//        qCritical() << "Start job failed.";
//    }
}

void SshClient::HandleChannelInitializationFailure(const QString &)
{
    qCritical() << "Channel initialize failure: " << IpAndPort();
}

void SshClient::HandleChannelJobFinished(const QSsh::SftpJobId job, const QString &info)
{
    qCritical() << "Channel job finished: " << IpAndPort() << ", job: " << job << ", " << info;
}

void SshClient::HandleChannelClosed()
{
    qCritical() << "Channel closed: " << IpAndPort();
}

void SshClient::HandleFileInfoAvailable(QSsh::SftpJobId job, const QList<QSsh::SftpFileInfo> &fileInfoList)
{
    qCritical() << "File info: " << IpAndPort() << ", job: " << job << ", total: " << fileInfoList.size();
}
}  // namespace QEditor
