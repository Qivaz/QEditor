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
    if (timer_ != nullptr) {
        timer_->stop();
        delete timer_;
        timer_ = nullptr;
    }
    if(sshSocket_ != nullptr){
        delete sshSocket_;
        sshSocket_ = nullptr;
    }
    if (thread_ != nullptr) {
        thread_->requestInterruption();
        thread_->quit();
        delete thread_;
        thread_ = nullptr;
    }
}

void SshClient::Initialize()
{
    thread_ = new QThread(this);
    connect(thread_,SIGNAL(finished()), this, SLOT(slotThreadFinished()));
    this->moveToThread(thread_);
    thread_->start();

    //之后的逻辑都得通过信号和槽接通
    connect(this,SIGNAL(sigInitForClild()), this, SLOT(slotInitForClild()));
    emit sigInitForClild();
}

void SshClient::Uninitialize()
{
    thread_->quit();
}

int SshClient::Send(const QString &strMessage)
{
    qCritical()<<"SshClient ssh send "<<strMessage;

    int size = 0;
    if(connected_ && sendAble_){
       size = shell_->write(strMessage.toLatin1().data());
       qCritical()<<"SshClient ssh send successfully, size: "<< size;
    }else{
       qCritical()<<"SshClient::send() ssh未连接 或 shell未连接:"<<IpAndPort();
    }

    return size;
}

void SshClient::slotResetConnection(QString strIpPort)
{
    if(this->IpAndPort() == strIpPort){
        this->slotDisconnected();
    }
}

void SshClient::slotSend(QString strIpPort, QString strMessage)
{
    if(0 != ipPort_.compare(strIpPort)){
        return;
    }

    Send(strMessage);
}

void SshClient::slotSendByQByteArray(QString strIpPort, QByteArray arrMsg)
{
    if(0 != ipPort_.compare(strIpPort)){
        return;
    }

    if(connected_){
       shell_->write(arrMsg);
    }else{
       qCritical()<<"SshClient::send(QString strMessage) 发送失败 未建立连接:"<<IpAndPort();
    }
}

void SshClient::slotInitForClild()
{
    parameters_.port = port_;
    parameters_.userName = user_;
    parameters_.password = pwd_;
    parameters_.host = ip_;
    parameters_.timeout = 10;
    parameters_.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypePassword; //密码方式连接

    slotCreateConnection(); //连接

    if (timer_ == nullptr) {
        timer_ = new QTimer(this);
    }
    timer_->setInterval(RECONNET_SPAN_TIME);
    connect(timer_,SIGNAL(timeout()),this,SLOT(slotCreateConnection()));
    timer_->start();  //启动心跳定时器，每隔一段时间进入slotCreateConnection判断是否需要重连
}

void SshClient::slotCreateConnection()
{

    qCritical()<<"SshClient::slotCreateConnection检查连接" ;

    if(true == connected_)
        return;

    if(nullptr == sshSocket_){
        sshSocket_ = new QSsh::SshConnection(parameters_);
        connect(sshSocket_,SIGNAL(connected()),SLOT(slotConnected()));
        connect(sshSocket_,SIGNAL(error(QSsh::SshError)),SLOT(slotSshConnectError(QSsh::SshError)));
    }
    sshSocket_->connectToHost();
    qCritical()<<"SshClient::slotCreateConnection() 以ssh方式 尝试连接:"<<IpAndPort();
}

void SshClient::slotConnected()
{
    qCritical()<<"SshClient::slotConnected ssh已连接到:"<<IpAndPort();
    timer_->stop();

    shell_ = sshSocket_->createRemoteShell();
    connect(shell_.data(), SIGNAL(started()), SLOT(slotShellStart()));
    connect(shell_.data(), SIGNAL(readyReadStandardOutput()), SLOT(slotDataReceived()));
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

void SshClient::slotSshConnectError(QSsh::SshError sshError)
{
    sendAble_ = false;
    connected_ = false;
    emit sigConnectStateChanged(connected_, ip_, port_);

    timer_->start();

    switch(sshError){
    case QSsh::SshNoError:
        qCritical()<<"slotSshConnectError SshNoError"<<IpAndPort();
        break;
    case QSsh::SshSocketError:
        qCritical()<<"slotSshConnectError SshSocketError"<<IpAndPort(); //拔掉网线是这种错误
        break;
    case QSsh::SshTimeoutError:
        qCritical()<<"slotSshConnectError SshTimeoutError"<<IpAndPort();
        break;
    case QSsh::SshProtocolError:
        qCritical()<<"slotSshConnectError SshProtocolError"<<IpAndPort();
        break;
    case QSsh::SshHostKeyError:
        qCritical()<<"slotSshConnectError SshHostKeyError"<<IpAndPort();
        break;
    case QSsh::SshKeyFileError:
        qCritical()<<"slotSshConnectError SshKeyFileError"<<IpAndPort();
        break;
    case QSsh::SshAuthenticationError:
        qCritical()<<"slotSshConnectError SshAuthenticationError"<<IpAndPort();
        break;
    case QSsh::SshClosedByServerError:
        qCritical()<<"slotSshConnectError SshClosedByServerError"<<IpAndPort();
        break;
    case QSsh::SshInternalError:
        qCritical()<<"slotSshConnectError SshInternalError"<<IpAndPort();
        break;
    default:
        break;
    }

}

void SshClient::slotShellStart()
{
    sendAble_ = true;
    qCritical()<<"SshClient::slotShellStart Shell已连接:"<<IpAndPort();
}

void SshClient::slotShellError()
{
    qCritical()<<"SshClient::slotShellError Shell发生错误:"<<IpAndPort();
}

void SshClient::slotSend(const QString &strMessage)
{
    Send(strMessage);
}

void SshClient::slotDataReceived()
{
    qCritical();
    QByteArray byteRecv = shell_->readAllStandardOutput();
    QString strRecv = QString::fromUtf8(byteRecv);

//    if(strRecv.contains("password for")){
//        m_shell->write(m_strPwd.toLatin1().data());
//    }

    if(strRecv.isEmpty()) {
        return;
    }
    qCritical() << "Receive: " << strRecv;
    emit sigDataArrived(strRecv, ip_, port_);
}
}  // namespace QEditor
