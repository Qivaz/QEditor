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
    if(sshSocket_ != nullptr){
        delete sshSocket_;
        sshSocket_ = nullptr;
    }
}

void SshClient::Initialize()
{
    thread_ = new QThread(this);
//    connect(thread_, SIGNAL(finished()), this, SLOT(slotThreadFinished()));
    this->moveToThread(thread_);
    thread_->start();

    // To use signals and slots afterwards.
    connect(this, SIGNAL(sigInitForClild()), this, SLOT(slotInitForClild()));
    emit sigInitForClild();
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

void SshClient::slotResetConnection(const QString &ipPort)
{
    if(this->IpAndPort() == ipPort){
        this->slotDisconnected();
    }
}

void SshClient::slotInitForClild()
{
    parameters_.port = port_;
    parameters_.userName = user_;
    parameters_.password = pwd_;
    parameters_.host = ip_;
    parameters_.timeout = 10;
    parameters_.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypePassword;
    slotCreateConnection();  // Create connection.

    if (timer_ == nullptr) {
        timer_ = new QTimer(this);
    }
    timer_->setInterval(RECONNET_SPAN_TIME);
    connect(timer_, SIGNAL(timeout()), this, SLOT(slotCreateConnection()));
    timer_->start();  // Start heartbeat timer, call slotCreateConnection to check if reconnection needed in circles.
}

void SshClient::slotCreateConnection()
{
    qCritical();
    if(connected_) {
        return;
    }
    if(sshSocket_ == nullptr){
        sshSocket_ = new QSsh::SshConnection(parameters_);
        connect(sshSocket_, SIGNAL(connected()), SLOT(slotConnected()));
        connect(sshSocket_, SIGNAL(error(QSsh::SshError)), SLOT(slotSshConnectError(const QSsh::SshError&)));
    }
    sshSocket_->connectToHost();
    qCritical() << "Try to connect: " << IpAndPort();
}

void SshClient::slotConnected()
{
    qCritical() << "Ssh connected: " << IpAndPort();
    timer_->stop();

    shell_ = sshSocket_->createRemoteShell();
    connect(shell_.data(), SIGNAL(started()), SLOT(slotShellStart()));
    connect(shell_.data(), SIGNAL(readyReadStandardOutput()), SLOT(slotReceived()));
    connect(shell_.data(), SIGNAL(readyReadStandardError()), SLOT(slotShellError()));
    shell_.data()->start();

    connected_ = true;
    emit sigConnectStateChanged(connected_, ip_, port_);
}

void SshClient::slotDisconnected()
{
    qCritical();
    sshSocket_->disconnectFromHost();
}

void SshClient::slotThreadFinished()
{
    qCritical();
    thread_->deleteLater();
    this->deleteLater();
}

void SshClient::slotShellStart()
{
    qCritical() << "Shell connected: " << IpAndPort();
    shellConnected_ = true;
    emit sigShellConnected(ip_, port_);
}

void SshClient::slotShellError()
{
    qCritical() << "Shell error happpended: " << IpAndPort();
}

void SshClient::slotSend(const QString &message)
{
    Send(message);
}

void SshClient::slotReceived()
{
    qCritical();
    QByteArray recvByteArray = shell_->readAllStandardOutput();
    QString recv = QString::fromUtf8(recvByteArray);
    if(recv.isEmpty()) {
        return;
    }
    qCritical() << "Receive: " << recv;
    emit sigDataArrived(recv, ip_, port_);
}

void SshClient::slotSshConnectError(const QSsh::SshError &sshError)
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
}  // namespace QEditor
